#include <Arduino.h>
#include "startup.h"


// #ifdef HW100BP10829V200
//   byte OUTPUTPIN[] = {AD2,AD1,AD0,CTRLPULSE,ENCOIN,UNLOCK,BUZZ,GREEN_LED};
//   byte INPUTPIN[] = {PROG1,PROG2,DLOCK,COININ};

//   int TOTALINPUT = sizeof(INPUTPIN);
//   int TOTALOUTPUT = sizeof(OUTPUTPIN);
// #endif
//SevenSegmentTM1637 display(CLK,DIO);
//SevenSegmentExtended display(CLK,DIO);
//SevenSegmentFun display(CLK,DIO);
digitdisplay display(CLK,DIO);

Preferences cfgdata;
Config cfginfo;
//cfg cfinfo;

//String pb_uuid;
// String pb_merchantid;
// String pb_mqtthost;
// String pb_mqttuser;
// String pb_mqttpass;

int price[3]={0,0,0};
int stime[3]= {25,35,40};

int coin=0;
int coinValue=0;
int pricePerCoin=0;
int paymentby = 0;

// int pb_mqttport=1883;
int cfgState=0;
int waitFlag = 0;
bool dispflag=0;
int stateflag = 0;

int dispCount =0;

//int bill=0;

Timer serviceTime, waitTime, timeLeft, blinkWiFi;
int8_t serviceTimeID,waitTimeID,timeLeftID, blinkWiFiID;

WiFiMulti wifimulti;
WiFiClient espclient;
PubSubClient mqclient(espclient);

// String macaddr PROGMEM  = WiFi.macAddress();
// String deviceID PROGMEM = getdeviceid();





String ntpServer1 = "0.th.pool.ntp.org";
String ntpServer2 = "1.th.pool.ntp.org";

String json_config;
//Payboard
String pbRegTopic PROGMEM = "payboard/register";
String pbPubTopic PROGMEM = "payboard/backend/"; // payboard/backend/<merchantid>/<uuid>
String pbSubTopic PROGMEM = "payboard/"; //   payboard/<merchantid>/<uuid>


byte keyPress; //*** for keep keypress value.


//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 25200, 60000);

String formattedDate;
unsigned long epochtime;
String dayStamp;
String timeStamp;
int timeRemain=0;


secureEsp32FOTA esp32OTA("HW100BP10829", "1.0.0");

gpio_config_t io_config;
xQueueHandle gpio_evt_queue = NULL;




void IRAM_ATTR gpio_isr_handler(void* arg)
{
  long gpio_num = (long) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}




void gpio_task(void *arg){
    gpio_num_t io_num;  

    for(;;){
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {  
            Serial.printf("\nGPIO[%d] intr, val: %d \n", io_num, gpio_get_level(io_num));
        } 

        switch (io_num){
            case COININ:
                (gpio_get_level(io_num) == 0)?coin++:coin=coin;  
                Serial.printf("Coin now: %d\n", coin); 
                coinValue = pricePerCoin * coin;
                Serial.printf("CoinValue: %d\n", coinValue);
                paymentby = 1;
                break;
            // case BILLIN:
            //     (gpio_get_level(io_num) == 0)?bill++:bill=bill; 
            //     break;
            case MODESW:
                break;
            default:
                break;
        }  
    }
}



void interrupt(){

    io_config.intr_type = GPIO_INTR_NEGEDGE;
    io_config.pin_bit_mask = INTERRUPT_SET;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_up_en = (gpio_pullup_t)1;

    //configure GPIO with the given settings
    gpio_config(&io_config);

    /*********** create a queue to handle gpio event from isr ************/
    gpio_evt_queue = xQueueCreate(10, sizeof(long));

    /*********** Set GPIO handler task ************/
    xTaskCreate(gpio_task, "gpio_task", 1024, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    #ifdef HW100BP10829V200
      gpio_isr_handler_add((gpio_num_t)COININ, gpio_isr_handler, (void*) COININ);
    #endif

    #ifdef HAIERDRYER123
        gpio_isr_handler_add((gpio_num_t)BILLIN, gpio_isr_handler, (void*) BILLIN); 
        gpio_isr_handler_add((gpio_num_t)COININ, gpio_isr_handler, (void*) COININ);
        gpio_isr_handler_add((gpio_num_t)MODESW, gpio_isr_handler, (void*) MODESW);
    #endif
}








//*********************************** Setup is here. *********************************** 
void setup(){

  Serial.println("**************** Setup Device ****************");

  #ifdef NVS
  // This for formate EEPROM 
    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init(); // initialize the NVS partition.
    while(true);
  #endif

  Serial.begin(115200); 
  Serial.println();
  Serial.println("Setting up device...");


  //*** initial 7Segment Display
  display.begin();
  display.setBacklight(30);
  display.print("F0");
 
 
  //*** Initial GPIO

    //** Initial INPUT & OUTPUT PIN
  io_config.pin_bit_mask = INPUT_SET;  
  io_config.intr_type = GPIO_INTR_DISABLE;
  io_config.mode = GPIO_MODE_INPUT;
  io_config.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_config);

  //** Initial INPUT & OUTPUT PIN
  io_config.pin_bit_mask = OUTPUT_SET;
  io_config.intr_type = GPIO_INTR_DISABLE;
  io_config.mode = GPIO_MODE_INPUT_OUTPUT;
  gpio_config(&io_config);

  
  //*** Intial Interrupt
  interrupt();

  //Get WiFi Configuration
  display.print("F1");
  Serial.printf("WiFi Connecting.....\n");

  WiFi.mode(WIFI_AP_STA);
  wifimulti.addAP("Home173-AIS","1100110011");
  wifimulti.addAP("myWiFi","1100110011");
  for(int i=0;i<loadWIFICFG(cfgdata,cfginfo);i++){
    wifimulti.addAP(cfginfo.wifissid[i].ssid.c_str(),cfginfo.wifissid[i].key.c_str());
    Serial.printf("AddAP SSID[%d]: %s, Key[%d]: %s\n",i+1,cfginfo.wifissid[i].ssid.c_str(),i+1,cfginfo.wifissid[i].key.c_str());
  }
  Serial.printf("Connection WiFi...\n");
  while(!WiFi.isConnected()){
    display.print("nF");
    digitalWrite(GREEN_LED,LOW);
    wifimulti.run();
    delay(2000);
  }
    blinkGPIO(WIFI_LED,400);
  Serial.printf("WiFi Connected...");
  WiFiinfo();
  
  display.print("F2");
  Serial.printf("Load default configuration.\n");
  initCFG(cfginfo); 
  cfginfo.deviceid = getdeviceid();
  cfginfo.asset.mac = WiFi.macAddress();

  //**** Getting config from NV-RAM
  cfgdata.begin("config",false);
  if(cfgdata.isKey("merchantid")){
    Serial.printf("Replace configuration by using NV-RAM\n");
    getNVCFG(cfgdata,cfginfo);
  }
  cfgdata.end();


  int sz = sizeof(cfginfo.product)/sizeof(cfginfo.product[0]);
  for(int i=0;i<sz;i++){
    //Serial.printf("Before Stime[%d]: %d\n",i+1,stime[i]);
    price[i] = int(cfginfo.product[i].price);
    //stime[i] = cfginfo.product[i].stime;
    //Serial.printf("After Stime[%d]: %d\n",i+1,stime[i]);
  }

  //*** Set price per coin
  if(cfginfo.asset.coinModule){
    pricePerCoin = 1;  //CoinModule is 1 or enum Multi
  }else{
    pricePerCoin = 10; //CoinModule is 0 or enum SINGLE
  }

  cfgdata.begin("config",false); //***<<<<<<<<< config preferences
  // Get UUID
  if(cfgdata.isKey("uuid")){
    Serial.printf("Getting UUID from NV-RAM\n");
    cfginfo.payboard.uuid = cfgdata.getString("uuid","").c_str();
  }else{//Device not register
    Serial.printf("Device not register\n");

    Serial.println(cfginfo.asset.mac);
    payboard backend;
    backend.uri_register = cfginfo.payboard.apihost + "/v1.0/device/register";
    backend.merchantID=cfginfo.payboard.merchantid;
    backend.merchantKEY=cfginfo.payboard.merchantkey;
    backend.appkey=cfginfo.payboard.apikey;

    int rescode = backend.registerDEV(cfginfo.asset.mac.c_str(),cfginfo.payboard.uuid);
    Serial.printf("uuid now: "); Serial.println(cfginfo.payboard.uuid);
    if(rescode == 200){
      cfgdata.putString("uuid",cfginfo.payboard.uuid);
      Serial.printf("Save uuid completed: %s\n",cfginfo.payboard.uuid.c_str());
    }else{
      Serial.printf("Rescode: %d\n",rescode);
    }
  }
  cfgdata.end();
  showCFG(cfginfo);



  // Set mqtt parameter
  pbPubTopic = pbPubTopic  + String(cfginfo.payboard.merchantid) +"/"+ String(cfginfo.payboard.uuid);
  pbSubTopic = pbSubTopic + String(cfginfo.payboard.merchantid) +"/"+ String(cfginfo.payboard.uuid);

  //Keep WiFi connection
  while(!WiFi.isConnected()){
    display.print("nF");
    digitalWrite(GREEN_LED,LOW);
    wifimulti.run();
    delay(2000);
  }
  blinkGPIO(GREEN_LED,400); 

  //**** Connecting MQTT
  display.print("F3");
  pbBackendMqtt();
  

  //*** Set NTP
  display.print("F4");
  Serial.printf("\nConnecting to TimeServer --> ");
  configTime(6*3600,3600,ntpServer1.c_str(),ntpServer2.c_str());
  printLocalTime();
  time_t tnow;
  time(&tnow);
  cfgdata.begin("lastboot",false);
  cfgdata.putULong("epochtime",tnow);
  cfgdata.putString("timestamp",ctime(&tnow));
  cfgdata.end();
  DBprintf("Lastest booting: (%ld) -> %s.\n",tnow,ctime(&tnow));


  //******  Check stateflag 
  display.print("F5");
  cfgdata.begin("config",false);
  if(cfgdata.isKey("stateflag")){
    stateflag = cfgdata.getInt("stateflag",0);
    Serial.printf("stateflag before: %d\n",stateflag);
    String jsonmsg;
    StaticJsonDocument<100> doc;

    if(stateflag == 1){ // After Action Reboot
      doc["response"]="reboot";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="Rebooted";
      serializeJson(doc,jsonmsg);  

      if(!mqclient.connected()){
        pbBackendMqtt();
      }else{
        mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
      }
      cfgdata.putInt("stateflag",0);
      stateflag = 0;
      Serial.printf("stateflag after: %d\n",stateflag);
      cfgState = 3;
    }else if(stateflag ==2){ // After Action OTA
      doc["response"]="ota";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="Updated";
      doc["firmware"]=cfginfo.asset.firmware;
      serializeJson(doc,jsonmsg);  

      if(!mqclient.connected()){
        pbBackendMqtt();
      }else{
        mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
      }
      cfgdata.putInt("stateflag",0);
      stateflag = 0;
      Serial.printf("stateflag after: %d\n",stateflag);
      cfgState=3;
    }else if(stateflag == 5){ // Last Service not finish but may be power off.
      display.print("PE"); //Power Outage Event
      cfgState = stateflag;
      dispflag = 1;
      timeRemain = cfgdata.getInt("timeremain",0);
      Serial.printf("Not finish job found with [%d] minutes remain.\n",timeRemain);
      
      switch(cfginfo.asset.assettype){
        case WASHER:   // 0 =  Washing Machine
          if(!digitalRead(PROG1)){ // Check is machine running by read LED if 0=running  , 1 = off
          //if(isHome(PROG1)){ //Machine on service
            Serial.printf("Resuming job for orderID: %s\n",cfginfo.asset.orderid.c_str());
            serviceTimeID = serviceTime.after(60*1000*timeRemain,serviceEnd);
            timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
            //serviceEnd();
          }else{
            stateflag = 0;
            cfgState = 3;
            dispflag = 0;
            timeRemain = 0;
            cfgdata.putInt("stateflag",stateflag);
            cfgdata.putInt("timeremmain",timeRemain);
            Serial.printf("Recover power outage\n");
          }
          break;
        case DRYER:   // 1 = Dryer Machine
          break;
      }

    }else{
      cfgState = 3;
    }
  }else{
    Serial.printf(" It is here \n");
    stateflag = 0;
    cfgdata.putInt("stateflag",stateflag);
    cfgState = 3;
  }
  cfgdata.end();

  display.print("F6");
  Serial.printf("\n****************************************\n");
  Serial.printf("\nSystem Ready for service.\n");    
  delay(500);  
} 
//*--------------------------------- End of Setup. ---------------------------------*// 






//*********************************** LOOP is here. *********************************** 
void loop(){

  keyPress=display.comReadByte();
  switch(keyPress){
    case 244:
        initFS(LITTLEFS);
        Serial.println("Remove config.json");
        deleteFile(LITTLEFS,"/config.json");
        if(!isFile(LITTLEFS,"/config.json")){
          Serial.println("**** Config.json not found.");
        }  
        LITTLEFS.end();
        display.print("dE"); //Delete Completed
        delay(1000);
        cfgState = 2;  
        break;
    case 245:
        Serial.print("Delete uuid...");
        cfgdata.begin("config",false);
        cfgdata.remove("uuid");
        if(!cfgdata.isKey("uuid")){
          Serial.println("successful.");
        }
        cfgdata.end();  
        break;
    case 246:
        coinValue += 10;
        Serial.printf("Insert Coin: %d\n",coinValue);
        break;
    case 247:
        // String cfginfoJSON;

        // initFS(LITTLEFS);

        // cfginfoJSON = cfgJSON(cfginfo);
        // Serial.println("Create config.json");
        // writeFile(LITTLEFS,"/config.json",cfginfoJSON.c_str());
        // if(isFile(LITTLEFS,"/config.json")){
        //     Serial.println("**** Payboard.json save completed.");
        // }
        // LITTLEFS.end();
        // display.print("CC"); //Delete Completed
        // delay(1000);
        break;
  }
  

  if(WiFi.isConnected()){
    blinkGPIO(GREEN_LED,300); 
    //blinkWiFiID = blinkWiFi.pulseImmediate(GREEN_LED,650,HIGH);

    if(!mqclient.connected() && (cfgState >= 2)){
      pbBackendMqtt();
    }

    //Serial.printf("cfgState: %d\n",cfgState);

    switch(cfgState){
      case 1: //*** Not register not Configuration.
          display.print("C1");
          pbRegisMqtt();
          break;
      case 2: //*** Register but activate from backend.
          display.print("C2");
          //pbBackendMqtt();
          break;
      case 3: //*** get config from File System.
          //Serial.printf(" ------------ This is in cfgState 3 -----------%d\n",coinValue);
          digitalWrite(ENCOIN,HIGH);   // Waiting for Coin

          if(coinValue > 0){
            cfgState = 4;
          }else{
            String dispPrice;
            if(price[0]!=0){
              dispPrice = String(price[0]);
            }
            if(price[1]!=0){
              dispPrice = dispPrice + "--" +String(price[1]);
            }
            if(price[2]!=0){
              dispPrice = dispPrice + "--" +String(price[2]);
            }
            display.scrollingText(dispPrice.c_str(),1);
          }
          break;
      case 4: //*** After 1st coin insert
          switch(cfginfo.asset.assettype){
            case WASHER:// 0 = Washing Machine
              if((coinValue == price[0]) && (waitFlag == 0)){\
                waitFlag = 1;
                //cfgState = 5;
                Serial.printf("Program 1 :%d\n", coinValue);
                waitTimeID=waitTime.after(60*1000*0.3,prog1start);
                
              }else if((coinValue == price[1]) && (waitFlag <= 1)){
                waitTime.stop(waitTimeID);
                waitFlag =2;
                //cfgState = 5;
                Serial.printf("Program 2 :%d\n", coinValue);
                
                waitTimeID=waitTime.after(60*1000*0.3,prog2start);
                
              }else if((coinValue == price[2]) && (waitFlag <= 2)){
                display.setBacklight(30);
                display.print(coinValue);
                display.setColonOn(true);
                delay(300);

                waitTime.stop(waitTimeID);
                waitFlag= 3;
                //cfgState = 5;
                Serial.printf("Program 3 :%d\n", coinValue);
                waitTimeID=waitTime.after(60*1000*0.3,prog3start);
                //prog3start();
              }else{
                display.setBacklight(30);
                display.print(coinValue);
                display.setColonOn(true);
              }

              break;
            case DRYER: // 1 = Dryer Machine
              if(coinValue == price[0]){
                Serial.printf("Dryer Program-1 for 60 mins :%d\n", coinValue);
                waitTimeID=waitTime.after(60*1000*0.3,prog1start);
              }else{
                display.setBacklight(30);
                display.print(coinValue);
                display.setColonOn(true);
              }
              break;
          }
          break;
      case 5:
          //Serial.printf("Service running please wait\n");
          display.setBacklight(30);
          display.setColonOn(false);

          // Serial.printf("dispflag: %d\n",dispflag);
          // Serial.printf("WaitFlag: %d\n",waitFlag);

          if(!dispflag){
            switch(waitFlag){
              case 1://Prog1
                display.print("P1");
                delay(300);
                break;
              case 2://Prog2
                display.print("P2");
                delay(300);
                break;
              case 3://Prog3
                display.print("P3");
                delay(300);
                break;
            }
            dispflag = true;
          }else{
            switch(waitFlag){
              case 1://Prog1
                display.animation4(display,200,2);
                break;
              case 2://Prog2
                display.animation1(display,200,2);
                break;
              case 3://Prog3
                display.animation3(display,200,2);
                break;
              default:
                display.animation2(display,200,2);
                break;
            }
          }



          //ani1.animation1(display,100,2);
          break;
      case 6:
          break;
    }
  }else{
    digitalWrite(GREEN_LED,LOW);
    digitalWrite(0,LOW);
    Serial.printf("WiFi Connecting.....\n");
    wifimulti.run();
     
    delay(1500);
    if(WiFi.isConnected()){
      Serial.println("connected");
      WiFiinfo();

      Serial.print("cfgState: ");
      Serial.println(cfgState);
    }  
  }
  

  serviceTime.update();
  waitTime.update();
  timeLeft.update();
  mqclient.loop();

}
//*--------------------------------- End of LOOP. ---------------------------------*// 









void pbRegCallback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived pbRegisterCallback with topic: ");
  Serial.println(topic);

  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
 
  DynamicJsonDocument doc(1024);
  //doc.clear();
  //DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  if(String(cfginfo.payboard.uuid).isEmpty()){
    if(!doc["uuid"].isNull()){
      cfginfo.payboard.uuid = doc["uuid"].as<String>();
      Serial.print("Got UUID: ");
      //Serial.print(pb_uuid);
      Serial.print(cfginfo.payboard.uuid);
      Serial.println(" device registered\n");
      mqclient.unsubscribe(pbSubTopic.c_str());
      
      mqclient.setCallback(pbCallback);
      pbSubTopic = "payboard/" + String(cfginfo.payboard.merchantid)+ "/" + String(cfginfo.payboard.uuid);
      mqclient.subscribe(pbSubTopic.c_str());

      //*** Save uuid to eeprom.
      cfgdata.begin("config",false);
      cfgdata.putString("uuid",cfginfo.payboard.uuid);
      cfgdata.end();

      display.print("C2");
      cfgState = 2; // Register 
    }
  }else{
    Serial.printf("Device already registered.\n");
  }
  //doc.clear();
}






void pbRegisMqtt(){
  Serial.printf("Registering to backend server\n");
  mqclient.setServer(cfginfo.payboard.mqtthost.c_str(),cfginfo.payboard.mqttport);
  mqclient.setCallback(pbRegCallback);

  pbSubTopic = "payboard/" + String(cfginfo.payboard.merchantid)+ "/" + String(cfginfo.asset.mac);
  Serial.printf("   Register Mqtt connecting ...");
  while(!mqclient.connect(cfginfo.deviceid.c_str(),cfginfo.payboard.mqttuser.c_str(),cfginfo.payboard.mqttpass.c_str())){
    Serial.printf(".");
    delay(500);
  }
  Serial.printf("connected\n");
  mqclient.subscribe(pbSubTopic.c_str());
  Serial.printf("   Subscribe Topic: %s\n",pbSubTopic.c_str());


  DynamicJsonDocument doc(256);
  //doc.clear();
  String mqMsg  = "";
  doc["merchantid"]=cfginfo.payboard.merchantid;
  doc["mac"]=cfginfo.asset.mac;
  doc["model"]=cfginfo.asset.model;
  doc["firmware"]=cfginfo.asset.firmware;
  serializeJson(doc,mqMsg);

  if(mqclient.connected()){
    Serial.print("   Publish topic: ");
    Serial.println(pbRegTopic);
    Serial.print("   Message: ");
    Serial.println(mqMsg);
    mqclient.publish(pbRegTopic.c_str(),mqMsg.c_str());
    delay(500);
  }else{
    Serial.println("Publish failed due to Mqtt not connect.");
  }
  //doc.clear();
}




void pbCallback(char* topic, byte* payload, unsigned int length){
  String jsonmsg;
  DynamicJsonDocument doc(1024);

  Serial.println();
  Serial.println("Message arrived pbCallback with topic: ");
  Serial.println(topic);
  Serial.println();

  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  DeserializationError error = deserializeJson(doc, payload);
  // Test if parsing succeeds.

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  String action = doc["action"];
  Serial.print("\nThis action paramater: ");
  Serial.println(action);

  if(action == "config"){

    int sz = doc["detail"].size();
    Serial.printf("detail array size: %d\n",sz);

    cfgdata.begin("config",false);

    for(int x=0;x<(3-sz);x++){
      Serial.printf(" delete index: %d\n",3-x);
      cfgdata.putString(("sku"+ String(3-x)).c_str(),"");
      cfgdata.putFloat(("price"+String(3-x)).c_str(),0);
      cfgdata.putInt(("stime"+ String(3-x)).c_str(),0);      
      price[2-x]=0;

    }  

    for(int i=0;i<sz;i++){
      String sku = doc["detail"][i]["sku"].as<String>();
      price[i] = doc["detail"][i]["price"].as<float>();
      // stime[]={30,40,40};

      Serial.print("sku"+ String(i+1) +": ");
      Serial.println(sku);
      Serial.print("price"+String(i+1)+": ");
      Serial.println(price[i]);

      cfgdata.putString(("sku"+ String(i+1)).c_str(),sku);
      cfgdata.putFloat(("price"+String(i+1)).c_str(),price[i]);
      cfgdata.putInt(("stime"+ String(i+1)).c_str(),stime[i]);

      cfginfo.product[i].sku = sku;
      cfginfo.product[i].price = price[i];
      cfginfo.product[i].stime = stime[i];

      Serial.printf("Saving stime[%d]: %d\n",i,stime[i]);
    }

    cfgdata.end();
    doc.clear();
    doc["response"]="config";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;      
    doc["state"]="saved";
    doc["desc"]="config saved";
    
    cfgState = 3;
    display.print("C3");

  }else if(action == "paid"){
    //Paid and then start service.
    Serial.printf("Response for action: paid.\n");

    paymentby = 2; // 1=coin, 2= qr, 3=kiosk , 4 = free
    int paidprice = doc["price"].as<int>();
    //String trans = doc["orderNo"].as<String>();
    cfginfo.asset.orderid = doc["orderNo"].as<String>();
    // cfgdata.begin("config",false);
    // cfgdata.putString("orderid",cfginfo.asset.orderid);
    // cfgdata.end();
  
    Serial.printf(" Customer paid for: %d\n",paidprice);
    doc.clear();
    doc["response"] = "paid";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;  
    doc["state"]="accepted";
    doc["desc"]="accepted orderid: " + cfginfo.asset.orderid;

    //coinValue = paidprice;
  }else if(action == "countcoin"){
    /*  Not use Jul64
    Serial.printf("Response for action: coincount.\n");
    //{"action":"countcoin","orderNo":"02210526162202176","price":"10.00"}
    // two idea 1st: after insert coin machine work immediately.
    // 2nd: machine not operate if cannot update to backend.

    String resmsg = doc["StatusCode"].as<String>();
    String msg = doc["Message"].as<String>();
    String trans = doc["ResultValues"]["transactionId"].as<String>();

    cfginfo.asset.orderid = trans;


    doc["response"] = "countcoin";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="accepted";
    doc["desc"]="accepted transaction: " + trans;
    */
  }else if(action == "ping"){

    Serial.printf("response action PING\n");

    doc.clear();
    doc["response"]="ping";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["rssi"]=WiFi.RSSI();
    switch(cfgState){
      case 3: // Available (waiting for job)
          doc["state"] = "Available";
          break;
      case 4: // Available (waiting for job)
          doc["state"] = "Booked"; // Coin inserted
          break;
      case 5: // Available (waiting for job)
          doc["state"] = "Busy"; // On Service
          break;
    }
    //doc["state"]=cfgState;
    doc["timeRemain"] = timeRemain;

  }else if( (action == "reset") || (action=="reboot") || (action=="restart")){
      //set stateflag = 2 flag
    Serial.printf("Accept request action reboot\n");

    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",1);
    cfgdata.end();

    doc.clear();
    doc["response"]="reboot";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;  
    doc["state"]="accepted";
    doc["disc"] = "Asset rebooting.";

    serializeJson(doc,jsonmsg);
    Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
    if(!mqclient.connected()){
      pbBackendMqtt();
    }
    mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
    delay(500);    
    
    ESP.restart();

  }else if(action == "ota"){
    WiFiClientSecure clientForOta;

    esp32OTA._host="www.flipup.net"; //e.g. example.com
    esp32OTA._descriptionOfFirmwareURL="/firmware/HW100BP10829/firmware.json"; //e.g. /my-fw-versions/firmware.json
    //esp32OTA._certificate=test_root_ca;
    esp32OTA.clientForOta=clientForOta;
  
    bool shouldExecuteFirmwareUpdate=esp32OTA.execHTTPSCheck();
    if(shouldExecuteFirmwareUpdate){
      cfginfo.asset.firmware = esp32OTA._firwmareVersion.c_str();      
      //saveCFG(cfginfo,LITTLEFS);   (13Jul64)
      
      //set stateflag = 2 flag
      cfgdata.begin("config",false);
      cfgdata.putInt("stateflag",2);
      cfgdata.end();

      doc.clear();
      doc["response"] = "ota";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["firmware"] = esp32OTA._firwmareVersion;
      doc["state"] = "accepted";
      doc["disc"] = "Firmware upgrading then rebooting in few second."; 
      serializeJson(doc,jsonmsg);
      Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
      Serial.print("Ver: "); Serial.println(esp32OTA._firwmareVersion);

      Serial.println("Firmware updating, It's will take few second");
      esp32OTA.executeOTA();
    }else{
      doc["merchanttid"] = cfginfo.payboard.merchantid;
      doc["uuid"] = cfginfo.payboard.uuid;
      doc["response"] = "ota";
      doc["state"] = "FAILED";
      doc["desc"] = "Firmware update failed, version may be the same."; 
    }

    serializeJson(doc,jsonmsg);
    Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
    if(!mqclient.connected()){
      pbBackendMqtt();
    }
    mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
    delay(500); 

  }else if(action == "setwifi"){
    // {"action":"setwifi","index":"1","ssid":"Home173-AIS","key":"1100110011","reconnect":"1"}

    String ssid = doc["ssid"].as<String>();
    String key = doc["key"].as<String>();
    int wifireconn = doc["reconnect"].as<int>();
    int index = doc["index"].as<int>();
    
    cfgdata.begin("wificfg",false);
    cfgdata.putString(("ssid"+ (String)(index)).c_str(),ssid);
    cfgdata.putString(("key"+ (String)(index)).c_str(),key);
      Serial.printf("Setting WiFi with following\n");
      Serial.print(("ssid"+ (String)(index))+": ");
      Serial.println(cfgdata.getString(("ssid"+ (String)(index)).c_str()));
      Serial.print(("key"+ (String)(index))+": ");
      Serial.println(cfgdata.getString(("key"+ (String)(index)).c_str()));
    cfgdata.end();

    doc.clear();
    doc["response"] = "setwifi";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    
    wifimulti.addAP(ssid.c_str(),key.c_str());
    if(wifireconn){
      WiFi.disconnect();
      wifimulti.run();
      doc["state"]="Reconnected";
      doc["desc"]="setWiFi completed  and reconnected";
    }else{
      doc["state"]="Changed";
      doc["desc"]="setWiFi conpleted but will effect next boot.";
    }
  
  }else if(action == "coinmodule"){
    cfginfo.asset.coinModule = (doc["coinmodule"].as<String>() == "single")?SINGLE:MULTI; //  SINGLE=0, MULTI=1
    (cfginfo.asset.coinModule == MULTI)?pricePerCoin=1:pricePerCoin=10;

    doc.clear();
    doc["response"] = "coinmodule";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="changed";

    (cfginfo.asset.coinModule == MULTI)?doc["desc"]="Change coinModule to: MULTI":doc["desc"]="Change coinModule to: SINGLE";

  }else if(action == "orderid"){

  }else if(action == "assettype"){  
    // {"action":"assettype","assettype":"0"}

    
    cfginfo.asset.assettype = doc["assettype"].as<int>();
    cfgdata.begin("config",false);
    cfgdata.putInt("assettype",0);
    cfgdata.end();

    doc.clear();
    doc["response"] = "assettype";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="changed"; 
    if(cfginfo.asset.assettype){// 1 = Dryer
      doc["desc"]="Set assetType to DRYER";
    }else{// 0 = Washer
      doc["desc"]="Set assetType to WASHER";
    }
    
  }else if(action == "payboard"){// To set payboard parameter
    String params = doc["params"];

    cfgdata.begin("config",false);
    if(params.equals("all")){
        cfginfo.payboard.uuid = doc["uuid"].as<String>();
        cfgdata.putString("uuid",cfginfo.payboard.uuid);
      
        cfginfo.payboard.merchantid = doc["merchantid"].as<String>();;
        cfginfo.payboard.merchantkey = doc["merchantkey"].as<String>();
        cfgdata.putString("merchantid",cfginfo.payboard.merchantid);
        cfgdata.putString("merchantkey",cfginfo.payboard.merchantkey);
      
        cfginfo.payboard.apihost = doc["apihost"].as<String>();
        cfginfo.payboard.apikey = doc["apikey"].as<String>();
        cfgdata.putString("apihost",cfginfo.payboard.apihost);
        cfgdata.putString("apikey",cfginfo.payboard.apikey);
      
        cfginfo.payboard.mqtthost = doc["mqtthost"].as<String>();
        cfginfo.payboard.mqttport = doc["mqttportt"].as<int>();
        cfginfo.payboard.mqttuser = doc["mqttuser"].as<String>();
        cfginfo.payboard.mqttpass = doc["mqttpass"].as<String>();
        cfgdata.putString("mqtthost",cfginfo.payboard.mqtthost);
        cfgdata.putInt("mqttport",cfginfo.payboard.mqttport);
        cfgdata.putString("mqttuser",cfginfo.payboard.mqttuser);
        cfgdata.putString("mqttpass",cfginfo.payboard.mqttpass);
    }else{
      if(params.equals("uuid")){
        cfginfo.payboard.uuid = doc["uuid"].as<String>();
        cfgdata.putString("uuid",cfginfo.payboard.uuid);
      }else if(params.equals("merchantid")){
        cfginfo.payboard.merchantid = doc["merchantid"].as<String>();;
        cfginfo.payboard.merchantkey = doc["merchantkey"].as<String>();
        cfgdata.putString("merchantid",cfginfo.payboard.merchantid);
        cfgdata.putString("merchantkey",cfginfo.payboard.merchantkey);
      }else if(params.equals("apihost")){
        cfginfo.payboard.apihost = doc["apihost"].as<String>();
        cfginfo.payboard.apikey = doc["apikey"].as<String>();
        cfgdata.putString("apihost",cfginfo.payboard.apihost);
        cfgdata.putString("apikey",cfginfo.payboard.apikey);
      }else if(params.equals("mqtthost")){
        cfginfo.payboard.mqtthost = doc["mqtthost"].as<String>();
        cfginfo.payboard.mqttport = doc["mqttportt"].as<int>();
        cfginfo.payboard.mqttuser = doc["mqttuser"].as<String>();
        cfginfo.payboard.mqttpass = doc["mqttpass"].as<String>();
        cfgdata.putString("mqtthost",cfginfo.payboard.mqtthost);
        cfgdata.putInt("mqttport",cfginfo.payboard.mqttport);
        cfgdata.putString("mqttuser",cfginfo.payboard.mqttuser);
        cfgdata.putString("mqttpass",cfginfo.payboard.mqttpass);
      }
    }
    cfgdata.end();
  }else if(action == "backend"){

  }else if(action == "stateflag"){ //stateflag is flag for mark action before reboot  ex 1 is for reboot action, 2 for ota action

  }else if(action == "jobcancel"){
    //stateflag = 0;
    //orderid ="";
    //cfgStatte = 3;
    //waitFlag =0;
    //dispflag = 0;

  }else if(action == "jobcreate"){
    coinValue = doc["price"].as<int>();
    paymentby = doc["paymentby"].as<int>();  //1 = coin , 2 = qr, 3 = kiosk , 4 = free

    Serial.print("waitFlag: ");
    Serial.println(waitFlag);

    doc.clear();
    doc["response"] = "jobcreate";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="created";
    doc["desc"]="Manual create job.";

  }else if(action == "nvsdelete"){
    String msg;

    Serial.printf("NVS size before delete: %d\n",cfgdata.freeEntries());
    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init(); // initialize the NVS partition.
    msg = "NVS size after delete: "+ (String)cfgdata.freeEntries();
    Serial.println(msg);

    doc.clear();
    doc["response"] = "nvsdelete";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="deleted";
    doc["desc"]=msg;    
  }

  serializeJson(doc,jsonmsg);
  Serial.println();
  Serial.print("pbPubTopic: "); Serial.println(pbPubTopic);
  Serial.print("Jsonmsg: ");Serial.println(jsonmsg);

  if(!mqclient.connected()){
    pbBackendMqtt();
  }
  mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
  delay(500);
}







void pbBackendMqtt(){
    //Serial.println("[pbBackendMqtt]");
          // Serial.println(cfginfo.payboard.uuid);
          // Serial.println(cfginfo.payboard.merchantid);
          // Serial.println(cfginfo.payboard.mqtthost);
          // Serial.println(cfginfo.payboard.mqttport);
          // Serial.println(cfginfo.payboard.mqttuser);
          // Serial.println(cfginfo.payboard.mqttpass);

    if(!mqclient.connected()){
      mqclient.setServer(cfginfo.payboard.mqtthost.c_str(),cfginfo.payboard.mqttport);
      mqclient.setCallback(pbCallback);

      
      pbSubTopic = "payboard/" + String(cfginfo.payboard.merchantid) + "/" + String(cfginfo.payboard.uuid);
    
      Serial.printf("Backend Mqtt connecting ...");
      while(!mqclient.connect(cfginfo.deviceid.c_str(),cfginfo.payboard.mqttuser.c_str(), cfginfo.payboard.mqttpass.c_str())){
        Serial.printf(".");
        delay(500);
      }
      Serial.printf("connected\n");
      mqclient.subscribe(pbSubTopic.c_str());
      Serial.printf("   Subscribe Topic: %s\n",pbSubTopic.c_str());
    }        
}


int checkUUID(){
  Preferences cfgROM;

  cfgROM.begin("config",true);
  if(cfgROM.isKey("uuid")){
    return true; //
  }else{
    return false;
  }
  cfgROM.end();
}


String getUUID(){
  Preferences cfgROM;
  String myuuid;
  cfgROM.begin("config",true);
  myuuid = cfgROM.getString("uuid");
  cfgROM.end();
  return myuuid;
}





void showPrice(SevenSegmentTM1637 &disp,int &count,byte max, byte min){
  //disp.setColonOn(true);

  if(count <= 40){
    if((count%5)==0){
      disp.setBacklight(max);
      disp.print((int)cfginfo.product[0].price);
    }else{
      disp.setBacklight(min);
      disp.print((int)cfginfo.product[0].price);
    }
    count++;
  }else if(count <= 80){
    if((count%5)==0){
      disp.setBacklight(max);
      disp.print((int)cfginfo.product[1].price);
    }else{
      disp.setBacklight(min);
      disp.print((int)cfginfo.product[1].price);
    }
    count++;
  }else if(count <=120){
    if((count%5)==0){
      disp.setBacklight(max);
      disp.print((int)cfginfo.product[2].price);
    }else{
      disp.setBacklight(min);
      disp.print((int)cfginfo.product[2].price);
    }
    count++;
  }else{
    count = 0;
  }
}


void prog1start(){

  payboard backend;
  String response;
  int rescode;
  
  digitalWrite(ENCOIN,LOW); // Disable Coin Module

  //if(1){   //Comment this row when production
  if(startProg(1)){  //unComment this row when production
    Serial.printf("Starting Prog1Start , Paymentby %d\n",paymentby);
    cfgState = 5;
    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",cfgState);

    backend.merchantID=cfginfo.payboard.merchantid;
    backend.merchantKEY=cfginfo.payboard.merchantkey;
    backend.appkey=cfginfo.payboard.apikey;

    while(!WiFi.isConnected()){
      wifimulti.run();
    }

    switch(paymentby){
      case 1: // by Coin
        backend.uri_countCoin = cfginfo.payboard.apihost + "/v1.0/device/countcoin";
        rescode = backend.coinCounter(cfginfo.payboard.uuid.c_str(),coinValue,response);
        if(rescode == 200){
          Serial.printf("Response trans: %s\n",response.c_str());
          cfgdata.putString("orderid",response);
        }else{
          Serial.printf("Response code: %d\n",rescode);
          Serial.printf("Response trans: %s\n",response.c_str());
        }
        break;
      case 2:// by QR
        cfgdata.putString("orderid",cfginfo.asset.orderid);
        backend.uri_deviceStart = cfginfo.payboard.apihost + "/v1.0/device/start";
        rescode = backend.deviceStart(cfginfo.asset.orderid.c_str(),response);

        if(rescode == 200){
          if(response == "success"){
            Serial.printf("Pro1Start backend undated\n");
          } 
        }else{
          Serial.printf("Rescode: %d\n",rescode);
        }
        break;
    }
    cfgdata.end();

    //startProg(1);

    Serial.printf(" Program Time: %d\n",stime[0]);
    //stime[0]=1; // for testing only
    serviceTimeID=serviceTime.after((60*1000*stime[0]),serviceEnd);
    timeRemain = stime[0];
    timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
    Serial.printf("On service of Program-1 for %d minutes\n",stime[0]);
  }else{
    coin=0;
    coinValue=0;
    cfgState = 6;
    display.print("E1");
    Serial.printf("Failed to start service of Program-1\n");
  }
}


void prog2start(){

  payboard backend;
  String response;
  int rescode;

  digitalWrite(ENCOIN,LOW);
  //if(1){   //Comment this row when production
  if(startProg(2)){    //unComment this row when production
    Serial.printf("Starting Prog1Start., Paymentby %d\n",paymentby);
    cfgState = 5;
    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",cfgState);

    backend.merchantID=cfginfo.payboard.merchantid;
    backend.merchantKEY=cfginfo.payboard.merchantkey;
    backend.appkey=cfginfo.payboard.apikey;

    while(!WiFi.isConnected()){
      wifimulti.run();
    }

    switch(paymentby){
      case 1: // by Coin
        backend.uri_countCoin = cfginfo.payboard.apihost + "/v1.0/device/countcoin";
        rescode = backend.coinCounter(cfginfo.payboard.uuid.c_str(),coinValue,response);
        if(rescode == 200){
          Serial.printf("Response trans: %s\n",response.c_str());
          cfgdata.putString("orderid",response);
        }else{
          Serial.printf("Response code: %d\n",rescode);
          Serial.printf("Response trans: %s\n",response.c_str());
        }
        break;
      case 2:// by QR
        cfgdata.putString("orderid",cfginfo.asset.orderid);
        backend.uri_deviceStart = cfginfo.payboard.apihost + "/v1.0/device/start";
        rescode = backend.deviceStart(cfginfo.asset.orderid.c_str(),response);

        if(rescode == 200){
          if(response == "success"){
            Serial.printf("Pro1Start backend undated\n");
          } 
        }else{
          Serial.printf("Rescode: %d\n",rescode);
        }
        break;
    }
    cfgdata.end();

    //startProg(2);

    Serial.printf(" Program Time: %d\n",stime[1]);
    //stime[1]=1; // for testing only
    serviceTimeID=serviceTime.after((60*1000*stime[1]),serviceEnd);
    timeRemain = stime[1];
    timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
    Serial.printf("On service of Program-2 for %d minutes\n",stime[1]);
  }else{
    coin=0;
    coinValue=0;
    cfgState = 6;
    display.print("E2");
    Serial.printf("Failed to start service of Program-2\n");
  }
}


void prog3start(){

  payboard backend;
  String response;
  int rescode;

  digitalWrite(ENCOIN,LOW);
  //if(1){   //Comment this row when production
  if(startProg(3)){   //unComment this row when production
    Serial.printf("Starting Prog1Start., Paymentby %d\n",paymentby);
    cfgState = 5;
    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",cfgState);

    backend.merchantID=cfginfo.payboard.merchantid;
    backend.merchantKEY=cfginfo.payboard.merchantkey;
    backend.appkey=cfginfo.payboard.apikey;

    while(!WiFi.isConnected()){
      wifimulti.run();
    }

    switch(paymentby){
      case 1: // by Coin
        backend.uri_countCoin = cfginfo.payboard.apihost + "/v1.0/device/countcoin";
        rescode = backend.coinCounter(cfginfo.payboard.uuid.c_str(),coinValue,response);
        if(rescode == 200){
          Serial.printf("Response trans: %s\n",response.c_str());
          cfgdata.putString("orderid",response);
        }else{
          Serial.printf("Response code: %d\n",rescode);
          Serial.printf("Response trans: %s\n",response.c_str());
        }
        break;
      case 2:// by QR
        cfgdata.putString("orderid",cfginfo.asset.orderid);
        backend.uri_deviceStart = cfginfo.payboard.apihost + "/v1.0/device/start";
        rescode = backend.deviceStart(cfginfo.asset.orderid.c_str(),response);

        if(rescode == 200){
          if(response == "success"){
            Serial.printf("Pro1Start backend undated\n");
          } 
        }else{
          Serial.printf("Rescode: %d\n",rescode);
        }
        break;
    }
    cfgdata.end();
    //startProg(3);
    Serial.printf(" Program Time: %d\n",stime[2]);
    //stime[2]=1; // for testing only
    serviceTimeID=serviceTime.after((60*1000*stime[2]),serviceEnd);
    timeRemain = stime[2];
    timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
    Serial.printf("On service of Program-3 for %d minutes\n",stime[2]);
  }else{
    coin=0;
    coinValue=0;
    cfgState = 6;
    display.print("E3");
    Serial.printf("Failed to start service of Program-3\n");
  }
}

void serviceLeft(){
  Serial.printf("Service Time remain: %d\n",--timeRemain);
  cfgdata.begin("config",false);
  cfgdata.putInt("timeremain",timeRemain);
  cfgdata.end();
}

void serviceEnd(){
  timeLeft.stop(timeLeftID);
  if(digitalRead(PROG1)){ // digitalRead(PROG1) if get  0 = machine still running.
    timeRemain = 0;
    coin=0;
    coinValue = 0;
    cfgState = 3;
    waitFlag = 0;
    dispflag = 0;


    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",0);
    cfgdata.putString("orderid","");
    cfgdata.putInt("timeremain",0);
    cfgdata.end();
    Serial.printf("Job Finish.  Poweroff machine soon.\n");
  }else{
    Serial.printf("Job still running. wait for one more minute\n");
    //serviceTime.stop(serviceTimeID);
    serviceTimeID=serviceTime.after((60*1000*1),serviceEnd);
    timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
  }

}
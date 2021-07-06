#include <Arduino.h>
#include "startup.h"



#ifdef NVS
  #include <nvs_flash.h>
#endif

#ifdef HW100BP10829V200
  byte OUTPUTPIN[] = {AD2,AD1,AD0,CTRLPULSE,ENCOIN,UNLOCK,BUZZ,GREEN_LED};
  byte INPUTPIN[] = {PROG1,PROG2,DLOCK,COININ};

  int TOTALINPUT = sizeof(INPUTPIN);
  int TOTALOUTPUT = sizeof(OUTPUTPIN);
#endif
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

int coinValue=0;
int pricePerCoin=0;
// int pb_mqttport=1883;
int waitFlag = 0;
bool dispflag=0;

Timer serviceTime, waitTime;

int8_t serviceTimeID,waitTimeID;

WiFiMulti wifimulti;
WiFiClient espclient;
PubSubClient mqclient(espclient);

// String macaddr PROGMEM  = WiFi.macAddress();
// String deviceID PROGMEM = getdeviceid();

String SoftAP_NAME PROGMEM = "BT_" + getdeviceid();
IPAddress SoftAP_IP(192,168,8,20);
IPAddress SoftAP_GW(192,168,8,1);
IPAddress SoftAP_SUBNET(255,255,255,0);

gpio_config_t io_config;
xQueueHandle gpio_evt_queue = NULL;
int coin=0;
//int bill=0;
int paymentby = 0;
int stateflag = 0;



String json_config;
//Payboard
String pbRegTopic PROGMEM = "payboard/register";
String pbPubTopic PROGMEM = "payboard/backend/"; // payboard/backend/<merchantid>/<uuid>
String pbSubTopic PROGMEM = "payboard/"; //   payboard/<merchantid>/<uuid>

byte keyPress; //*** for keep keypress value.
byte cfgState=0;
int dispCount =0;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String epochtime;
String dayStamp;
String timeStamp;
int timeleft;

secureEsp32FOTA esp32OTA("HW100BP10829", "1.0.0");

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
    //gpio_config_t io_conf;

    //io_config.intr_type = GPIO_INTR_ANYEDGE;
    io_config.intr_type = GPIO_INTR_NEGEDGE;
    //io_config.intr_type = GPIO_INTR_POSEDGE;
    //io_config.intr_type = GPIO_INTR_LOW_LEVEL;
    //io_config.intr_type = GPIO_INTR_HIGH_LEVEL;
    
    io_config.pin_bit_mask = INPUT_SET;
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
 
  //LITTLEFS.format();

  //digitalWrite(0,LOW);
  digitalWrite(COININ,HIGH);

  Serial.begin(115200); 
  Serial.println();
  Serial.println("Booting...");


  //*** initial 7Segment Display
  display.begin();
  display.setBacklight(30);
  display.print("F0");
 
 
  //*** Initial GPIO
  initOUTPUT(TOTALOUTPUT,OUTPUTPIN);
  initINPUT(TOTALINPUT,INPUTPIN);

  //digitalWrite(ENCOIN,HIGH);
  //pinMode(LED1,OUTPUT);
  
  //*** Intial Interrupt
  interrupt();

  //Get WiFi Configuration
  Serial.printf("WiFi Connecting.....\n");
  WiFi.mode(WIFI_AP_STA);
  //Check WiFi Config from rom
  cfgdata.begin("wificfg",false);
    if(cfgdata.isKey("ssid")){
      String ssid = cfgdata.getString("ssid");
      String key = cfgdata.getString("key");
      Serial.printf("SSID: %s, KEY: %s\n",ssid.c_str(),key.c_str());
      wifimulti.addAP(ssid.c_str(),key.c_str());
    }
  cfgdata.end();
  wifimulti.addAP("Home173-AIS","1100110011");
  wifimulti.addAP("BTnet","1100110011");

  //WiFi Setting
  while(!WiFi.isConnected()){
    display.print("nF");
    digitalWrite(GREEN_LED,LOW);
    wifimulti.run();
    delay(2000);
  }

  display.print("F1");
  blinkGPIO(GREEN_LED,500); 
  Serial.println("...Wifi Connected...");
  WiFiinfo();
  delay(300);


  display.print("F2");
  initCFG(cfginfo);
  //Preparing API data
  cfginfo.deviceid = getdeviceid();
  cfginfo.asset.mac = WiFi.macAddress();

  //Getting config from NV-RAM
  getNVCFG(cfgdata,cfginfo);

  int sz = sizeof(cfginfo.product)/sizeof(cfginfo.product[0]);
  for(int i=0;i<sz;i++){
    price[i] = cfginfo.product[i].price;
    stime[i] = cfginfo.product[i].stime;
  }
  pbPubTopic = pbPubTopic  + String(cfginfo.payboard.merchantid) +"/"+ String(cfginfo.payboard.uuid);
  pbSubTopic = pbSubTopic + String(cfginfo.payboard.merchantid) +"/"+ String(cfginfo.payboard.uuid);

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
    backend.uri_register = cfginfo.backend.apihost + "/v1.0/device/register";
    backend.merchantID=cfginfo.payboard.merchantid;
    backend.merchantKEY=cfginfo.payboard.merchantkey;
    backend.appkey=cfginfo.backend.apikey;

    int rescode = backend.registerDEV(cfginfo.asset.mac.c_str(),cfginfo.payboard.uuid);
    Serial.printf("uuid now: "); Serial.println(cfginfo.payboard.uuid);
    if(rescode == 200){
      cfgdata.putString("uuid",cfginfo.payboard.uuid);
      Serial.printf("Save uuid completed: %s\n",cfginfo.payboard.uuid.c_str());
    }else{
      Serial.printf("Rescode: %d\n",rescode);
    }
  }

  //Keep WiFi connection
  while(!WiFi.isConnected()){
    display.print("nF");
    digitalWrite(GREEN_LED,LOW);
    wifimulti.run();
    delay(2000);
  }

  blinkGPIO(GREEN_LED,500);
  pbBackendMqtt();
  

  display.print("F3");
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
    }else if(stateflag == 5){ // Last Service not finish yet
      cfgState = 5;
      if(!cfginfo.asset.orderid.isEmpty()){
          Serial.print("Last orderID: ");
      }
    }
  }else{
    stateflag = 0;
    cfgdata.putInt("stateflag",stateflag);
  }

  cfgdata.end();

  cfgState = 3;

  // Serial.printf("\nStep 8: Connecting to TimeServer --> ");
  // //Set NTP
  // timeClient.begin();
  // timeClient.setTimeOffset(25200);  //GMT+7
  // Serial.printf("Done.\n");

  //Connecting Mqtt 


  Serial.printf("\n****************************************\n");
  Serial.printf("\nSystem Ready for service.\n");      
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
    blinkGPIO(GREEN_LED,500); 

    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    epochtime = timeClient.getEpochTime();
    //Serial.println(formattedDate);
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0,splitT);
    timeStamp = formattedDate.substring(splitT+1,formattedDate.length()-1);

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
          digitalWrite(ENCOIN,HIGH);

          if(coinValue > 0){
            cfgState = 4;
            //paymentby = 1;
            
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
            //showPrice(display,dispCount,30,10);
            //coinValue = 0; // This for test only can delete.
          }
          break;
      case 4: //*** After 1st coin insert
          //Serial.printf("----------- This is cfgState 4 ------------");
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
      case 5:
          //Serial.printf("Service running please wait\n");
          display.setBacklight(30);
          display.setColonOn(false);

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

    paymentby = 2;
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
    Serial.printf("Response for action: coincount.\n");
    //{"action":"countcoin","orderNo":"02210526162202176","price":"10.00"}
    //*** two idea 1st: after insert coin machine work immediately.
    //*** 2nd: machine not operate if cannot update to backend.

    String resmsg = doc["StatusCode"].as<String>();
    String msg = doc["Message"].as<String>();
    String trans = doc["ResultValues"]["transactionId"].as<String>();

    doc["response"] = "countcoin";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="accepted";
    doc["desc"]="accepted transaction: " + trans;

  }else if(action == "ping"){

    Serial.printf("response action PING\n");

    doc.clear();
    doc["response"]="ping";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["rssi"]=WiFi.RSSI();
    doc["state"]=cfgState;

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
      saveCFG(cfginfo,LITTLEFS);
      
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
    String ssid = doc["ssid"].as<String>();
    String key = doc["key"].as<String>();
    bool wifireconn = doc["reconnect"].as<bool>();

    cfgdata.begin("wificfg",false);
    cfgdata.putString("ssid",ssid);
    cfgdata.putString("key",key);
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

  }else if(action == "backend"){

  }else if(action == "payboard"){

  }else if(action == "orderid"){

  }else if(action == "stateflag"){ //stateflag is flag for mark action before reboot  ex 1 is for reboot action, 2 for ota action

  }else if(action == "jobcancel"){
    //stateflag = 0;
    //orderid ="";
    //cfgStatte = 3;
  }else if(action == "jobcreate"){
    coinValue = doc["price"].as<int>();
    paymentby = doc["paymentby"].as<int>();  //1 = coin , 2 = qr, 3 = free

    Serial.print("Waitflag: ");
    Serial.println(waitFlag);

    doc.clear();
    doc["response"] = "jobcreate";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="created";
    doc["desc"]="Manual create job.";

  }

  serializeJson(doc,jsonmsg);
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



void WiFiinfo(){
      Serial.printf("\nWiFi Connect to\n");
      Serial.print("   SSID: ");
      Serial.println(WiFi.SSID());
      Serial.print("   IP: ");
      Serial.println(WiFi.localIP());
      Serial.println();

      WiFi.softAPConfig(SoftAP_IP, SoftAP_GW, SoftAP_SUBNET);
      WiFi.softAP( SoftAP_NAME.c_str(),"1100110011",8,false,2 );
      Serial.print("   Soft-AP Name : ");
      Serial.println(WiFi.softAPSSID());
    
      Serial.print("   Soft-AP IP address : ");
      Serial.println(WiFi.softAPIP());
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
  
  if(1){   //Comment this row when production
  //if(startProg(1)){  //unComment this row when production
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

    startProg(1);

    stime[0]=1; // for testing only
    serviceTimeID=serviceTime.after((60*1000*stime[0]),serviceEnd);
    Serial.printf("Starting program 1\n");
  }else{
    coin=0;
    coinValue=0;
    cfgState = 6;
    display.print("E1");
    Serial.printf("Start Program 1 failed\n");
  }
}


void prog2start(){

  payboard backend;
  String response;
  int rescode;

  if(1){   //Comment this row when production
  //if(startProg(2)){    //unComment this row when production
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

    startProg(2);

    stime[1]=1; // for testing only
    serviceTimeID=serviceTime.after((60*1000*stime[1]),serviceEnd);
    Serial.printf("Starting program 2\n");
  }else{
    coin=0;
    coinValue=0;
    cfgState = 6;
    display.print("E2");
    Serial.printf("Start Program 2 failed\n");
  }
}


void prog3start(){

  payboard backend;
  String response;
  int rescode;

  if(1){   //Comment this row when production
  //if(startProg(3)){   //unComment this row when production
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

    stime[2]=1; // for testing only
    serviceTimeID=serviceTime.after((60*1000*stime[2]),serviceEnd);
    Serial.printf("Starting program 3\n");
  }else{
    coin=0;
    coinValue=0;
    cfgState = 6;
    display.print("E3");
    Serial.printf("Start Program 3 failed\n");
  }
}



void serviceEnd(){

  if(digitalRead(PROG1)){
    coinValue = 0;
    coin=0;
    cfgState = 3;
    waitFlag = 0;

    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",0);
    cfgdata.putString("orderid","");
    cfgdata.end();
    Serial.printf("Service Finish\n");
  }else{
    serviceTime.stop(serviceTimeID);
    serviceTimeID=serviceTime.after((60*1000*1),serviceEnd);
  }

}
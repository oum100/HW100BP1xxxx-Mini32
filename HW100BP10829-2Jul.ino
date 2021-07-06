#include <Arduino.h>
#include "startup.h"
//#include <nvs_flash.h>


//SevenSegmentTM1637 display(CLK,DIO);
//SevenSegmentExtended display(CLK,DIO);
//SevenSegmentFun display(CLK,DIO);
digitdisplay display(CLK,DIO);

Preferences cfgdata;
Config cfginfo;

String pb_uuid;
// String pb_merchantid;
// String pb_mqtthost;
// String pb_mqttuser;
// String pb_mqttpass;

int price[3];
int stime[3];
int coinValue=0;
int pricePerCoin=0;
// int pb_mqttport=1883;
int waitFlag = 0;

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
int paymentby = -1;
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

    //io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_config.intr_type = GPIO_INTR_NEGEDGE;
    //io_conf.intr_type = GPIO_INTR_POSEDGE;
    //io_conf.intr_type = GPIO_INTR_LOW_LEVEL;
    //io_conf.intr_type = GPIO_INTR_HIGH_LEVEL;
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
  Serial.begin(115200); 
  Serial.println();
  Serial.println("Booting...");
  // nvs_flash_erase(); // reformats
  // nvs_flash_init(); // initializes
  // while(true);


  //*** initial 7Segment Display
  display.begin();
  display.setBacklight(10);
  display.print("C0");
 
 
  //*** Initial GPIO
  initOUTPUT(TOTALOUTPUT,OUTPUTPIN);
  initINPUT(TOTALINPUT,INPUTPIN);

  //digitalWrite(ENCOIN,HIGH);
  //pinMode(LED1,OUTPUT);
  
  //*** Intial Interrupt
  interrupt();

  //Check Config file if found load them.
  if(initFS(LITTLEFS)){
    //LITTLEFS.format();
    Serial.println("Looing for config.json");
    if(isFile(LITTLEFS,"/config.json")){
      //*** Load config from config.json
      
      json_config = readFile2(LITTLEFS,"/config.json");
      Serial.println();
      Serial.print("Config from FS: ");
      Serial.println(json_config);
      LITTLEFS.end();

      readCFG(cfginfo,json_config);
      //showCFG(cfginfo);

      // pb_uuid = cfginfo.payboard.uuid;
      // pb_merchantid = cfginfo.payboard.merchantid;
      // pb_mqtthost = cfginfo.payboard.mqtthost;
      // pb_mqttport = cfginfo.payboard.mqttport;
      // pb_mqttuser = cfginfo.payboard.mqttuser;
      // pb_mqttpass = cfginfo.payboard.mqttpass;

      int sz = sizeof(cfginfo.product)/sizeof(cfginfo.product[0]);
      for(int i=0;i<sz;i++){
        price[i] = cfginfo.product[i].price;
        stime[i] = cfginfo.product[i].stime;
      }

      //Set mqtt parameter
      pbPubTopic = pbPubTopic  + String(cfginfo.payboard.merchantid) +"/"+ String(cfginfo.payboard.uuid);
      pbSubTopic = pbSubTopic + String(cfginfo.payboard.merchantid) +"/"+ String(cfginfo.payboard.uuid);

      Serial.printf("Set pbPubTopic %s\n",pbPubTopic.c_str());
      Serial.printf("Set pbSubTopic %s\n",pbSubTopic.c_str());

      //*** Set price per coin
      if(cfginfo.asset.coinModule){
        pricePerCoin = 1;
      }else{
        pricePerCoin = 10;
      }

      cfgState = 3;  // read config
    }else{ // config.json not found

      //*** Initial Config parameters
      initCFG(cfginfo); //Loade Default Config
      //showCFG(cfginfo);
      
      cfginfo.header = "EITC";
      cfginfo.asset.mac = WiFi.macAddress().c_str();
      cfginfo.deviceid = getdeviceid().c_str();



      // cfginfo.deviceid = deviceID.c_str();
      // cfginfo.asset.mac = macaddr.c_str();
      cfginfo.asset.coinModule = SINGLE;

      //Check ROM for uuid 
      cfgdata.begin("config",false);
      if(cfgdata.isKey("uuid")){
        Serial.println("   Device registered but not configure.");
        
        pb_uuid = cfgdata.getString("uuid");
        cfginfo.payboard.uuid = pb_uuid.c_str();
        cfgState = 2;
      }else{
        Serial.println("   Device not register and not configure.");
        cfgState = 1;
      }
      cfgdata.end();
    }
    LITTLEFS.end();
  }else{
    Serial.printf("Mounting File System failed");
    display.print("E1");
  }

  
  //Check reboot Flag
  cfgdata.begin("config",false);
  if(cfgdata.isKey("stateflag")){
    stateflag = cfgdata.getInt("stateflag");
    Serial.printf("\nstateflag: %d\n",stateflag);
  }else{
    Serial.printf("stateflag key not found\n");
    cfgdata.putInt("stateflag",0);
  }
  cfgdata.end();




  // Serial.printf("\nStep 8: Connecting to TimeServer --> ");
  // //Set NTP
  // timeClient.begin();
  // timeClient.setTimeOffset(25200);  //GMT+7
  // Serial.printf("Done.\n");

  //Connecting Mqtt 


  Serial.printf("\n****************************************\n");
  Serial.printf("\nSystem booting\n");      
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
        String cfginfoJSON;

        initFS(LITTLEFS);

        cfginfoJSON = cfgJSON(cfginfo);
        Serial.println("Create config.json");
        writeFile(LITTLEFS,"/config.json",cfginfoJSON.c_str());
        if(isFile(LITTLEFS,"/config.json")){
            Serial.println("**** Payboard.json save completed.");
        }
        LITTLEFS.end();
        display.print("CC"); //Delete Completed
        delay(1000);
        break;
  }
  

  if(!WiFi.isConnected()){
    digitalWrite(0,LOW);
    Serial.printf("WiFi Connecting.....\n");

    wifimulti.addAP("Home173-AIS","1100110011");
    wifimulti.addAP("BTnet","1100110011");
    WiFi.mode(WIFI_AP_STA);
    wifimulti.run();
     
    delay(3000);
    if(WiFi.isConnected()){
      Serial.println("connected");
      WiFiinfo();

      Serial.print("cfgState: ");
      Serial.println(cfgState);

    }    
  }else{
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

    if(stateflag > 0){
      String jsonmsg ="";
      StaticJsonDocument<200> doc;
      cfgdata.begin("config",false);
      switch(stateflag){
        case 1: //This from action REBOOT
            // String jsonmsg ="";
            // StaticJsonDocument<200> doc;
            Serial.printf("Request reboot completed.\n");
            
            doc["response"]="reboot";
            doc["merchantid"]=cfginfo.payboard.merchantid;
            doc["uuid"]=cfginfo.payboard.uuid;
            doc["state"]="Rebooted";
            serializeJson(doc,jsonmsg);
            doc.clear();

            if(!mqclient.connected()){
              pbBackendMqtt();
            }
            mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
            delay(300);
            //Publish mqtt back to backend.
            
            cfgdata.putInt("stateflag",0);
            
            break;
        case 2:  //This from action OTA
            // String jsonmsg ="";
            // StaticJsonDocument<200> doc; 
            Serial.printf("Request OTA completed.\n");
            
            doc["response"]="ota";
            doc["merchantid"]=cfginfo.payboard.merchantid;
            doc["uuid"]=cfginfo.payboard.uuid;
            doc["state"]="updated";
            serializeJson(doc,jsonmsg);
            doc.clear();

            if(!mqclient.connected()){
              pbBackendMqtt();
            }
            mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
            cfgdata.putInt("stateflag",0);
            break;
        case 5:  //This from Busy or onService
            Serial.printf("Asset Busy.\n");
            break;    
      } 
      cfgdata.end();
      stateflag = 0;
    }

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
          
          digitalWrite(ENCOIN,HIGH);
        
          if(coinValue > 0){
            cfgState = 4;
            paymentby = 0;
          }else{
            String tmp = String(price[0]) + "--" + String(price[1]) + "--" + String(price[2]);
            display.scrollingText(tmp.c_str(),2);
            //showPrice(display,dispCount,30,10);
            coinValue = 0;
          }
          
          
          break;
      case 4: //*** After 1st coin insert
          switch(paymentby){
            case 0: // by Coin
              break;
            case 1: // by ThaiQR
              break;
            case 2:
              break;
          }

          if((coinValue == price[0]) && (waitFlag == 0)){
            waitFlag = 1;
            //cfgState = 5;
            Serial.printf("Program 1 :%d\n", coinValue);
            waitTimeID=waitTime.after(60*1000*0.15,prog1start);
            
          }else if((coinValue == price[1]) && (waitFlag <= 1)){
            waitTime.stop(waitTimeID);
            waitFlag =2;
            //cfgState = 5;
            Serial.printf("Program 2 :%d\n", coinValue);
            
            waitTimeID=waitTime.after(60*1000*0.15,prog2start);
            
          }else if((coinValue == price[2]) && (waitFlag <= 2)){
            waitTime.stop(waitTimeID);
            waitFlag= 3;
            //cfgState = 5;
            Serial.printf("Program 3 :%d\n", coinValue);
            prog3start();
            
            
          }else{
            display.setBacklight(30);
            display.print(coinValue);
            display.setColonOn(true);
          }


          break;
      case 5:
          //Serial.printf("Service running please wait\n");
          display.setBacklight(20);
          display.setColonOn(false);
          switch(waitFlag){
            case 1:
              display.bouncingBall(15, 80);
              break;
            case 2:
              display.animation1(display,100,2);
              break;
            case 3:
              display.animation2(display,100,2);
              break;
            default:
              break;
          }

          //ani1.animation1(display,100,2);
          break;
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
      cfginfo.payboard.uuid = doc["uuid"];
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
  mqclient.setServer(cfginfo.payboard.mqtthost,cfginfo.payboard.mqttport);
  mqclient.setCallback(pbRegCallback);

  pbSubTopic = "payboard/" + String(cfginfo.payboard.merchantid)+ "/" + String(cfginfo.asset.mac);
  Serial.printf("   Register Mqtt connecting ...");
  while(!mqclient.connect(cfginfo.deviceid,cfginfo.payboard.mqttuser,cfginfo.payboard.mqttpass)){
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
  Serial.print("Message arrived pbCallback with topic: ");
  Serial.println(topic);

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

  String action = doc["action"].as<String>();
  Serial.print(" This action paramater: ");
  Serial.println(action);

  doc.clear();
  doc["merchantid"]=cfginfo.payboard.merchantid;
  doc["uuid"]=cfginfo.payboard.uuid;

  if(action == "config"){
    // int sz = doc["detail"].size();
    Serial.print("Array size: ");Serial.println(doc["detail"].size());
    if(initFS(LITTLEFS)){
      String cfginfoJSON PROGMEM;

      cfginfoJSON = cfgJSON(cfginfo);
      Serial.print("configJSON: ");
      Serial.println(cfginfoJSON);
      writeFile2(LITTLEFS,"/config.json",cfginfoJSON.c_str());
      LITTLEFS.end();
      Serial.println("  configCFG save completed.");      
      delay(500);
      ESP.restart();
    }else{
      Serial.println("   File System failed");
    }
    

    cfgState = 3;
    display.print("C3");

  }else if(action == "paid"){
    //Paid and then start service.

  }else if(action == "payment"){
    //{"qrtxt":"00020101021230830016A0000006770101120115010556006812755021800000021052602511503180002105261622021335303764540520.005802TH5910GBPrimePay630486BA","orderNo":"210526162202133","price":20}

  }else if(action == "countcoin"){
    //{"action":"countcoin","orderNo":"02210526162202176","price":"10.00"}
    //*** two idea 1st: after insert coin machine work immediately.
    //*** 2nd: machine not operate if cannot update to backend.

  }else if(action == "ping"){

    Serial.printf("response action PING\n");

    doc.clear();
    doc["response"]="ping";
    doc["rssi"]=WiFi.RSSI();
    doc["state"]=cfgState;

    serializeJson(doc,jsonmsg);
    Serial.print("Jsonmsg: ");
    Serial.println(jsonmsg);

    mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());

  }else if( (action == "reset") || (action=="reboot") || (action=="restart")){
      //set stateflag = 2 flag
      Serial.printf("Accept request action reboot\n");

      cfgdata.begin("config",false);
      cfgdata.putInt("stateflag",1);
      cfgdata.end();
        
      doc["response"]="reboot";
      doc["state"]="accepted";
      doc["discreption"] = "Asset rebooting.";

      serializeJson(doc,jsonmsg);
      Serial.print("Jsonmsg: ");
      Serial.println(jsonmsg);
      
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
      doc["firmware"] = esp32OTA._firwmareVersion;
      doc["state"] = "accepted";
      doc["discreption"] = "Firmware upgrading then rebooting in few second."; 
      serializeJson(doc,jsonmsg);
      Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
      Serial.print("Ver: "); Serial.println(esp32OTA._firwmareVersion);

      if(!mqclient.connected()){
        pbBackendMqtt();
      }
      mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
    
      //DBprintf(" - Firmware: %s\n\n",firmware.c_str());  
      delay(500);

      Serial.println("Firmware updating, It's will take few second");
      esp32OTA.executeOTA();
    }else{
      doc["response"] = "OTA";
      doc["merchanttid"] = cfginfo.payboard.merchantid;
      doc["uuid"] = cfginfo.payboard.uuid;
      doc["STATUS"] = "FAILED";
      doc["DESC"] = "Firmware update failed, version may be the same."; 
    }
  }
}


void pbBackendMqtt(){
          Serial.println("[pbBackendMqtt]");
          Serial.println(cfginfo.payboard.uuid);
          Serial.println(cfginfo.payboard.merchantid);
          Serial.println(cfginfo.payboard.mqtthost);
          Serial.println(cfginfo.payboard.mqttport);
          Serial.println(cfginfo.payboard.mqttuser);
          Serial.println(cfginfo.payboard.mqttpass);

    if(!mqclient.connected()){
      mqclient.setServer(cfginfo.payboard.mqtthost,cfginfo.payboard.mqttport);
      mqclient.setCallback(pbCallback);

      
      pbSubTopic = "payboard/" + String(cfginfo.payboard.merchantid) + "/" + String(cfginfo.payboard.uuid);
    
      Serial.printf("Backend Mqtt connecting ...");
      while(!mqclient.connect(cfginfo.deviceid,cfginfo.payboard.mqttuser, cfginfo.payboard.mqttpass)){
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
  cfgState = 5;
  cfgdata.begin("config",false);
  cfgdata.putInt("stateflag",5);
  cfgdata.end();
  serviceTimeID=serviceTime.after((60*1000*1),serviceEnd);
  Serial.printf("Starting program 1\n");
}

void prog2start(){
  cfgState = 5;
  cfgdata.begin("config",false);
  cfgdata.putInt("stateflag",5);
  cfgdata.end();
  serviceTimeID=serviceTime.after((60*1000*1),serviceEnd);
  Serial.printf("Starting program 2\n");
}

void prog3start(){
  cfgState = 5;
  cfgdata.begin("config",false);
  cfgdata.putInt("stateflag",5);
  cfgdata.end();
  serviceTimeID=serviceTime.after((60*1000*1),serviceEnd);
  Serial.printf("Starting program 3\n");
}


void serviceEnd(){
  coinValue = 0;
  cfgState = 3;
  Serial.printf("Service Finish\n");
}
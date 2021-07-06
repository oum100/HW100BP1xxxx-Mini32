#include <Arduino.h>
#include "startup.h"
//#include <nvs_flash.h>

// ******************************  Coin global parameters ******************************
//Preferences iotcfg;


//  7 Segment display
SevenSegmentTM1637 display(CLK,DIO);
int keyPress=0;

//WiFi 
WiFiMulti wifimulti;

//Time Server
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP);
String formattedDate;
String epochtime;
String dayStamp;
String timeStamp;

//Mqtt server
WiFiClient espclient;
PubSubClient mqclient(espclient);

//PubSubClient mq2client(esp2client);

// Asset Configuration
// Config config;
//DeviceConfig cfgInfo;

DeviceConfig  devcfg(devcfg.cfginfo);

enum pmType {COIN,THAIQR};
int paymentby = -1;
int coinValue;
int pricePerPulse;

String macaddr = WiFi.macAddress();
String deviceID = getdeviceid();
String SoftAP_NAME = "BT_" + deviceID;
IPAddress SoftAP_IP(192,168,8,20);
IPAddress SoftAP_GW(192,168,8,1);
IPAddress SoftAP_SUBNET(255,255,255,0);

//Payboard
String pbRegTopic = "payboard/register";
String pbPubTopic = "payboard/backend";
String pbSubTopic="";
//String pbSubTopic = "payboard/" + String(config.payboard.merchantid)+ "/" + String(config.asset.mac);

// Timer
Timer serviceTime;

// Stage and Flag
int state;
int isConfigure;
int isRegister;
int paymentGW = 1; //1=payboard, 2=other;
int cfgState = 0;
// ******************************  Coin global parameters ******************************

void setup(){
  Serial.begin(115200);

  // nvs_flash_erase(); // erase the NVS partition and...
  // nvs_flash_init(); // initialize the NVS partition.
  // while(true);

  isRegister = true;
  isConfigure = true;
  int sz1= sizeof(devcfg.cfginfo);
  Serial.print("size cfginfo: ");
  Serial.println(sz1);

  sz1= sizeof(devcfg.cfginfo.wifissid);
  Serial.print("size wifissid: ");
  Serial.println(sz1);

  sz1= sizeof(devcfg.cfginfo.wifissid[0]);
  Serial.print("size wifi0: ");
  Serial.println(sz1);

  sz1= sizeof(devcfg.cfginfo.product);
  Serial.print("size product: ");
  Serial.println(sz1);

  sz1= sizeof(devcfg.cfginfo.product[0]);
  Serial.print("size product0: ");
  Serial.println(sz1); 

  //Setting 7Segment
  display.begin();
  display.setBacklight(10);
  display.print("S1");
  delay(500);

  //setup GPIO

  initOUTPUT(TOTALOUTPUT,OUTPUTPIN);
  initINPUT(TOTALINPUT,INPUTPIN);
  display.print("S2");
  delay(500);

  //Setup WiFi
  wifimulti.addAP("Home173-AIS","1100110011");
  wifimulti.addAP("BTnet","1100110011");
  WiFi.mode(WIFI_AP_STA);
  display.print("S3");
  delay(500);

  //Initial Interrupt
  interrupt();

 // mq2client.setServer("flipup.net",1883);
 // mq2client.setCallback(btCallback);
  

  if(paymentGW ==1){
    //Check register and configuration
      if(!LITTLEFS.begin(true)){
        Serial.printf("FileSystem Mount Failed\n");
        return;
      }
      if(!isFile(LITTLEFS,"/config.json")){
        Serial.println("Device not register and not configure.");
        isConfigure = false;
        isRegister = false;
        cfgState = 1;
        
      }
        
  }

  // if(paymentGW == 10){ // Payment is payboard
  //   //Verifiy Config
  //   iotcfg.begin("config",false); //false = readwrite, true = readonly
  //   if( (iotcfg.isKey("payboard_uuid")) && (iotcfg.isKey("product1.name")) ){
  //     //registered
  //     Serial.println("Loading Payboard configuration");
  //     devcfg.readCFG(devcfg.cfginfo);
  //     isRegister = true;
  //     isConfigure = true;
  //     cfgState = 1;
  //     display.print("S4");
  //     delay(1000);

  //   }else{
  //     if(iotcfg.isKey("payboard_uuid")){
  //       Serial.println("Device register but not configure.");
  //       isRegister = true;
  //       isConfigure = false;
  //       cfgState = 2;
  //     }else{
  //       Serial.println("Device not register and not configure.");
  //       isConfigure = false;
  //       isRegister = false;
  //       cfgState = 3;
  //     }

  //     display.print("CF");
  //     delay(500);
  //   }
  //   iotcfg.end();
  //   // Serial.println(cfgState);
  // }



  if(paymentGW == 2){

  }
 
 
  Serial.printf("System booting.....please wait\n");
  Serial.printf("System Information.\n");

}





void loop(){
  // WiFi Connection
  // if((keyPress = display.comReadByte()) == 244){
  //   Serial.println("Clear uuid");
  //   // iotcfg.begin("config",false);
  //   // iotcfg.remove("payboard_uuid");
  //   // iotcfg.end();
  // }
  if(WiFi.isConnected()){
    blinkGPIO(GREEN_LED,500); 
    //Serial.println("WiFi connected");
    // if(!mq2client.connected()){
    //   mq2client.connect("flipup","sammy","password");
    // }
   
    //Time Server
    // while(!timeClient.update()) {
    //   timeClient.forceUpdate();
    // }
      
  }else{
      digitalWrite(GREEN_LED,LOW);
      Serial.printf("WiFi Connecting.....\n");
      wifimulti.run();
      delay(2000);
      // if(WiFi.isConnected()){
      //   //Time Server
      //   timeClient.begin();
      //   timeClient.setTimeOffset(25200);
      //   WiFiinfo();
      // }
  }


  
  if(paymentGW ==1){
   
    // switch(cfgState){
    //   case 1: //**** Device already register and configure   (Keep Mqtt conncted)
    //       if(!mqclient.connected()){
    //         while(!mqclient.connect(deviceID.c_str(),devcfg.cfginfo.payboard.mqttuser,devcfg.cfginfo.payboard.mqttpass)){
    //           Serial.printf(".");
    //           delay(500);
    //         }
    //         mqclient.subscribe(pbSubTopic.c_str());
    //       }
    //       break;
    //   case 2: //**** Device already register but not configure.
    //       mqclient.setServer(devcfg.cfginfo.payboard.mqtthost,devcfg.cfginfo.payboard.mqttport);
    //       mqclient.setCallback(pbBackendCallback);
    //       pbSubTopic = "payboard/" + String(devcfg.cfginfo.payboard.merchantid)+ "/" + String(devcfg.cfginfo.payboard.uuid);
    //       if(!mqclient.connected()){
    //         while(!mqclient.connect(deviceID.c_str(),devcfg.cfginfo.payboard.mqttuser,devcfg.cfginfo.payboard.mqttpass)){
    //           Serial.printf(".");
    //           delay(500);
    //         }
    //         mqclient.subscribe(pbSubTopic.c_str());
    //       }
    //       break;
    //   case 3:   //**** Device not register and not configure
    //       mqclient.setServer(devcfg.cfginfo.payboard.mqtthost,devcfg.cfginfo.payboard.mqttport);
    //       mqclient.setCallback(pbRegCallback);
    //       pbSubTopic = "payboard/" + String(devcfg.cfginfo.payboard.merchantid)+ "/" + String(devcfg.cfginfo.asset.mac);

    //       if(!mqclient.connected()){
    //         while(!mqclient.connect(deviceID.c_str(),devcfg.cfginfo.payboard.mqttuser,devcfg.cfginfo.payboard.mqttpass)){
    //           Serial.printf(".");
    //           delay(500);
    //         }
    //         Serial.printf("connected\n");
    //         Serial.printf("Registering to backend server\n");
    //         mqclient.subscribe(pbSubTopic.c_str());
    //         Serial.printf("   Subscribe Topic: %s\n",pbSubTopic.c_str());

    //         DynamicJsonDocument doc(1024);
    //         String mqMsg = "";
    //         doc["merchantid"]=devcfg.cfginfo.payboard.merchantid;
    //         doc["mac"]=devcfg.cfginfo.asset.mac;
    //         doc["model"]=devcfg.cfginfo.asset.model;
    //         doc["firmware"]=devcfg.cfginfo.asset.firmware;
    //         serializeJson(doc,mqMsg);

    //         mqclient.publish(pbRegTopic.c_str(),mqMsg.c_str());
    //         //delay(1000);
    //       }
    //       break;
    // }
    //isRegister = true;
    if(!isRegister){
      mqclient.setServer(devcfg.cfginfo.payboard.mqtthost,devcfg.cfginfo.payboard.mqttport);
      mqclient.setCallback(pbRegCallback);
      pbSubTopic = "payboard/" + String(devcfg.cfginfo.payboard.merchantid)+ "/" + String(devcfg.cfginfo.asset.mac);

      if(!mqclient.connected()){
        while(!mqclient.connect(deviceID.c_str(),devcfg.cfginfo.payboard.mqttuser,devcfg.cfginfo.payboard.mqttpass)){
          Serial.printf(".");
          delay(500);
        }
        Serial.printf("connected\n");
        Serial.printf("Registering to backend server\n");
        mqclient.subscribe(pbSubTopic.c_str());
        Serial.printf("   Subscribe Topic: %s\n",pbSubTopic.c_str());

        DynamicJsonDocument doc(256);
        String mqMsg = "";
        doc["merchantid"]=devcfg.cfginfo.payboard.merchantid;
        doc["mac"]=devcfg.cfginfo.asset.mac;
        doc["model"]=devcfg.cfginfo.asset.model;
        doc["firmware"]=devcfg.cfginfo.asset.firmware;
        serializeJson(doc,mqMsg);


        if(mqclient.connected()){
          Serial.print("   Publish topic: ");
          Serial.println(mqMsg);
          mqclient.publish(pbRegTopic.c_str(),mqMsg.c_str());
        }else{
          Serial.println("Mqtt not connect.");
        }
        
        //delay(1000);
      }
    }
    isRegister=false;
    if((isRegister)&&(!isConfigure)){
      Serial.println(" it is here");
      mqclient.setServer(devcfg.cfginfo.payboard.mqtthost,devcfg.cfginfo.payboard.mqttport);
      mqclient.setCallback(pbBackendCallback);
      pbSubTopic = "payboard/" + String(devcfg.cfginfo.payboard.merchantid)+ "/" + String(devcfg.cfginfo.payboard.uuid);
      if(WiFi.isConnected()){
        if(!mqclient.connected()){
          Serial.printf("Mqtt Connecting...");
          while(!mqclient.connect(deviceID.c_str(),devcfg.cfginfo.payboard.mqttuser,devcfg.cfginfo.payboard.mqttpass)){
            Serial.printf(".");
            delay(500);
          }
          mqclient.subscribe(pbSubTopic.c_str());
          Serial.println(pbSubTopic);
        }
      }
    }else{
      pbSubTopic = "payboard/" + String(devcfg.cfginfo.payboard.merchantid)+ "/" + String(devcfg.cfginfo.payboard.uuid);
      if(!mqclient.connected()){
        while(!mqclient.connect(deviceID.c_str(),devcfg.cfginfo.payboard.mqttuser,devcfg.cfginfo.payboard.mqttpass)){
          Serial.printf(".");
          delay(500);
        }
        mqclient.subscribe(pbSubTopic.c_str());
      }
    }
  }



  if(paymentGW == 2){

  }






  if( isConfigure && (WiFi.isConnected()) ){
    //Serial.printf("System ready for service. waiting for customer\n");
    //delay(1000);
  }

    // ***** Loop for Library
    serviceTime.update();
    mqclient.loop();
    //mq2client.loop();
}









void btCallback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived Flipup from topic: ");
  Serial.println(topic);

  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


void pbRegCallback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived from topic: ");
  Serial.println(topic);

  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  DynamicJsonDocument doc(128);
  DeserializationError error = deserializeJson(doc, payload);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  String uuid = doc["uuid"];
  if(!uuid.isEmpty()){
    devcfg.cfginfo.payboard.uuid = uuid.c_str();
    // iotcfg.begin("config",false);
    // iotcfg.putString("payboard_uuid",devcfg.cfginfo.payboard.uuid);
    // iotcfg.end();
    Serial.print("Got UUID: ");
    Serial.println(devcfg.cfginfo.payboard.uuid);
    //mqclient.unsubscribe(pbSubTopic.c_str());
   // mqclient.disconnect();
    isRegister = true;
  }
}


void pbBackendCallback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived from topic: ");
  Serial.println(topic);

  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payload);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  // ***** Mqtt Command 
  String action = doc["action"];
  action.toLowerCase();

  if(action == "ping"){
    Serial.printf(" Action Ping");


  }else if(action == "payment"){
    Serial.printf(" Action payment");


  }else if(action == "config"){
    JsonArray jarray = doc["detail"].as<JsonArray>();
    int i = 0;

    JsonArray array = doc["detail"].as<JsonArray>();
    int inxProd = array.size();
    for(i=0;i<inxProd;i++){
      devcfg.cfginfo.product[i].name = doc["detail"][i]["number"];
      devcfg.cfginfo.product[i].price = float(doc["detail"][i]["price"]);
    }

    Serial.print("Config data: ");
    //Serial.println(saveCFG(config));
    
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


int newpayPOST(String server, String req, String &res){
  
  HTTPClient client;

  DBprintf(" --- Req to server: %s\n",server.c_str());
  DBprintf(" --- Req body: %s\n",req.c_str());

  client.begin(server);
  client.addHeader("Content-Type", "application/json");
  int rescode = client.POST(req.c_str());

  //responsebody = client.getString();
  res = client.getString();

  DBprintf(" --- Response Code: %d\n",rescode);
  DBprintf(" --- Response Body: %s\n",res.c_str());

  //res = &responsebody;

  client.end();
  return rescode;
}

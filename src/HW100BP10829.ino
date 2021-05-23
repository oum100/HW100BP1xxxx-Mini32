#include <Arduino.h>
#include "startup.h"


// ******************************  Coin global parameters ******************************
Preferences iotcfg;

//  7 Segment display
SevenSegmentTM1637 display(CLK,DIO);
int keyPress=0;

//WiFi 
WiFiMulti wifimulti;

//Time Server
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String epochtime;
String dayStamp;
String timeStamp;

//Mqtt server
WiFiClient espclient;
PubSubClient mqclient(espclient);

// Asset Configuration
Config config;

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
String pbSubTopic = "payboard/" + String(config.payboard.merchantid)+ "/" + String(config.asset.mac);

// Timer
Timer serviceTime;

// Stage and Flag
int state;
int isConfigure;
int isRegister;

// ******************************  Coin global parameters ******************************

void setup(){
  Serial.begin(115200);

  //Setting 7Segment
  display.begin();
  display.setBacklight(10);

  //setup GPIO
  initGPIO(); // From config.h

  //Setup WiFi
  wifimulti.addAP("Home173-AIS","1100110011");
  wifimulti.addAP("BTnet","1100110011");
  WiFi.mode(WIFI_AP_STA);

 iotcfg.begin("config",false);
  if(iotcfg.isKey("uuid")){
    Serial.println("uuid is key");
  }else{
    Serial.println("not found uuid");
  }

   //Check header and config file
  /*
  if(!LITTLEFS.begin(true)){
    Serial.printf("FileSystem Mount Failed\n");
    return;
  }
  if(!isFile(LITTLEFS,"/config.json")){
    isRegister  = false;
    isConfigure = false;
    state = 0;
    Serial.printf("System not register\n");
    display.print("CF");
    
    //Use default configuration  
    //******************   Initial Default parameter ******************  
      config.payboard.merchantid = "1000000104";
      config.payboard.mqtthost ="mq3.payboard.cc";
      config.payboard.mqttport = 1883;
      config.payboard.mqttuser = config.payboard.merchantid;
      config.payboard.mqttpass="mki9lvhjpp1xt4jxgjdjqxuhx2ihucgkgz9ledsylsu7terwtsnibhhjzrnsiiig";

      //macaddr = WiFi.macAddress();
      config.asset.mac = macaddr.c_str();
      config.asset.model ="HW10BP10829_V2.0.0";
      config.asset.firmware = "1.0.0";

      config.backend.apihost="https://cointracker100.herokuapp.com/cointracker/v1.0.0/devices";
      config.backend.apikey="87cdf9229caf9a7fa3fd1403bcc5dd97";
      config.backend.mqtthost="flipup.net";
      config.backend.mqttuser="sammy";
      config.backend.mqttpass="password";
      config.backend.mqttport = 1883;        

      pbSubTopic = "payboard/" + String(config.payboard.merchantid )+ "/" + String(config.asset.mac);
    
    Serial.printf("WiFi Connecting ...");
    while(wifimulti.run() != WL_CONNECTED){
      Serial.printf(".");
      delay(500);
    }
    Serial.printf("connected\n");

    Serial.printf("Mqtt Connecting ..."); 
    mqclient.setServer(config.payboard.mqtthost,config.payboard.mqttport);
    mqclient.setCallback(pbRegCallback);
    while(!mqclient.connect(deviceID.c_str(),config.payboard.mqttuser,config.payboard.mqttpass)){
      Serial.printf(".");
      delay(500);
    }
    Serial.printf("connected\n");
    Serial.printf("Registering to backend server\n");
    mqclient.subscribe(pbSubTopic.c_str());
    Serial.printf("   Subscribe Topic: %s\n",pbSubTopic.c_str());

    DynamicJsonDocument doc(256);
    String mqMsg = "";
    doc["merchantid"]=config.payboard.merchantid;
    doc["mac"]=config.asset.mac;
    doc["model"]=config.asset.model;
    doc["firmware"]=config.asset.firmware;
    serializeJson(doc,mqMsg);

    mqclient.publish(pbRegTopic.c_str(),mqMsg.c_str());

  }else{
    isConfigure = true;
    String cfginfo;
    //cfginfo = readFile1(LITTLEFS,"/config.json");
    Serial.printf(" Config Info: %s\n",cfginfo);
    //readCFG(config,cfginfo);
    state = 1;
  }
  */

  //

  Serial.printf("System booting.....please wait\n");
  Serial.printf("System Information.\n");
  Serial.printf("   MacAddress: %s\n",config.asset.mac);
  Serial.printf("   Model: %s\n",config.asset.model);
  Serial.printf("   Firmwall: %s\n",config.asset.firmware);


  /* Test newpay only
  int pri = 2;
  String url = "https://newpay.dev/ksher";
  String reqmsg = "{\"device_id\":\"10010101\",\"amount\":\"" + String(pri) + "\"}";
  Serial.println(reqmsg);
  String resmsg;
  newpayPOST(url,reqmsg,resmsg);
  */
}


void loop(){
// WiFi Connection
if(WiFi.isConnected()){
  blinkGPIO(GREEN_LED,500); 
  
  //Time Server
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
    
}else{
    digitalWrite(GREEN_LED,LOW);
    Serial.printf("WiFi Connecting.....\n");
    wifimulti.run();
    delay(800);
    if(WiFi.isConnected()){

      //Time Server
      timeClient.begin();
      timeClient.setTimeOffset(25200);

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
}

if( !isConfigure && !(String(config.payboard.uuid).isEmpty()) ){

  pbSubTopic = "payboard/"+String(config.payboard.merchantid)+"/"+(config.payboard.uuid);
  mqclient.setCallback(pbBackendCallback);
  mqclient.subscribe(pbSubTopic.c_str());
  Serial.printf(" Got uuid but not config yet\n");

  //String msg = saveCFG(config);
  //Serial.println(msg);

  isConfigure = true;
  

  // Serial.printf("\nRegistering and getting configuration processes.\n");
  // mqclient.setServer(config.payboard.mqtthost,config.payboard.mqttport);
  // mqclient.setCallback(pbRegCallback);
  // // ***** This is register process for Payboard Payment Gateway.
  
  // Serial.printf("   Mqtt connecting ...");
  // while(!mqclient.connect(deviceID.c_str(),config.payboard.mqttuser,config.payboard.mqttpass)){
  //   Serial.printf(".");
  // }
  // Serial.printf("connected.\n");
  // Serial.printf("   SubTopic: %s\n",pbSubTopic.c_str());
  // mqclient.subscribe(pbSubTopic.c_str());



  // // ***** This is register process for Flipup

  // isConfigure = true;

  //Mqtt Server
}


if( isConfigure && (WiFi.isConnected()) ){
  //Serial.printf("System ready for service. waiting for customer\n");
  //delay(1000);
}

  // ***** Loop for Library
  serviceTime.update();
  mqclient.loop();
}





void pbRegCallback(char* topic, byte* payload, unsigned int length){
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

  config.payboard.uuid = doc["uuid"];
  Serial.print("UUID: ");
  Serial.println(config.payboard.uuid);
  mqclient.unsubscribe(pbSubTopic.c_str());
  isRegister = true;

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
      config.product[i].name = doc["detail"][i]["number"];
      config.product[i].price = float(doc["detail"][i]["price"]);
    }

    Serial.print("Config data: ");
    Serial.println(saveCFG(config));
    
  }
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
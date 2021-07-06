#include <Arduino.h>
#include "startup.h"


SevenSegmentTM1637 display(CLK,DIO);

//Preferences cfgdata;
Config cfginfo;
//String cfginfoJSON;
String pbuuid;


WiFiMulti wifimulti;
WiFiClient espclient;
PubSubClient mqclient(espclient);

String macaddr  = WiFi.macAddress();
String deviceID = getdeviceid();
String SoftAP_NAME  = "BT_" + deviceID;
IPAddress SoftAP_IP(192,168,8,20);
IPAddress SoftAP_GW(192,168,8,1);
IPAddress SoftAP_SUBNET(255,255,255,0);

//Payboard
String pbRegTopic = "payboard/register";
String pbPubTopic = "payboard/backend";
String pbSubTopic = "";

byte keyPress; //*** for keep keypress value.
byte cfgState=0;



void setup(){
  Serial.begin(115200); 

  //*** initial 7Segment Display
  display.begin();
  display.setBacklight(10);
  display.print("C0");

  //*** Initial GPIO
  initOUTPUT(TOTALOUTPUT,OUTPUTPIN);
  initINPUT(TOTALINPUT,INPUTPIN);
  //pinMode(LED1,OUTPUT);
  
  //*** Intial Interrupt
  interrupt();

  initCFG(cfginfo);

  if(initFS(LITTLEFS)){

  }





  Serial.printf("\n****************************************\n");
  Serial.printf("\nSystem booting\n");
}


void loop(){
  if((keyPress=display.comReadByte())==244){
    initFS(LITTLEFS);
    Serial.println("Remove asset.json");
    deleteFile(LITTLEFS,"/asset.json");
    if(!isFile(LITTLEFS,"/asset.json")){
      Serial.println("**** Asset.json not found.");
    }

    deleteFile(LITTLEFS,"/backend.json");
    if(!isFile(LITTLEFS,"/backend.json")){
      Serial.println("**** Backend.json not found.");
    }

    deleteFile(LITTLEFS,"/payboard.json");
    if(!isFile(LITTLEFS,"/payboard.json")){
      Serial.println("**** Payboard.json not found.");
    }

    deleteFile(LITTLEFS,"/config.json");
    if(!isFile(LITTLEFS,"/config.json")){
      Serial.println("**** Config.json not found.");
    }  
    LITTLEFS.end();
    display.print("DC"); //Delete Completed
    delay(1000);
  }


  if((keyPress=display.comReadByte())==247){

    String cfginfoJSON;

    initFS(LITTLEFS);

    cfginfoJSON = payboardJSON(cfginfo);
    Serial.println("Create payboard.json");
    writeFile(LITTLEFS,"/payboard.json",cfginfoJSON.c_str());
    if(isFile(LITTLEFS,"/payboard.json")){
        Serial.println("**** Payboard.json save completed.");
    }

    cfginfoJSON = backendJSON(cfginfo);
    Serial.println("Create backend.json");
    writeFile(LITTLEFS,"/backend.json",cfginfoJSON.c_str());
    if(isFile(LITTLEFS,"/backend.json")){
        Serial.println("**** backend.json save completed.");
    }

    cfginfoJSON = assetJSON(cfginfo);
    Serial.println("Create asset.json");
    writeFile(LITTLEFS,"/asset.json",cfginfoJSON.c_str());
    if(isFile(LITTLEFS,"/asset.json")){
        Serial.println("**** asset.json save completed.");
    }

    LITTLEFS.end();
    display.print("CC"); //Delete Completed
    delay(1000);
  }    


}
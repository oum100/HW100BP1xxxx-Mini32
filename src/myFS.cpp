#include "myFS.h"
#include <Arduino.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <ArduinoJson.h>
//#include "config.h"
#include <WiFi.h>
//#include "backend.h"


#define FORMAT_LITTLEFS_IF_FAILED true


// LITTLEFS 
boolean initFS(fs::FS &fs){
    if(!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.printf("LITTLEFS Failed to initial.");
        return false;
    }else{
        return true;
    }
}

void Format(fs::FS &fs){
    LITTLEFS.format();
}


boolean isFile(fs::FS &fs, const char * path){
    if(fs.exists(path)){
        return true;
    }
    return false;
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);
    if(!fs.exists(path)){
      Serial.printf("File not found\n");
      return;
    }
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
    return;
}


String readFile1(fs::FS &fs, const char* path){
    String fileresult;
    if(!fs.exists(path)){
        return "File not found";
    }

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        return "Failed to open file";
    }
    while(file.available()){
        fileresult = file.readString();
    }
    file.close();
    return fileresult;
}


void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("  Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("  ailed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("  file written");
    } else {
        Serial.println("  write failed");
    }
    file.close();
}


void writeFile2(fs::FS &fs, const char * path, const char * message){
    if(!fs.exists(path)){
  		if (strchr(path, '/')) {
              Serial.printf("  Create missing folders of: %s\r\n", path);
  			char *pathStr = strdup(path);
  			if (pathStr) {
  				char *ptr = strchr(pathStr, '/');
  				while (ptr) {
  					*ptr = 0;
  					fs.mkdir(pathStr);
  					*ptr = '/';
  					ptr = strchr(ptr+1, '/');
  				}
  			}
  			free(pathStr);
  		}
    }

    Serial.printf("  Writing file to: %s\r\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("  failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("  file written");
    } else {
        Serial.println("  write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\r\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("- file renamed");
    } else {
        Serial.println("- rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}





void readCFG(Config &cfg,String cfgdata){
    DynamicJsonDocument doc(1024);

    //Step1 parse cfgdata
    //save to config config structure

    DeserializationError error = deserializeJson(doc, cfgdata);
    if (error) {
        Serial.print(F("[readCFG]->deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    cfg.header = doc["header"];

    cfg.asset.assetid = doc["asset"]["assetid"];
    cfg.asset.mac = doc["asset"]["mac"];
    cfg.asset.model = doc["asset"]["model"];
    cfg.asset.firmware = doc["asset"]["firmware"];
    cfg.asset.user = doc["asset"]["user"];
    cfg.asset.pass = doc["asset"]["pass"];

    cfg.backend.apihost = doc["backend"]["apihost"];
    cfg.backend.apikey = doc["backend"]["apikey"];
    cfg.backend.mqtthost = doc["backend"]["apikey"];
    cfg.backend.mqttport = doc["backend"]["apikey"];
    cfg.backend.mqttuser = doc["backend"]["apikey"];
    cfg.backend.mqttpass = doc["backend"]["apikey"];

    cfg.payboard.merchantid = doc["payboard"]["merchantid"];
    cfg.payboard.uuid = doc["payboard"]["uuid"];
    cfg.payboard.mqtthost = doc["payboard"]["apikey"];
    cfg.payboard.mqttport = doc["payboard"]["apikey"];
    cfg.payboard.mqttuser = doc["payboard"]["apikey"];
    cfg.payboard.mqttpass = doc["payboard"]["apikey"];

    for(int i=0;i<3;i++){
        cfg.wifissid[i].ssid = doc["wifi"][i]["ssid"];
        cfg.wifissid[i].key = doc["wifi"][i]["key"];
        cfg.product[i].name = doc["product"][i]["name"];
        cfg.product[i].price = doc["product"][i]["price"];
        cfg.product[i].stime = doc["product"][i]["stime"];
    }
}


String saveCFG(Config &cfg){
    DynamicJsonDocument doc(1024);

    doc["header"] = cfg.header;

    doc["asset"]["assetid"] = cfg.asset.assetid;
    doc["asset"]["mac"]=cfg.asset.mac;
    doc["asset"]["model"]=cfg.asset.model;
    doc["asset"]["firmware"]=cfg.asset.firmware;
    doc["asset"]["user"]=cfg.asset.user;
    doc["asset"]["pass"]=cfg.asset.pass;

    doc["backend"]["apihost"]=cfg.backend.apihost;
    doc["backend"]["apikey"]=cfg.backend.apikey;
    doc["backend"]["mqtthost"]=cfg.backend.mqtthost;
    doc["backend"]["mqttport"]=cfg.backend.mqttport;
    doc["backend"]["mqttuser"]=cfg.backend.mqttuser;
    doc["backend"]["mqttpass"]=cfg.backend.mqttpass;

    doc["payboard"]["merchanttid"]=cfg.payboard.merchantid;
    doc["payboard"]["uuid"]=cfg.payboard.uuid;
    doc["payboard"]["mqtthost"]=cfg.payboard.mqtthost;
    doc["payboard"]["mqttport"]=cfg.payboard.mqttport;
    doc["payboard"]["mqttuser"]=cfg.payboard.mqttuser;
    doc["payboard"]["mqttpass"]=cfg.payboard.mqttpass;

    Serial.print("Size :: ");
    int sz = sizeof(cfg);
    Serial.println(sz);
    

    doc["product"][0]["name"]=String(cfg.product[0].name);
    doc["product"][0]["price"]=cfg.product[0].price;
    doc["product"][1]["name"]=String(cfg.product[1].name);
    doc["product"][1]["price"]=cfg.product[1].price;
    doc["product"][2]["name"]=String(cfg.product[2].name);
    doc["product"][2]["price"]=cfg.product[2].price;


    for(int i=0;i<3;i++){

    }

    String jsondoc;
    serializeJson(doc,jsondoc);
    return jsondoc;
}


void initialCFG(Config &cfg){
    cfg.header = "EITC";
    cfg.asset.assetid = "";
    cfg.asset.mac = "";
    cfg.asset.model ="HaierDryer_V123";
    cfg.asset.firmware = "1.0.0";
    cfg.asset.user = "admin";
    cfg.asset.pass = "admin1@#";

    cfg.backend.apihost = "https://cointracker100.herokuapp.com/cointracker/v1.0.0/devices";
    cfg.backend.apikey = "apikey";
    cfg.backend.mqtthost ="flipup.net";
    cfg.backend.mqttuser ="sammy";
    cfg.backend.mqttpass ="password";
    cfg.backend.mqttport = 1883;

    cfg.wifissid[0].ssid = "Home173-AIS";
    cfg.wifissid[0].key = "1100110011";

    //for Payboard
    cfg.payboard.merchantid="1000000104";
    cfg.payboard.uuid="";
    cfg.payboard.mqtthost="mq3.payboard.cc";
    cfg.payboard.mqttport=1883;
    cfg.payboard.mqttuser=cfg.payboard.merchantid;
    cfg.payboard.mqttpass="mki9lvhjpp1xt4jxgjdjqxuhx2ihucgkgz9ledsylsu7terwtsnibhhjzrnsiiig";

}

void payboardCFG(Config &cfg){

}
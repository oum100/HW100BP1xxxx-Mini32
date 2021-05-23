#ifndef myFS_h
#define myFS_h

#include <Arduino.h>
#include "FS.h"
//#include "config.h"

struct Product{
    const char* name;
    float price;
    int stime;
};

struct SSID{
    const char* ssid;
    const char* key;
};

struct Payboard{
    const char* merchantid;
    const char* uuid;
    const char* mqtthost;
    int mqttport;
    const char* mqttuser;
    const char* mqttpass;
};

struct Asset{
    const char* assetid;
    const char* mac;
    const char* model;
    const char* firmware;
    const char* user;
    const char* pass;
    const char* cointype;
};

struct Backend{
    const char* apikey;
    const char* apihost;
    const char* mqtthost;
    int  mqttport;
    const char* mqttuser;
    const char* mqttpass;
};

struct Config{
    const char* header;
    const char* deviceid;
    Asset asset;
    Backend backend;
    SSID wifissid[];
    Payboard payboard;
    Product product[];
};


boolean initFS(fs::FS &fs);
void Format(fs::FS &fs);
boolean isFile(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
String readFile1(fs::FS &fs, const char* path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void writeFile2(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);

String readFile1(fs::FS &fs, const char* path);

void readCFG(Config &cfg,String cfgdata);
String saveCFG(Config &cfg);
void initialCFG(Config &cfg);
void payboardCFG(Config &cfg);

String getdeviceid(void);

#endif
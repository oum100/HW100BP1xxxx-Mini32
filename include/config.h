#ifndef config_h
#define config_h

#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>
#include <Preferences.h>


struct Product{
    String sku;
    float  price;
    int stime;
};

struct SSID{
    String ssid;
    String key;
};

struct Payboard{
    String uuid;
    String merchantid;
    String merchantkey;
    String apihost;
    String apikey;
    String mqtthost;
    int mqttport;
    String mqttuser;
    String mqttpass;
};

struct Asset{
    String assetid;
    String merchantid;
    String orderid;
    String mac;
    String model;
    String firmware;
    String user;
    String pass;
    int coinModule;
    int assettype; //***  0=Washer , 1 = Dryer
};

struct Backend{
    String apikey;
    String apihost;
    String mqtthost;
    int  mqttport;
    String mqttuser;
    String mqttpass;
};

struct Config{
    String header;
    String deviceid;
    Asset asset;
    Backend backend;
    SSID wifissid[3];
    Payboard payboard;
    Product product[3];
};

enum cointype {SINGLE,MULTI};
enum machinetype {WASHER, DRYER};

void initOUTPUT(int inx,byte *pin);
void initINPUT(int inx, byte *pin);
void blinkGPIO(int pin, int btime);


String cfgJSON(Config &cfg);
void cfgJSON2(Config &cfg,String &jsoncfg);
void readCFG(Config &cfg,String &jsoncfg);

void readAssetCFG(Config &cfg,String &jsoncfg);
//void readAssetCFG(Config &cfg,String &jsoncfg);
void readBackendCFG(Config &cfg,String &jsoncfg);
void readPayboardCFG(Config &cfg,String &jsoncfg);

bool saveCFG(Config &cfg,fs::FS &fs);




String  payboardJSON(Config &cfg);
void payboardJSON2(Config &cfg,String &jsconfig);

String  assetJSON(Config &cfg);
void assetJSON2(Config &cfg,String &jsconfig);

String  backendJSON(Config &cfg);
void backendJSON2(Config &cfg,String &jsconfig);

void initCFG(Config &cfg);
void showCFG(Config &cfg);

void getnvPbCFG(Preferences nvcfg, Config &cfg);
void getnvBackend(Preferences nvcfg, Config &cfg);
void getnvAssetCFG(Preferences nvcfg, Config &cfg);
void getnvProduct(Preferences nvcfg, Config &cfg);
void getNVCFG(Preferences nvcfg, Config &cfg);

int loadWIFICFG(Preferences nvcfg,Config &cfg);
void printLocalTime();
void WiFiinfo(void);
#endif
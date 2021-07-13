#include "config.h"
#include "myFS.h"
#include "DataEEprom.h"
#include "hw10010829.h"
#include "payboardAPI.h"
//#include "interrupt.h"
#include <LITTLEFS.h>
#include <Preferences.h>
#include "animation.h"

//Wifi Library
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
//#include <WebServer.h>
//#include <DNSServer.h>
//#include "WiFiManager.h"
#include <HTTPClient.h>

//TimeStamp
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiMulti.h>

//Display Library
#include "SevenSegmentTM1637.h"
#include "SevenSegmentExtended.h"
#include "SevenSegmentFun.h"

//Utility Library
#include <ArduinoJson.h>
//#include "EEPROM.h"
//#include <rBase64.h>
//#include <StringSplitter.h>
//#include "qrcode.h"
#include "Ticker.h"
//#include <ArduinoOTA.h>
#include "Timer.h"
#include "Event.h"
#include "driver/gpio.h"

/***************** OTA Library *****************/
#include "esp32fota.h"
//#include <Update.h>

/***************** MQTT Library *****************/
#include <PubSubClient.h>

/***************** Http Handle Library *****************/
#include "Httphandler.h"

#define DBprintf Serial.printf



    #ifdef HW10014826
        //Output IO
        #define CTRLPOWER   26
        #define POWER_RLY   26

        #define CTRLSTART   18
        #define START_RLY   18

        #define CTRLTEMP    32
        #define TEMP_RLY    32

        #define CTRLRINSE   27
        #define RINSE_RLY   27

        #define CTRLSPEED   15
        #define SPEED_RLY   15

        #define SWL1      22
        #define SWL2      21
        #define SWL3      33
        #define SWL4      14

        #define WIFILED   2

        #define ENCOIN    4    //Coin
        //#define UNLOCK    25    //Coin

        #define BOOKLED   19   

        //INPUT IO
        #define COININ    35    //Coin
        #define DSTATE    5
        #define DLOCK     23
        #define MODESW      39
        #define MACHINEDC     34

        
        //#define LED60M    25

        //Display IO
        #define CLK 17
        #define DIO 16

        #define BUZZ 2

        //Interrutp set
        #define INPUT_SET ((1ULL<<COININ)|(1ULL<<MODESW)|(1ULL<<DSTATE))

        const byte NUMBER_OF_OUTPUT = 12;
        byte OUTPUTPIN[NUMBER_OF_OUTPUT] = {CTRLPOWER,CTRLSTART,CTRLTEMP,CTRLRINSE,CTRLSPEED,SWL1,SWL2,SWL3,SWL4,ENCOIN,BOOKLED,WIFILED};
        const byte NUMBER_OF_INPUT = 2;
        byte INPUTPIN[NUMBER_OF_INPUT] = {MACHINEDC,DLOCK};
    #endif
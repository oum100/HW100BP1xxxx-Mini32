// ******** v1.0.5 ********
// #define FixedMAC "8C:AA:B5:00:00:01"   // Mac addreess for test only
// Production MAC Address Haier HW100BP10829

// RGH-18:  List of device that required Fixed MAC
// #define FixedMAC "8C:AA:B5:85:AB:BC"   //  Mac for INS-004  
// #define FixedMAC "8C:AA:B5:85:A0:AC"   // Mac for WH-010  
// #define FixedMAC "3C:E9:0E:54:C6:BC"   // Mac for WH-012  


// SkyView: List of device that required Fixed MAC
// #define FixedMAC "3C:E9:0E:54:C4:C0" //WM-004 Skyview: VUFRVUCLV2ZANEM
// #define FixedMAC "3C:E9:0E:54:C2:70"  // WM-010 Skyview shop
// #define FixedMAC "3C:E9:0E:54:C3:38"     // WM-012 Skyview shop

// Production MAC Address Haier HW100BP14826
// #define FixedMAC ""   // Mac for 
 




/*------------------------  Enable SHADOWPAYBOARD API --------------------------*/
#define SHADOWPAYBOARD     //update coin payment to shadow-payboard backend


/*-------------- Define machine model. select either one*/
// #define HW100BP10829
// #define HW100BP14826    
  //Important for in HW100BP14826.h define HW100BP14826ALLNEW_101x for RGB_LED pin 2 (move from pin 19)
#define HW150BP14896



#ifdef HW100BP10829
    #include "hw10010829.h"
#elif defined(HW100BP14826)
    #include "HW100BP14826.h"
#elif defined(HW150BP14896)
    #include "HW150BP14896.h"
#endif

/*------------------------  Display Device ----------------------- */
// #define LCD1602
// #define TM1637
// #define HT16K33
// #define USE_BOOKLED
#define USE_RGBLED

// #define DEBUG_INPUT  //delay 500 to see console result.

#include "config.h"
//#include "myFS.h"
//#include "DataEEprom.h"


#include "payboardAPI.h"
//#include "interrupt.h"
///#include <LITTLEFS.h>
#include <Preferences.h>
#include "animation.h"

//Wifi Library
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
//#include <WebSerial.h>
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


#include <time.h>

#include <nvs_flash.h>

// ******** v1.0.10 ********
#ifdef SHADOWPAYBOARD
    #include "shadowPbAPI.h"
#endif

// ******** v1.0.5 ********
#ifdef USE_RGBLED
    #include <FastLED.h>
#endif

#define DBprintf Serial.printf
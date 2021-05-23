#include "config.h"
#include "myFS.h"
#include "DataEEprom.h"
#include "hw10010829.h"
#include <LITTLEFS.h>
#include <Preferences.h>

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

//Utility Library
#include <ArduinoJson.h>
#include "EEPROM.h"
#include <rBase64.h>
#include <StringSplitter.h>
//#include "qrcode.h"
#include "Ticker.h"
#include <ArduinoOTA.h>
#include "Timer.h"
#include "Event.h"
#include "driver/gpio.h"

/***************** OTA Library *****************/
#include <Update.h>

/***************** MQTT Library *****************/
#include <PubSubClient.h>

/***************** Http Handle Library *****************/
#include "Httphandler.h"

#define DBprintf Serial.printf

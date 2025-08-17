#include <Arduino.h>
#include "startup.h"

//V1.0.12
// 1. Change step in ServiceEnd by moving reset value into if.  Better handling timeout but machine not finish.
// 2. Add ctrlReset in HW100BP14826


//******** v1.0.9 ********
// 1. เพิ่ม progDrain , progRinseSpin สำหรับการควบคุมโปรแกรม ซัก แบบบังคับให้เต็มประสิทธิภาพเครื่องในเวลาที่ต้องการ
//    Step 1: บังคับให้ ซักที่ MIX เป็นเวลา 8 นาที แล้วปิดเครื่อง   
//          แก้ใน progStart() -> washProgram MIX  
//         เพิ่ม progTimerID = progTimer.after ให้เรื่อก progDrain หลังจากผ่านไป 8 นาที
//    Step 2: เปิดใหม่ให้ SPIN เป็นเวลา 1 นาที แล้วปิด  (ให้เอาน้ำแฟบออก)
//          washProgram SPIN
//          เพิ่ม progTimerID = progTimer.after ให้เรื่อก progRinseSpin หลังจากผ่านไป 1 นาที
//    Step 3: เปิดใหม่ RINSESPIN แบบ 2 น้ำ ซึ่งจะให้เวลา 26 นาที ปล่อยให้ทำงานจำจบ
//          washProgram RINSESPIN (0,0,6)

//******** v1.0.8 ********
// 1. Rename project name to HW100BP1xxxx
// 2. Add deboncing to interupt 
// 3. Modify pbBackendMqtt and fpBackendMqtt to have skipPriMqtt and skipSecMqtt
// 4. Add new machine model HW150BP14896  (HW150BP14896.h and  HW150BP14896.cpp)


// ******** v1.0.7 ********
// 1. Fixed bug

// ******** v1.0.6 ********
/*
   1. Add support HW100BP14826 
      1.1 BOOK_LED status when machine is available.
   2. Add mqttStateUpdate function to update state to backend server every 1 minute.
   3. Remove defined Fixed Mac into start.h
   4. Add define display device in file startup.h
   5. Add delete NV in setup() function.
   6. Add cointimeout function to check coin timeout.
   
*/


/*---------------------- This is WashPoin RGH 18 version -----------------------*/

// #ifdef HW100BP10829V200
//   byte OUTPUTPIN[] = {AD2,AD1,AD0,CTRLPULSE,ENCOIN,UNLOCK,BUZZ,GREEN_LED};
//   byte INPUTPIN[] = {PROG1,PROG2,DLOCK,COININ};

//   int TOTALINPUT = sizeof(INPUTPIN);
//   int TOTALOUTPUT = sizeof(OUTPUTPIN);
// #endif
//SevenSegmentTM1637 display(CLK,DIO);
//SevenSegmentExtended display(CLK,DIO);
//SevenSegmentFun display(CLK,DIO);



//---------------------------- Function declare here. -------------------------
void recvMsg(uint8_t *data, size_t len);
void IRAM_ATTR gpio_isr_handler(void* arg);
void gpio_task(void *arg);
void init_interrupt();
void mqttCheckConnection();
void mqttStateUpdate();
void pbRegCallback(char* topic, byte* payload, unsigned int length);
void pbRegisMqtt();
void pbCallback(char* topic, byte* payload, unsigned int length);
void pbBackendMqtt();
int checkUUID();
String getUUID();
void showPrice(SevenSegmentTM1637 &disp,int &count,byte max, byte min);
void progStart();
void serviceLeft();

void serviceEnd();
void resetState();
void progDrain();
void progRinseSpin();



#ifdef USE_RGBLED
void toggleRGB(CRGB* leds, int ledIndex, CRGB color1, CRGB color2);
#endif


#ifdef FLIPUPMQTT
void fpCallback(char* topic, byte* payload, unsigned int length);
void fpBackendMqtt();
#endif 

#ifdef USE_RGBLED // in startup.h
  #define NUM_LEDS 1
  CRGB leds[NUM_LEDS];

  // void blinkRGB(const CRGB &color,int btime,int period){
  //   unsigned long tymNow;
  //   for(int i = btime; i<=btime; i++){
  //     tymNow = millis();
  //     int num2nds = (tymNow/1000);
  //     if(num2nds % 2 == 0)fill_solid(leds, NUM_LEDS, color);
  //     if(num2nds % 2 > 0)fill_solid(leds, NUM_LEDS, color);
  //     FastLED.show();
  //   }
  // }

#endif

#ifdef HW100BP10829
  BP10829 washer;
#elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
  BP14826 washer;
#elif defined(HW150BP14896)
  BP14896ES9 washer;
#endif


digitdisplay display(CLK,DIO);

Preferences cfgdata;
Config cfginfo;
//cfg cfinfo;

//String pb_uuid;
// String pb_merchantid;
// String pb_mqtthost;
// String pb_mqttuser;
// String pb_mqttpass;

int price[3]={0,0,0};
int stime[3]= {30,40,50};  // Actual 23, 28, 38      For Skyview
int unit[3]={2,2,2}; //v1.0.9 add sku unit to 2 (1:Second, 2:Minute, 3:Mililiter, 4:liter)
int prodcounter=0;
// int stime[3]= {20,30,40};  // Actual 23, 28, 38      For RGH18

float coinwaittimeout =0.5;

int coin=0;
int coinValue=0;
int pricePerCoin=0;
int paymentby = 0;

// int pb_mqttport=1883;
int drainTime = 2;  // 20 Jan 25  Add for progDrain function
int washTime =10;  // 20 Jan 25 add this parameter to set washing time.
int rinseCount = 2;

int cfgState=0;
int waitFlag = 0;
bool dispflag=0;
int stateflag = 0;
// bool jobStarted = false;
bool machineStart = false;
bool DlockState = true;

u32_t dispCount =0;

String disperr="";
String disptxt="";
bool disponce = 0;

//int bill=0;

Timer serviceTime, waitTime, timeLeft, blinkWiFi, mqttPing, blinkRGB, progTimer, progTimer2;
int8_t serviceTimeID,waitTimeID,timeLeftID, blinkWiFiID, mqttPingID, blinkRGBID, progTimerID, progTimer2ID;

unsigned long stateUpdateTimer=0;
int updeteAvailable =0; //Every 60 minutes or 1 hour
int updateBusy = 0; // Every 1 minute

time_t tnow;

WiFiMulti wifimulti;
WiFiClient espclient;
PubSubClient mqclient(espclient);

String eventName;  // Added 1 Feb 2567

// #define FLIPUPMQTT
#ifdef FLIPUPMQTT
  WiFiClient fpclient;
  PubSubClient mqflipup(fpclient);
#endif



String ntpServer1 = "1.th.pool.ntp.org";
String ntpServer2 = "asia.pool.ntp.org";

String json_config;
//Payboard
String pbRegTopic PROGMEM = "payboard/register";
String pbPubTopic PROGMEM = "payboard/backend/"; // payboard/backend/<merchantid>/<uuid>
String pbSubTopic PROGMEM = "payboard/"; //   payboard/<merchantid>/<uuid>

#ifdef FLIPUPMQTT
String fpPubTopic PROGMEM = "/flipup/backend";
String fpSubTopic PROGMEM = "/flipup/";
#endif

//Flag for active mqtt server
bool skipPriMqtt = false; //v1.0.13  Active payboard Mqtt server
bool skipSecMqtt = false; //v1.0.13  Active flipup Mqtt server

int mqttRetryLimit = 5;
int sericeEndRetryLimit = 30;
int serviceEndCount = 0;

byte keyPress; //*** for keep keypress value.


//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 25200, 60000);

String formattedDate;
unsigned long epochtime;
String dayStamp;
String timeStamp;
int timeRemain=0;


unsigned long twifi = 0;
unsigned long tdiff = 0;
int wifitimeout = 5; //in Minutes


//secureEsp32FOTA esp32OTA("HW100BP10829", "1.0.0");

//AsyncWebServer server(80);

#ifdef USE_RGBLED
void toggleRGB(CRGB* leds, int ledIndex, CRGB color1, CRGB color2) {
    if (leds[ledIndex] == color2) {
      leds[ledIndex] = color1;  // Turn specified LED on
      // Serial.println("ToogleRGB Set Color1");
    } else {
      leds[ledIndex] = color2;  // Turn specified LED off
      // Serial.println("ToogleRGB Set Color2");
    }
    FastLED.show();  // Update the LED display
    delay(500);
}
#endif



//********************************* Webserial Callback function **********************************
void recvMsg(uint8_t *data, size_t len){
  //WebSerial.println("Received Data...");
  String msg = "";
  for(int i=0; i < len; i++){
    msg += char(data[i]);
  }
  //WebSerial.println(msg);
}

gpio_config_t io_config;
xQueueHandle gpio_evt_queue = NULL;


//Update  25 Jul 2567  add coinCount, lastDebounceTime, debounceDelay for debouncing 
volatile int coinCount = 0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void IRAM_ATTR gpio_isr_handler(void* arg)
{

  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    long gpio_num = (long) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
    lastDebounceTime = currentTime;
  }

  /* before 1.0.8 
  long gpio_num = (long) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
  */


}

void IRAM_ATTR coinISR() {
  if((cfgState >= 3) && (cfgState <= 5)){
    unsigned long currentTime = millis();
    if ((currentTime - lastDebounceTime) > debounceDelay) {
      coinValue = coinValue + pricePerCoin;
      Serial.printf("coinISR:->CoinValue: %d\n", coinValue);
      lastDebounceTime = currentTime;
    }
  }
}

void IRAM_ATTR dlockISR(){
  if(cfgState == 5){

  }
}


void gpio_task(void *arg){
    gpio_num_t io_num;  

    for(;;){
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {  
            Serial.printf("\nGPIO[%d] intr, val: %d \n", io_num, gpio_get_level(io_num));
        } 

        switch (io_num){

            #ifdef COININ
            case COININ:
                if(gpio_get_level(io_num) == 0){
                  if((cfgState >= 3) && (cfgState <= 5)){
                    paymentby = 1;  // Set 1 = paymentby COIN
                    coinValue = coinValue + pricePerCoin;
                    Serial.printf("CoinValue: %d\n", coinValue);
                  }
                }  
                break;
            #endif
            
            #ifdef BILLIN
            // case BILLIN:
            //     (gpio_get_level(io_num) == 0)?bill++:bill=bill; 
            //     break;
            #endif

            #ifdef DSTATE
            case DSTATE:
                if(gpio_get_level(io_num) == 0){
                  Serial.printf("[intr]->Door Open\n");
                }else{
                  Serial.printf("[intr]->Door Close\n");
                }
                break;
            #endif

            #ifdef MODESW
            case MODESW:
              break;
            #endif

            case DLOCK:
              if(gpio_get_level(io_num) == 0){
                DlockState = 0; 
                Serial.printf("DLock Stage: UnLock");
              }else{
                DlockState = 1; 
                Serial.printf("DLock Stage: Lock");
              }
              break;
            default:
                break;
        }  
    }
}



void init_interrupt(){

    io_config.intr_type = GPIO_INTR_NEGEDGE;
    io_config.pin_bit_mask = INTERRUPT_SET;
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_up_en = (gpio_pullup_t)1;

    //configure GPIO with the given settings
    gpio_config(&io_config);

    /*********** create a queue to handle gpio event from isr ************/
    gpio_evt_queue = xQueueCreate(10, sizeof(long));

    /*********** Set GPIO handler task ************/
    xTaskCreate(gpio_task, "gpio_task",2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    #ifdef COINENABLE
      gpio_isr_handler_add((gpio_num_t)COININ, gpio_isr_handler, (void*) COININ);
    #endif

    #ifdef HW100BP10829V200
      gpio_isr_handler_add((gpio_num_t)COININ, gpio_isr_handler, (void*) COININ);
    #endif

    #ifdef HAIERDRYER123
        gpio_isr_handler_add((gpio_num_t)BILLIN, gpio_isr_handler, (void*) BILLIN); 
        gpio_isr_handler_add((gpio_num_t)COININ, gpio_isr_handler, (void*) COININ);
        gpio_isr_handler_add((gpio_num_t)MODESW, gpio_isr_handler, (void*) MODESW);
    #endif
}

#ifdef SHADOWPAYBOARD    //define in startup.h file
  shadowPb shadowPbCoinTrans;
#endif

/*   --------------------------------------- WiFi Connection ---------------------------------------------*/

void connectToWiFi(WiFiMulti& wifiMulti, int maxRetries, bool restartOnFailure) {
  int retryCount = 0;

  Serial.print("Connecting to Wi-Fi");

  // Keep trying to connect until the retry limit is reached
  while (wifiMulti.run() != WL_CONNECTED && retryCount < maxRetries) {
    delay(1000);

    #if defined (TM1637)
      display.print("nC"); //WiFi config
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #elif defined(USE_BOOKLED)
      toggleGPIO(BOOK_LED);
    #elif defined(USE_RGBLED)
      toggleRGB(leds,0,CRGB::Magenta, CRGB::Black);
      Serial.println("Point-1");
    #endif

    retryCount++;
    Serial.printf("...%d/%d",retryCount,maxRetries);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    // Serial.print("IP Address: ");
    // Serial.println(WiFi.localIP());
    WiFiinfo();
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
    
    if (restartOnFailure) {
      Serial.println("Restarting ESP32...");
      delay(1000); // Wait a second before restarting
      ESP.restart();  // Restart the ESP32
    } else {
      Serial.println("No restart. Continuing...");
    }
  }
}


/*   --------------------------------------- Added 2 Aug 23 ---------------------------------------------*/

void mqttCheckConnection(){
  time_t tnow;
  time(&tnow);
  Serial.println();
  Serial.print(ctime(&tnow));
  Serial.printf("  |- Mqtt Check Connnection --> ");
  
  // Serial.println("MQTT Check Connection");
  if(!mqclient.connected()){
    Serial.println("Connection lost. MQTT Reconnect");
    pbBackendMqtt();
  }else{
    Serial.println("Connection alive.");
  }
}

/* ------------------------------------------------------------------------------------------------------*/

void mqttStateUpdate(){
  String jsonmsg;
  time_t tnow;
  DynamicJsonDocument doc(512);
  time(&tnow);

  Serial.printf("%s: Mqtt State Update.",ctime(&tnow));

  doc["response"]="Status";
  doc["time"]=String(ctime(&tnow));
  doc["event"]=eventName;
  doc["merchantid"]=cfginfo.payboard.merchantid;
  doc["uuid"]=cfginfo.payboard.uuid;
  doc["rssi"]=WiFi.RSSI();
  switch(cfgState){
    case 3: // Available (waiting for job)
        doc["state"] = "Available";
        break;
    case 4: // Available (waiting for job)
        doc["state"] = "Booked"; // Coin inserted
        break;
    case 5: // Available (waiting for job)
        doc["state"] = "Busy"; // On Service
        break;
    case 10:
        doc["state"] = "Offline";
        break;
  }
  //doc["state"]=cfgState;
  doc["firmware"]=cfginfo.asset.firmware;
  doc["timeRemain"] = timeRemain;

  serializeJson(doc,jsonmsg);
  Serial.println();
  Serial.print("mqttStateUpdate: "); Serial.println(pbPubTopic);
  Serial.print("Jsonmsg: ");Serial.println(jsonmsg);

  if(!mqclient.connected()){
    pbBackendMqtt();
  }
  mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());

  #ifdef FLIPUPMQTT
    if(!mqflipup.connected()){
      fpBackendMqtt();
    }
    mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
  #endif

}




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
      cfginfo.payboard.uuid = doc["uuid"].as<String>();
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

      #if defined (TM1637)
        display.print("UC"); //UUID /config
      #elif defined(HT16K33)
      #elif defined(LCD1602)
      #endif

      delay(200);
      cfgState = 2; // Register 
    }
  }else{
    Serial.printf("Device already registered.\n");
  }
  //doc.clear();
}






void pbRegisMqtt(){
  Serial.printf("Registering to backend server\n");
  mqclient.setServer(cfginfo.payboard.mqtthost.c_str(),cfginfo.payboard.mqttport);
  mqclient.setCallback(pbRegCallback);

  pbSubTopic = "payboard/" + String(cfginfo.payboard.merchantid)+ "/" + String(cfginfo.asset.mac);
  Serial.printf("   Register Mqtt connecting ...");
  while(!mqclient.connect(cfginfo.deviceid.c_str(),cfginfo.payboard.mqttuser.c_str(),cfginfo.payboard.mqttpass.c_str())){
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
  Serial.println("Message arrived pbCallback with topic: ");
  Serial.println(topic);
  Serial.println();

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

  String action = doc["action"];
  Serial.print("\nThis action paramater: ");
  Serial.println(action);

  if(action == "config"){
    #if defined(TM1637)
      display.scrollingText("A-CF",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
      lcdText(0,0,"Action: Config",1000);
    #endif   

    int sz = doc["detail"].size();
    Serial.printf("detail array size: %d\n",sz);

    cfgdata.begin("config",false);
    // cfgdata.putString("merchantid",cfginfo.payboard.merchantid);

    for(int x=0;x<(3-sz);x++){
      Serial.printf(" delete index: %d\n",3-x);
      cfgdata.putString(("sku"+ String(3-x)).c_str(),"");
      cfgdata.putFloat(("price"+String(3-x)).c_str(),0);
      cfgdata.putInt(("stime"+ String(3-x)).c_str(),0);    
      cfgdata.putInt(("unit"+ String(3-x)).c_str(),0);     //v1.0.9  
      price[2-x]=0;
    }  

    for(int i=0;i<sz;i++){
      String sku = doc["detail"][i]["sku"].as<String>();
      price[i] = doc["detail"][i]["price"].as<float>();

      // v1.0.9  add period and unit into product
      stime[i] = doc["detail"][i]["period"].as<int>();
      unit[i] = doc["detail"][i]["unit"].as<int>();
      
      
      if(int(stime[i]) == 0){
        Serial.println("Calculate stime");
        if(price[i] > 10){
          stime[i] = (price[i]/10)*15;
        }else{
          stime[i] = price[i];
        }  
      }

      Serial.print("sku"+ String(i+1) +": ");
      Serial.println(sku);
      Serial.print(" |-price"+String(i+1)+": ");
      Serial.println(price[i]);
      Serial.print(" |-stime"+String(i+1)+": ");
      Serial.println(stime[i]);  
      Serial.print(" |-unit"+String(i+1)+": ");
      Serial.println(unit[i]);

      cfgdata.putString(("sku"+ String(i+1)).c_str(),sku);
      cfgdata.putFloat(("price"+String(i+1)).c_str(),price[i]);
      cfgdata.putInt(("stime"+ String(i+1)).c_str(),stime[i]);
      cfgdata.putInt(("unit"+ String(i+1)).c_str(),unit[i]); // v1.0.9 3 Aug 2023

      cfginfo.product[i].sku = sku;
      cfginfo.product[i].price = price[i];
      cfginfo.product[i].stime = stime[i];
      cfginfo.product[i].unit = unit[i]; // v1.0.9 3 Aug 2023

      Serial.printf("   |- New stime[%d]: %d was saved\n",i,stime[i]);
    }

    cfgdata.end();
    doc.clear();
    doc["response"]="config";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;      
    doc["state"]="Config saved";
    doc["desc"]="config saved";
    
    cfgState = 3;
    delay(200);
  }else if(action =="setmerchant"){ // {"action":"setmerchant","merchantid":"1234567890","merchantkey":"1234567890"}
    #if defined(TM1637)
      display.scrollingText("A-MC",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
      lcdText(0,0,"merchantID",1000);
    #endif  

    String newMerchantid = doc["merchantid"].as<String>();
    String newMerchantkey = doc["merchantkey"].as<String>();

    cfgdata.begin("config",false);
    cfgdata.putString("merchantid",newMerchantid);
    cfgdata.putString("merchantkey",newMerchantkey);
    cfgdata.end();

    doc.clear();
    doc["response"]="setmerchant";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["merchantkey"]=cfginfo.payboard.merchantkey;
    doc["state"]="MerchantID saved";
    doc["desc"]="merchantid and merchantkey saved. System will reboot in 5 seconds";

    serializeJson(doc,jsonmsg);
    //Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
    if(!mqclient.connected()){
      pbBackendMqtt();
    }
    mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());

    #ifdef FLIPUPMQTT
      if(!mqflipup.connected()){
        fpBackendMqtt();
      }
      mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
    #endif    

    ESP.restart();


  }else if(action == "setmqtt"){    // {"action":"setmqtt","mqtthost":"xxx.xxx.xxx.xxx","mqttport":"1883",
                                    // "mqttuser":"xxx","mqttpass":"xxx"}
    #if defined(TM1637)
      display.scrollingText("A-Mqtt",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
      lcdText(0,0,"FixedMac",1000);
    #endif  

    String newMqtthost = doc["mqtthost"].as<String>();
    String newMqttport = doc["mqttport"].as<String>();
    String newMqttuser = doc["mqttuser"].as<String>();
    String newMqttpass = doc["mqttpass"].as<String>();

    cfgdata.begin("config",false);
    cfgdata.putString("mqtthost",newMqtthost);
    cfgdata.putString("mqttport",newMqttport);
    cfgdata.putString("mqttuser",newMqttuser);
    cfgdata.putString("mqttpass",newMqttpass);
    cfgdata.end();

    doc.clear();
    doc["response"]="setmqtt";
    doc["desc"]="mqtt config saved. System will reboot in 5 seconds";
    delay(5000);
    ESP.restart();

  }else if(action == "stateupdate"){ // {"action":"stateupdate","available":"60","busy":"1"}
    #if defined(TM1637)
      display.scrollingText("S-UA",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
      lcdText(0,0,"stateUpdate",1000);
    #endif   

    Serial.printf("Response for action: stateupdate.\n");
    int setAvailable = doc["available"].as<int>();
    int setBusy = doc["busy"].as<int>();

    cfgdata.begin("config",false);
    cfgdata.putInt("updateAvailable",setAvailable);
    cfgdata.putInt("updateBusy",setBusy);
    cfgdata.end();

    updeteAvailable = setAvailable;
    updateBusy = setBusy;
    cfginfo.asset.updateAvailable = updeteAvailable;
    cfginfo.asset.updateBusy = updateBusy;
    
    Serial.printf("New updateAvailable NV-RAM saved is:  %d\n",cfginfo.asset.updateAvailable);
    Serial.printf("New UpdateBusy NV-RAM saved is:  %d\n",cfginfo.asset.updateBusy);

    doc.clear();
    doc["response"]="stateUpdate";
    doc["updateAvailable"]=cfginfo.asset.updateAvailable;
    doc["updateBusy"]=cfginfo.asset.updateBusy;
    doc["state"]="MQTT saved";
    doc["desc"]="stateupdate saved";

    serializeJson(doc,jsonmsg);
    //Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
    if(!mqclient.connected()){
      pbBackendMqtt();
    }
    mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());

    #ifdef FLIPUPMQTT
      if(!mqflipup.connected()){
        fpBackendMqtt();
      }
      mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
    #endif  
  }else if(action == "setmac"){
    
    #if defined (TM1637)
      display.scrollingText("S-Addr",2); // Accepted Config
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif

    Serial.printf("Response for action: setmac.\n");
    String newmac = doc["mac"].as<String>();

    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",4);
    cfgdata.putString("fixedmac",newmac);
    
    if(cfgdata.isKey("uuid")){
      cfgdata.remove("uuid");
    }
    cfginfo.asset.mac = cfgdata.getString("fixedmac");
    cfgdata.end();

    //cfginfo.asset.mac = newmac;
   
    Serial.printf("New Mac Address:  %s\n",cfginfo.asset.mac.c_str());

    doc.clear();
    doc["response"]="setmac";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;      
    doc["mac"]=cfginfo.asset.mac;
    doc["state"]="MAC saved";
    doc["desc"]="New mac address saved";
    serializeJson(doc,jsonmsg);
    //Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
    if(!mqclient.connected()){
      pbBackendMqtt();
    }
    mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());

    #ifdef FLIPUPMQTT
      if(!mqflipup.connected()){
        fpBackendMqtt();
      }
      mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
    #endif
    delay(500); 
    ESP.restart();

  }else if(action == "delmac"){
    
    #if defined (TM1637)
      display.scrollingText("d_Addr",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    Serial.printf("Response for action: delete mac.\n");

    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",6);
    if(cfgdata.isKey("fixedmac")){
      cfgdata.remove("fixedmac");
      cfgdata.remove("uuid");
    }

    cfgdata.end();
    cfginfo.asset.mac = "";
    Serial.printf("Deleted Mac Address:  \"%s\" \n",cfginfo.asset.mac.c_str());
    doc.clear();
    doc["response"]="delmac";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;      
    doc["state"]="MAC Deleted";
    doc["desc"]="Deleted Mac Address.";
    serializeJson(doc,jsonmsg);
    //Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
    if(!mqclient.connected()){
      pbBackendMqtt();
    }
    mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());

    #ifdef FLIPUPMQTT
      if(!mqflipup.connected()){
        fpBackendMqtt();
      }
      mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
    #endif
    delay(500); 

    ESP.restart(); 
  }else if(action == "paid"){
    //Paid and then start service.
    #if defined (TM1637)
      display.scrollingText("PAId",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    Serial.printf("Response for action: paid.\n");

    paymentby = 2; // 1=coin, 2= qr, 3=kiosk , 4 = free
    //int paidprice = doc["price"].as<int>();
    coinValue = doc["price"].as<int>();

    if(coinValue == price[0]){
      waitFlag = 0;
    }else if(coinValue == price[1]){
      waitFlag = 1;
    }else if(coinValue == price[2]){
      waitFlag = 2;
    }

    cfginfo.asset.orderid = doc["orderNo"].as<String>();
    cfgdata.begin("config",false);
    cfgdata.putString("orderid",cfginfo.asset.orderid);
    cfgdata.end();
  
    Serial.printf(" Customer paid for: %d\n",coinValue);
    doc.clear();
    doc["response"] = "paid";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;  
    doc["state"]="Payment received";
    doc["desc"]="Payment has been received for OrderID: " + cfginfo.asset.orderid;
    delay(200);
      
  }else if(action == "countcoin"){
    /*  Not use Jul64
    Serial.printf("Response for action: coincount.\n");
    //{"action":"countcoin","orderNo":"02210526162202176","price":"10.00"}
    // two idea 1st: after insert coin machine work immediately.
    // 2nd: machine not operate if cannot update to backend.

    String resmsg = doc["StatusCode"].as<String>();
    String msg = doc["Message"].as<String>();
    String trans = doc["ResultValues"]["transactionId"].as<String>();

    cfginfo.asset.orderid = trans;


    doc["response"] = "countcoin";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="accepted";
    doc["desc"]="accepted transaction: " + trans;
    */
  }else if(action == "ping"){
    #if defined (TM1637)
      display.scrollingText("Ping",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    Serial.printf("response action PING\n"); 

    doc.clear();
    doc["response"]="ping";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["rssi"]=WiFi.RSSI();
    switch(cfgState){
      case 3: // Available (waiting for job)
          doc["state"] = "Available";
          break;
      case 4: // Available (waiting for job)
          doc["state"] = "Booked"; // Coin inserted
          break;
      case 5: // Available (waiting for job)
          doc["state"] = "Busy"; // On Service
          break;
      case 10:
          doc["state"] = "Offline";
          break;
    }
    //doc["state"]=cfgState;
    doc["firmware"]=cfginfo.asset.firmware;
    doc["timeRemain"] = timeRemain;

  }else if( (action == "reset") || (action=="reboot") || (action=="restart")){
    #if defined (TM1637)
      display.scrollingText("RSt",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
      //set stateflag = 2 flag
    Serial.printf("Accept request action reboot\n");

    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",1);  //To handle stateflag = 5  then reboot too.
    cfgdata.end();

    doc.clear();
    doc["response"]="reboot";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;  
    doc["state"]="Rebooting";
    doc["disc"] = "Rebooting please wait for success.";

    serializeJson(doc,jsonmsg);
    Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
    if(!mqclient.connected()){
      pbBackendMqtt();
    }
    mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());

   #ifdef FLIPUPMQTT
      if(!mqflipup.connected()){
        fpBackendMqtt();
      }
      mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
    #endif

    delay(500);    
    
    ESP.restart();

  }else if(action == "ota"){
    WiFiClientSecure clientForOta;
    digitalWrite(ENCOIN,LOW); // ENCoin off

    #if defined (TM1637)
      display.scrollingText("OtA",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif

    #ifdef HW100BP10829
    secureEsp32FOTA esp32OTA("HW100BP10829", cfginfo.asset.firmware.c_str());
    esp32OTA._descriptionOfFirmwareURL="/firmware/HW100BP10829/firmware.json"; //e.g. /my-fw-versions/firmware.json
    #endif

    #ifdef HW100BP14826
    secureEsp32FOTA esp32OTA("HW100BP14826", cfginfo.asset.firmware.c_str());
    esp32OTA._descriptionOfFirmwareURL="/firmware/HW100BP14826/firmware.json"; //e.g. /my-fw-versions/firmware.json
    #endif

    #ifdef HW100BP14826ALLNEW
    secureEsp32FOTA esp32OTA("HW100BP14826ALLNEW", cfginfo.asset.firmware.c_str());
    esp32OTA._descriptionOfFirmwareURL="/firmware/HW100BP14826ALLNEW/firmware.json"; //e.g. /my-fw-versions/firmware.json
    #endif

    #ifdef HW150BP14896
    secureEsp32FOTA esp32OTA("HW150BP14896", cfginfo.asset.firmware.c_str());
    esp32OTA._descriptionOfFirmwareURL="/firmware/HW150BP14896/firmware.json"; //e.g. /my-fw-versions/firmware.json
    #endif

    esp32OTA._host="www.flipup.net"; //e.g. example.com
    //esp32OTA._certificate=test_root_ca;
    esp32OTA._firwmareVersion = cfginfo.asset.firmware;
    esp32OTA.clientForOta=clientForOta;
  
    bool shouldExecuteFirmwareUpdate=esp32OTA.execHTTPSCheck();
    if(shouldExecuteFirmwareUpdate){
        cfginfo.asset.firmware = esp32OTA._firwmareVersion.c_str();      
        //set stateflag = 2 flag
        cfgdata.begin("config",false);
        cfgdata.putInt("stateflag",2);
        cfgdata.putString("firmware",cfginfo.asset.firmware);
        cfgdata.end();

        doc.clear();
        doc["response"] = "ota";
        doc["merchantid"]=cfginfo.payboard.merchantid;
        doc["uuid"]=cfginfo.payboard.uuid;
        doc["firmware"] = esp32OTA._firwmareVersion;
        doc["state"] = "Upgrading firmware";
        doc["disc"] = "Firmware upgrading then rebooting in few second."; 

        serializeJson(doc,jsonmsg);
        Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
        Serial.print("Ver: "); Serial.println(esp32OTA._firwmareVersion);
        Serial.println("Firmware updating, It's will take few second");

        if(!mqclient.connected()){
          pbBackendMqtt();
        }
        mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());

        #ifdef FLIPUPMQTT
          if(!mqflipup.connected()){
            fpBackendMqtt();
          }
          mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
        #endif


        #if defined (TM1637)
          display.scrollingText("F-otA-",2); // Upgrade Failed
          display.print("UF"); // Upgrade Failed
        #elif defined(HT16K33)
        #elif defined(LCD1602)
        #endif

        esp32OTA.executeOTA_REBOOT();
    }else{
      doc["merchanttid"] = cfginfo.payboard.merchantid;
      doc["uuid"] = cfginfo.payboard.uuid;
      doc["response"] = "ota";
      doc["state"] = "Upgrading Failed";
      doc["desc"] = "Firmware update failed, version may be the same."; 

      serializeJson(doc,jsonmsg);
      Serial.print("Jsonmsg: ");Serial.println(jsonmsg);
      if(!mqclient.connected()){
        pbBackendMqtt();
      }
      mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());

      #ifdef FLIPUPMQTT
        if(!mqflipup.connected()){
          fpBackendMqtt();
        }
        mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
      #endif
    }

    delay(1000);
  }else if(action == "firmware"){  
    display.scrollingText("S-FiE",2); // Accepted Config
    #if defined (TM1637)
      display.scrollingText("S-FiE",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif

    doc.clear();
    doc["response"] = "firmware";
    doc["state"]="Firmware requested";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["firmware"]=cfginfo.asset.firmware;
    doc["desc"]="Current firmware version: " + cfginfo.asset.firmware;
  }else if(action == "setwifi"){
    // {"action":"setwifi","index":"1","ssid":"Home173-AIS","key":"1100110011","reconnect":"1"}


    #if defined (TM1637)
      display.scrollingText("S-SSid",2); 
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif

    String ssid = doc["ssid"].as<String>();
    String key = doc["key"].as<String>();
    int wifireconn = doc["reconnect"].as<int>();
    int index = doc["index"].as<int>();
    
    cfgdata.begin("wificfg",false);
    cfgdata.putString(("ssid"+ (String)(index)).c_str(),ssid);
    cfgdata.putString(("key"+ (String)(index)).c_str(),key);
      Serial.printf("Setting WiFi with following\n");
      Serial.print(("ssid"+ (String)(index))+": ");
      Serial.println(cfgdata.getString(("ssid"+ (String)(index)).c_str()));
      Serial.print(("key"+ (String)(index))+": ");
      Serial.println(cfgdata.getString(("key"+ (String)(index)).c_str()));
    cfgdata.end();

    doc.clear();
    doc["response"] = "setwifi";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    
    wifimulti.addAP(ssid.c_str(),key.c_str());
    if(wifireconn){
      WiFi.disconnect();

      #if defined (TM1637)
        display.print("nF"); // Upgrade Failed
      #elif defined(HT16K33)
      #elif defined(LCD1602)
      #endif

      // digitalWrite(WIFI_LED,LOW);
      wifimulti.run();
      doc["state"]="Reconnected";
      doc["desc"]="setWiFi completed  and reconnected";
    }else{
      doc["state"]="New SSID Saved";
      doc["desc"]="setWiFi conpleted but will effect next boot.";
    }
  
  }else if(action == "coinmodule"){

    #if defined (TM1637)
      display.scrollingText("C-tPE",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    cfginfo.asset.coinModule = (doc["coinmodule"].as<String>() == "single")?SINGLE:MULTI; //  SINGLE=0, MULTI=1
    (cfginfo.asset.coinModule == MULTI)?pricePerCoin=1:pricePerCoin=10;

    doc.clear();
    doc["response"] = "coinmodule";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="CoinModuled changed";

    (cfginfo.asset.coinModule == MULTI)?doc["desc"]="Change coinModule to: MULTI":doc["desc"]="Change coinModule to: SINGLE";

  }else if(action == "coinwaittimeout"){ // // {"action":"coinwaittimeout","coinwaittimeout":3}

    #if defined (TM1637)
      display.scrollingText("S-Tout",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    cfginfo.asset.coinwaittimeout = doc["coinwaittimeout"].as<float>();
    cfgdata.begin("config",false);
    cfgdata.putFloat("coinwaittimeout",cfginfo.asset.coinwaittimeout);
    cfgdata.end();

    doc.clear();
    doc["response"] = "coinwaittimeout";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="CoinWaitTimeout Changed";
    doc["desc"] = "Set CoinWaitTimeout: " + String(cfginfo.asset.coinwaittimeout);

  }else if(action == "orderid"){

    #if defined (TM1637)
      display.scrollingText("A-oId",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
  }else if(action == "assettype"){  
    // {"action":"assettype","assettype":"0"}
  
    #if defined (TM1637)
      display.scrollingText("A-ASSt",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    cfginfo.asset.assettype = doc["assettype"].as<int>();
    cfgdata.begin("config",false);
    cfgdata.putInt("assettype",0);
    cfgdata.end();

    doc.clear();
    doc["response"] = "assettype";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="Order changed"; 
    
  }else if(action == "payboard"){// To set payboard parameter
    #if defined (TM1637)
      display.scrollingText("PbCFg",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    String params = doc["params"];
    bool merchantflag = 0;

    cfgdata.begin("config",false);
    if(params.equals("all")){
        cfginfo.payboard.uuid = doc["uuid"].as<String>();
        cfgdata.putString("uuid",cfginfo.payboard.uuid);
      
        cfginfo.payboard.merchantid = doc["merchantid"].as<String>();;
        cfginfo.payboard.merchantkey = doc["merchantkey"].as<String>();
        cfgdata.putString("merchantid",cfginfo.payboard.merchantid);
        cfgdata.putString("merchantkey",cfginfo.payboard.merchantkey);
      
        cfginfo.payboard.apihost = doc["apihost"].as<String>();
        cfginfo.payboard.apikey = doc["apikey"].as<String>();
        cfgdata.putString("apihost",cfginfo.payboard.apihost);
        cfgdata.putString("apikey",cfginfo.payboard.apikey);
      
        cfginfo.payboard.mqtthost = doc["mqtthost"].as<String>();
        cfginfo.payboard.mqttport = doc["mqttportt"].as<int>();
        cfginfo.payboard.mqttuser = doc["mqttuser"].as<String>();
        cfginfo.payboard.mqttpass = doc["mqttpass"].as<String>();
        cfgdata.putString("mqtthost",cfginfo.payboard.mqtthost);
        cfgdata.putInt("mqttport",cfginfo.payboard.mqttport);
        cfgdata.putString("mqttuser",cfginfo.payboard.mqttuser);
        cfgdata.putString("mqttpass",cfginfo.payboard.mqttpass);
    }else{
    
      if(params.equals("uuid")){
        cfginfo.payboard.uuid = doc["uuid"].as<String>();
        cfgdata.putString("uuid",cfginfo.payboard.uuid);
      }else if(params.equals("merchantid")){
        // {"action":"payboard","params":"merchantid","merchantid":""}
        cfginfo.payboard.merchantid = doc["merchantid"].as<String>();;
        cfgdata.putString("merchantid",cfginfo.payboard.merchantid);
        merchantflag = true;
      }else if(params.equals("merchantkey")){
        // {"action":"payboard","params":"merchantid","merchantkey":""}
        cfginfo.payboard.merchantkey = doc["merchantkey"].as<String>();
        cfgdata.putString("merchantkey",cfginfo.payboard.merchantkey);    
      }else if(params.equals("apihost")){
        cfginfo.payboard.apihost = doc["apihost"].as<String>();
        cfgdata.putString("apihost",cfginfo.payboard.apihost);
      }else if(params.equals("apikey")){
        cfginfo.payboard.apikey = doc["apikey"].as<String>();
        cfgdata.putString("apikey",cfginfo.payboard.apikey);  
      }else if(params.equals("mqtthost")){
        cfginfo.payboard.mqtthost = doc["mqtthost"].as<String>();
        cfginfo.payboard.mqttport = doc["mqttportt"].as<int>();
        cfginfo.payboard.mqttuser = doc["mqttuser"].as<String>();
        cfginfo.payboard.mqttpass = doc["mqttpass"].as<String>();
        cfgdata.putString("mqtthost",cfginfo.payboard.mqtthost);
        cfgdata.putInt("mqttport",cfginfo.payboard.mqttport);
        cfgdata.putString("mqttuser",cfginfo.payboard.mqttuser);
        cfgdata.putString("mqttpass",cfginfo.payboard.mqttpass);
      }
    }
    cfgdata.end();
    doc.clear();
    doc["response"] = "payboard";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="Payboard config changed"; 
    if(merchantflag){
      mqclient.disconnect();
      #ifdef FLIPUPMQTT
        mqflipup.disconnect();
      #endif
    }
    
  }else if(action == "backend"){
    #if defined (TM1637)
      display.scrollingText("A-BECF",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
  }else if(action == "stateflag"){ //stateflag is flag for mark action before reboot  ex 1 is for reboot action, 2 for ota action
    #if defined (TM1637)
      display.scrollingText("S-SFlag",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
  }else if(action == "spin"){
    // bool machineStart = false;
    #ifdef USE_BOOKLED
      digitalWrite(BOOK_LED,LOW);
    #endif

    //******* v1.0.5 **********
    // #ifdef USE_RGBLED  // look at startup.h
    //   leds[0] = CRGB::Red;
    //   FastLED.show();
    // #endif

    #if defined (TM1637)
      display.scrollingText("A-SPIN",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif

    #ifdef HW100BP10829
      //  washer.buttonCtrl(SPIN,1,1000);
    #elif defined(HW100BP14826)
      machineStart = washer.washProgram(washer.SPIN,0,0,0); // 7 mins
    // #elif defined(HW100BP14826ALLNEW_101x)
    //   machineStart = washer.washProgram(washer.SPIN,0,0,0); // 7 mins
    #elif defined(HW150BP14896)
      machineStart = washer.washProgram(washer.SPIN,0,0,0);
    #endif

    if(machineStart){  //unComment this row when production
      //Set cfgstate
      cfgState = 5;
      timeRemain = 8;

      cfgdata.begin("config",false);
        cfgdata.putInt("stateflag",cfgState);
        cfgdata.putInt("timeremain",timeRemain);
      cfgdata.end();

      
      Serial.printf(" Program Time: %d\n",timeRemain);
      serviceTimeID=serviceTime.after((60*1000*timeRemain),serviceEnd);
      timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
      Serial.printf("On service washing for %d minutes\n",timeRemain);

      Serial.printf("stime:%d\n",stime[waitFlag-1]);
      Serial.printf("timeremain:%d\n",timeRemain);
   
      doc.clear();
      doc["response"] = "spin";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="SPIN Started";
      doc["desc"]="Manual run SPIN program";
    }else{
      cfgState = 6;
    }
  }else if(action == "rinsespin"){  
    // bool machineStart = false;
    #ifdef USE_BOOKLED
      digitalWrite(BOOK_LED,LOW);
    #endif

    //******* v1.0.5 **********
    // #ifdef USE_RGBLED  // look at startup.h
    //   leds[0] = CRGB::Red;
    //   FastLED.show();
    // #endif
  
    #if defined (TM1637)
      display.scrollingText("RAS",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif

    #ifdef HW100BP10829
      // buttonCtrl(0,1,1000);  //old version
      // washer.buttonCtrl(0,1,1000);
    #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
      machineStart = washer.washProgram(washer.RINSESPIN,0,0,6); //Rinse about 26 mins
    #elif defined(HW150BP14896)
      machineStart = washer.washProgram(washer.SPORT,1,1,0);
    #endif

    if(machineStart){  //unComment this row when production
      cfgState = 5;

      #ifdef HW100BP10829
        timeRemain = 38;
      #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
        timeRemain = 28;
      #elif defined(HW150BP14896)
        timeRemain = 28;
      #endif

      cfgdata.begin("config",false);
        cfgdata.putInt("stateflag",cfgState);
        cfgdata.putInt("timeremain",timeRemain);
      cfgdata.end();
      
      Serial.printf(" Program Time: %d\n",timeRemain);
      serviceTimeID=serviceTime.after((60*1000*timeRemain),serviceEnd);
      timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
      Serial.printf("On service washing for %d minutes\n",timeRemain);

      Serial.printf("stime:%d\n",stime[waitFlag-1]);
      Serial.printf("timeremain:%d\n",timeRemain);
      Serial.printf("result:%d\n",(stime[waitFlag-1]-timeRemain));

      doc.clear();
      doc["response"] = "rinsespin";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="RinseSpin Started";
      doc["desc"]="Manual run RinseSpin program";    
    }else{
      cfgState = 6;
    }
  }else if(action == "selfclean"){  
    // bool machineStart = false;
    #ifdef USE_BOOKLED
      digitalWrite(BOOK_LED,LOW);
    #endif

    //******* v1.0.5 **********
    // #ifdef USE_RGBLED  // look at startup.h
    //   leds[0] = CRGB::Red;
    //   FastLED.show();
    // #endif
    
    #if defined (TM1637)
      display.scrollingText("CLEAn",2); // Upgrade Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif

    #ifdef HW100BP10829
    // buttonCtrl(0,1,1000);
    #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
      machineStart = washer.washProgram(washer.SELFCLEAN,0,0,0);
    #elif defined(HW150BP14896)
      machineStart = washer.washProgram(washer.CLEAN,0,0,0);
    #endif

    if(machineStart){  //unComment this row when production
      cfgState = 5;
      timeRemain = 60;
      cfgdata.begin("config",false);
        cfgdata.putInt("stateflag",cfgState);
        cfgdata.putInt("timeremain",timeRemain);
      cfgdata.end();
      
      Serial.printf(" Program Time: %d\n",timeRemain);
      serviceTimeID=serviceTime.after((60*1000*timeRemain),serviceEnd);
      timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
      Serial.printf("On service washing for %d minutes\n",timeRemain);

      Serial.printf("stime:%d\n",stime[waitFlag-1]);
      Serial.printf("timeremain:%d\n",timeRemain);
      Serial.printf("result:%d\n",(stime[waitFlag-1]-timeRemain));

      doc.clear();
      doc["response"] = "selfclean";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="SelfClean started";
      doc["desc"]="Manual run SelfClean program";    
    }else{
      cfgState=6;
    } 
  }else if(action == "startpause"){ 
      #ifdef TM1637
        display.scrollingText("Pause",1);
      #elif defined(HT16K33)
      #elif defined(LCD1602)
      #endif

      #ifdef HW100BP10829
        // washer.buttonCtrl(washer.START,1,1000);
      #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
        washer.ctrlStart();
      #elif defined(HW150BP14896)
        washer.ctrlStart();
      #endif

      Serial.println("Manual rquest for START/PAUSE.");

      resetState();
      doc.clear();
      doc["response"] = "startpause";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="StartPause Completed";
      doc["desc"]="Manual StartPause machine.";

  }else if(action == "turnon"){  
  
      #if defined (TM1637)
        display.scrollingText("t-on",2);
      #elif defined(HT16K33)
      #elif defined(LCD1602)
      #endif
      #ifdef USE_BOOKLED
        digitalWrite(BOOK_LED,LOW);
      #endif

      // #ifdef USE_RGBLED
      //   leds[0] = CRGB::Red;
      //   FastLED.show();
      // #endif

      #ifdef HW100BP10829
        washer.buttonCtrl(washer.POWER,1,1000);
      #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
        machineStart =  washer.ctrlPower(POWER_RLY,MACHINEDC,washer.TURNON);
      #elif defined(HW150BP14896)
        washer.ctrlPower(POWER_RLY,MACHINEDC,washer.TURNON);
      #endif

      Serial.println("Request to turn on machine, Turn on completed.");
      if(machineStart){
        cfgState = 5;

        doc.clear();
        doc["response"] = "turnon";
        doc["merchantid"]=cfginfo.payboard.merchantid;
        doc["uuid"]=cfginfo.payboard.uuid;
        doc["state"]="TurnON Completed";
        doc["desc"]="Manual turn on machine.";
      }else{
        resetState();
        doc.clear();
        doc["response"] = "turnon";
        doc["merchantid"]=cfginfo.payboard.merchantid;
        doc["uuid"]=cfginfo.payboard.uuid;
        doc["state"]="TurnON Failed";
        doc["desc"]="Request turn on. But Machine not on.";
      }
      


  }else if(action == "turnoff"){  
      
      #if defined (TM1637)
        display.scrollingText("t-oFF",2);
      #elif defined(HT16K33)
      #elif defined(LCD1602)
      #endif
      #ifdef USE_BOOKLED
        digitalWrite(BOOK_LED,HIGH);
      #endif

      #ifdef USE_RGBLED
        leds[0] = CRGB::Green;
        FastLED.show();
      #endif

      #ifdef HW100BP10829
        washer.buttonCtrl(washer.POWER,1,1000); //POWER Button
      #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
        machineStart = washer.ctrlPower(POWER_RLY,MACHINEDC,washer.TURNOFF);
      #elif defined(HW150BP14896)
        washer.ctrlPower(POWER_RLY,MACHINEDC,washer.TURNOFF);
      #endif
      
      Serial.println("Request to turn off machine, Turn OFF machine.");
      if(machineStart){ // Now machine is off
        resetState();
        doc.clear();
        doc["response"] = "turnoff";
        doc["merchantid"]=cfginfo.payboard.merchantid;
        doc["uuid"]=cfginfo.payboard.uuid;
        doc["state"]="TurnOFF Completed";
        doc["desc"]="Manual turn off completed.";
      }else{ //Request turn off but not off.
        cfgState = 5;
        doc.clear();
        doc["response"] = "turnoff";
        doc["merchantid"]=cfginfo.payboard.merchantid;
        doc["uuid"]=cfginfo.payboard.uuid;
        doc["state"]="TurnOFF Failed";
        doc["desc"]="Manual turn off failed" ;
      }



  }else if(action == "jobcancel"){
  
    #if defined (TM1637)
      display.scrollingText("J-CAn",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif

        //Check timeRemain equal or less than 1. Then allow to cancel job completely.    
    if((stime[waitFlag-1]-timeRemain)<=5){
      //Turn off machine.
      #ifdef HW100BP10829
        washer.buttonCtrl(washer.POWER,1,1000);
      #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)      
        washer.ctrlStart();
        washer.ctrlPower(POWER_RLY,MACHINEDC,washer.TURNOFF);
      #elif defined(HW150BP14896)
        washer.ctrlStart();
        washer.ctrlPower(POWER_RLY,MACHINEDC,washer.TURNOFF);
      #endif

      Serial.printf("stime:%d\n",stime[waitFlag-1]);
      Serial.printf("timeremain:%d\n",timeRemain);
      Serial.printf("result:%d\n",(stime[waitFlag-1]-timeRemain));

      Serial.println("Turn off machine from job cancelation.");

      serviceEnd();
      resetState(); //Add 2 Nov 24
      doc.clear();
      doc["response"] = "jobcancel";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="Service canceled";
      doc["desc"]="Manual cancel job.";
    }else{

      Serial.println("Cannot cancel job, job started more than 3 minutes");

      doc.clear();
      doc["response"] = "jobcancel";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="Ignore cancelation request";
      doc["desc"]="Cannot cancel job, job started more than 1 minutes";
    }
    delay(5000);
  }else if(action == "resetstate"){
      // Reset serviceTimeID
      serviceTime.stop(serviceTimeID); //Stop main timer
      timeLeft.stop(timeLeftID);  //Stop update every minute
      progTimer.stop(progTimerID);

      #ifdef HW100BP10829
        washer.buttonCtrl(washer.POWER,1,1000);
      #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
        washer.ctrlReset();
      #elif defined(HW150BP14896)
        washer.ctrlCancel(); // Make sure ctrlCancel same as 14826
      #endif
      
      Serial.printf("[ResetState]->stime:%d\n",stime[waitFlag-1]);
      Serial.printf("[ResetState]->timeremain:%d\n",timeRemain);
      Serial.printf("[ResetState]->result:%d\n",(stime[waitFlag-1]-timeRemain));
      Serial.println("[ResetState]->, Clear time remain");


      resetState();
      doc.clear();
      doc["response"] = "resetstate";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="Resetstate completed";
      doc["desc"]="Reset state";

  }else if(action == "jobcreate"){
    #if defined (TM1637)
      display.scrollingText("J-Add",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    coinValue = doc["price"].as<int>();
    paymentby = doc["paymentby"].as<int>();  //1 = coin , 2 = qr, 3 = kiosk , 4 = free

    if(coinValue == price[0]){
      waitFlag = 0;
    }else if(coinValue == price[1]){
      waitFlag = 1;
    }else if(coinValue == price[2]){
      waitFlag = 2;
    }

    Serial.printf("CoinValue: %d\n",coinValue);
    Serial.printf("waitFlag: %d\n",waitFlag);

    doc.clear();
    doc["response"] = "jobcreate";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="Job created";
    doc["desc"]="Manual create job.";

  }else if(action == "nvsdelete"){
    String msg;  
    #if defined (TM1637)
      display.scrollingText("n-dEL",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    Serial.printf("NVS size before delete: %d\n",cfgdata.freeEntries());
    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init(); // initialize the NVS partition.
    msg = "NVS size after delete: "+ (String)cfgdata.freeEntries();
    Serial.println(msg);

    doc.clear();
    doc["response"] = "nvsdelete";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="NV-RAM deleted";
    doc["desc"]=msg;    

    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",3);
    cfgdata.end();
    delay(3000);

    ESP.restart();
  }else if(action == "selftest"){
    String msg;
  
    #if defined (TM1637)
      display.scrollingText("tEST",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    msg = "Machine Selftest finished";
    Serial.printf("%s\n",msg.c_str());

    #ifdef HW100BP10829
      washer.selftest(AD0,AD1,AD2,CTRLPULSE);
    #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
      washer.washProgram(washer.QUICK,0,0,0); // 15 mins
    #elif defined(HW150BP14896)
      washer.washProgram(washer.SPORT,0,0,0); // 31 mins
    #endif

    doc.clear();
    doc["response"] = "nvsdelete";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="NV-RAM deleted";
    doc["desc"]=msg;    
  }else if(action == "offline"){
    #if defined (TM1637)
      display.scrollingText("OFFLinE",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    digitalWrite(ENCOIN,LOW); // ENCoin off
    cfgState = 10;// CFGState 10 offline
    timeRemain = 0;
    disponce = 0;
    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",cfgState);
    cfgdata.putInt("timeremain",timeRemain);
    cfgdata.end();   

    doc.clear();
    doc["response"] = "offline";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="Device is set of Offline mode.";
    doc["desc"]="Asset is in OFFLINE mode";    

  }else if(action == "online"){

    #if defined (TM1637)
      display.scrollingText("OnLinE",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    resetState();

    doc.clear();
    doc["response"] = "online";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="Device set to online mode.";
    doc["desc"]="Asset is in ONLINE mode";       
  }else if(action == "setntp"){ // {"action":"setntp","ntpinx":1,"ntpserver":"xxx.xxx.xxx.xxx"}
    
    #if defined (TM1637)
      display.scrollingText("S-ntP",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    int ntpInx = doc["ntpinx"].as<int>();
    String ntpValue = doc["value"].as<String>();

    (ntpInx == 1)?cfginfo.asset.ntpServer1=ntpValue : cfginfo.asset.ntpServer2 = ntpValue;

    doc.clear();
    doc["response"] = "setntp";
    doc["merchantid"]=cfginfo.payboard.merchantid;
    doc["uuid"]=cfginfo.payboard.uuid;
    doc["state"]="Set NTP completed";
    doc["desc"]="New NTP server updated. Please reboot to active it.";  

  }


  serializeJson(doc,jsonmsg);
  Serial.println();
  Serial.print("pbPubTopic: "); Serial.println(pbPubTopic);
  Serial.print("Jsonmsg: ");Serial.println(jsonmsg);


  if(!mqclient.connected()){
    pbBackendMqtt();
  }
  mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());


  #ifdef FLIPUPMQTT
    if(!mqflipup.connected()){
      fpBackendMqtt();
    }
    mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
  #endif

  //delay(500);
}


#ifdef FLIPUPMQTT
  void fpCallback(char* topic, byte* payload, unsigned int length){
    pbCallback(topic,payload,length);
  }

void fpBackendMqtt(){
  if(!mqflipup.connected() && (!skipSecMqtt) ){
    mqflipup.setServer(cfginfo.backend.mqtthost.c_str(),cfginfo.backend.mqttport);
    mqflipup.setCallback(fpCallback);

    int mqttRetry = 1;
    Serial.printf("Secondary Mqtt connecting ...");

    //Second mqtt
    
    while(!mqflipup.connected() && (mqttRetry <= mqttRetryLimit) ){
      mqflipup.connect(cfginfo.deviceid.c_str(),cfginfo.backend.mqttuser.c_str(), cfginfo.backend.mqttpass.c_str());
      Serial.printf(".%d",mqttRetry++);

      if(mqflipup.connected()){
        Serial.printf("Connected to secondary Mqtt server.\n");
        mqflipup.subscribe(fpSubTopic.c_str());
        Serial.printf("    The secondary MQTT subscribe Topic is: %s\n",fpSubTopic.c_str());
      }else{
        if(mqttRetry > mqttRetryLimit){
          skipSecMqtt = true;
        }
        Serial.printf("%d: Attemp connecting secondary MQTT server.\n",mqttRetry);
        delay(3000);
      } 
    }
  }   

  if(mqflipup.connected()){
    Serial.printf("connected\n");
    mqflipup.subscribe(fpSubTopic.c_str());
    Serial.printf("   Subscribe Topic: %s\n",fpSubTopic.c_str());
  }        
}
#endif


void pbBackendMqtt(){
  int mqttRetry = 1;
  // if(mqclient.connected() != 0){
  if(!mqclient.connected() && (!skipPriMqtt) ){
    mqclient.setServer(cfginfo.payboard.mqtthost.c_str(),cfginfo.payboard.mqttport);
    mqclient.setCallback(pbCallback);
    // Add two line below in version 1.0.8
    mqclient.setKeepAlive(30);
    // mqclient.setSocketTimeout(30);

    //v1.0.9 add mqtt buffer size
    mqclient.setBufferSize(512);

    pbSubTopic = "payboard/" + String(cfginfo.payboard.merchantid) + "/" + String(cfginfo.payboard.uuid);
  
    
    Serial.printf("Primary Mqtt connecting ...");

    while(!mqclient.connected() && (mqttRetry <= mqttRetryLimit)){
      mqclient.connect(cfginfo.deviceid.c_str(),cfginfo.payboard.mqttuser.c_str(), cfginfo.payboard.mqttpass.c_str());
      Serial.printf(".");
      Serial.printf("%d",mqttRetry++);

      if(mqclient.connected()){
        Serial.printf("Connected to primary Mqtt server.\n");
        mqclient.subscribe(pbSubTopic.c_str());
        Serial.printf("   The primary MQTT subscribe Topic is: %s\n",pbSubTopic.c_str());
      }else{
        if(mqttRetry > mqttRetryLimit){
          skipPriMqtt = true;
          Serial.println("\nRestarting due to MQTT Connection failed over limitation.");
          ESP.restart();
        }
        Serial.printf("%d: Attemp connecting primary MQTT server.\n",mqttRetry);
        delay(3000);
      }
    }
  }        

  if(mqclient.connected()){
    #ifdef USE_BOOKLED
      digitalWrite(BOOK_LED,HIGH);
    #endif

    //******* v1.0.5 **********
    #ifdef USE_RGBLED  // look at startup.h
      leds[0] = CRGB::Green;
      FastLED.show();
    #endif
    Serial.println();
    Serial.printf("Mqtt connected\n");
    mqclient.subscribe(pbSubTopic.c_str());
    Serial.printf("   Subscribe Topic: %s\n",pbSubTopic.c_str());
  }        
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


void progDrain(){   // Add 21 Nov 24
  //Step 1: Cancel existing wash program
  washer.ctrlCancel();
  // washer.ctrlPower(POWER_RLY,MACHINEDC,washer.TURNOFF);
  //Step 2: Start Drain detrgent water 1 mins
  machineStart = washer.washProgram(washer.SPIN,0,0,0);
  //Step 3: Start full rinse and spin
  if(machineStart){
    progTimer2ID = progTimer2.after(60*1000*drainTime,progRinseSpin);
  }
}

void progRinseSpin(){
  //Step 1: Cancel existing wash program
  washer.ctrlCancel();

  #ifdef HW100BP10829
  #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
    //Step 2: Start RinseSpin program for 26 mins
    int rinsePress=0;
    switch(rinseCount){
      case 0: 
        rinsePress = 4;
        break;
      case 1:
        rinsePress = 5;
        break;
      case 2:
        rinsePress = 6;
        break;
      case 3:
        rinsePress = 0;
        break;
      case 4:
        rinsePress = 2;
        break;
      case 5:
        rinsePress = 3;
        break;                
    }
    machineStart = washer.washProgram(washer.RINSESPIN,0,0,rinsePress);
  #elif defined(HW150BP14896)
    //Step 2: Start RinseSpin program for 26 mins
    
  #endif
}








void progStart(){

  payboard backend;
  String response;
  int rescode;
  // bool machineStart = false;
  
  
  digitalWrite(ENCOIN,LOW); // Disable Coin Module

  #ifdef USE_BOOKLED
    digitalWrite(BOOK_LED,LOW);
  #endif

  //This select program base on coinValue 
  #ifdef HW100BP10829
  switch(waitFlag){
    case 1:
      machineStart = washer.startProg(11); 
      break;
    case 2:
      machineStart = washer.startProg(12);
      break;
    case 3:
      machineStart = washer.startProg(3);
      break;
  }
  #elif defined(HW100BP14826) || defined(HW100BP14826ALLNEW)
    switch(waitFlag){
      case 1: //Price 1 37mins  
        //Run wash program for 10 mins then RinseSpin 26 mins
        // machineStart = washer.washProgram(washer.QUICK,3,0,2);  //v1.0.8
        machineStart = washer.washProgram(washer.MIX,0,0,0);  // v1.0.9  ซักเต็ม 10kg 

        //New progRinseSpin 20Nov24 
        if(machineStart){
          Serial.println("Run Program wash and progDrain");
          drainTime = 3;
          washTime = 8;
          rinseCount = 2; //Set RinseSpin Program 2 Times
          progTimerID = progTimer.after(60*1000*washTime,progDrain); // 37 mins---8 mins + 3min Spin  + 26 RinseSpin total 35mins
        }
        //End progRenseSPIN --------------------

        break;
      case 2: //Price 2 60 mins
        // machineStart = washer.washProgram(washer.DISINFECTION,2,0,0);
        machineStart = washer.washProgram(washer.MIX,2,0,6);  //New MIX 10kg Load (temp 40, speed 1000 ,Rinse 1)
        // machineStart = washer.washProgram(washer.SHIRT,1,0,6);  //Previous use SHIRT (SKYVIEW)
        //New progRinseSpin 20Nov24
        if(machineStart){
          Serial.println("Set timer: progRinseSpin");
          drainTime = 3;
          washTime = 30;
          rinseCount = 2;  // Set RinseSpin Program 2 Times
          progTimerID = progTimer.after(60*1000*washTime,progDrain); // 60 (30 + 4 + 26)

          // progTimerID = progTimer.after(60*1000*25,progRinseSpin); //25 mins + 26 RinseSpin
        }
        //End progRenseSPIN --------------------
        break;
      case 3: //Price 3  90 mins
        machineStart = washer.washProgram(washer.MIX,3,0,6);  //New MIX 10kg LOAD
        // machineStart = washer.washProgram(washer.SHIRT,3,0,6);  //Previous use SHIRT
        break;
    }
  #elif defined(HW150BP14896)
    switch(waitFlag){
      case 1: // Price 1 for 30mins
        machineStart = washer.washProgram(washer.SPORT,2,1,0);  //36Mins
        break;
      case 2: // Price 2 for 60mins
        machineStart = washer.washProgram(washer.MIX,0,0,0); // about 1:07hrs
        break;
      case 3: // Price 3 for 90mins
        machineStart = washer.washProgram(washer.MIX,1,0,0); // about 1:30 hrs
        break;
    }
  #endif

  //if(1){   //Comment this row when production
  if(machineStart){  //unComment this row when production
    Serial.printf("Starting washing for , Paymentby %d\n",paymentby);
    cfgState = 5;
    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",cfgState);
    cfgdata.putInt("timeremain",timeRemain);

    backend.merchantID=cfginfo.payboard.merchantid;
    backend.merchantKEY=cfginfo.payboard.merchantkey;
    backend.appkey=cfginfo.payboard.apikey;

    connectToWiFi(wifimulti,10,true); //For testing
    // tdiff=0;
    // while (!WiFi.isConnected()) { 
    //   if(twifi == 0){
    //     twifi = millis();
    //     Serial.print("TWifi:");
    //     Serial.println(twifi);
    //   }

    //   #if defined (TM1637)
    //      display.print("nF");
    //   #elif defined(HT16K33)
    //   #elif defined(LCD1602)
    //   #endif

    //   #ifdef USE_RGBLED

    //   #endif

    //   //WebSerial.println("[nF]->WiFi Connected");
    //   // digitalWrite(WIFI_LED,LOW);
    //   wifimulti.run();

    //   tdiff = millis() - twifi;
    //   // Serial.print("Tdiff:");
    //   // Serial.println(tdiff);

    //   if( tdiff > 60*1000*wifitimeout ){
    //     Serial.println("Rebooting ESP due wifi not connect");
    //     #if defined (TM1637)
    //       display.print("WrSt");
    //     #elif defined(HT16K33)
    //     #elif defined(LCD1602)
    //     #endif
    //     delay(2000);
    //     ESP.restart();
    //   }
    // }

    switch(paymentby){
      case 1: // by Coin
        backend.uri_countCoin = cfginfo.payboard.apihost + "/v1.0/device/countcoin";
        rescode = backend.coinCounter(cfginfo.payboard.uuid.c_str(),coinValue,response);
        if(rescode == 200){
          Serial.printf("Response trans: %s\n",response.c_str());
          cfgdata.putString("orderid",response);
        }else{
          Serial.printf("Response code: %d\n",rescode);
          Serial.printf("Response trans: %s\n",response.c_str());
        }

        //new Cash transaction to shadowPayboard API
        #ifdef SHADOWPAYBOARD
          shadowPbCoinTrans.uri_countCoin = cfginfo.backend.apihost+"/api/transaction/new";
          if(cfgState == 4){
            rescode = shadowPbCoinTrans.coinCounter(cfginfo.payboard.uuid.c_str(),coinValue,"paid","cash",response);
          }else if(cfgState == 5){

          }

          if(rescode == 200){
            Serial.printf("[ProgStart] Shadow-Payboard response trans: %s\n",response.c_str());
            cfgdata.putString("orderid",response); //Save Order id
          }else{
            Serial.printf("[ProgStart] Shadow-Payboard response code: %d\n",rescode);
            Serial.printf("[ProgStart] Shadow-Payboard response trans: %s\n",response.c_str());
          }
        #endif

        break;
      case 2:// by QR
        cfgdata.putString("orderid",cfginfo.asset.orderid);
        backend.uri_deviceStart = cfginfo.payboard.apihost + "/v1.0/device/start";
        rescode = backend.deviceStart(cfginfo.asset.orderid.c_str(),response);

        if(rescode == 200){
          if(response == "success"){
            Serial.printf("Pro1Start backend undated\n");
          } 
        }else{
          Serial.printf("Rescode: %d\n",rescode);
        }
        break;
      case 3: // by Kiosk
        break;
      case 4: // by Free  no record to backend
        break;
    }
    cfgdata.end();

    //Set timer for wash program ตามโปรแกรมที่ได้เลือกไว้ ในขั้นตอนการหยอดเหรียญ coinValue
    Serial.printf(" Program Time: %d\n",timeRemain);
    Serial.printf("On service washing for %d minutes\n",timeRemain);
    serviceTimeID=serviceTime.after((60*1000*timeRemain),serviceEnd);  //Timer for whole process
    timeLeftID = timeLeft.every(60*1000*1,serviceLeft); //Timer for update every minute
    disptxt = "";
    disptxt = "Job started.";

    mqttStateUpdate();

  }else{
    coin=0;
    coinValue=0;
    cfgState = 6;
    #if defined (TM1637)
        display.print("JF");
    #elif defined(HT16K33)
    #elif defined(LCD1602)
    #endif
    Serial.printf("Failed to start job.\n");
  }
}



void serviceLeft(){
  Serial.println("[serviceLeft]->Executing");
  if(timeRemain > 0){
    // if(timeRemain > 0){
    timeRemain = timeRemain - 1;
    Serial.printf("Service Time remain: %d\n",timeRemain);
    cfgdata.begin("config",false);
    cfgdata.putInt("timeremain",timeRemain);
    cfgdata.end();
    // }
  }else{
    timeRemain = 0;
    Serial.printf("TimeRemain is 0. But Job not finish yet.");
  }
  mqttStateUpdate();
}


void serviceEnd(){
  Serial.println("[ServiceEnd]: Executing");
  payboard backend;
  String response;
  int rescode;

  timeLeft.stop(timeLeftID);
  

  // Check Machine finish job by door unlock
  #ifdef TAWEE
  //This if PROG1 for K.Tawee shop (no doorlock)
  if( digitalRead(PROG1) ){ // digitalRead(PROG1) if get  0 = machine still running.
  #endif

  #ifdef HW100BP10829
  if( digitalRead(PROG1) || digitalRead(DLOCK) ){ //edited 10 Oct 65
  #endif

  #ifdef HW100BP14826
    //During Ending LED on machine turn off before door unlock.
  if(!washer.isDoorLock(DLOCK)){  //Machine unlock door when finish.
  #endif

  #ifdef HW100BP14826ALLNEW
    //During Ending LED on machine turn off before door unlock.
    if(!washer.isDoorLock(DLOCK)){
  #endif

  #ifdef HW150BP14896
  if(!washer.isDoorLock(DLOCK) || (!washer.isMachineON(MACHINEDC))){
  #endif

    coin=0;
    timeRemain = 0;
    coinValue = 0;
    cfgState = 3;
    waitFlag = 0;
    dispflag = 0;


    cfgdata.begin("config",false);
    cfgdata.putInt("stateflag",0);
    cfgdata.putString("orderid","");
    cfgdata.putInt("timeremain",0);
    cfgdata.end();

    backend.merchantID=cfginfo.payboard.merchantid;
    backend.merchantKEY=cfginfo.payboard.merchantkey;
    backend.appkey=cfginfo.payboard.apikey;

    //Prepare notify to backend
    switch(paymentby){
      case 1: // by  Coin
        Serial.printf("Coin Job Finished.\n");
        break;
      case 2: // by QR
        Serial.printf("Sending QR acknoloedge to backend\n");

        backend.uri_deviceStart = cfginfo.payboard.apihost + "/v1.0/device/stop";
        rescode = backend.deviceStart(cfginfo.asset.orderid.c_str(),response);

        if(rescode == 200){
          if(response == "success"){
            Serial.printf("Pro1Start backend undated\n");
          } 
        }else{
          Serial.printf("Rescode: %d\n",rescode);
        }
        break;
      case 3: // by Kiosk
        Serial.printf("Kiosk Job finished.\n");
        break;
      case 4: // Free job by Admin
        Serial.printf("Free Job finished\n");
        break;
    }
  
    //Power off machine
    Serial.printf("Job Finish.  Poweroff machine soon.\n");
    washer.ctrlPower(POWER_RLY,MACHINEDC,washer.TURNOFF);    //Press on power button.
    #ifdef USE_BOOKLED
      digitalWrite(BOOK_LED,HIGH);
    #endif

    //******* v1.0.5 **********
    #ifdef USE_RGBLED  // look at startup.h
      leds[0] = CRGB::Green;
      FastLED.show();
    #endif
    mqttStateUpdate();
  }else{ // Door is lock -- machine not finish job yet then wait for one more minute
    if(serviceEndCount < sericeEndRetryLimit){  //19 Jul 25 add this condition to force serviceEnd after 30 retries.
      Serial.printf("Job still running. wait for one more minute\n");
      disptxt="Wait one more minute for machine finish.";

      timeRemain = 1;
      serviceTimeID=serviceTime.after((60*1000*timeRemain),serviceEnd);
      timeLeftID = timeLeft.every(60*1000*timeRemain,serviceLeft);  // Add on 23 Mar 25   
      serviceEndCount ++; 
    }else{
      resetState();
    }
  }
}

void resetState()
{
  coin = 0;
  timeRemain = 0;
  coinValue = 0;
  paymentby = 0;

  cfgState = 3;
  waitFlag = 0;
  dispflag = 0;

  // pauseflag = false;

  // firstExtPaid = 0;
  // extpaid = 0;
  // extraPay = 0;
  disperr="";
  disptxt="";
  disponce = 0;
  serviceEndCount = 0;

  cfgdata.begin("config",false);
  cfgdata.putInt("stateflag",0);
  cfgdata.putString("orderid","");
  cfgdata.putInt("timeremain",0);
  cfgdata.end();

  #if defined (TM1637)
      display.print("Rt");
  #elif defined(HT16K33)
  #elif defined(LCD1602)
  #endif
  delay(2000);
}


//*********************************** Setup is here. *********************************** 
void setup(){
  Serial.begin(115200);
    //*** initial 7Segment Display

  #if defined (TM1637)
        /* for 4 Digits */
    display.begin(4,0);
    display.setCursor(0,1);

    /* for 2 Digits */
    //display.begin();
    display.setBacklight(30);
    display.print("St"); //Setup
  #elif defined(HT16K33)
  #elif defined(LCD1602)
  #endif

  delay(200);

  #ifdef USE_BOOKLED   //in startup.h
    digitalWrite(BOOK_LED,LOW);
  #endif

  #ifdef USE_RGBLED   //in startup.h
    FastLED.addLeds<SK6812, RGB_LED, RGB>(leds, NUM_LEDS);
    FastLED.setBrightness(80);

    leds[0] = CRGB::Black;
    FastLED.show();
  #endif

  Serial.println("**************** Setup Device ****************");

  #ifdef NVS
  // This for formate EEPROM 
    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init(); // initialize the NVS partition.
    while(true);
  #endif

  // Serial.begin(115200); 
  Serial.println();
  Serial.println("Setting up device...");
  
  #ifdef USE_RGBLED
    //When setup wifi and mqtt not connect yet. Then set RGB to Magenta
    leds[0] = CRGB::Magenta;
    FastLED.show();
  #endif

  //************************* v1.0.6  Delete NV ? *****************************
  Serial.printf("\n\nDelete NV-RAM data?  y/Y to delete or any to continue.\n");
  Serial.printf("Wait for key 15 secs ");
  int wtime = 0;
  while((Serial.available() < 1) && (wtime <30)){
    Serial.print("*");
    wtime++;
    delay(500);
  }

  byte keyin = ' ';  // Accept key y or Y from serial port only
  while(Serial.available() > 0){
    keyin = Serial.read();
      //Serial.println(keyin);
  }

  if((keyin == 121) || (keyin == 89)){

    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init(); // initialize the NVS partition.
    Serial.println(" NV-RAM Deleted");

    #if defined(TM1637)
      display.scrollingText("-dEL-dAtA",2);
    #elif defined(HT16K33)
    #elif defined(LCD1602)
      lcdText(0,0,"NV-RAM Deleted.",1000);    
    #endif

  }
  Serial.printf("\n************** Setting up this device **************\n");

 
 //******* Initial GPIO
  initGPIO(INPUT_SET,OUTPUT_SET);


  //*** Intial Interrupt
  // #define COINISR
  #ifdef COINISR
    pinMode(COININ, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(COININ), coinISR, FALLING);     
  #else
    init_interrupt();
  #endif

  // This while for reading door lock only
  // while(1){
  //   Serial.println(digitalRead(DLOCK));
  //   delay(3000);
  // }
  
  //Get WiFi Configuration
  
  #if defined(TM1637)
    display.print("Cn"); //Config Network
  #elif defined(HT16K33)
  #elif defined(LCD1602)
  #endif
  
  delay(200);
  Serial.printf("WiFi Connecting.....\n");

  WiFi.mode(WIFI_STA);
  
  //wifimulti.addAP("OYO Happylandn guest","12345678");
  //wifimulti.addAP("Home173-AIS","1100110011");
  //wifimulti.addAP("WashcoinRGH18","1100110011");
  wifimulti.addAP("RGH18","1100110011");
  wifimulti.addAP("washpoiint","1100110011");
  wifimulti.addAP("myWiFi","1100110011");
  wifimulti.addAP("alvaWiFi","1100110011");
  // wifimulti.addAP("RGH18","1100110011");
  //wifimulti.addAP("WashCoin","p@ssw0rd456$");

  for(int i=0;i<loadWIFICFG(cfgdata,cfginfo);i++){
    wifimulti.addAP(cfginfo.wifissid[i].ssid.c_str(),cfginfo.wifissid[i].key.c_str());
    Serial.printf("AddAP SSID[%d]: %s, Key[%d]: %s\n",i+1,cfginfo.wifissid[i].ssid.c_str(),i+1,cfginfo.wifissid[i].key.c_str());
  }

 
  //Settting WiFi Connection 16Jul25
  connectToWiFi(wifimulti, 10, true);

  /*  This comment for testing */
  // Serial.printf("Setup->WiFi connecting...\n"); 
  // wtime=0;
  // while ( (WiFi.status() != WL_CONNECTED) && (wtime < 100)) {  
  //   #if defined(TM1637)
  //     display.scrollingText("nF",2);
  //     // display.print("nF");    
  //   #elif defined(HT16K33)
  //   #elif defined(LCD1602)
  //     lcdText(0,0,"WiFi Connecting",300);
  //   #endif  

  //   wifimulti.run();
  //   Serial.print(".");
  //   wtime++;
  //   delay(500);
  // }

  // if(wtime > 100){
  //   Serial.printf("WiFi connecting timeout....Restarting device. \n");
  //   #if defined(TM1637)
  //     display.scrollingText("F-RSt",2);
  //   #elif defined(HT16K33)
  //   #elif defined(LCD1602)
  //     lcdText(0,2,"Reset Device",1000);
  //   #endif      
  //   delay(1000);
  //   ESP.restart();
  // }
  /*  This comment for testing */




  // blinkGPIO(WIFI_LED,400);
  Serial.printf("WiFi Connected...");
  //WebSerial.begin(&server);
  //WebSerial.msgCallback(recvMsg);
  WiFiinfo();
  
  #if defined(TM1637)
    display.print("LC"); //Load Config
    delay(200);
  #elif defined(HT16K33)
  #elif defined(LCD1602)  
  #endif

  Serial.printf("Loading the configuration.\n");
  initCFG(cfginfo); 

  cfginfo.deviceid = getdeviceid();
  cfginfo.asset.assetid = cfginfo.deviceid;

   // To fix mac address 
  #ifdef FixedMAC // at top of this file
    cfginfo.asset.mac = FixedMAC;
  #else
    cfginfo.asset.mac = WiFi.macAddress();  // Using device mac address
  #endif

  // Coin Wait Timeout
  coinwaittimeout = cfginfo.asset.coinwaittimeout;
  Serial.print(" Coin Wait Timeout: ");
  Serial.println(coinwaittimeout);

  Serial.println();
  Serial.println("---------Device Infomation--------");
  Serial.printf("Device ID: %s\n",cfginfo.deviceid.c_str());
  Serial.printf("MacAddress: %s\n",cfginfo.asset.mac.c_str());

 

  //**** Getting config from NV-RAM
  Serial.println("--------- Getting Merchant Information --------");
  cfgdata.begin("config",false);
  if(cfgdata.isKey("merchantid")){
    cfginfo.payboard.merchantid = cfgdata.getString("merchantid");
    Serial.printf("  1. Used Merchantid from NV-RAM: %s\n",cfginfo.payboard.merchantid.c_str());
  }else{
    if(cfginfo.payboard.merchantid.isEmpty()){
      cfginfo.payboard.merchantid = "1000000104";  //this is default mmerchant id
    }
    Serial.printf("  1. Used Mechantid from Initialized: %s\n",cfginfo.payboard.merchantid.c_str());
  }

    //-------------------- Mqtt Information -------------------
  if(cfgdata.isKey("mqtthost")){
    cfginfo.payboard.mqtthost = cfgdata.getString("mqtthost");
    cfginfo.payboard.mqttport = cfgdata.getInt("mqttport");
    cfginfo.payboard.mqttuser = cfgdata.getString("mqttuser");
    cfginfo.payboard.mqttpass = cfgdata.getString("mqttpass");

    Serial.printf("  1.1. Used Mqtt host from NV-RAM: %s\n",cfginfo.payboard.mqtthost.c_str());
  }else{
    Serial.printf("  1.1. Used Mqtt host from Initialized: %s\n",cfginfo.payboard.mqtthost.c_str());
  }


  //------------------- Fixed Mac Address Information -------
  if(cfgdata.isKey("fixedmac")){
    cfginfo.asset.mac = cfgdata.getString("fixedmac");
    Serial.printf("  2.Used NV-RAM Fixed MacAddress: %s\n",cfginfo.asset.mac.c_str());
  }else{
    #ifdef FixedMAC
       Serial.printf("  2.Useed Define Fixed MacAddress: %s\n",cfginfo.asset.mac.c_str());
    #else
       Serial.printf("  2.Used Device MacAddress: %s\n",cfginfo.asset.mac.c_str());
    #endif
  }


  if(cfgdata.isKey("sku1")){
    prodcounter=getnvProduct(cfgdata,cfginfo);
    Serial.printf("  3. Used product information from NV-RAM\n");
  }else{
    Serial.printf("  3. Used product infomation from Initialized\n");
  }


  int sz = sizeof(cfginfo.product)/sizeof(cfginfo.product[0]);
  for(int i=0;i<sz;i++){
    Serial.printf("     Before Stime[%d]: %d\n",i+1,stime[i]);
    price[i] = int(cfginfo.product[i].price);
    stime[i] = cfginfo.product[i].stime;
    Serial.printf("       |_After Stime[%d]: %d\n",i+1,stime[i]);
  }


  //------------------- upadateAvailable and updateBusy Information -------
 
  if(cfgdata.isKey("updateAvailable")){
    cfginfo.asset.updateAvailable = cfgdata.getInt("updateAvailable");
    updeteAvailable = cfginfo.asset.updateAvailable;
    Serial.printf("  4. Used updateAvailable from NV-RAM: %d minutes\n",updeteAvailable );
  }else{
    updeteAvailable = cfginfo.asset.updateAvailable;
    Serial.printf("  5. Used updateAvailable from Initialized: %d minutes\n",updeteAvailable );
  }

  if(cfgdata.isKey("updateBusy")){
    cfginfo.asset.updateBusy = cfgdata.getInt("updateBusy");
    updateBusy = cfginfo.asset.updateBusy;
    Serial.printf("  5.1 Used updateBusy from NV-RAM: %d minutes\n",updateBusy );
  }else{
    updateBusy = cfginfo.asset.updateBusy;
    Serial.printf("  5.1 Used updateBusy from Initialized: %d minutes\n",updateBusy );
  }

  cfgdata.end();

  cfgdata.begin("config",false); //***<<<<<<<<< config preferences   // Don't delete this lin

  //------------------- Coin Module Information -------
  //*** Set price per coin
  if(cfgdata.isKey("coinModule")){
    cfginfo.asset.coinModule = cfgdata.getInt("coinModule");
    Serial.printf("  6. Used coinModule from NV-RAM: %d\n",cfginfo.asset.coinModule );
    Serial.printf("      -- 0: Single Coin Acceptor,  1: Multi Coin Acceptor\n");
  }else{
    // cfginfo.asset.coinModule = 0;
    Serial.printf("  6. Used coinModule from Initialized: %d\n",cfginfo.asset.coinModule );
    Serial.printf("      -- 0: Single Coin Acceptor,  1: Multi Coin Acceptor\n");
  }
  if(cfginfo.asset.coinModule){
    pricePerCoin = 1;  //CoinModule is 1 or enum Multi
  }else{
    pricePerCoin = 10; //CoinModule is 0 or enum SINGLE
  }

  
  // ************  Get UUID ***************
  if(cfgdata.isKey("uuid")){
    Serial.printf("Getting UUID from NV-RAM\n");
    cfginfo.payboard.uuid = cfgdata.getString("uuid","").c_str();
  }else{//Device not register
    Serial.printf("Device not register\n");

    #if defined (TM1637)
      display.print("df"); // Device Failed
      delay(200);
    #elif defined(HT16K33)
    #elif defined(LCD1602)  
    #endif
    Serial.println(cfginfo.asset.mac);
    payboard backend;
    backend.uri_register = cfginfo.payboard.apihost + "/v1.0/device/register";
    backend.merchantID=cfginfo.payboard.merchantid;
    backend.merchantKEY=cfginfo.payboard.merchantkey;
    backend.appkey=cfginfo.payboard.apikey;

    int rescode = backend.registerDEV(cfginfo.asset.mac.c_str(),cfginfo.payboard.uuid);
    Serial.printf("uuid now: "); Serial.println(cfginfo.payboard.uuid);
    if(rescode == 200){
      cfgdata.putString("uuid",cfginfo.payboard.uuid);
      Serial.printf("Save uuid completed: %s\n",cfginfo.payboard.uuid.c_str());
    }else{
      Serial.printf("Rescode: %d\n",rescode);
    }
  }
  cfgdata.end();
  showCFG(cfginfo);

  // Set mqtt parameter
  pbPubTopic = pbPubTopic  + String(cfginfo.payboard.merchantid) +"/"+ String(cfginfo.payboard.uuid);
  pbSubTopic = pbSubTopic + String(cfginfo.payboard.merchantid) +"/"+ String(cfginfo.payboard.uuid);

  #ifdef FLIPUPMQTT
  fpPubTopic = fpPubTopic + String(cfginfo.asset.merchantid) +"/"+ String(cfginfo.asset.assetid);
  fpSubTopic = fpSubTopic + String(cfginfo.asset.merchantid) +"/"+ String(cfginfo.asset.assetid);
  #endif

  
  //Keep WiFi connection 
  connectToWiFi(wifimulti,10,true); //For testing



  if(WiFi.isConnected()){
    twifi = 0;
    tdiff = 0;
    // blinkGPIO(WIFI_LED,400); 

    //**** Connecting  MQTT
    #if defined(TM1637)
      display.print("HF"); // Host Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)        
    #endif
    delay(200);
    pbBackendMqtt();
    
    #ifdef FLIPUPMQTT
    fpBackendMqtt();
    #endif

  
    //*** Set NTP
    #if defined(TM1637)
      display.print("tF"); //Time Failed
    #elif defined(HT16K33)
    #elif defined(LCD1602)  
    #endif
    delay(200);
    Serial.printf("\nConnecting to TimeServer --> ");
    configTime(6*3600,3600,cfginfo.asset.ntpServer1.c_str(),cfginfo.asset.ntpServer2.c_str());
    printLocalTime();
    time_t tnow;
    time(&tnow);
    cfgdata.begin("lastboot",false);
    cfgdata.putULong("epochtime",tnow);
    cfgdata.putString("timestamp",ctime(&tnow));
    cfgdata.end();
    DBprintf("Lastest booting: (%ld) -> %s.\n",tnow,ctime(&tnow));
    delay(500);
  }

  //******  Check stateflag 
  #if defined(TM1637)
    display.print("SF"); //StateFlag
    delay(200);
  #elif defined(HT16K33)
  #elif defined(LCD1602)  
  #endif
  
  cfgdata.begin("config",false);
  if(cfgdata.isKey("stateflag")){
    stateflag = cfgdata.getInt("stateflag",0);
    Serial.printf("Stateflag now: %d\n",stateflag);
    String jsonmsg;
    StaticJsonDocument<100> doc;

    switch(stateflag){
      case 1: //Action after reboot
          timeRemain = cfgdata.getInt("timeremain",0);
          #if defined(TM1637)
            display.scrollingText("Fi-RSt",2);
          #elif defined(HT16K33)
          #elif defined(LCD1602)        
          #endif
          Serial.printf("Before reboot timeRemain is : %d\n",stateflag);

          if(timeRemain > 0){
            cfgState = 5;
            stateflag = cfgState;
          }else{
            stateflag = 0;
            cfgState = 3;
          }
          cfgdata.putInt("stateflag",stateflag);

          Serial.printf("After reboot stateflag: %d\n",stateflag);
          Serial.printf("After reboot cfgState: %d\n",cfgState);
          Serial.printf("After reboot TimeRemain: %d\n",timeRemain);

          
          doc["response"]="reboot";
          doc["merchantid"]=cfginfo.payboard.merchantid;
          doc["uuid"]=cfginfo.payboard.uuid;
          doc["state"]="reboot_done";
          doc["desc"]="Reboot after action:Reboot complete";
          serializeJson(doc,jsonmsg);  

          if(!mqclient.connected()){
            pbBackendMqtt();
          }else{
            mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
          }

          #ifdef FLIPUPMQTT
            if(!mqflipup.connected()){
              fpBackendMqtt();
            }else{
              mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
            }
          #endif
        break;
      case 2: // Action after OTA done
          #if defined(TM1637)
          display.scrollingText("Fi-OtA",2);
          #elif defined(HT16K33)
          #elif defined(LCD1602)  
          #endif
          Serial.printf("Before OTA stateflag: %d\n",stateflag);
          doc["response"]="ota";
          doc["merchantid"]=cfginfo.payboard.merchantid;
          doc["uuid"]=cfginfo.payboard.uuid;
          doc["state"]="ota_done";
          doc["desc"]="Reboot after action:OTA complete";
          doc["firmware"]=cfginfo.asset.firmware;
          serializeJson(doc,jsonmsg);  

          if(!mqclient.connected()){
            pbBackendMqtt();
          }else{
            mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
          }

          #ifdef FLIPUPMQTT
            if(!mqflipup.connected()){
              fpBackendMqtt();
            }else{
              mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
            }
          #endif


          cfgdata.putInt("stateflag",0);
          stateflag = 0;
          Serial.printf("After OTA stateflag: %d\n",stateflag);
          cfgState=3;    
        break;
      case 3: //Action after NVSdelete
          #if defined(TM1637)
          display.scrollingText("Fi-n-dEL",2);
          #elif defined(HT16K33)
          #elif defined(LCD1602)  
          #endif
          doc["response"]="nvsdelete";
          doc["merchantid"]=cfginfo.payboard.merchantid;
          doc["uuid"]=cfginfo.payboard.uuid;
          doc["state"]="nvs_done";
          doc["desc"]="Reboot after action:nvs_delete complete";
          doc["firmware"]=cfginfo.asset.firmware;
          serializeJson(doc,jsonmsg);  

          if(!mqclient.connected()){
            pbBackendMqtt();
          }else{
            mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
          }

          #ifdef FLIPUPMQTT
            if(!mqflipup.connected()){
              fpBackendMqtt();
            }else{
              mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
            }
          #endif

          cfgdata.putInt("stateflag",0);
          stateflag = 0;
          Serial.printf("stateflag after: %d\n",stateflag);
          cfgState=3;      
        break;
      case 4: //Action after mac address change
          #if defined(TM1637)
          display.scrollingText("Fi-SEt-Addr",2);
          #elif defined(HT16K33)
          #elif defined(LCD1602)  
          #endif
          doc["response"]="setmac";
          doc["merchantid"]=cfginfo.payboard.merchantid;
          doc["uuid"]=cfginfo.payboard.uuid;
          doc["state"]="macAddress Added";
          doc["desc"]="Reboot after action: setmac completed";
          doc["firmware"]=cfginfo.asset.firmware;
          serializeJson(doc,jsonmsg);  

          if(!mqclient.connected()){
            pbBackendMqtt();
          }else{
            mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
          }

          #ifdef FLIPUPMQTT
            if(!mqflipup.connected()){
              fpBackendMqtt();
            }else{
              mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
            }
          #endif          

          cfgdata.putInt("stateflag",0);
          stateflag = 0;
          Serial.printf("stateflag after: %d\n",stateflag);
          cfgState = 3;
        break;   
      case 5: // Action when found job not yet finish.
          #if defined(TM1637)
          display.scrollingText("J-Cont",2);
          #elif defined(HT16K33)
          #elif defined(LCD1602)      
          #endif
          Serial.printf("Before resume stateflag: %d\n",stateflag);
          
          delay(200);
          cfgState = stateflag;
          dispflag = 1;
          timeRemain = cfgdata.getInt("timeremain",0);
          Serial.printf("Job not finish. Found this job with [%d] minutes remain.\n",timeRemain);

          //V1.0.3 
          #ifdef TAWEE
          if(!digitalRead(PROG1)){ // Check is machine running by read LED if 0=running  , 1 = off
          #else
          if(digitalRead(DLOCK)){
          #endif
          // end edit V1.0.3
          //if(isHome(PROG1)){ //Machine on service
            Serial.printf("Resuming job for orderID: %s\n",cfginfo.asset.orderid.c_str());
            #if defined(TM1673)
            display.print("ARJ"); // Automatic Resume Job
            #endif
            #ifdef USE_BOOKLED
              digitalWrite(BOOK_LED,LOW);
            #endif

            #ifdef USE_RGBLED
            #endif

            delay(2000);
            serviceTimeID = serviceTime.after(60*1000*timeRemain,serviceEnd);
            timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
            //serviceEnd();
          }else{
            stateflag = 0;
            cfgState = 3;
            dispflag = 0;
            timeRemain = 0;

            Serial.printf("Automatic cancel not job.\n");
            #if defined(TM1637)
            display.print("ACJ"); //Automatic Cancel Job
            delay(2000);
            #elif defined(HT16K33)
            #elif defined(LCD1602)        
            #endif
            cfgdata.putInt("stateflag",stateflag);
            cfgdata.putInt("timeremmain",timeRemain);
          }

          Serial.printf("After resume the stateflag is: %d\n",stateflag);  // if 5 resume job, if 3 clear job.      
        break;
      case 6:
          #if defined(TM1637)
          display.scrollingText("Fi-dEL-Addr",2);
          #elif defined(HT16K33)
          #elif defined(LCD1602)      
          #endif
          doc["response"]="delmac";
          doc["merchantid"]=cfginfo.payboard.merchantid;
          doc["uuid"]=cfginfo.payboard.uuid;
          doc["state"]="macAddress deleted";
          doc["desc"]="Reboot after action: delmac completed";
          doc["firmware"]=cfginfo.asset.firmware;
          serializeJson(doc,jsonmsg);  

          if(!mqclient.connected()){
            pbBackendMqtt();
          }else{
            mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
          }

          #ifdef FLIPUPMQTT
            if(!mqflipup.connected()){
              fpBackendMqtt();
            }else{
              mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
            }
          #endif          

          cfgdata.putInt("stateflag",0);
          stateflag = 0;
          Serial.printf("stateflag after: %d\n",stateflag);
          cfgState = 3;     
        break;
      case 10: // Device Offline
          #if defined(TM1637)
          display.scrollingText("oFFLinE",2);
          display.print("OF");     
          #elif defined(HT16K33)
          #elif defined(LCD1602)      
          #endif 
          cfgState = 10;
          Serial.printf("******************** This Asset is OFFLINE. ******************** \n");
        break;
      default: // Time Remain error clear old job.
          cfgdata.putInt("stateflag",0);
          stateflag = 0;
          cfgState = 3;
          if(timeRemain != 0){ 
            cfgdata.putInt("timeremain",0);
            timeRemain = cfgdata.getInt("timeremain");
            Serial.printf("Reseting TimeRemain: %d\n",timeRemain);
          }      
        break;
    }
/*
    if(stateflag == 1){ // After Action Reboot

      Serial.printf("Before reboot stateflag: %d\n",stateflag);

      doc["response"]="reboot";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="reboot_done";
      doc["desc"]="Reboot after action:Reboot complete";
      serializeJson(doc,jsonmsg);  

      if(!mqclient.connected()){
        pbBackendMqtt();
      }else{
        mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
      }

      #ifdef FLIPUPMQTT
        if(!mqflipup.connected()){
          fpBackendMqtt();
        }else{
          mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
        }
      #endif

      cfgdata.putInt("stateflag",0);
      stateflag = 0;
      Serial.printf("After reboot stateflag: %d\n",stateflag);
      cfgState = 3;
    }else if(stateflag ==2){ // After Action OTA
      Serial.printf("Before OTA stateflag: %d\n",stateflag);
      doc["response"]="ota";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="ota_done";
      doc["desc"]="Reboot after action:OTA complete";
      doc["firmware"]=cfginfo.asset.firmware;
      serializeJson(doc,jsonmsg);  

      if(!mqclient.connected()){
        pbBackendMqtt();
      }else{
        mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
      }

      #ifdef FLIPUPMQTT
        if(!mqflipup.connected()){
          fpBackendMqtt();
        }else{
          mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
        }
      #endif


      cfgdata.putInt("stateflag",0);
      stateflag = 0;
      Serial.printf("After OTA stateflag: %d\n",stateflag);
      cfgState=3;
    }else if(stateflag == 3){ //After nvsdelete
      display.scrollingText("n-dEL",2);
      doc["response"]="nvsdelete";
      doc["merchantid"]=cfginfo.payboard.merchantid;
      doc["uuid"]=cfginfo.payboard.uuid;
      doc["state"]="nvs_done";
      doc["desc"]="Reboot after action:nvs_delete complete";
      doc["firmware"]=cfginfo.asset.firmware;
      serializeJson(doc,jsonmsg);  

      if(!mqclient.connected()){
        pbBackendMqtt();
      }else{
        mqclient.publish(pbPubTopic.c_str(),jsonmsg.c_str());
      }

      #ifdef FLIPUPMQTT
        if(!mqflipup.connected()){
          fpBackendMqtt();
        }else{
          mqflipup.publish(fpPubTopic.c_str(),jsonmsg.c_str());
        }
      #endif

      cfgdata.putInt("stateflag",0);
      stateflag = 0;
      Serial.printf("stateflag after: %d\n",stateflag);
      cfgState=3;      
    }else if(stateflag == 5){ // Last Service not finish but may be power off.
      Serial.printf("Before resume stateflag: %d\n",stateflag);
      display.print("PF"); //Power Outage Event
      delay(200);
      cfgState = stateflag;
      dispflag = 1;
      timeRemain = cfgdata.getInt("timeremain",0);
      Serial.printf("Job not finish. Found this job with [%d] minutes remain.\n",timeRemain);

      //V1.0.3 
      #ifdef TAWEE
      if(!digitalRead(PROG1)){ // Check is machine running by read LED if 0=running  , 1 = off
      #else
      if(!digitalRead(DLOCK)){
      #endif
      // end edit V1.0.3
      //if(isHome(PROG1)){ //Machine on service
        Serial.printf("Resuming job for orderID: %s\n",cfginfo.asset.orderid.c_str());
        display.print("ARJ"); // Automatic Resume Job
        delay(2000);
        serviceTimeID = serviceTime.after(60*1000*timeRemain,serviceEnd);
        timeLeftID = timeLeft.every(60*1000*1,serviceLeft);
        //serviceEnd();
      }else{
        stateflag = 0;
        cfgState = 3;
        dispflag = 0;
        timeRemain = 0;

        Serial.printf("Automatic cancel not job.\n");
        display.print("ACJ"); //Automatic Cancel Job
        delay(2000);
        cfgdata.putInt("stateflag",stateflag);
        cfgdata.putInt("timeremmain",timeRemain);
      }

      Serial.printf("After resume the stateflag is: %d\n",stateflag);  // if 5 resume job, if 3 clear job.
    }else{
      cfgState = 3;
    }
    */
  }else{
    Serial.printf(" It is here \n");
    stateflag = 0;
    cfgdata.putInt("stateflag",stateflag);
    cfgState = 3;
  }
  cfgdata.end();

Serial.printf("cfgState: %d\n",cfgState);
  #if defined(TM1637) 
  display.scrollingText("Func",2); 
  #elif defined(HT16K33)
  #elif defined(LCD1602)    
  #endif

  delay(200);
  Serial.printf("\n\n");
  Serial.printf("******************************************************\n");
  Serial.printf("*      System Ready for service. Firmware: %s      *\n",cfginfo.asset.firmware.c_str());  
  Serial.printf("******************************************************\n");    

  //------------------------------ v 1.0.6 ---------------------------------//
  // Add 2 Aug 23

  mqttPingID = mqttPing.every(60*1000*60, mqttStateUpdate); //Check mqtt every 60 minutes.

} 
//*--------------------------------- End of Setup. ---------------------------------*// 






//*********************************** LOOP is here. *********************************** 
void loop(){

  #ifdef DEBUG_INPUT
    washer.isMachineON(MACHINEDC);
    washer.isDoorLock(DLOCK);
    delay(500);
  #endif


  if(WiFi.isConnected()){
    //******* v1.0.5 **********
    // #ifdef USE_RGBLED  // look at hdv70ed.h
    //   leds[0] = CRGB::Green;
    //   FastLED.show();
    // #endif
    twifi = 0;
    tdiff = 0;

    if(!mqclient.connected() && (cfgState >= 2)){
      #ifdef USE_BOOKLED
        digitalWrite(BOOK_LED,HIGH);
      #endif

      //******* v1.0.5 **********
      #ifdef USE_RGBLED  // look at hdv70ed.h
        // leds[0] = CRGB::Blue;
        // FastLED.show();

        toggleRGB(leds,0,CRGB::Magenta, CRGB::Black);
        Serial.println("Point-2");
      #endif
      pbBackendMqtt();

    }else{
      #ifdef FLIPUPMQTT
        if(!mqflipup.connected() && (cfgState >= 2)){
          fpBackendMqtt();
        }    
      #endif

      switch(cfgState){
        case 1: //*** Not register not Configuration.
            #if defined(TM1637)
            display.print("dC");
            #elif defined(HT16K33)
            #elif defined(LCD1602)      
            #endif
            pbRegisMqtt();
            break;
        case 2: //*** Register but activate from backend.
            #if defined(TM1637)
            display.print("dA");
            #elif defined(HT16K33)
            #elif defined(LCD1602)  
            #endif
            //pbBackendMqtt();
            break;
        case 3: //loop3 //*** get config from File System.
            //Serial.printf(" ------------ This is in cfgState 3 -----------%d\n",coinValue);
            digitalWrite(ENCOIN,HIGH);   // Waiting for Coin
            #ifdef USE_BOOKLED
              digitalWrite(BOOK_LED,HIGH);     
            #endif

            //******* v1.0.5 **********
            #ifdef USE_RGBLED  // look at hdv70ed.h
              leds[0] = CRGB::Green;
              FastLED.show();
            #endif

            // if(washer.isMachineON(MACHINEDC)){
            //   Serial.println("Machine is on without payment. Then turn off machine.");
            //   washer.ctrlPower(POWER_RLY,MACHINEDC,washer.TURNOFF);
            // }

            // Serial.print("MachineDC: ");
            // Serial.println(digitalRead(MACHINEDC));
            if(coinValue > 0){
              cfgState = 4;
            }else{
              String dispPrice, dispStime;
              
              dispPrice = "--";
              dispStime = "--";
              if(price[0]!=0){
                dispPrice =  dispPrice + String(price[0]) + "--";
                dispStime = dispStime + String(stime[0]) + "--";
              }
              if(price[1]!=0){
                dispPrice = dispPrice + String(price[1]) + "--";
                dispStime = dispStime + String(stime[1]) + "--";
              }
              if(price[2]!=0){
                dispPrice = dispPrice + String(price[2]) + "--";
                dispStime = dispStime + String(stime[2]) + "--";
              }
              #if defined(TM1637)
              display.scrollingText(dispPrice.c_str(),1);
              #elif defined(HT16K33)
              #elif defined(LCD1602)      
              #endif

              if(dispCount==0){  //initial dispCounter to be now.
                dispCount=millis();
                Serial.println();
                Serial.println("State 3: Machine Available.");
                Serial.printf("|--Service Price: %s\n",dispPrice.c_str());
                Serial.printf("|--Service Time: %s\n",dispStime.c_str());
                Serial.println();
              }else{
                if( (millis() - dispCount) >= 10000){
                  dispCount = 0;
                }
              }
            }
            break;
        case 4: //loop4 //*** After coinValue > 0  

            if(dispCount == 0){
              dispCount = millis();
              Serial.println();
              Serial.println("State 4: Waiting for payment complete.");
            }else{
              if( (millis() - dispCount) >= 10000){
                dispCount = 0;
              }
            }


            disptxt = "";
            if(coinValue < 10){
              disptxt = "-0" +String(coinValue) +"-";
            }else{
              disptxt = "-" +String(coinValue) +"-";
            }

            #if defined(TM1637)
              display.setBacklight(30);
              display.setColonOn(false);
              display.setCursor(0,0);
              display.print(disptxt);
              display.setCursor(0,1);

            #elif defined(HT16K33)
            #elif defined(LCD1602)     
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("In Progress");
              lcd.SetCursor(0,1);
              lcd.print("Time Remain: " + disptxt);

            #endif

            #ifdef USE_BOOKLED
              // digitalWrite(BOOK_LED,LOW);
              // toggleGPIO(BOOK_LED);
            #endif
            #ifdef USE_RGBLED  // look at startup.h
              leds[0] = CRGB::Blue;
              FastLED.show();
            #endif


            if((coinValue == price[0]) && (waitFlag == 0)){\
              waitFlag = 1;
              //cfgState = 5;
              Serial.println("Wash Program 1");
              Serial.printf("Program 1 :%d\n", coinValue);
              timeRemain = stime[0];
              waitTimeID=waitTime.after(60*1000*coinwaittimeout,progStart);
              //waitTimeID=waitTime.after(60*1000*0.3,prog1start);
              dispCount = 0;
            }else if((coinValue == price[1]) && (waitFlag <= 1)){
              waitTime.stop(waitTimeID);
              waitFlag =2;
              //cfgState = 5;
              Serial.println("Wash Program 2");
              Serial.printf("Program 2 :%d\n", coinValue);
              timeRemain = stime[1];
              waitTimeID=waitTime.after(60*1000*coinwaittimeout,progStart);
              //waitTimeID=waitTime.after(60*1000*0.3,prog2start);
              dispCount = 0;
            }else if((coinValue == price[2]) && (waitFlag <= 2)){
              waitTime.stop(waitTimeID);
              waitFlag= 3;
              //cfgState = 5;
              Serial.println("Wash Program 3");
              Serial.printf("Program 3 :%d\n", coinValue);
              timeRemain = stime[2];
              waitTimeID=waitTime.after(60*1000*coinwaittimeout,progStart);
              //waitTimeID=waitTime.after(60*1000*0.3,prog3start);
              //prog3start();
              dispCount = 0;
            }

            if(waitFlag == (prodcounter-1)){
              digitalWrite(ENCOIN,LOW);
            }
            stateUpdateTimer=0;

            break;
        case 5: //loop5

            if(dispCount == 0){
              dispCount = millis();
              Serial.println();
              Serial.println("State 5: Machine is busy.");
              Serial.printf("State 5: Time Remain: %d\n",timeRemain);
              Serial.printf("ServiceTime %d: %u\n",serviceTimeID, serviceTime.getOperTime(serviceTimeID));
              Serial.printf("waitTime %d: %u\n",waitTimeID, waitTime.getOperTime(waitTimeID));
              Serial.printf("timeLeft %d: %u\n",timeLeftID, timeLeft.getOperTime(timeLeftID));
               Serial.printf("mqttPing %d: %u\n",mqttPingID, mqttPing.getOperTime(mqttPingID));
              Serial.printf("progTimer %d: %u\n",progTimerID, progTimer.getOperTime(progTimerID));
              Serial.printf("progTimer2 %d: %u\n",progTimer2ID, progTimer2.getOperTime(progTimer2ID));

            }else{
              if( (millis() - dispCount) >= 10000){
                dispCount = 0;
              }
            }

            if(!disptxt.isEmpty()){
              Serial.printf("State 5: %d\n",disptxt);
              disptxt = "";
            }

            //If have error in disperr
            if(!disperr.isEmpty()){
              disptxt = "";
              disptxt = disperr +"-";
              Serial.println("Error: "+ disptxt);
              disptxt = "";
            }

  
            #if defined(TM1637)
              disptxt = "";
              if(timeRemain <10){
                disptxt = disptxt+ "T-0" +String(timeRemain);
              }else{
                disptxt = disptxt + "T-" + String(timeRemain);
              }
              display.setBacklight(30);
              display.setColonOn(false);
              display.scrollingText(disptxt.c_str(),1);
              delay(1000);
              display.animation1(display,500,10,1);
              disptxt="";
            #elif defined(HT16K33)
            #elif defined(LCD1602)
              disptxt = "";
              if(timeRemain <10){
                disptxt = disptxt+ "T-0" +String(timeRemain);
              }else{
                disptxt = disptxt + "T-" + String(timeRemain);
              }
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("In Progress");
              lcd.setCursor(0,1);
              lcd.print("Time Remain: " + disptxt);
              disptxt="";
              delay(1000);
            #endif    

            #ifdef USE_BOOKLED
              digitalWrite(BOOK_LED,LOW);
              // toggleGPIO(BOOK_LED);
            #endif


            #ifdef USE_RGBLED  // look at startup.h
              // Serial.println("LED: Blinking Green");
              toggleRGB(leds,0,CRGB::Green, CRGB::Black);

              // if(machineStart){
              //   // leds[0] = CRGB::Red;
              //   // FastLED.show();
              //    toggleRGB(leds,0,CRGB::Green, CRGB::Black);
              // }else{
              //   toggleRGB(leds,0,CRGB::Red, CRGB::Black);
              // }
            #endif
            
        
            break;
        case 6: //loop6  Faliled to start job
              #if defined(TM1637)
                display.scrollingText("JFts",1);
              #elif defined(HT16K33)
              #elif defined(LCD1602)
                lcdText(0,0,"Failed to start job",1000);
              #endif  
              #ifdef USE_BOOKLED
                // digitalWrite(BOOK_LED,LOW);
                // toggleGPIO(BOOK_LED);
              #endif

              #ifdef USE_RGBLED  // look at startup.h
                // Serial.println("LED: Blinking Red");
                toggleRGB(leds,0,CRGB::Red, CRGB::Black);
                // if(!machineStart){
                //   toggleRGB(leds,0,CRGB::Red, CRGB::Black);
                // }         
              #endif

              
              Serial.println("Error: ******** Failed to start job. ********");
              
            break;
        case 10:
            #if defined(TM1637)
              display.scrollingText("--OFF--",1);
            #elif defined(HT16K33)
            #elif defined(LCD1602)
              lcdText(0,0,"Out of service",1000);
            #endif  
            
            if(!disponce){
              Serial.printf("******************** This Asset is OFFLINE. ******************** \n");
              disponce = 1;
            }
            break;
      }
    }
  }else{
    // digitalWrite(WIFI_LED,LOW);
    #ifdef USE_BOOKLED
      digitalWrite(BOOK_LED,LOW);
    #endif

    //******* v1.0.5 **********
    #ifdef USE_RGBLED  // look at hdv70ed.h
      // leds[0] = CRGB::Blue;
      // FastLED.show();

      toggleRGB(leds,0,CRGB::Magenta, CRGB::Black);
      Serial.println("Point-3");
    #endif

    digitalWrite(0,LOW);

    //Always keep WiFi connected.  16Jul25
    connectToWiFi(wifimulti,10,true); //For testing

    // แก้ที่นี้ เข็คจำนวน retry เกินให้ reboot
    /*  This comment for testing */
    // Serial.printf("WiFi Connecting.....\n");
    // while (!WiFi.isConnected()) { 
      // if(twifi == 0){
      //   twifi = millis();
      //   Serial.print("TWifi:");
      //   Serial.println(twifi);
      // }
    //   #if defined (TM1637)
    //     display.print("nF");
    //   #elif defined(HT16K33)
    //   #elif defined(LCD1602)
    //   #endif
    //   //WebSerial.println("[nF]->WiFi Connected");
    //   digitalWrite(WIFI_LED,LOW);
    //   wifimulti.run();

      // tdiff = millis() - twifi;
    //   // Serial.print("Tdiff:");
    //   // Serial.println(tdiff);

      // if( tdiff > 60*1000*wifitimeout ){
      //   Serial.println("Rebooting ESP due wifi not connect");
      //   #if defined (TM1637)
      //     display.print("WrSt");
      //   #elif defined(HT16K33)
      //   #elif defined(LCD1602)
      //   #endif
      //   delay(2000);
      //   ESP.restart();
      // }
    // }
    /*  This comment for testing */  
     
    //delay(1500);
    if(WiFi.isConnected()){
      Serial.println("connected");
      WiFiinfo();

      //******* v1.0.5 **********
      // #ifdef USE_RGBLED  // look at hdv70ed.h
      //   leds[0] = CRGB::Green;
      //   FastLED.show();
      // #endif
      Serial.print("cfgState: ");
      Serial.println(cfgState);  
    }  
  }
  

  serviceTime.update();
  waitTime.update();
  timeLeft.update();
  mqttPing.update();
  progTimer.update();
  progTimer2.update();

  mqclient.loop();
  #ifdef FLIPUPMQTT
   mqflipup.loop();
  #endif


  #ifdef USE_RGBLED

  #endif

}
//*--------------------------------- End of LOOP. ---------------------------------*// 
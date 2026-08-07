#include <Arduino.h>
#include <HTTPClient.h>
#include "shadowPbAPI.h"
#include <ArduinoJson.h>

// shadowPb::shadowPb(){

// }

int shadowPb::coinCounter(const char *uuid, int amount, String status, String paymentBy,String &trans){
    String rjson;
    return coinCounter(uuid,amount,status,paymentBy,trans,rjson);
}

int shadowPb::coinCounter(const char *uuid, int amount, String status, String paymentBy,String &trans,String &resJson){
    HTTPClient http;
    String reqbody;
    String resbody;
    StaticJsonDocument<512> doc;

    Serial.println("************* shadow Payboard coinCounter function **************");
    Serial.print("Auid: "); Serial.println(uuid);
    Serial.printf("Amount: %d\n",amount);
    Serial.printf("Status: %s\n",status);
    Serial.printf("Payment by: %s\n",paymentBy);
    Serial.printf("Uri_countCoin: %s\n", this->uri_countCoin.c_str());
    // Serial.println(this->uri_countCoin);
    

    http.begin(this->uri_countCoin);
    http.addHeader("Content-Type","application/json");

    doc["deviceUuid"]= uuid;
    doc["amount"]=amount;
    doc["status"]=status;
    doc["paymentBy"]=paymentBy;
    serializeJson(doc,reqbody);
    //msgbody = "{\"deviceName\":\"" + String(macaddr) + "\"}";

    Serial.print("reqbody: ");
    Serial.println(reqbody);

    int rescode = http.POST(reqbody);

    if(rescode == 200){
        resbody = http.getString();
        Serial.print("resbody: ");
        Serial.println(resbody);
        http.end();

        doc.clear();
        DeserializationError error = deserializeJson(doc,resbody);
       
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return 500;
        }else{
            if(doc["StatusCode"] == "0000"){
                //response.msg = doc["Message"].as<String>();
                trans = doc["ResultValues"]["transactionId"].as<String>();
                resJson = resbody;
                Serial.println(trans);
                return rescode;
            }
        }
    }else{
        Serial.print("Failed update to shadowPb_countCoin API: ");
        // Serial.print(rescode + "Desc: ");
        Serial.println(http.getString());
        http.end();
        return rescode;
    }

    // HTTP 200 but Shadow Payboard business status is not successful.
    return 500;
}

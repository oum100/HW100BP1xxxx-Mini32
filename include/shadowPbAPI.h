#ifndef shadowPbAPI_h
#define shadowPbAPI_h

#include <Arduino.h>
#include <Preferences.h>

class shadowPb{
    public:
        String host_shadowPb = "https://shadow-payboard.vercel.app";
        String uri_countCoin =  "https://shadow-payboard.vercel.app/api/transaction/new";

        // shadowPb::shadowPb();
        int coinCounter(const char *uuid, int amount, String status, String paymentBy,String &trans);
        int coinCounter(const char *uuid, int amount, String status, String paymentBy,String &trans,String &resJson);
};


#endif
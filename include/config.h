#ifndef config_h
#define config_h

#include <Arduino.h>

    #define HW10010829
    #ifdef HW10010829
        //Output IO
        #define AD0         26
        #define AD1         18
        #define AD2         19
        #define AD3         23
        #define CTRLPULSE   32  

        #define ENCOIN      27
        #define UNLOCK      25 

        #define RED_LED     4
        #define GREEN_LED   22
        #define BLUE_LED    21
        #define WIFI_LED    2   
        #define BUZZ        14

        // #define TXDU1       17 
        // #define RXDU1       16  
        #define CLK         17
        #define DIO         16

        #define SCL         17
        #define SDA         16

        //Input IO
        #define MODESW      39
        #define COININ      35
        #define DLOCK       33 
        #define PROG1       34
        #define PROG2       5

        #define INPUT_SET ((1ULL<<MODESW)|(1ULL<<COININ))


        byte OUTPUTPIN[] = {AD2,AD1,AD0,CTRLPULSE,ENCOIN,UNLOCK,BUZZ,GREEN_LED};
        byte INPUTPIN[] = {PROG1,PROG2,DLOCK};

        int TOTALINPUT = sizeof(INPUTPIN);
        int TOTALOUTPUT = sizeof(OUTPUTPIN);


    #endif

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
        byte outputPin[NUMBER_OF_OUTPUT] = {CTRLPOWER,CTRLSTART,CTRLTEMP,CTRLRINSE,CTRLSPEED,SWL1,SWL2,SWL3,SWL4,ENCOIN,BOOKLED,WIFILED};
        const byte NUMBER_OF_INPUT = 2;
        byte inputPin[NUMBER_OF_INPUT] = {MACHINEDC,DLOCK};
    #endif


void initGPIO(){
    int i=0;
    for(i=0;i<TOTALOUTPUT;i++){
        pinMode(OUTPUTPIN[i],OUTPUT);
        digitalWrite(OUTPUTPIN[i],LOW);
    }

    for(i=0;i<TOTALINPUT;i++){
        pinMode(INPUTPIN[i],INPUT_PULLUP);
    }
}


void blinkGPIO(int pin, int btime){
    if(digitalRead(pin)){         
        digitalWrite(pin,LOW);
        delay(btime);
    }else{
        digitalWrite(pin,HIGH);
        delay(btime);
    }
}


#endif
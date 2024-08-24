#include <Arduino.h>
#include "animation.h"
#include <LiquidCrystal_I2C.h>

#ifndef HW100BP14826_h
#define HW100BP14826_h

#define HW100BP14826
    #ifdef HW100BP14826
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

        #define WIFI_LED   2

        #define ENCOIN    4    //Coin
        //#define UNLOCK    25    //Coin

        #define BOOK_LED   19   
        #define GREEN_LED   19

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
        #define INPUT_SET ((1ULL<<COININ)|(1ULL<<MODESW)|(1ULL<<DSTATE)|(1ULL<<MACHINEDC))

        // #define INTERRUPT_SET ((1ULL<<MODESW))
        #define INTERRUPT_SET ((1ULL<<MODESW)|(1ULL<<COININ))
        #define OUTPUT_SET ( (1ULL<<SWL1) |(1ULL<<SWL2) |(1ULL<<SWL3) |(1ULL<<SWL4) |(1ULL<<CTRLPOWER) |(1ULL<<CTRLSTART) |(1ULL<<CTRLTEMP)|(1ULL<<CTRLRINSE)|(1ULL<<CTRLSPEED)|(1ULL<<ENCOIN)|(1ULL<<BOOK_LED)|(1ULL<<WIFI_LED) )
        
    #endif

    class BP14826 {
        public:
            // enum speed{S400,S600,S800,S1000,S1200,S1400};
            // enum temp{T30,T40,T60,T90};
            // enum rinse{R1,R2,R3,R4,R5};
            enum powerMODE {TURNOFF, TURNON};
            enum prog{
                INTENSE,QUICK,WOOL,SHIRT,
                DUVET,DISINFECTION,MIX,SELFCLEAN,
                RINSESPIN,SPIN,HYGIENIC,CASUAL,
                KIDS,UNDERWARE,BEEDING,DELICATE};

            BP14826(void);
            void pulseGEN(bool logic, int qty, int width, int object);
            void ctrlStart();
            bool ctrlPower(int pwrPin, int machineDC,powerMODE mode);
            void ctrlSpeed(int speed);
            void ctrlTemp(int tmp);
            void ctrlRinse(int rinse);
            void ctrlProg(int prog);
            void selfTest();

            bool washProgram(int prog, int tmp, int speed, int rinse);
            int runProgram(int prog, int tmp, int speed ,int rinse,LiquidCrystal_I2C &lcd,int &err);
            int runProgram(int prog,int tmp, int speed ,int rinse,digitdisplay &disp,int &err);
            // void progStart();
            // void servicEnd();

            bool isMachineON(int pin);// 1 = ON, 0 = Off
            bool isMachineON(int pin,int &err);// 1 = ON, 0 = Off

            bool isDoorLock(int pin);
    };

    // enum speed{S800,S1000,S1200,S1400};
    // enum temp{T30,T40,T60,T90};
    // enum rinse{R1,R2,R3};
    // enum prog{};

    // void pulseGEN(bool logic, int qty, int width, int object);
    // void ctrlStart();
    // void ctrlPower();
    // void ctrlSpeed(int speed);
    // void ctrlTemp(int tmp);
    // void ctrlRinse(int rinse);
    // void ctrlProg(int prog);
    // bool washProgram(int prog, int tmp, int speed, int rinse);
    // void progStart();
    // void servicEnd();

#endif
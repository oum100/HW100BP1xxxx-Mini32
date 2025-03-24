#include <Arduino.h>
#include "animation.h"
#include <LiquidCrystal_I2C.h>

#ifndef HW150BP14986_h
#define HW150BP14986_h

#define HW150BP14986
    #ifdef HW150BP14986
        //Output IO
        #define CTRLPOWER   26
        #define POWER_RLY   26

        #define CTRLSTART   18
        #define START_RLY   18

        #define CTRLTEMP    32
        #define TEMP_RLY    32

        #define CTRLRINSE   27
        #define RINSE_RLY   27

        #define CTRLSPEED   25
        #define SPEED_RLY   25

        #define SWL1      22
        #define SWL2      21
        #define SWA      22
        #define SWB      21
        #define SWLA      22
        #define SWLB      21

        // #define SWL3      33
        // #define SWL4      14

        #define WIFI_LED   2

        #define ENCOIN    4    //Coin
        #define ENCASH    4
        #define UNLOCK    2    //Coin

        #define BOOK_LED   14   
        #define RGB_LED     14
        

        //INPUT IO
        #define COININ    5    //Coin
        #define BILLIN  33      //Bank Note
        #define DLOCK     23
        #define MACHINEDC   19
        #define CLEAN_LED   36
        #define SPORT_LED   39
        #define MIX_LED     35
        #define SPIN_LED    34
   
        //#define LED60M    25

        //Display IO
        #define CLK 17
        #define DIO 16

        #define BUZZ 13

        //Interrutp set
        #define INPUT_SET ((1ULL<<COININ)|(1ULL<<BILLIN)|(1ULL<<DLOCK)|(1ULL<<MACHINEDC)|(1ULL<<CLEAN_LED)|(1ULL<<SPORT_LED)|(1ULL<<MIX_LED)|(1ULL<<SPIN_LED))

        // #define INTERRUPT_SET ((1ULL<<MODESW))
        #define INTERRUPT_SET ((1ULL<<BILLIN)|(1ULL<<COININ))
        #define OUTPUT_SET ( (1ULL<<SWLA) |(1ULL<<SWLB)|(1ULL<<CTRLPOWER) |(1ULL<<CTRLSTART) |(1ULL<<CTRLTEMP)|(1ULL<<CTRLRINSE)|(1ULL<<CTRLSPEED)|(1ULL<<ENCASH)|(1ULL<<BOOK_LED)|(1ULL<<UNLOCK)|(1ULL<<BUZZ) )
        
    #endif

    class BP14896ES9 {
        public:
            // enum speed{S400,S600,S800,S1000,S1200,S1400};
            // enum temp{T30,T40,T60,T90};
            // enum rinse{R1,R2,R3,R4,R5};
            enum powerMODE {TURNOFF, TURNON};
            enum PROGRAM{CLEAN,MIX,SPIN,SPORT};

            BP14896ES9(void);
            void pulseGEN(bool logic, int qty, int width, int object);
            void ctrlStart();
            void ctrlPower(int pwrPin, int machineDC,powerMODE mode);
            void ctrlSpeed(int speed);
            void ctrlTemp(int tmp);
            void ctrlRinse(int rinse);
            void ctrlProg(int prog);
            void ctrlProg(int prog,int direction);
            void selfTest(void);
            void ctrlCancel(void);
            bool washProgram(int prog, int tmp, int speed, int rinse);
        
            // int runProgram(int prog, int tmp, int speed ,int rinse,LiquidCrystal_I2C &lcd,int &err);
            // int runProgram(int prog,int tmp, int speed ,int rinse,digitdisplay &disp,int &err);
            // void progStart();
            // void servicEnd();

            bool isMachineON(int pin);// 1 = ON, 0 = Off
            bool isMachineON(int pin,int &err);// 1 = ON, 0 = Off

            bool isDoorLock(int pin);
            void send_DataEncoder(int APin, int BPin, int pulseCount, unsigned long pulseWidth, boolean Direction);
    };
#endif
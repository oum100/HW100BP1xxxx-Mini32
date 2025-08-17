#include <Arduino.h>
#include "animation.h"
#include <LiquidCrystal_I2C.h>

#ifndef HW100BP14826_h
#define HW100BP14826_h

#define HW100BP14826
//   #define HW100BP14826ALLNEW_101x   //*********** บอร์ดสีม่วง เจ้าปัญหา ให้ uncomment นี้ด้วย

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
    // #define WATER_RLY 2

    #define ENCOIN    4    //Coin
    //#define UNLOCK    25    //Coin

    #ifndef HW100BP14826ALLNEW_101x 
        #define BOOK_LED   19   
        #define RGB_LED   19
        #define WATER_RLY 2
        #define MACHINEDC     34
    #endif

    //INPUT IO
    #define COININ    35    //Coin
    #define DSTATE    5
    #define DLOCK     23
    #define MODESW      39
   

    #define ROTART_A    25
    #define ROTART_B    5
    //#define LED60M    25

    //Display IO
    #define CLK 17
    #define DIO 16

    
    #define BUZZ 2

    //Interrutp set
    #define INPUT_SET ((1ULL<<COININ)|(1ULL<<MODESW)|(1ULL<<MACHINEDC)|(1ULL<<DLOCK))

    // #define INTERRUPT_SET ((1ULL<<MODESW))
    #define INTERRUPT_SET ((1ULL<<COININ)|(1ULL<<MODESW))
    // #define INTERRUPT_SET ((1ULL<<COININ)|(1ULL<<MODESW)|(1ULL<<DLOCK))
    #define OUTPUT_SET ( (1ULL<<SWL1) |(1ULL<<SWL2) |(1ULL<<SWL3) |(1ULL<<SWL4) |(1ULL<<CTRLPOWER) |(1ULL<<CTRLSTART) |(1ULL<<CTRLTEMP)|(1ULL<<CTRLRINSE)|(1ULL<<CTRLSPEED)|(1ULL<<ENCOIN)|(1ULL<<BOOK_LED)|(1ULL<<WIFI_LED) )
    
#endif

#ifdef HW100BP14826ALLNEW_101x
    // //Output IO
    // #define CTRLPOWER   26
    // #define POWER_RLY   26

    // #define CTRLSTART   18
    // #define START_RLY   18

    // #define CTRLTEMP    32
    // #define TEMP_RLY    32

    // #define CTRLRINSE   27
    // #define RINSE_RLY   27

    // #define CTRLSPEED   15
    // #define SPEED_RLY   15

    // #define CTRLWATER   25
    #define WATER_RLY   25

    // #define SWL1      22
    // #define SWL2      21
    // #define SWL3      33
    // #define SWL4      14

    #define WIFI_LED   2
    #define BOOK_LED   2   
    #define RGB_LED   2

    // #define ENCOIN    4    //Coin
   
    // #define BOOK_LED   2  
    // #define RGB_LED    2

    // //INPUT IO
    // #define COININ     5    //Coin
    // #define DLOCK      23
    #define MACHINEDC     19

    // //LDR input
    // #define LDR_SPIN        34
    // #define LDR_MIX         35
    // #define LDR_CLEAN       36
    // #define LDR_SPORT       39

    // #define ROTART_A    22
    // #define ROTART_B    21

    // //Display IO
    // #define CLK 17
    // #define DIO 16

    // #define BUZZ 13

    // //Interrutp set
    // #define INPUT_SET ( (1ULL<<COININ) | (1ULL<<MACHINEDC) | (1ULL<<DLOCK) )

    // // #define INTERRUPT_SET ((1ULL<<MODESW))
    // #define INTERRUPT_SET ((1ULL<<COININ))
    // // #define INTERRUPT_SET ((1ULL<<COININ)|(1ULL<<MODESW)|(1ULL<<DLOCK))
    // #define OUTPUT_SET ( (1ULL<<SWL1) |(1ULL<<SWL2) |(1ULL<<SWL3) |(1ULL<<SWL4) |(1ULL<<CTRLPOWER) |(1ULL<<CTRLSTART) |(1ULL<<CTRLTEMP)|(1ULL<<CTRLRINSE)|(1ULL<<CTRLSPEED)|(1ULL<<ENCOIN)|(1ULL<<BOOK_LED)|(1ULL<<WIFI_LED) ) 
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
            void selfTest(void);
            void ctrlCancel(void);
            void ctrlReset(void);
            bool washProgram(int prog, int tmp, int speed, int rinse);
            // int runProgram(int prog, int tmp, int speed ,int rinse,LiquidCrystal_I2C &lcd,int &err);
            // int runProgram(int prog,int tmp, int speed ,int rinse,digitdisplay &disp,int &err);
            // void progStart();
            // void servicEnd();

            bool isMachineON(int pin);// 1 = ON, 0 = Off
            bool isMachineON(int pin,int &err);// 1 = ON, 0 = Off

            bool isDoorLock(int pin);
    };
#endif
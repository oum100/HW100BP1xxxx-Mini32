#include <Arduino.h>
#include "animation.h"
#include <LiquidCrystal_I2C.h>

//#define TAWEE
#define COTTON 1
#define SELFCLEAN 6
#define QUICKWASH 11

#define  HW100BP10829V200
    #ifdef  HW100BP10829V200
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
        #define BOOK_LED    4

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
        #define DSTATE      33 
        #define PROG1       34
        #define PROG2       5

        #define INPUT_SET ((1ULL<<COININ)|(1ULL<<MODESW)|(1ULL<<DLOCK)|(1ULL<<PROG1)|(1ULL<<PROG2))

        #define INTERRUPT_SET ((1ULL<<MODESW)|(1ULL<<COININ))
        #define OUTPUT_SET ( (1ULL<<AD0) |(1ULL<<AD1) |(1ULL<<AD2) |(1ULL<<AD3) |(1ULL<<CTRLPULSE) |(1ULL<<ENCOIN)|(1ULL<<UNLOCK)|(1ULL<<WIFI_LED) |(1ULL<<GREEN_LED) |(1ULL<<BUZZ) )


        // byte OUTPUTPIN[] = {AD2,AD1,AD0,CTRLPULSE,ENCOIN,UNLOCK,BUZZ,GREEN_LED};
        // byte INPUTPIN[] = {PROG1,PROG2,DLOCK,COININ};

        // int TOTALINPUT = sizeof(INPUTPIN);
        // int TOTALOUTPUT = sizeof(OUTPUTPIN);


        //Below follow BP14826
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
        #define RGB_LED   19

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
        //End BP14826

    #endif



// enum buttons{POWER,UP,DOWN, START,RINSE,TEMP,SPIN,WORK};
// void selftest(int A2,int A1,int A0, int CTRLSW);
// void buttonCtrl( int button, int pulse, int duty);
// void bcdconverter(int value, int *bit3, int *bit2, int *bit1, int *bit0);

// bool isHome(int ldrPin);
// bool startProg(int prognum);


// void srvProgram(int prog,int temp, int water);
// void selfClean(int refposition);

class BP10829 {
    public:
        BP10829(void);
        enum powerMODE {TURNOFF, TURNON};
        enum buttons{POWER,UP,DOWN, START, RINSE,TEMP,SPIN,WORK};

        //original
        void selftest(int A2,int A1,int A0, int CTRLSW);
        void buttonCtrl( int button, int pulse, int duty);
        void bcdconverter(int value, int *bit3, int *bit2, int *bit1, int *bit0);

        bool isHome(int ldrPin);
        bool startProg(int prognum);

        //Below add 26Aug24
        void pulseGEN(bool logic, int qty, int width, int object);
        void ctrlStart();
        bool ctrlPower(int pwrPin, int machineDC,powerMODE mode);
        void ctrlSpeed(int speed);
        void ctrlTemp(int tmp);
        void ctrlRinse(int rinse);
        void ctrlProg(int prog);
        void selfTest(void);
        void ctrlCancel(void);

        bool washProgram(int prog, int tmp, int speed, int rinse);
        int runProgram(int prog, int tmp, int speed ,int rinse,LiquidCrystal_I2C &lcd,int &err);
        int runProgram(int prog,int tmp, int speed ,int rinse,digitdisplay &disp,int &err);
        // void progStart();
        // void servicEnd();

        bool isMachineON(int pin);// 1 = ON, 0 = Off
        bool isMachineON(int pin,int &err);// 1 = ON, 0 = Off

        bool isDoorLock(int pin);
};
#ifndef animation_h
#define animation_h

#include <Arduino.h>
#include <SevenSegmentTM1637.h>


class digitdisplay{ 
    public:
        digitdisplay();
        void animation1(SevenSegmentTM1637 &disp,int dtime,int repeat);
};



#endif
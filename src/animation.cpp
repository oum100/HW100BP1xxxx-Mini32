#include "animation.h"
#include <Arduino.h>
#include <SevenSegmentTM1637.h>
#include <SevenSegmentAsciiMap.h>


byte anima1[]={8,64,1};

//Display pattern
uint8_t RUNBOX1[] = { 0x00, 0b01100011, 0b01011100, 0x00 };
uint8_t RUNBOX2[] = { 0x00, 0b01011100, 0b01100011, 0x00 };

digitdisplay::digitdisplay(){
};

void digitdisplay :: animation1(SevenSegmentTM1637 &disp,int dtime,int repeat){
    for(int round=0;round<repeat;round++){
        for(int i=0;i<3;i++){
            disp.printRaw(anima1[i],0);
            disp.printRaw(anima1[i],1);
            delay(dtime);
        }
    }
}
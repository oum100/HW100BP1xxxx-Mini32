#include <Arduino.h>
#include "hw10010829.h"




//enum buttons{POWER,UP,DOWN, START,RINSE,TEMP,SPIN,WORK};

void bcdconverter(int value, int *bit3, int *bit2, int *bit1, int *bit0){


  switch(value){
      case  0:  *bit3=0; *bit2=0; *bit1=0; *bit0=0; break; // Power
      case  1:  *bit3=0; *bit2=0; *bit1=0; *bit0=1; break; // UP
      case  2:  *bit3=0; *bit2=0; *bit1=1; *bit0=0; break; // Down
      case  3:  *bit3=0; *bit2=0; *bit1=1; *bit0=1; break; // Start
      case  4:  *bit3=0; *bit2=1; *bit1=0; *bit0=0; break; // Rinse
      case  5:  *bit3=0; *bit2=1; *bit1=0; *bit0=1; break; // Temp    
      case  6:  *bit3=0; *bit2=1; *bit1=1; *bit0=0; break; // Spin
      case  7:  *bit3=0; *bit2=1; *bit1=1; *bit0=1; break; // BUSY
      case  8:  *bit3=1; *bit2=0; *bit1=0; *bit0=0; break;
      case  9:  *bit3=1; *bit2=0; *bit1=0; *bit0=1; break;
      case  10:  *bit3=1; *bit2=0; *bit1=1; *bit0=0; break;
      case  11:  *bit3=1; *bit2=0; *bit1=1; *bit0=1; break;
      case  12:  *bit3=1; *bit2=1; *bit1=0; *bit0=0; break;
      case  13:  *bit3=1; *bit2=1; *bit1=0; *bit0=1; break;
      case  14:  *bit3=1; *bit2=1; *bit1=1; *bit0=0; break;      
      case  15:  *bit3=1; *bit2=1; *bit1=1; *bit0=1; break;     
  }
}

void buttonCtrl( int button, int pulse, int duty,int IO0,int IO1,int IO2, int IO3){
    int digit0=0, digit1=0,digit2=0,digit3=0;

    switch(button) {
    case POWER: 
         bcdconverter(POWER,&digit3,&digit2,&digit1,&digit0);
         Serial.println("POWER Switch: OFF");
         break;
    case UP: 
         bcdconverter(UP,&digit3,&digit2,&digit1,&digit0);
         Serial.println("UP Switch: OFF"); 
         break;
    case DOWN: 
         bcdconverter(DOWN,&digit3,&digit2,&digit1,&digit0);
         Serial.println("DOWN Switch: OFF"); 
         break;
     case START: 
         bcdconverter(START,&digit3,&digit2,&digit1,&digit0);
         Serial.println("START Switch: OFF");
         break;
     case RINSE: 
         bcdconverter(RINSE,&digit3,&digit2,&digit1,&digit0);
         Serial.println("RINSE Switch: OFF"); 
         break;
    case TEMP: 
         bcdconverter(TEMP,&digit3,&digit2,&digit1,&digit0);
         Serial.println("TEMP Switch: OFF");
         break;    
    case SPIN: 
         bcdconverter(SPIN,&digit3,&digit2,&digit1,&digit0);
         Serial.println("SPIN Switch: OFF");
         break;                                                
    }
    Serial.printf("bitvalue: %i %i %i %i : %d\n",digit3,digit2,digit1,digit0,POWER);
    for(int i=1; i <= pulse; i++){
        digitalWrite(IO2,digit2);
        digitalWrite(IO1,digit1);
        digitalWrite(IO0,digit0);
        digitalWrite(IO3,HIGH);
        delay(duty);
        digitalWrite(IO3,LOW); 
        delay(duty);          
    }

}

void selftest(int IO0,int IO1,int IO2, int CTRLSW){
    int bb3=0,bb2=0,bb1=0,bb0=0;
    for(int i = 0;i<16;i++){
    bcdconverter(i,&bb3,&bb2,&bb1,&bb0);
    Serial.println();
    Serial.printf("bitvalue: %i %i %i %i : %d    ",bb3,bb2,bb1,bb0,i);
    digitalWrite(IO2,bb2);
    digitalWrite(IO1,bb1);
    digitalWrite(IO0,bb0);
    digitalWrite(CTRLSW,HIGH);
    delay(3000);
    digitalWrite(CTRLSW,LOW);
    Serial.print("Done");
  }
}
#include <Arduino.h>
#include "hw10010829.h"
//#include "startup.h"




enum buttons{POWER,UP,DOWN, START,RINSE,TEMP,SPIN,WORK};

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


void buttonCtrl( int button, int pulse, int duty){
  int digit0=0, digit1=0,digit2=0,digit3=0;

  switch(button) {
    case POWER: 
         bcdconverter(POWER,&digit3,&digit2,&digit1,&digit0);
         Serial.printf("bitvalue: %i %i %i %i : %d\n",digit3,digit2,digit1,digit0,POWER);
         for(int i=1; i <= pulse; i++){
            Serial.println("POWER Switch: ON");
            digitalWrite(AD2,digit2);
            digitalWrite(AD1,digit1);
            digitalWrite(AD0,digit0);
            digitalWrite(CTRLPULSE,HIGH);
            delay(duty);
            digitalWrite(CTRLPULSE,LOW); 
            delay(duty);  
            Serial.println("POWER Switch: OFF");         
         }
         break;
    case UP: 
         bcdconverter(UP,&digit3,&digit2,&digit1,&digit0);
         Serial.printf("bitvalue: %i %i %i %i : %d\n",digit3,digit2,digit1,digit0,POWER);
         for(int i=1; i <= pulse; i++){
            Serial.println("UP Switch: ON");
            digitalWrite(AD2,digit2);
            digitalWrite(AD1,digit1);
            digitalWrite(AD0,digit0);
            digitalWrite(CTRLPULSE,HIGH);
            delay(duty);
            digitalWrite(CTRLPULSE,LOW); 
            delay(duty);
            Serial.println("UP Switch: OFF");           
         }
         break;
    case DOWN: 
         bcdconverter(DOWN,&digit3,&digit2,&digit1,&digit0);
         Serial.printf("bitvalue: %i %i %i %i : %d\n",digit3,digit2,digit1,digit0,POWER);
         for(int i=1; i <= pulse; i++){
            Serial.println("DOWN Switch: ON");
            digitalWrite(AD2,digit2);
            digitalWrite(AD1,digit1);
            digitalWrite(AD0,digit0);
            digitalWrite(CTRLPULSE,HIGH);
            delay(duty);
            digitalWrite(CTRLPULSE,LOW); 
            delay(duty); 
            Serial.println("DOWN Switch: OFF");          
         }
         break;
     case START: 
         bcdconverter(START,&digit3,&digit2,&digit1,&digit0);
         for(int i=1; i <= pulse; i++){
            Serial.println("START Switch: ON");
            digitalWrite(AD2,digit2);
            digitalWrite(AD1,digit1);
            digitalWrite(AD0,digit0);
            digitalWrite(CTRLPULSE,HIGH);
            delay(duty);
            digitalWrite(CTRLPULSE,LOW); 
            delay(duty);
            Serial.println("START Switch: OFF");           
         }
         break;
     case RINSE: 
         bcdconverter(RINSE,&digit3,&digit2,&digit1,&digit0);
         for(int i=1; i <= pulse; i++){
            Serial.println("RINSE Switch: ON");
            digitalWrite(AD2,digit2);
            digitalWrite(AD1,digit1);
            digitalWrite(AD0,digit0);
            digitalWrite(CTRLPULSE,HIGH);
            delay(duty);
            digitalWrite(CTRLPULSE,LOW);
            delay(duty);  
            Serial.println("RINSE Switch: OFF");          
         }
         break;
    case TEMP: 
         bcdconverter(TEMP,&digit3,&digit2,&digit1,&digit0);
         for(int i=1; i <= pulse; i++){
            Serial.println("TEMP Switch: ON");
            digitalWrite(AD2,digit2);
            digitalWrite(AD1,digit1);
            digitalWrite(AD0,digit0);
            digitalWrite(CTRLPULSE,HIGH);
            delay(duty);
            digitalWrite(CTRLPULSE,LOW); 
            delay(duty);
            Serial.println("TEMP Switch: OFF");           
         }
         break;    
    case SPIN: 
         bcdconverter(SPIN,&digit3,&digit2,&digit1,&digit0);
         for(int i=1; i <= pulse; i++){
            Serial.println("SPIN Switch: ON");
            digitalWrite(AD2,digit2);
            digitalWrite(AD1,digit1);
            digitalWrite(AD0,digit0);
            digitalWrite(CTRLPULSE,HIGH);
            delay(duty);
            digitalWrite(CTRLPULSE,LOW); 
            delay(duty);
            Serial.println("SPIN Switch: OFF");           
         }
         break;                                                
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



// This use for check status of ldr at specific location.
bool isHome(int pin){
  bool pstate = true;
  int i = 0;

  do{
    pstate = digitalRead(pin);
    i++;
    Serial.printf("Pstate: %d, inx %d\n",pstate,i);
  }while( (pstate != 0) && (i<50) );

  if(i>=50){
    return true;
  }else{
    return false;
    
  }
}


bool startProg(int prognum){

  buttonCtrl(POWER,1,1000);

  int i = 1;
  
  //Finding for home position
  while( (isHome(PROG1) != 0) && (i <= 14) ){
    Serial.print("MachineDC: ");
    Serial.println(digitalRead(PROG1));
    Serial.print("i: ");Serial.println(i);
    buttonCtrl(UP,1,300);    // From Cotton  send 6 pulse to set SelfClean
    //delay(100);
    i++;
  }  

  if(i <= 14){
    switch(prognum){
      case 1:
          //  At Quick mode 20 degree, 2rinse, 23mins
          buttonCtrl(RINSE,2,200);   //Set temp to 0degree;
          buttonCtrl(TEMP,2,200);   //Set temp to 30degree;
          break;
      case 2:
          // From Cotton  send 11 pulse to set Quick 15min with 30degree , 2rinse, 33mins
          buttonCtrl(RINSE,2,200);   //Set temp to 0degree;
          buttonCtrl(TEMP,3,200);   //Set temp to 30degree;
          break;      
      case 3:
          // From Cotton  send 11 pulse to set Quick 15min with 40degree , 2rinse . 38mins
          buttonCtrl(RINSE,2,200);   //Set temp to 0degree;
          buttonCtrl(TEMP,4,200);   //Set temp to 30degree;
          break;     
    }
    return true;
  }else{
    return false;
  }
}
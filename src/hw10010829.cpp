#include <Arduino.h>
#include "hw10010829.h"
//#include "startup.h"


BP10829::BP10829(void){

}

// enum buttons{POWER,UP,DOWN, START,RINSE,TEMP,SPIN,WORK};

void BP10829::bcdconverter(int value, int *bit3, int *bit2, int *bit1, int *bit0){


  switch(value){
      case  0:  *bit3=0; *bit2=0; *bit1=0; *bit0=0; break; // Power
      case  1:  *bit3=0; *bit2=0; *bit1=0; *bit0=1; break; // UP
      case  2:  *bit3=0; *bit2=0; *bit1=1; *bit0=0; break; // Down
      case  3:  *bit3=0; *bit2=0; *bit1=1; *bit0=1; break; // Start
      case  4:  *bit3=0; *bit2=1; *bit1=0; *bit0=0; break; // Rinse
      case  5:  *bit3=0; *bit2=1; *bit1=0; *bit0=1; break; // Temp    
      case  6:  *bit3=0; *bit2=1; *bit1=1; *bit0=0; break; // Spin
      case  7:  *bit3=0; *bit2=1; *bit1=1; *bit0=1; break; // BUSY
      case  8:  *bit3=1; *bit2=0; *bit1=0; *bit0=0; break; // N/A
      case  9:  *bit3=1; *bit2=0; *bit1=0; *bit0=1; break; // N/A
      case  10:  *bit3=1; *bit2=0; *bit1=1; *bit0=0; break; // N/A
      case  11:  *bit3=1; *bit2=0; *bit1=1; *bit0=1; break; // N/A
      case  12:  *bit3=1; *bit2=1; *bit1=0; *bit0=0; break; // N/A
      case  13:  *bit3=1; *bit2=1; *bit1=0; *bit0=1; break; // N/A
      case  14:  *bit3=1; *bit2=1; *bit1=1; *bit0=0; break;   // N/A   
      case  15:  *bit3=1; *bit2=1; *bit1=1; *bit0=1; break;   // N/A  
  }
}


void BP10829::buttonCtrl( int button, int pulse, int duty){
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

void BP10829::selftest(int IO0,int IO1,int IO2, int CTRLSW){ //IO) GPIO addr bit 0, IO1 bit 1, IO2 bit2
  int bb3=0,bb2=0,bb1=0,bb0=0;
  for(int i = 0;i<16;i++){
    bcdconverter(i,&bb3,&bb2,&bb1,&bb0);
    Serial.println();
    Serial.printf("bitvalue: %i %i %i %i : %d    ",bb3,bb2,bb1,bb0,i);
    digitalWrite(IO2,bb2);
    digitalWrite(IO1,bb1);
    digitalWrite(IO0,bb0);
    digitalWrite(CTRLSW,HIGH);
    delay(1500);
    digitalWrite(CTRLSW,LOW);
    Serial.print("Done");
    i==7?delay(1000):delay(0);
  }
}



// This use for check status of ldr at specific location.
bool BP10829::isHome(int pin){
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


bool BP10829::startProg(int prognum){

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
    /*  This for K.Thawee shop 3 program  use case 1 ,2, 3*/
    /*  Regent use case 11, 12, 13 */
    switch(prognum){
      case 1:
          //  At Quick mode ,temp 20, rinse 2, 23mins
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
          buttonCtrl(TEMP,4,200);   //Set temp to 40degree;
          break;     
      case 11:   // Quick wash 15 min, 2 rinse, 20degre
          for(int i = 1;i<=11;i++){
            buttonCtrl(UP,1,300);
          }
          buttonCtrl(RINSE,2,200);   //Set temp to 0degree;
          buttonCtrl(TEMP,3,200);   //Set temp to 30degree;
          break;
      case 12:
          //buttonCtrl(TEMP,2,200);   //Set temp to 30degree;
          break;
      case 13:
          break;
      
    }

    delay(2000);// Set Delay wait for customer insert detergent.
    buttonCtrl(START,1,200);
    return true;
  }else{
    return false;
  }
}

void BP10829::pulseGEN(bool logic, int qty, int width, int object){

}

void BP10829::ctrlStart(){
  buttonCtrl(START,1,200);
}

bool BP10829::ctrlPower(int pwrPin, int machineDC,powerMODE mode){
  buttonCtrl(POWER,1,1000);
  return true;

  /*  //Not use yet
  int moretime=500;
  int retrylimit=0;
  Serial.println();
  Serial.print("Performing ctrlPower\n");
  // pulseGEN(HIGH,1,1000,pwrPin);
  if(mode == 1){  // You want to Power ON machine
    delay(500);

    if(!isMachineON(machineDC)){ // Check machineDC if 0 (off) then power ON it.
      Serial.println("[ctrlPower]: Turning on machine");
      pulseGEN(HIGH,1,1000,pwrPin);  //Turn on
      if(isMachineON(machineDC)){
        Serial.println("[ctrlPower]: Machine is on");
        return true;
      }else{
        Serial.println("[ctrlPower]: Machine not responseding power on");
        return false; 
      }
    }else{
      Serial.println("[ctrlPower]: Machine is already on");
      return true;
    }
  }else{    //You want to Power off machine
    Serial.println("Waiting for checking machine of 30sec.");
    // delay(39000);
    if(isMachineON(machineDC)){ // Check machineDC if 0 (off) then power ON it.
      Serial.println("[ctrlPower]: Turning off machine");
      pulseGEN(HIGH,1,1000,pwrPin);
      delay(1000);
      if(!isMachineON(machineDC)){
        Serial.println("[ctrlPower]: Machine is completely off");
        return true; //Turn off successful
      }else{
        Serial.println("[ctrlPower]: Machine not responseding power off");
        return false; 
      }
    }else{
      //Machine is off
      pulseGEN(HIGH,1,1000,pwrPin);   //Machine is of then turn on first. Then if on turn it off.
      if(isMachineON(machineDC)){
        Serial.println("[ctrlPower]: Machine suppose to be off. But it is on then turn it off again.");
        pulseGEN(HIGH,1,1000,pwrPin);
      }else{
        Serial.println("[ctrlPower]: Machine is already off");
      }
      return true;  //Turn off successful
    }
  }
  */
}

void BP10829::ctrlSpeed(int speed){
  buttonCtrl(SPIN,1,200);
}

void BP10829::ctrlTemp(int tmp){
  buttonCtrl(TEMP,1,200);
}
void BP10829::ctrlRinse(int rinse){
  buttonCtrl(RINSE,1,200);
}
void BP10829::ctrlProg(int prog){}
void BP10829::selfTest(){}


bool BP10829::washProgram(int prog, int tmp, int speed, int rinse){
  Serial.print("[washProgram]: Performing washProgram\n");

  if(ctrlPower(POWER_RLY,MACHINEDC,TURNON)){
    return true;
  }else{
    return false;
  }
}


int BP10829::runProgram(int prog, int tmp, int speed ,int rinse,LiquidCrystal_I2C &lcd,int &err){
  return false;
}
int BP10829::runProgram(int prog,int tmp, int speed ,int rinse,digitdisplay &disp,int &err){
  return false;
}
// void progStart();
// void servicEnd();

bool BP10829::isMachineON(int pin){
    int err;
    return isMachineON(pin,err);
}// 1 = ON, 0 = Off

bool BP10829::isMachineON(int pin,int &err){
    if(digitalRead(pin)){
        Serial.printf("[isMachineOn]-> OFF\n");
        return false;
    }else{
        Serial.printf("[isMachineOn]-> ON\n");
        return true;
    }
}// 1 = ON, 0 = Off

bool BP10829::isDoorLock(int pin){
    bool doorState = false;
    doorState = digitalRead(pin);

    if(doorState){
        Serial.printf("[isDoorLock]->Door Lock\n");
        return true; // Door Lock
    }else{
        Serial.printf("[isDoorLock]->Door Unlock\n");
        return false; //Door Unlock
    }
}



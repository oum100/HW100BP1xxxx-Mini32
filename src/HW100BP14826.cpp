#include "HW100BP14826.h"
#include "animation.h"
#include <LiquidCrystal_I2C.h>

BP14826::BP14826(){

}

void BP14826::pulseGEN(bool logic, int qty, int width, int object) {
  int i = 0;
  long time1 = 0;
  long time2 = 0;
  long timediff = 0;

  Serial.println("Performing pulseGen");

  digitalWrite(object, !logic);

  for (i = 1; i <= qty; i++) {
    Serial.printf("(Pulse: %d) ", i);
    time1 = millis();
    time2 = millis();
    timediff = time2 - time1;
    digitalWrite(object, logic);
    Serial.printf("(Logic: %d ", digitalRead(object));
    while (timediff != width) {
      time2 = millis();
      timediff = time2 - time1;
    }
    Serial.printf("Time: %li )", timediff);
    digitalWrite(object, !logic);
    Serial.printf("(Logic: %d ", digitalRead(object));
    time1 = millis();
    time2 = millis();
    timediff = time2 - time1;
    while (timediff != width) {
      time2 = millis();
      timediff = time2 - time1;
    }
    Serial.printf("Time: %li )\n", timediff);
  }
  Serial.println("Finish pulseGen");
}



void BP14826::ctrlStart(){    
    Serial.println();
    Serial.print("Performing ctrlStart.\n");
    pulseGEN(HIGH,1,500,START_RLY);
    Serial.print("Finish ctrlStart.\n");
}

bool BP14826::ctrlPower(int pwrPin, int machineDC,powerMODE mode){
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
        delay(1500); //Add delay
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
}

void BP14826::ctrlSpeed(int speed){
    Serial.println();
    for(int i = 0 ; i < speed ; i++){
      Serial.print("Performing ctrlSpeed\n");
      pulseGEN(HIGH,1,500,SPEED_RLY);
      Serial.print("Finish ctrlSpeed\n");
    }
}

void BP14826::ctrlTemp(int tmp){
  Serial.println();
  for(int i = 0 ; i < tmp ; i++){
    Serial.print("Performing ctrlTemp\n");
    pulseGEN(HIGH,1,500,TEMP_RLY);
    Serial.print("Finish ctrlTemp\n");
  }  
}

void BP14826::ctrlRinse(int rinse){
  Serial.println();
  for(int i = 0 ; i < rinse ; i++){
    Serial.print("Performing ctrlRinse\n");
    pulseGEN(HIGH,1,500,RINSE_RLY);
    Serial.print("Finish ctrlRinse\n");
  }  
}


void BP14826::ctrlProg(int prog){
  byte bit = 0;  

  switch(prog){
    case 0:
      prog = 10;
      break;
    case 1:
      prog = 7;
      break;
    case 2:
      prog = 15;
      break;
    case 3:
      prog = 11;
      break;    
    case 4:
      prog = 3;
      break;
    case 5:
      prog = 2;
      break;
    case 6:
      prog = 5;
      break;
    case 7:
      prog = 14;
      break;    
    case 8:
      prog = 6;
      break;
    case 9:
      prog = 4;
      break;
    case 10:
      prog = 12;
      break;
    case 11:
      prog = 8;
      break;    
    case 12:
      prog = 0;
      break;
    case 13:
      prog = 1;
      break;
    case 14:
      prog = 9;
      break;
    case 15:
      prog = 13;
      break;  

  }

  Serial.printf("Performing ctrlProg:%d, binary: ",prog);

  bit = (prog & 0x08) >> 3;
  Serial.print(bit);
  digitalWrite(SWL1, bit);

  bit = (prog & 0x04) >> 2;
  Serial.print(bit);
  digitalWrite(SWL2, bit);

  bit = (prog & 0x02) >> 1;
  Serial.print(bit);
  digitalWrite(SWL3, bit);

  bit = (prog & 0x01) >> 0;
  Serial.print(bit);  
  digitalWrite(SWL4, bit);

  Serial.println();
}

void BP14826::selfTest(void){
  Serial.println("[selfTest]: Performing selfTest");
  washProgram(QUICK,0,0,0);
}


void BP14826::ctrlReset(void){
  Serial.println("[ctrlReset]->request cancel job.");

  if(isMachineON(MACHINEDC)){ //Machine is on   latest update 19Mar25 create technic1 and 2. but 2 is better.
    Serial.println("[ctrlReset]->Machine now is on.");

    //Technic 2  to maker sure door is unlock 
    ctrlPower(POWER_RLY,MACHINEDC,TURNOFF);
    ctrlPower(POWER_RLY,MACHINEDC,TURNON);
    delay(500);
    ctrlPower(POWER_RLY,MACHINEDC,TURNOFF);

  }else{ //Machine is off
    Serial.println("[ctrlReset]->Machine now is off");
    Serial.println("TurnON --> 500ms --> TurnOff");
    ctrlPower(POWER_RLY,MACHINEDC,TURNON);
    delay(500);
    ctrlPower(POWER_RLY,MACHINEDC,TURNOFF);
  }
}


void BP14826::ctrlCancel(void){
  Serial.println("[ctrlCancel]->request cancel job. By turn machine off.");
  ctrlPower(POWER_RLY,MACHINEDC,TURNOFF);
  // if(isMachineON(MACHINEDC)){ //Machine is on   latest update 19Mar25 create technic1 and 2. but 2 is better.
  //   Serial.println("[ctrlCancel]->Machine now is on. Then Turn off machine.");
  //   ctrlPower(POWER_RLY,MACHINEDC,TURNOFF);
  // }else{ //Machine is off
  //   Serial.println("[ctrlCancel]->Machine now is off");
  // }
}


bool BP14826::washProgram(int prog, int tmp, int speed, int rinse) {
  Serial.print("[washProgram]: Performing washProgram\n");
  // ctrlPower(POWER_RLY,MACHINEDC,TURNON); 
  // delay(500);
  if(ctrlPower(POWER_RLY,MACHINEDC,TURNON)){
    // Added 7Aug23: Fixed bug continue run same program. then setting rinse error. To fix this force machine
    // set to MIX first then set as request program again.
      if(prog == 0){ 
        ctrlProg(QUICK);
        delay(500);
      }else{
        ctrlProg(MIX);
        delay(500);
      }
      //**********************************************************************
      
      ctrlProg(prog);
      delay(500);

      if(tmp > 0){
        ctrlTemp(tmp);
        delay(500);
      }

      if(speed > 0){
        ctrlSpeed(speed);
        delay(500);
      }

      if(rinse > 0){
        ctrlRinse(rinse);
        delay(500);
      }


      int startTryCount=0;
      do{
        if(startTryCount < 3){
          ctrlStart();
          startTryCount++;
          Serial.printf("[washProgram] --> Starting program time %d\n",startTryCount);
          switch(startTryCount){  // increase delay for each retry
            case 1: delay(5000); break;
            case 2: delay(8000); break;
            case 3: delay(11000); break;
          }
        }else{
          Serial.print("[washProgram]: Machine Failed to start\n");
          return false;
        }
        // delay(300);
      }while( (!isDoorLock(DLOCK)) && (startTryCount < 3) );
      Serial.print("[washProgram]: Machine start successful.\n");
      return true;
  }else{
    Serial.print("[washProgram]: Machine start failed.\n");
    return false;
  }
}

bool BP14826::isMachineON(int pin){
    int err;
    return isMachineON(pin,err);
}

bool BP14826::isMachineON(int pin,int &err){
    if(digitalRead(pin)){
        Serial.printf("[isMachineOn]-> OFF\n");
        return false;
    }else{
        Serial.printf("[isMachineOn]-> ON\n");
        return true;
    }
}


bool BP14826::isDoorLock(int pin){
    bool doorState;
    int countLock = 0;
    int countUnlock = 0;

    doorState = digitalRead(pin);
    
    // for(int i =1;i>=3;i++){
    //    delay(300);
    //   if(digitalRead(pin)){
    //     countLock++;
    //   }else{
    //     countUnlock++;
    //   }
     
    // }

    // Serial.print("countLock:");Serial.println(countLock);
    // Serial.print("countUnlock:");Serial.println(countUnlock);

    // if(countLock < countUnlock){
    //   doorState = true;
    // }else{
    //   doorState = false;
    // }


    
    //May need to use value from interrupt

    if(doorState){ 
        Serial.printf("[isDoorLock]->Door Lock\n");
        return true; // Door Lock
    }else{
        Serial.printf("[isDoorLock]->Door Unlock\n");
        return false; //Door Unlock
    }
}


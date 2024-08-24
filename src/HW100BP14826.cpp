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
    if(mode == 1){  // Power ON machine
      delay(1000);
      if(!isMachineON(machineDC)){ // Check machineDC if 0 (off) then power ON it.
        Serial.println("[ctrlPower]: Turning on machine");
        pulseGEN(HIGH,1,1000,pwrPin);
        // delay(1000);
        if(isMachineON(machineDC)){
          Serial.println("[ctrlPower]: Machine is on");
          return true;
        }else{
          Serial.println("[ctrlPower]: Machine not responseding power on");
          return true; 
        }
      }else{
        Serial.println("[ctrlPower]: Machine is already on");
        return true;
      }
    }else{    //Power off machine
      Serial.println("Waiting for checking machine of 30sec.");
      // delay(39000);
      if(isMachineON(machineDC)){ // Check machineDC if 0 (off) then power ON it.
        Serial.println("[ctrlPower]: Turning off machine");
        pulseGEN(HIGH,1,1000,pwrPin);
        delay(1000);
        if(!isMachineON(machineDC)){
          Serial.println("[ctrlPower]: Machine is completely off");
          return true;
        }else{
          Serial.println("[ctrlPower]: Machine not responseding power off");
          return false; 
        }
        // do{
        //   pulseGEN(HIGH,1,1000,pwrPin);
        //   delay(moretime);
        //   moretime+=500;
        //   retrylimit++;
        // }while( (isMachineON(machineDC)==true) && (retrylimit<3) );
        // if(retrylimit>=3){
        //   Serial.println("[ctrlPower]: Machine is not responding");
        //   return false;
        // }else{
        //   Serial.println("[ctrlPower]: Machine is off");
        //   return true;
        // }
      }else{
        //Test machine is really off ?
        pulseGEN(HIGH,1,1000,pwrPin);
        if(isMachineON(machineDC)){
          Serial.println("[ctrlPower]: Machine suppose to be off. But it is on then turn it off again.");
          pulseGEN(HIGH,1,1000,pwrPin);
        }else{
          Serial.println("[ctrlPower]: Machine is already off");
        }
        return true;
      }
    }
    
    // Serial.print("Finish ctrlPower\n");
    // return isMachineON(machineDC);
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

void BP14826::selfTest(){
  Serial.println("[selfTest]: Performing selfTest");
  washProgram(QUICK,0,0,0);
}

bool BP14826::washProgram(int prog, int tmp, int speed, int rinse) {
  Serial.print("[washProgram]: Performing washProgram\n");
  ctrlPower(POWER_RLY,MACHINEDC,TURNON); 
  delay(500);
  // Added 7Aug23: Fixed bug continue run same program. then setting rinse error. So, set to default  prog 0 first.
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

  // Set delay wait for customer
  // ctrlStart(); //Start
  // ctrlStart(); //Stop
  //delay(500);
  int startTryCount=0;
  do{
    ctrlStart();
    Serial.printf("[washProgram] --> Start program %d try\n",startTryCount+1);
    startTryCount++;
    delay(5000);
  }while( (!isDoorLock(DLOCK)) && (startTryCount <3) );

  if(isDoorLock(DLOCK)){
    Serial.print("[washProgram]: start successful.\n");
    return 1;
  }else{
    Serial.print("[washProgram]: Failed to start\n");
    pulseGEN(HIGH,20,500,BOOK_LED);
    return 0;
  }
  
}

int BP14826::runProgram(int prog, int tmp, int speed ,int rinse,LiquidCrystal_I2C &lcd,int &err){
  int retry = 0;

  while(retry < 3){
    if(!isMachineON(MACHINEDC)){
        ctrlPower(POWER_RLY,MACHINEDC,TURNON); // Power on machine
        Serial.printf("[runProgram]-> Power On machine, But machine not response. Retry:%d\n",retry+1);
        retry++;
    }else{
        Serial.printf("[runProgram]-> Power On machine, Machine response. Retry:%d\n",retry++);
        //Setting washing program
        ctrlProg(prog); // Set program sport
        ctrlTemp(tmp);
        ctrlSpeed(speed);
        ctrlRinse(rinse); // Set rinse program to 2
        delay(5000);
        ctrlStart();
        return 1;    // Washing start sucessfuly
    }
  }
  if(retry >= 3){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Program not ON");
    delay(3000);
    return 0;
  }
}

int BP14826::runProgram(int prog,int tmp, int speed ,int rinse,digitdisplay &disp,int &err){
  int retry = 0;

  while(retry < 3){
    if(!isMachineON(MACHINEDC)){
        ctrlPower(POWER_RLY,MACHINEDC,TURNON); // Power on machine
        Serial.printf("[runProgram]-> Power On machine, But machine not response. Retry:%d\n",retry+1);
        retry++;
    }else{
        Serial.printf("[runProgram]-> Power On machine, Machine response. Retry:%d\n",retry++);
        //Setting washing program
        ctrlProg(prog); // Set program sport
        ctrlTemp(tmp);
        ctrlSpeed(speed);
        ctrlRinse(rinse); // Set rinse program to 2
        delay(5000);
        ctrlStart();
        return 1;    // Washing start sucessfuly
    }
  }
  if(retry >= 3){
      disp.scrollingText("-PgE-",5);
      disp.print("PgE");
      delay(3000);
      return 0;
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
    if(digitalRead(pin)){
        Serial.printf("[isDoorClose]->Door Lock\n");
        return true; // Door Lock
    }else{
        Serial.printf("[isDoorClose]->Door Unlock\n");
        return false; //Door Unlock
    }
}

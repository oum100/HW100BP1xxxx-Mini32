#include "HW150BP14896.h"
#include "animation.h"
#include <LiquidCrystal_I2C.h>

BP14896ES9::BP14896ES9(){}

void BP14896ES9::pulseGEN(bool logic, int qty, int width, int object) {
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


void BP14896ES9::ctrlPower(int pwrPin, int machineDC,powerMODE mode){
    int moretime=500;
    int retrylimit=0;
    Serial.println();
    Serial.print("Performing ctrlPower\n");

    if(isMachineON(machineDC)){ //Machine start now ON
      if(mode == 0){
        Serial.println("Requesting TURNOFF --> Founded machine ON. Then turn machine OFF");
        pulseGEN(HIGH,1,2000,pwrPin);
      }
      else if(mode == 1){
        Serial.println("Requesting TURNON --> Founded machine ON. Then do nothing");
      }
    }else{//Machine start now OFF
      if(mode == 0){
        Serial.println("Requesting TURNOFF --> Founded machine OFF. Then do nothing");
      }
      else if(mode == 1){
        Serial.println("Requesting TURNON --> Founded machine OFF. Then turn machine ON");
        pulseGEN(HIGH,1,2000,pwrPin);
      }
    }
}

void BP14896ES9::ctrlStart(){    
    Serial.println();
    Serial.print("Performing ctrlStart.\n");
    pulseGEN(HIGH,1,500,START_RLY);
    Serial.print("Finish ctrlStart.\n");
}





void BP14896ES9::ctrlSpeed(int speed){
    Serial.println();
    for(int i = 0 ; i < speed ; i++){
      Serial.print("Performing ctrlSpeed\n");
      pulseGEN(HIGH,1,500,SPEED_RLY);
      Serial.print("Finish ctrlSpeed\n");
    }
}

void BP14896ES9::ctrlTemp(int tmp){
  Serial.println();
  for(int i = 0 ; i < tmp ; i++){
    Serial.print("Performing ctrlTemp\n");
    pulseGEN(HIGH,1,500,TEMP_RLY);
    Serial.print("Finish ctrlTemp\n");
  }  
}

void BP14896ES9::ctrlRinse(int rinse){
  Serial.println();
  for(int i = 0 ; i < rinse ; i++){
    Serial.print("Performing ctrlRinse\n");
    pulseGEN(HIGH,1,500,RINSE_RLY);
    Serial.print("Finish ctrlRinse\n");
  }  
}

void BP14896ES9::ctrlProg(int program){
  ctrlProg(program,1);
}

void BP14896ES9::ctrlProg(int program,int direction){
    int prog = 0;
    if(!digitalRead(MACHINEDC)){
        Serial.println("Rotating for program -> ");
        switch(program){
            case CLEAN_LED: //enum PROGRAM->CLEAN = 0
                Serial.printf("CLEAN\n");
                break;
            case MIX_LED: //enum PROGRAM->MIX = 1
                Serial.printf("MIX\n");
                break;    
            case SPIN_LED: //enum PROGRAM->SPIN = 2
                Serial.printf("SPIN\n");
                break;  
            case SPORT_LED: //enum PROGRAM->SPORT = 3
                Serial.printf("SPORT\n");
                break;                               
        }
        prog = digitalRead(program);  // Read to check, Is it at program needed ?
        while(!prog){
            send_DataEncoder(SWA,SWB,2,400,direction);   //Rotating until program needed.
            delay(500);
            prog = digitalRead(program);
        }
    }else{
        Serial.println("Machine not on. Terminated control.");
    }  
}

void BP14896ES9::selfTest(void){
  Serial.println("[selfTest]: Performing selfTest");
}

void BP14896ES9::ctrlCancel(void){
  Serial.println("[ctrlCancel]->request cancel");
  if(isDoorLock(DLOCK)){
    ctrlStart();
  }
  delay(300);
  ctrlPower(POWER_RLY,MACHINEDC,TURNOFF);
  
  if(!isMachineON(MACHINEDC)){
    Serial.println("[ctrlCancel]->Completed machine is off");
  }

}

bool BP14896ES9::washProgram(int prog, int tmp, int speed, int rinse) {
  Serial.print("[washProgram]: Performing washProgram\n");
  ctrlPower(POWER_RLY,MACHINEDC,TURNON); 
  delay(500);
  
  switch(prog){
    case CLEAN:
      ctrlProg(CLEAN_LED);
      break;
    case MIX:
      ctrlProg(MIX_LED);
      break;
    case SPIN:
      ctrlProg(SPIN_LED);
      break;
    case SPORT:
      ctrlProg(SPORT_LED);
      break;
  }
  delay(300);

  if(tmp > 0){
    ctrlTemp(tmp);
    delay(300);
  }

  if(speed > 0){
    ctrlSpeed(speed);
    delay(300);
  }

  if(rinse > 0){
    ctrlRinse(rinse);
    delay(300);
  }

  int startTryCount=0;
  do{
    ctrlStart();
    startTryCount++;
    Serial.printf("[washProgram] --> Starting program time %d\n",startTryCount);
    switch(startTryCount){  // increase delay for each retry
      case 1: delay(2500); break;
      case 2: delay(5000); break;
      case 3: delay(10000); break;
    }
  }while( (!isDoorLock(DLOCK)) && (startTryCount <3) );

  if(startTryCount <= 3){
    Serial.print("[washProgram]: Machine start successful.\n");
    return true;
  }else{
    Serial.print("[washProgram]: Machine Failed to start\n");
    return false;
  }
}




// int BP14896ES9::runProgram(int prog, int tmp, int speed ,int rinse,LiquidCrystal_I2C &lcd,int &err){
//   int retry = 0;

//   while(retry < 3){
//     if(!isMachineON(MACHINEDC)){
//         ctrlPower(POWER_RLY,MACHINEDC,TURNON); // Power on machine
//         Serial.printf("[runProgram]-> Power On machine, But machine not response. Retry:%d\n",retry+1);
//         retry++;
//     }else{
//         Serial.printf("[runProgram]-> Power On machine, Machine response. Retry:%d\n",retry++);
//         //Setting washing program
//         ctrlProg(prog); // Set program sport
//         ctrlTemp(tmp);
//         ctrlSpeed(speed);
//         ctrlRinse(rinse); // Set rinse program to 2
//         delay(5000);
//         ctrlStart();
//         return 1;    // Washing start sucessfuly
//     }
//   }
//   if(retry >= 3){
//     lcd.clear();
//     lcd.setCursor(0,0);
//     lcd.print("Program not ON");
//     delay(3000);
//     return 0;
//   }
// }

// int BP14896ES9::runProgram(int prog,int tmp, int speed ,int rinse,digitdisplay &disp,int &err){
//   int retry = 0;

//   while(retry < 3){
//     if(!isMachineON(MACHINEDC)){
//         ctrlPower(POWER_RLY,MACHINEDC,TURNON); // Power on machine
//         Serial.printf("[runProgram]-> Power On machine, But machine not response. Retry:%d\n",retry+1);
//         retry++;
//     }else{
//         Serial.printf("[runProgram]-> Power On machine, Machine response. Retry:%d\n",retry++);
//         //Setting washing program
//         ctrlProg(prog); // Set program sport
//         ctrlTemp(tmp);
//         ctrlSpeed(speed);
//         ctrlRinse(rinse); // Set rinse program to 2
//         delay(5000);
//         ctrlStart();
//         return 1;    // Washing start sucessfuly
//     }
//   }
//   if(retry >= 3){
//       disp.scrollingText("-PgE-",5);
//       disp.print("PgE");
//       delay(3000);
//       return 0;
//   }
// }




bool BP14896ES9::isMachineON(int pin){
    int err;
    return isMachineON(pin,err);
}

bool BP14896ES9::isMachineON(int pin,int &err){
    if(digitalRead(pin)){
        Serial.printf("[isMachineOn]-> OFF\n");
        return false;
    }else{
        Serial.printf("[isMachineOn]-> ON\n");
        return true;
    }
}


bool BP14896ES9::isDoorLock(int pin){
    delay(3);
    if(digitalRead(pin)){
        Serial.printf("[isDoorLock]->Door Lock\n");
        return true; // Door Lock
    }else{
        Serial.printf("[isDoorLock]->Door Unlock\n");
        return false; //Door Unlock
    }
}



void BP14896ES9::send_DataEncoder(int APin, int BPin, int pulseCount, unsigned long pulseWidth, boolean Direction)
{

 boolean A_Array[4]={1,1,0,0};
 boolean B_Array[4]={0,1,1,0};   

  int i;
  int static ArrayIndex=0;

  Serial.println();
  for(i=0;i<pulseCount;i++)
  {
    Serial.print(i);
    Serial.print(" ");
    if(Direction)
    {
      ArrayIndex++;
      if(ArrayIndex>=4) ArrayIndex=0;
      digitalWrite(APin, A_Array[ArrayIndex]); 
      digitalWrite(BPin, B_Array[ArrayIndex]);     
    }
    else
    {
      ArrayIndex--;
      if(ArrayIndex<0) ArrayIndex=3;
      digitalWrite(APin, A_Array[ArrayIndex]); 
      digitalWrite(BPin, B_Array[ArrayIndex]);   
    }
    delay(pulseWidth/2);
  }
}
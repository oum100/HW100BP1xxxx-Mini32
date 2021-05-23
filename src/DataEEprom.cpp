
#include <Arduino.h>
#include "DataEEprom.h"
#include "EEPROM.h"
//#include "config.h"

#define DEBUG
#ifdef DEBUG
  #define DBprintf Serial.printf
#endif



int ROM::clearData(unsigned int startoffset, unsigned int endoffset, unsigned int romsize, byte data){
  int i;
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n(CR_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  for (i = startoffset; i < endoffset; i++) {
    EEPROM.write(i, data);
    if (EEPROM.read(i) != data) {
      EEPROM.end();
    }
  }
  EEPROM.commit();
  EEPROM.end();  
  return i;
}


String ROM::readStrData(unsigned int startoffset,int romsize){
  String str;
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[RD_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }
  //EEPROM.readBytes(start,data,end);
  //strcpy(data, EEPROM.readString(startoffset).c_str());

  str = EEPROM.readString(startoffset).c_str();
  //EEPROM.commit();
  EEPROM.end();
  return str;  
}

String ROM::readData(unsigned int startoffset, unsigned int len, int romsize){
  String str = "";


  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[RD_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  int i;
  for (i = startoffset; i < (startoffset + len); i++) {
    str += char(EEPROM.read(i));
    //Serial.printf("%c ",str);
  }
  
  EEPROM.end();
  //Serial.printf("\n[eeprom.readData]Total String: %s\n",str);
  return str;  
}

String ROM::readByteData(unsigned int startoffset, int romsize){
  String str = "";

  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[RBD_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  str = String(EEPROM.readByte(startoffset));
  // Serial.print("[readBypeData]->");
  // Serial.println(str);
  //EEPROM.commit();
  EEPROM.end();
  return str;  
}

String ROM::readIntData(unsigned int startoffset, int romsize){
  String str = "";
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[RID_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  str = String(EEPROM.readInt(startoffset));
  //EEPROM.commit();
  EEPROM.end();
  return str;  
}

String ROM::readShortData(unsigned int startoffset, int romsize){
  String str = "";
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[RID_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  str = String(EEPROM.readShort(startoffset));
  //EEPROM.commit();
  EEPROM.end();
  return str;  
}

bool ROM::writeStrData(unsigned int startoffset, const char* data, int romsize){
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[WRSTR_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }
  //EEPROM.writeBytes(start,data,strlen(data));
  EEPROM.writeString(startoffset, data);
  EEPROM.commit();
  EEPROM.end();
  return true; 
}

bool ROM::writeData(unsigned int startoffset, unsigned int len, String data, int romsize){
  //Blink1.detach();
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[WR_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  Serial.print("Write EEPROM:"); Serial.println(data.length());

  for (int i = startoffset; i < (startoffset + len); i++) {
    if ((i - startoffset) < data.length()) {
      EEPROM.write(i, data[i - startoffset]);
      DBprintf("%i:%c ", i, data[i - startoffset]);
    } else {
      EEPROM.write(i, '\0');
    }
  }
  Serial.println("");
  EEPROM.commit();
  EEPROM.end();
  return true;
}

bool ROM::writeByteData(unsigned int startoffset, byte data, int romsize){
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[WR_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  EEPROM.writeByte(startoffset, data);
  EEPROM.commit();
  EEPROM.end();
  return true;  
}

bool ROM::writeIntData(unsigned int startoffset, int data, int romsize){
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[WR_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  EEPROM.writeInt(startoffset, data);
  EEPROM.commit();
  EEPROM.end();
  return true; 
}

bool ROM::writeShortData(unsigned int startoffset, int data, int romsize){
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[WR_EEPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  EEPROM.writeShort(startoffset, data);
  EEPROM.commit();
  EEPROM.end();
  return true; 
}

void ROM::displayData(unsigned int startoffset, int romsize){
  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[DP_EPPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  for (int i = startoffset; i < (romsize / 16); i++) {
    DBprintf("Addr:%2d-->", i * 16);
    for (int j = 0; j < 16; j++) {
      DBprintf(" %2X", EEPROM.read((i * 16) + j));
    }
    Serial.println();
  }
}

bool ROM::matchData(unsigned int startoffset, const char* data, int romsize){
  char dataRead[] = "";

  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[MD_EPPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }
  bool result;
  EEPROM.readBytes(startoffset, dataRead, strlen(data));
  if (memcmp(dataRead, data, strlen(data)) == 0) {
    //Serial.println("[MT_EEPROM]:Match");
    result = true;
  } else {
    //Serial.println("[MT_EEPROM]:Not Match");
    result = false;
  }
  EEPROM.end();
  return result;  
}


String ROM::readHeader(int romsize){
  int i=0;
  char header;
  String result="";
  //int headerOffset[]={0,55,152,409};

  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[RDH_EPPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  for(i=0;i<4;i++){
    //Serial.println (headerOffset[i]);
    header = EEPROM.readByte(headerOffset[i]);
    result = result + header;
  }
 
  EEPROM.end();
  return result;
}


bool ROM::writeHeader(int romsize){
  //String headerStr="EITC";
  char header[]="";
  //int headerOffset[]={0,55,152,409};

  if (!EEPROM.begin(romsize)) {
    DBprintf("\n[WRH_EPPROM]Failed to initialise EEPROM");
    DBprintf("Restarting...");
    delay(1000);
    ESP.restart();
  }

  for(int i=0;i<headerStr.length();i++){
    EEPROM.write(headerOffset[i],headerStr[i]);
    EEPROM.commit();
  }


  for(int i=0;i<4;i++){
    header[i] = EEPROM.read(headerOffset[i]);
    Serial.printf("[writeHeader]: %c\n",header[i]);
    if(header[i] != headerStr[i]){
      return false;
    }
  }
  return true;
}


/********** DataEEprom class use to read and write data from EEProm **********/
#include <Arduino.h>

class ROM {
  public:
    int clearData(unsigned int startoffset, unsigned int endoffset, unsigned int romsize, byte data);
    String readStrData(unsigned int startoffset,int romsize);
    String readData(unsigned int startoffset, unsigned int len, int romsize);
    String readByteData(unsigned int startoffset, int romsize);
    String readIntData(unsigned int startoffset, int romsize);
    String readShortData(unsigned int startoffset, int romsize);
    bool writeStrData(unsigned int startoffset, const char* data, int romsize);
    bool writeData(unsigned int startoffset, unsigned int len, String data, int romsize);
    bool writeByteData(unsigned int startoffset, byte data, int romsize);
    bool writeIntData(unsigned int startoffset, int data, int romsize);
    bool writeShortData(unsigned int startoffset, int data, int romsize);
    void displayData(unsigned int startoffset, int romsize);
    bool matchData(unsigned int startoffset, const char* data, int romsize);
    String readHeader(int romsize);
    String origHeader(int count,String str);
    bool writeHeader(int romsize);   
    int headerOffset[4]={0,47,98,444};
    String headerStr="EITC";  
};

#ifndef myFS_h
#define myFS_h

#include <Arduino.h>
#include "FS.h"
//#include "config.h"


boolean initFS(fs::FS &fs);
void Format(fs::FS &fs);
boolean isFile(fs::FS &fs, const char * path);

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void createDir(fs::FS &fs, const char * path);
void removeDir(fs::FS &fs, const char * path);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);

String readFile2(fs::FS &fs, const char * path);
void writeFile2(fs::FS &fs, const char * path, const char * message);
void deleteFile2(fs::FS &fs, const char * path);


// void readCFG(Config &cfg,String cfgdata);
// String saveCFG(Config &cfg);
// void initialCFG(Config &cfg);
// void payboardCFG(Config &cfg);

//String getdeviceid(void);

#endif
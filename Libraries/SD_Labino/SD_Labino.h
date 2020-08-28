/*
 * SD_Labino.h - library for managing SD logging
 * Created by Lautaro Silbergleit, August 26, 2020
 */
#ifndef SD_Labino_h
#define SD_Labino_h
#include <SPI.h>
#include <SD.h>

#include "Arduino.h"

class SD_Labino
{
public:
    SD_Labino(int SDPin, int activityPin = -1, String fileName = "log.txt");
    bool init(bool sessionBegin = true);
    bool Log(String logMsg);
    String GetLastLog();
    String ReadFile();
    String GetFileName();
private:
    void Pin(bool enable);
    String ReadSerial();
public:
    bool FileWrite(String msg);

    int _SDPin;
    int _activityPin;
    String _logLine;
    File _logFile;
    String _fileName;
    unsigned int _logSession = 0;
};

#endif
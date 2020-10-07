/*
 * SD_Labino.h - library for managing SD logging
 * Created by Lautaro Silbergleit, August 26, 2020
 */
#ifndef SD_Labino_h
#define SD_Labino_h
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>

#include "Arduino.h"

#define JSON_SIZE 200

class SD_Labino
{
public:
    SD_Labino(int SDPin, int activityPin = -1, String fileName = "json.txt");
    bool begin(bool sessionBegin = true);
    bool OpenStream();
    char ReadStream();
    bool IsStreamAvailable();
    void CloseStream();
    String GetLastLog();
    String ReadFile();
    String GetFileName();
    bool SaveJson(bool clear=true);
private:
    void Pin(bool enable);
    String ReadSerial();
public:
    bool FileWrite(String msg);
    StaticJsonDocument<JSON_SIZE> jsonDoc;    
private:
    int _SDPin;
    int _activityPin;
    String _logLine;
    String _fileName;
    File _logFile;
    unsigned int _logSession = 0;
};

#endif
#ifndef Logging_h
#define Logging_h
#include "Arduino.h"
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define BUFFER_SIZE 64

class Logging
{
public:
    Logging(byte sdPin, SoftwareSerial *bt, int sdActivityPin=-1, int btActivityPin=-1, String fileName = "log.txt");
    bool sd_init(bool beginLog=false);
    void bt_init();
    void SetFileName(String newName);
    bool FileWrite(String log);
    byte BTCommands(); // 0 -> no command; 1 -> OK; 2 -> get file
    bool SaveJson();
    bool BTSendFile();
private:
    void Led(int pin, bool enable);
    String ReadSerial();
    bool ReadBT();
    void SendBT(String msg);
public:
    StaticJsonDocument<200> jsonDoc;
private:
    SoftwareSerial *_bt;
    byte _sdPin;
    byte _rxPin;
    byte _txPin;
    int _sdActivity;
    int _btActivity;
    String _fileName;

    String _logString;
    File _logFile;
    char _btBuffer[BUFFER_SIZE];
    char _btInChar;
};


#endif
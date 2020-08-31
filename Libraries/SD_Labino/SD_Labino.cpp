#include "Arduino.h"
#include "SD_Labino.h"

SD_Labino::SD_Labino(int SDPin, int activityPin, String fileName)
{
    _SDPin = SDPin;
    _activityPin = activityPin;  
    _fileName = fileName;  
    while (!Serial){}
    if (_activityPin >= 0)
    {
        pinMode(_activityPin, OUTPUT);
    }
}

bool SD_Labino::init(bool sessionBegin)
{
    Serial.print("SD init... ");
    this->Pin(true);
    if (!SD.begin(_SDPin))
    {
        Serial.println(F("error!"));
        if (sessionBegin)
        {
            while (true) {}
        }
        return false;
    }
    else
    {
        this->Pin(false);
        Serial.println(F("done"));
        if (sessionBegin)
        {
            Serial.print(F("Write log entry: "));
            _logLine = this->ReadSerial();
            Serial.println(_logLine);
            _logLine += '\n';
        }
        return true;
    }
}

bool SD_Labino::Log(String logMsg)
{
    _logLine = F("Log ");
    _logLine += String(_logSession) + '\n';
    int lastIndex = -1;
    int index = 0;
    while (true)
    {
    index = logMsg.indexOf('\n', index + 1);
    if (index == -1)
        break;
    _logLine += '\t' + logMsg.substring(lastIndex + 1, index) + '\n';
    lastIndex = index;
    }
    _logSession++;
    return this->FileWrite(_logLine);
}

bool SD_Labino::OpenStream()
{
    _logFile = SD.open(_fileName);
    if(_logFile)
        return true;
    else
        return false;
    
}

char SD_Labino::ReadFromStream()
{
    return _logFile.read();
}

String SD_Labino::GetLastLog()
{
    return _logLine;
}

String SD_Labino::GetFileName()
{
    return _fileName;
}

String SD_Labino::ReadFile()
{
    this->Pin(true);
    _logFile = SD.open(_fileName);
    if(_logFile)
    {
        String readFile = "";
        Serial.println(_logFile + ':');
        while (_logFile.available())
        {
            readFile += (char)_logFile.read();
        }
        _logFile.close();
        Serial.println(readFile);
        return readFile;
    }
    else
    {
        Serial.println(F("File error!"));
        this->init(false);
        return "";
    }
    
}

void SD_Labino::Pin(bool enable)
{
    if (_activityPin >= 0)
    {
        digitalWrite(_activityPin, enable);
    }
}

String SD_Labino::ReadSerial()
{
    unsigned int time = millis();
    while (Serial.available() == 0)
    {
        if (millis() - time >= 35) // user input timeout
        {
            return F("Session begin");
        }
        return Serial.readString();
    }
    
}

bool SD_Labino::FileWrite(String msg)
{
    Serial.print(F("logging... "));
    this->Pin(true);
    _logFile = SD.open(_fileName, FILE_WRITE);
    if (_logFile)
    {
        _logFile.print(msg);
        _logFile.close();
        this->Pin(false);
        Serial.println(F("done"));
        return true;
    }
    else
    {
        Serial.println(F("File error!"));
        this->init(false);
        return false;
    }
    
}
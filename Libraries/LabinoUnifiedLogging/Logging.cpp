#include "Arduino.h"
#include "Logging.hpp"

// SD
Logging::Logging(byte sdPin, SoftwareSerial *bt, int sdActivityPin, int btActivityPin, String fileName)
{
    _sdPin = sdPin;
    _bt = bt;
    _sdActivity = sdActivityPin;
    _btActivity = btActivityPin;
    _fileName = fileName;
    if (_sdActivity >= 0)
        pinMode(_sdActivity, OUTPUT);
    if (_btActivity >= 0)
        pinMode(_btActivity, OUTPUT);
}

bool Logging::sd_init(bool beginLog)
{
    Serial.print(F("SD init... "));
    this->Led(_sdActivity, true);
    if (!SD.begin(_sdPin))
    {
        Serial.println(F("error!"));
        if (beginLog)
        {
            while (true) {}
        }
        return false;
    }
    else
    {
        this->Led(_sdActivity, false);
        Serial.println(F("done"));
        if (beginLog)
        {
            Serial.print(F("Write log entry: "));
            _logString = this->ReadSerial();
            Serial.println(_logString);
            _logString += '\n';
            this->FileWrite(_logString);
        }
        return true;
    }
}

void Logging::SetFileName(String newName)
{
    _fileName = newName;
}

bool Logging::FileWrite(String log)
{
    Serial.print(F("logging... "));
    this->Led(_sdActivity, true);
    _logFile = SD.open(_fileName, FILE_WRITE);
    if (_logFile)
    {
        _logFile.print(log);
        _logFile.close();
        this->Led(_sdActivity, false);
        Serial.println(F("done"));
        return true;
    }
    else
    {
        Serial.println(F("file error!"));
        this->sd_init(false);
        return false;
    }
}

void Logging::Led(int pin, bool enable)
{
    if (pin >= 0)
        digitalWrite(pin, enable);
}

String Logging::ReadSerial()
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


// Bluetooth
void Logging::bt_init()
{
    _bt->begin(38400);
    Serial.println(F("bluetooth init"));
}

byte Logging::BTCommands()
{
    if (!this->ReadBT())
        return 0;
    if (String(_btBuffer) == F("OK"))
    {
        this->SendBT(F("OK"));
        Serial.println(F("BT_OK"));
        return 1;
    }
    else if (String(_btBuffer) == F("AD+GETFILE"))
    {
        this->SendBT(F("OK"));
        return 2;
    }
    else
    {
        return 0;
    }
}

bool Logging::ReadBT()
{
    delay(100);
    byte byteCount = _bt->available();
    if (byteCount)
    {
        unsigned int i;
        for (i = 0; i < min(byteCount, (BUFFER_SIZE - 1)); i++)
        {
            _btInChar = _bt->read();
            _btBuffer[i] = _btInChar;
        }
        _btBuffer[i] = '\0';
        while (_bt->available()) {_btInChar = _bt->read();}
        return true;
    }
    else
    {
        return false;
    }
}

void Logging::SendBT(String msg)
{
    _bt->print(msg);
}

bool Logging::BTSendFile()
{
    Serial.println(F("send file... "));
    _logFile = SD.open(_fileName);
    if (_logFile)
    {
        this->SendBT(F("AD+FILE_BEGIN:"));
        while (_logFile.available())
        {
            this->SendBT(String(_logFile.read()));
        }
        this->SendBT(F("AD+FILE_END"));
        Serial.println(F("done"));
        return true;
    }
    Serial.println(F("error"));
    return false;
}

bool Logging::SaveJson()
{
    serializeJson(jsonDoc, _logString);
    this->FileWrite(_logString + '\n');
    jsonDoc.clear();
    _logString = "";
}
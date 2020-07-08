#include <SPI.h>
#include <SD.h>
#include <DHT.h>

#define DHTTYPE DHT22

//parameters (checktime should be greater than 2)
const int checkTime = 3;
const String logFileName = "log.txt";

//pins
const byte sdPin = 10;
const byte dht22Pin = 9;
const byte soilMoisturePins[] = {A0};
const byte pumpInPin = 3;
const byte pumpOutPin = 2;
const byte cardActivity = 4;

//General
unsigned long lastTime;

//logging
File logFile;
String logLine;
unsigned int logSession = 0;

//soil moisture sensor
int soilMoistureValues[sizeof(soilMoisturePins)];

//dht22
DHT dht(dht22Pin, DHTTYPE);
float temperature;
float humidity;

void setup()
{
  Serial.begin(9600);

  SD_init(true);
  PinsInit();
  dht_init();
}

void loop() 
{
  
}

//General
void PinsInit()
{
  pinMode(pumpInPin, OUTPUT);
  pinMode(pumpOutPin, OUTPUT);
  pinMode(cardActivity, OUTPUT);
}

void SetPin(byte pin, bool state)
{
  digitalWrite(pin, state);
}

// Logging
bool SD_init(bool serialTimeInput)
{
  while (!Serial){ }
  Serial.println(F("init SD... "));
  if (!SD.begin(sdPin))
  {
    Serial.println(F("error!"));
    while (true) {  }
  }
  Serial.println("done");
  if (serialTimeInput)
  {
    Serial.println(F("Write log entry: "));
    logLine = ReadSerial();
    logLine += '\n';
    FileWrite(logFileName, logLine);
  }
}

void FileWrite(String fileName, String msg)
{
  Serial.println(F("logging:"));
  Serial.print(msg);
  SetPin(cardActivity, true);
  logFile = SD.open(fileName, FILE_WRITE);
  if (logFile)
  {
    logFile.print(msg);
    logFile.close();
    Serial.println(F("done"));
    SetPin(cardActivity, false);
  }
  else
  {
    FileError();
  }
}

void FileError()
{
  Serial.println(F("error opening File!"));
  while(true) { }
}

String ReadSerial()
{
  while (Serial.available() == 0) { }
  return Serial.readString();
}

String LogFormatter()
{
  logLine = F("Log ");
  logLine += String(logSession);
  logLine += F(":\n\tHum: ");
  logLine += String(humidity);
  logLine += F("\n\tTemp: ");
  logLine += String(temperature);
  logLine += F("\n\tMoisture Vals:");
  for (byte i = 0; i < sizeof(soilMoisturePins); i++)
  {
    logLine += F("\n\t\t");
    logLine += String(i);
    logLine += F(": ");
    logLine += String(soilMoistureValues[i]);
  }
  logLine += F("\n\n");
  FileWrite(logFileName, logLine);
}

//Soil Moisture Sensor
void readSoilMoisture()
{
   for (byte i = 0; i < sizeof(soilMoisturePins); i++)
   {
    soilMoistureValues[i] = SMCalibration(analogRead(soilMoisturePins[i]));
   }
}

int SMCalibration(int raw)
{
  return (int)raw;
}

//DHT22

void dht_init()
{
  dht.begin();
}

void ReadDHT22()
{
   humidity = dht.readHumidity();
   temperature = dht.readTemperature();

   if (isnan(humidity) || isnan(temperature))
   {
      Serial.println(F("dht error"));
      if (isnan(humidity))
        humidity = -1;
      if (isnan(temperature))
        temperature = -1;
   }
}

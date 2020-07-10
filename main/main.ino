#include <SPI.h>
#include <SD.h>
#include <DHT.h>

#define DHTTYPE DHT22

//parameters (checktime should be greater than 2)
const int checkTime = 3 * 1000;
const int maxHumidity = 85;
const int minHumidity = 40;
const int floodTime = 1;
const int flushTime = 1;
const String logFileName = "log.txt";
unsigned int userInputTimeout = 10 * 1000;

//pins
const byte sdPin = 10;
const byte dht22Pin = 9;
const byte soilMoisturePins[] = {A0};
const byte pumpInPin = 3;
const byte pumpOutPin = 2;
const byte cardActivity = 4;

//General
unsigned long lastTime = 0;
bool tick = false;

//logging
File logFile;
String logLine;
unsigned int logSession = 0;
unsigned int failedWrites = 0;

//soil moisture sensor
int soilMoistureValues[sizeof(soilMoisturePins)];
float averageMoisture;

//dht22
DHT dht(dht22Pin, DHTTYPE);
float temperature;
float humidity;

//pump
byte pumpState = 0;
byte pumpLastState = 0;
bool pumpInState = 0;
bool pumpOutState = 0;
unsigned long pumpCheckSetTime = 0;
bool isFlooded = false;

void setup()
{
  Serial.begin(9600);

  SD_init(true);
  PinsInit();
  dht_init();
}

void loop() 
{
  if (millis() - lastTime >= checkTime)
  {
    tick = true;
    ReadDHT22();
    ReadSoilMoisture();
    
    lastTime = millis();
  }

  CheckPumpState();
  PumpCloseControl();

  delay(10); //optional
  
  if (tick)
  {
    LogToSD();
    tick = false;
  }
}

//General
void PinsInit()
{
  pinMode(pumpInPin, OUTPUT);
  pinMode(pumpOutPin, OUTPUT);
  pinMode(cardActivity, OUTPUT);
  SetPin(pumpInPin, false);
  SetPin(pumpOutPin, false);
  SetPin(cardActivity, false);
}

void SetPin(byte pin, bool state)
{
  digitalWrite(pin, state);
}

// Logging
bool SD_init(bool SD_notReinit)
{
  while (!Serial){ }
  Serial.print(F("init SD... "));
  if (!SD.begin(sdPin))
  {
    Serial.println(F("error!"));
    if (SD_notReinit)
    {
      while (true) {  }
    }
  }
  else if (!SD_notReinit)
  {
    SetPin(cardActivity, false);
    Serial.println("done");
  }
  
  if (SD_notReinit)
  {
    Serial.println("done");
    Serial.print(F("Write log entry: "));
    logLine = ReadSerial();
    Serial.println(F("done"));
    logLine += '\n';
    FileWrite(logFileName, logLine);
  }
}

void FileWrite(String fileName, String msg)
{
  Serial.print(F("logging... "));
  SetPin(cardActivity, true);
  logFile = SD.open("log.txt", FILE_WRITE);
  if (logFile)
  {
    logFile.print(msg);
    logFile.close();
    Serial.println(F("done"));
    SetPin(cardActivity, false);
    failedWrites = 0;
  }
  else
  {
    FileError();
  }
}

void FileError()
{
  Serial.println(F("File error!"));
  failedWrites++;
  SD_init(false);
}

String ReadSerial()
{
  unsigned int userInputTime = millis();
  while (Serial.available() == 0)
  {
    if (millis() - userInputTime >= userInputTimeout)
      return F("Session begin");
  }
  return Serial.readString();
}

void LogToSD()
{
  logLine = F("Log ");
  logLine += String(logSession);
  logLine += F(":\n\tHum: ");
  logLine += String(humidity);
  logLine += F("\n\tTemp: ");
  logLine += String(temperature);
  logLine += F("\n\tMoisture Level: ");
  if (pumpState == 0)
    logLine += F("right");
  else if (pumpState == 1)
    logLine += F("too wet");
  else
    logLine += F("too dry");
  logLine += F("\n\tMoisture Vals:");
  logLine += F("\n\t\tAvg: ");
  logLine += String(averageMoisture);
  for (byte i = 0; i < sizeof(soilMoisturePins); i++)
  {
    logLine += F("\n\t\t");
    logLine += String(i + 1);
    logLine += F(": ");
    logLine += String(soilMoistureValues[i]);
  }
  if (failedWrites > 0)
  {
    logLine += F("\n\tFAILED WRITES: ");
    logLine += String(failedWrites);
  }
  logLine += F("\n\n");
  FileWrite(logFileName, logLine);
  logSession++;
}

//Soil Moisture Sensor
void ReadSoilMoisture()
{
  unsigned int sum = 0;
  for (byte i = 0; i < sizeof(soilMoisturePins); i++)
  {
    soilMoistureValues[i] = SMCalibration(analogRead(soilMoisturePins[i]));
    sum += soilMoistureValues[i];
  }
  averageMoisture = sum / sizeof(soilMoisturePins);
}

int SMCalibration(int raw)
{
  //example that works with your hand.
  //In the air = too dry (850+)
  //One finger = right (400 - 850)
  //2+ fingers = too wet (400-)
  unsigned int value = constrain(raw, 0, 1000);
  return (int)(value / 10);
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

//Pumps
void CheckPumpState()
{
  if (averageMoisture <= minHumidity)
  {
    pumpState = 1;
  }
  else if (averageMoisture >= maxHumidity)
  {
    pumpState = 2;
  }
  else
  {
    pumpState = 0;
  }

  if (pumpLastState != pumpState)
  {
    if (pumpState == 1)
    {  
      if (!isFlooded)
      {    
        SetPin(pumpInPin, true);
        pumpInState = true;
        isFlooded = true;
        pumpCheckSetTime = millis();
        pumpLastState = 1;
      }
    }
    else if (pumpState == 2)
    {
      if (isFlooded)
      {      
        SetPin(pumpOutPin, true);      
        pumpOutState = true;
        pumpCheckSetTime = millis();
        pumpLastState = 2;
      }
    }
    else
    {
      pumpLastState = 0;
    }
  }
}

void PumpCloseControl()
{
  if (pumpInState)
  {
    if (millis() - pumpCheckSetTime >= floodTime * 1000)
    {
      SetPin(pumpInPin, false);
      pumpInState = false;
    }
  }
  if (pumpOutState)
  {
    if (millis() - pumpCheckSetTime >= flushTime * 1000)
    {
      SetPin(pumpOutPin, false);
      isFlooded = false;
      pumpOutState = false;
    }
  }
}

#include <SD_Labino.h>
#include <DHT.h>

#define DHTTYPE DHT22

// parameters
const unsigned int checkTime = 3 * 1000;
const unsigned int minHumidity = 40;
const unsigned int maxHumidity = 85;

// pins
const byte sdPin = 10;
const byte dht22Pin = 3;
const byte soilMoisturePins[] = {A0, A0};
const byte waterSensorPin = A1;
const byte pumpInPin = 7;
const byte pumpOutPin = 6;
const byte sdActivity = 5;
const byte btActivity = 5;

// general
unsigned long lastTime = 0;
bool tick = false;

// SD
SD_Labino sd = SD_Labino(sdPin, sdActivity);
String logString;

// soil moisture sensors
int soilMoistureValues[sizeof(soilMoisturePins)];
float averageMoisture;

// dht22
DHT dht(dht22Pin, DHTTYPE);
float temperature;
float humidity;

// pump
byte pumpState = 0;
byte pumpLastState = 0;
bool pumpInEnabled = false;
bool pumpOutEnabled = false;
bool isFlooded = false;

void setup()
{
  Serial.begin(9600);
  sd.init();
  dht.begin();
  pinMode(pumpInPin, OUTPUT);
  pinMode(pumpOutPin, OUTPUT);
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

  PumpStateManager();

  delay(10);

  if (tick)
  {
    Log();
    tick = false;
  }
}

void Log()
{
  logString = F("Hum: ");
  logString += String(humidity);
  logString += F("\nTemp: ");
  logString += String(temperature);
  logString += F("\nPump ");
  if (pumpState == 0)  logString += F("closed");
  else if (pumpState == 1)  logString += F("in open, out closed");
  else if (pumpState == 2)  logString += F("in closed, out open");
  logString += F("\nMoisture Vals:");
  logString += F("\n\tAvg: ");
  logString += String(averageMoisture);
  for (byte i = 0; i < sizeof(soilMoisturePins); i++)
  {
    logString += F("\n\t");
    logString += String(i + 1);
    logString += F(": ");
    logString += String(soilMoistureValues[i]);
  }

  sd.Log(logString);
}

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
  //note that map() function exists
  unsigned int value = constrain(raw, 0, 1000);
  return (int)(value / 10);
}

void ReadDHT22()
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println(F("dht error"));
    if (isnan(humidity))
    {
      humidity = -1;
    }
    if (isnan(temperature))
    {
      temperature = -1;
    }
  }
}

void PumpStateManager()
{
  if (averageMoisture <= minHumidity)
    pumpState = 1;
  else if (averageMoisture >= maxHumidity)
    pumpState = 2;
  else
    pumpState = 0;

  PumpControl();
}

int GetWaterLevelStatus()
{
  int value = analogRead(waterSensorPin);
  if (value <= 100)      // "dry"
    return 0;
  else if (value >= 500) // "flooded"
    return 2;
  else                   // filling or flushing
    return 1;
}

void PumpControl()
{
  int value = GetWaterLevelStatus();
  if (pumpState == 1 && value != 2) // have to fill
  {
    if (!pumpInEnabled)
    {
      pumpInEnabled = true;
      digitalWrite(pumpInPin, HIGH);
    }
    if (pumpOutEnabled)
    {
      pumpOutEnabled = false;
      digitalWrite(pumpOutPin, LOW);
    }
  }
  else if (pumpState == 2 && value != 2)
  {
    if (pumpInEnabled)
    {
      pumpInEnabled = false;
      digitalWrite(pumpInPin, LOW);
    }
    if (!pumpOutEnabled)
    {
      pumpOutEnabled = true;
      digitalWrite(pumpOutPin, HIGH);
    }
  }
}

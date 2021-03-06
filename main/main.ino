#include <SD_Labino.h>
#include <SoftwareSerial.h>
#include <DHT.h>

#define DHTTYPE DHT22

// parameters
unsigned int checkTime = 3 * 1000;
unsigned int minHumidity = 40;
unsigned int maxHumidity = 85;

// pins
const byte sdPin = 10;
const byte dht22Pin = 3;
const byte soilMoisturePins[] = {A0, A0};
const byte waterSensorPin = A1;
const byte pumpInPin = 7;
const byte pumpOutPin = 6;
const byte sdActivity = 5;

// general
unsigned long lastTime = 0;
bool tick = false;

// SD
const String fileName = "log1.txt";
SD_Labino sd = SD_Labino(sdPin, sdActivity, fileName);
String logString;

//esp8266
SoftwareSerial espSerial(8, 9);
const byte espActivity = 5;
StaticJsonDocument<50> espJson;

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
  espSerial.begin(115200);
  sd.begin();
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
    EspSendSensorValues();
    lastTime = millis();
  }

  PumpControl();

  if (tick)
  {
    Log();
    delay(10);
    EspSendFile();
    tick = false;
  }
}

void Log()
{
  Serial.println("");
  Serial.println(humidity);
  Serial.println(temperature);
  Serial.println(pumpState);
  Serial.println(averageMoisture);
  Serial.println("");
  sd.jsonDoc[F("humidity")] = humidity;
  sd.jsonDoc[F("temperature")] = temperature;
  sd.jsonDoc[F("pump state")] = pumpState;
  sd.jsonDoc[F("moisture average")] = averageMoisture;
  JsonArray moistureVals = sd.jsonDoc.createNestedArray(F("moisture values"));
  for (byte i = 0; i < sizeof(soilMoisturePins); i++)
  {
    moistureVals.add(soilMoistureValues[i]);
  }
  serializeJson(sd.jsonDoc, Serial);
  Serial.println();
  sd.SaveJson();
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
    pumpState = 1; // too dry
  else if (averageMoisture >= maxHumidity)
    pumpState = 2; // too wet
  else
    pumpState = 0; // normal
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

void PumpControl() // esto se puede ca,biar según sea necesario
{
  PumpStateManager();
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
  else if (pumpState == 2 && value != 0) // have to flush
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

bool EspSendFile()
{
  if(!sd.OpenStream())
  {
    Serial.println(F("Error opening file stream"));
    return false;
  }

  digitalWrite(espActivity, HIGH);
  espSerial.write('@');
  while (sd.IsStreamAvailable())
  {
    char in = sd.ReadStream();
    espSerial.write(in);
  }
  espSerial.write('@');
  digitalWrite(espActivity, LOW);
  sd.CloseStream();
  return true;
}

void EspSendSensorValues()
{
  espJson[F("soilmoisture")] = averageMoisture;
  espJson[F("temperature")] = temperature;
  espJson[F("humidity")] = humidity;
  espSerial.write('$');
  serializeJson(espJson, espSerial);
}

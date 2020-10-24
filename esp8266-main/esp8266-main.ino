#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <CircularBuffer.h>

#define CIRC_ARRAY_LENGTH 10

const char* ssid = "Wifi-Casa";
const char* password = "canotaje";

const byte pin = 2;

/*
 * values stored in json document
 * float soilMoisture
 * float temperature
 * float humidity
 * string pumpState
 */
StaticJsonDocument<50> jsonDoc;
float soilMoisture = 0;
float temperature = 0;
float humidity = 0;

// circular arrays containing info of the last logs of sonsor values to display in chart
CircularBuffer<float,CIRC_ARRAY_LENGTH> soilMoistureArray;
CircularBuffer<float,CIRC_ARRAY_LENGTH> temperatureArray;
CircularBuffer<float,CIRC_ARRAY_LENGTH> humidityArray;

const String fileName = "/log.txt";
File myFile;

AsyncWebServer server(80);

String readSoilMoisture() {
  return String(soilMoisture);
}

String readTemp() {
  return String(temperature);
}

String readHum() {
  return String(humidity);
}

String processor(const String& var){
  if(var == "SOILMOISTURE"){
    return readTemp();
  }
  if(var == "TEMPERATURE"){
    return readTemp();
  }
  else if(var == "HUMIDITY"){
    return readHum();
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);

  // begin SPIFFS
  if(!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // initialise circular arrays
  for (byte i = 0; i < CIRC_ARRAY_LENGTH; i++)
  {
    soilMoistureArray.push(0);
    temperatureArray.push(0);
    humidityArray.push(0);
  }
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(" .");
  }
  Serial.println(" done");

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/logging", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logging.html", String(), false, processor);
  });
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/data.html", String(), false, processor);
  });
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles.css", "text/css");
  });
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, fileName);
  });
  server.on("/soilmoisture", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemp().c_str());
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemp().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readHum().c_str());
  });

  // Start server
  server.begin();

  digitalWrite(pin, LOW);
}
 
void loop()
{
  if (Serial.available())
  {
    char in = (char)Serial.read();
    if (in == '@')
      FileFromSerial();
    else if (in == '$')
      JsonFromSerial();
  }
}

bool FileFromSerial() // call when char '@' is read from serial. end with '@'
{
  myFile = SPIFFS.open(fileName, "w");
  if (!myFile)
  {
    Serial.println("error opening file!");
    return false;
  }
  unsigned int readStartTime = millis();
  while (Serial.available())
  {
    char in = (char)Serial.read();
    if (in == '@')
      break;
    myFile.print(in);
    if (millis() - readStartTime > 10000)
      break;
  }
  myFile.println();
  myFile.close();
  return true;
}

bool JsonFromSerial() // call when '$' is read from serial
{
  while (!Serial.available()) {;}
  String in = Serial.readString();
  DeserializationError error = deserializeJson(jsonDoc, in);
  if (error)
    return false;

  // update variables
  soilMoisture = jsonDoc["soilmoisture"];
  temperature = jsonDoc["temperature"];
  humidity = jsonDoc["humidity"];
  
  soilMoistureArray.push(soilMoisture);
  temperatureArray.push(temperature);
  humidityArray.push(humidity);
}

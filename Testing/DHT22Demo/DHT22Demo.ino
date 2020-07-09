#include "DHT.h"
#define DHTTYPE DHT22

const int DHTPin = 5;
DHT dht(DHTPin, DHTTYPE);

float h = 0;
float t = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
   }
 
 
   Serial.print("Humidity: ");
   Serial.print(h);
   Serial.print(" %\t");
   Serial.print("Temperature: ");
   Serial.print(t);
   Serial.println(" *C ");

   delay(2000);
}

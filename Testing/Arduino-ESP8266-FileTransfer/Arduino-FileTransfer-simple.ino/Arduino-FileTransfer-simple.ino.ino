#include <SoftwareSerial.h>

SoftwareSerial espSerial(2, 3);

const String message = "hola";
const byte pin = 7; 
void setup()
{
  Serial.begin(9600);
  espSerial.begin(9600);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  while (!Serial) {;}
  while (!espSerial) {;}
}

void loop()
{
  if (espSerial.available()){
    String incomingString = espSerial.readString();
    digitalWrite(pin, HIGH);
    Serial.println("Received String: " + incomingString);
    delay(100);
    digitalWrite(pin, LOW);
  }
}

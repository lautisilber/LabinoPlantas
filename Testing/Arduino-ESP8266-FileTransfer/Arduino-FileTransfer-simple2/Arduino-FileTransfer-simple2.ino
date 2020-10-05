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
  digitalWrite(pin, HIGH);
  espSerial.write("Hello from Arduino!\n");
  if (espSerial.available())
  {
    Serial.println(espSerial.readString());
  }
  delay(100);
  digitalWrite(pin, LOW);
  delay(1900);
}

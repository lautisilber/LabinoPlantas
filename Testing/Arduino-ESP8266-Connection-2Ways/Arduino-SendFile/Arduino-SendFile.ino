#include <SoftwareSerial.h>

SoftwareSerial espSerial(2, 3);

const char sendChar = '$';

bool connectionBegin = 0;
String serialIn = "";
char lastIn = '\0';

void setup()
{
  espSerial.begin(115200);
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  while (!Serial) {;}
}

void loop()
{  
  if (Serial.available())
  {
    lastIn = (char)Serial.read();        
    if (lastIn == sendChar || espSerial.available())
    {
      Serial.println("writing to esp8266:");
      Serial.println(serialIn);
      espSerial.println(serialIn);
      serialIn = "";
      lastIn = '\0';
    }
    else
    {
      serialIn += lastIn;
    }
  }
}

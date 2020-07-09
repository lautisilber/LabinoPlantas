#include <SPI.h>
#include <SD.h>

const int delayTime = 5 * 1000;

File myFile;
const byte sdPin = 10;

const String fileName = "stabil.txt";

const byte ledPin = 4;

void setup() {
  Serial.begin(9600);
  while(!Serial){ }
  Serial.print("Initializing SD... ");
  if (!SD.begin(sdPin))
  {
    Serial.println("Could not initialize SD!");
    while(true) {delay(100);}
  }
  Serial.println("done!");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  StabilityTest();
}

void loop() {
  // put your main code here, to run repeatedly:

}

void StabilityTest()
{
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile)
  {
    Serial.print("printing... ");
    myFile.print("start... ");
    digitalWrite(ledPin, HIGH);
    delay(delayTime);;
    digitalWrite(ledPin, LOW);
    myFile.println("end");
    myFile.close();
    Serial.println("done");
  }
  else
  {
    Serial.println("couldn't open file");
  }
}

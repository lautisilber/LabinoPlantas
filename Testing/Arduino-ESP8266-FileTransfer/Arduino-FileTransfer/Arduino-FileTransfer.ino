#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

File myFile;

SoftwareSerial espSerial(2, 3);

const byte activityPin = 7;
const byte sdPin = 10;
const String fileName = "wifi.txt";

void setup() {
  Serial.begin(9600);
  pinMode(activityPin, OUTPUT);
  digitalWrite(activityPin, LOW);
  while (!Serial) {;}
  espSerial.begin(115200);

  // sd init
  Serial.print("Initializing SD card... ");
  pinMode(10, OUTPUT);
  if (!SD.begin(sdPin)) {
    Serial.println("failed!");
    return;
  }
  Serial.println("done.");

  // read file
  SendFile();
}

void loop()
{
}

void SendFile()
{
  myFile = SD.open(fileName, FILE_READ);
  if (!myFile)
  {
    Serial.print("error opening ");
    Serial.println(fileName);
    return;
  }

  digitalWrite(activityPin, HIGH);
  espSerial.write('@');
  while (myFile.available())
  {
    Serial.write(myFile.peek());
    espSerial.write(myFile.read());
  }
  espSerial.write('@');
  digitalWrite(activityPin, LOW);
  myFile.close();
}

void WriteFile()
{
  myFile = SD.open(fileName, FILE_WRITE);
  if (!myFile)
  {
    Serial.print("error opening ");
    Serial.println(fileName);
    return;
  }

  for (byte i = 0; i < 10; i++)
  {
    myFile.print("Testing... ");
    myFile.println(String(i + 1));
  }
  myFile.close();
}

void espSerialFlush()
{
  while (espSerial.available())
  {
    espSerial.read();
  }
}

#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

File myFile;

SoftwareSerial espSerial(2, 3);

const byte sdPin = 10;
const String fileName = "log.txt";

void setup() {
  Serial.begin(9600);
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
  ReadFile();
}

void loop() {
  if (espSerial.available())
  {
    espSerial.flush();
    ReadFile();
  }
}

void ReadFile()
{
  myFile = SD.open(fileName, FILE_READ);
  if (!myFile)
  {
    Serial.print("error opening ");
    Serial.println(fileName);
    return;
  }

  while (myFile.available())
  {
    Serial.write(myFile.peek());
    espSerial.write(myFile.read());
  }
  myFile.close();
}

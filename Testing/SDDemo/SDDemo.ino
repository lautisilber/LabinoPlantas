#include <SPI.h>
#include <SD.h>

File myFile;
const int sdPin = 10;

const char fileName[] = "test.txt";
const char message[] = "hola que tal bro\nTodo bien?";

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
  FileWrite();
  FileRead();
}

void loop() {
  // put your main code here, to run repeatedly:

}

bool FileWrite()
{
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile)
  {
    Serial.print("Printing to ");
    Serial.print(fileName);
    Serial.print("... ");
    myFile.println(message);
    myFile.close();
    Serial.println("done!");
    return true;
  }
  else
  {
    Serial.print("Could not open file ");
    Serial.println(fileName);
    return false;
  }
}

bool FileRead()
{
  myFile = SD.open(fileName);
  if (myFile)
  {
    Serial.print("\nReading '");
    Serial.print(fileName);
    Serial.println("':\n----------");
    while (myFile.available())
    {
      Serial.write(myFile.read());
    }
    myFile.close();
    Serial.println("----------\n");
    return true;
  }
  else
  {
    Serial.print("Could not read file: ");
    Serial.println(fileName);
    return false;
  }
}

#include <SPI.h>
#include <SD.h>

File myFile;
File root;
const int sdPin = 10;
String rootpath = "/";


void ListDir(File dir, int numTabs = 1);

void setup() {
  Serial.begin(9600);
  while(!Serial){ }
  Serial.print("Initializing SD... ");
  if (!SD.begin(sdPin))
  {
    Serial.println("Could not initialize SD!");
    while(true) {delay(100);}
  }
  root = SD.open("/");
  if (!root)
  {
    Serial.println("damn...");
  }
  Serial.println("done!");

  if (!Remove("/SPOTLI~1/STORE-V2/944868~1/TMPSPO~1.STA")) {Serial.println("remove failed");}
  else {Serial.println("Success!");}

  ListDir(root);
}

void loop() {

}

bool FileWrite(char fileName[], char message[])
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

bool FileRead(char fileName[])
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

void ListDir(File dir, int numTabs = 1)
{
  while (true)
  {
    File entry = dir.openNextFile();
    if (!entry)
    {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory())
    {
      Serial.println('/');
      ListDir(entry, numTabs + 1);
    }
    else
    {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

bool Remove(char fileName[])
{
  return SD.remove(fileName);
}

bool RmDir(char fileName[])
{
  return SD.rmdir(fileName);
}

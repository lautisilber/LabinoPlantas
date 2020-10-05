const byte pin = 2;
//String serialIn = "";

//const String fileName = "/test.txt";
//File myFile;

void setup() {
  Serial.begin(9600);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  while (!Serial) {;}
}

void loop()
{
  digitalWrite(pin, HIGH);
  Serial.println("Hello from esp8266!");
  delay(100);
  digitalWrite(pin, LOW);
  delay(1900);
}

void SerialFlush()
{
  while (Serial.available())
  {
    Serial.read();
  }
}

/*
void SPIFFSBegin()
{
  if (!SPIFFS.begin())
  {
    digitalWrite(pin, HIGH);
    while (true) {};
    return;
  }
}

void FileWrite()
{
  myFile = SPIFFS.open(fileName, "a+");
  if (!myFile)
  {
    digitalWrite(pin, HIGH);
    while (true) {};
    return;
  }
  myFile.println(serialIn);
  myFile.close();
}
*/

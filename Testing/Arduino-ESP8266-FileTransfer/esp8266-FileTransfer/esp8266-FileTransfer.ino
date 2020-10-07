#include <ESP8266WiFi.h>
#include <FS.h>


const char* SSID = "Wifi-Casa"; 
const char* PASSWORD = "canotaje";
WiFiServer server(80);

const String fileName = "/test.txt";
File myFile;
bool fileIn = false;

String fileString = "";

void setup() {
  Serial.begin(9600);
  while (!Serial) {;}
  // Connect to Wi-Fi network with SSID and PASSWORD
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address 
  Serial.println("");
  Serial.println("WiFi connected at IP address:");
  Serial.println(WiFi.localIP());
  // Start Web Server
  server.begin();

  //start SPIFFS
  Serial.print("begining SPIFFS... ");
  if (!SPIFFS.begin())
  {
    Serial.println("error");
    return;
  }
  Serial.println("done");

  SPIFFS.remove(fileName);
}

void loop() {
  fileIn = FileWriteFromSerial();
  WiFiClient client = server.available();
  if (!client) {
     return;
  }

  while(!client.available()){}

  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/filetest") != -1)  {
    FileRead();
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); 
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print("File says: ");
  client.print(fileString);
  client.println("</html>");
  Serial.println("");
  
}

bool FileWriteFromSerial()
{
  if (!Serial.available())
    return false;
  if ((char)Serial.read() != '@')
    return false;
  
  myFile = SPIFFS.open(fileName, "w");
  if (!myFile)
  {
    Serial.println("error opening file!");
    return false;
  }
  unsigned int readStartTime = millis();
  while (Serial.peek() != '@')
  {
    myFile.print((char)Serial.read());
    if (millis() - 20000 > readStartTime)
      break;
  }
  myFile.println();
  myFile.close();
  return true;
}

/*
void FileWrite()
{
  myFile = SPIFFS.open("/test.txt", "a+");
  if (!myFile)
  {
    Serial.println("error opening file!");
    return;
  }
  for (byte i = 0; i < 5; i++)
  {
    myFile.print("file write test ");
    myFile.println(String(i));
  }
  myFile.close();
}
*/

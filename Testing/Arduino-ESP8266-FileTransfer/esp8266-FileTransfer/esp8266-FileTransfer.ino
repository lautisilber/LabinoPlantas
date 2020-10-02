#include <ESP8266WiFi.h>
#include <FS.h>


const char* SSID = "Wifi-Casa"; 
const char* PASSWORD = "canotaje";
WiFiServer server(80);

File myFile;

String incomingStr = "";


void setup() {
  Serial.begin(115200);
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

  SPIFFS.remove("/test.txt");
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
     return;
  }

  while(!client.available()){}

  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/filetest") != -1)  {
    FileWrite();
    FileRead();
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); 
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print("Hello world!");
  client.println("<tr><td><a href=\"/filetest\"\"><br>Toggle LED<br><br></a></td></tr>");
  client.println("</html>");
  Serial.println("");
  
}

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

void FileRead()
{
  myFile = SPIFFS.open("/test.txt", "r");
  if (!myFile)
  {
    Serial.println("error opening file!");
    return;
  }
  while (myFile.available())
  {
    Serial.write(myFile.read());
  }
  myFile.close();
}

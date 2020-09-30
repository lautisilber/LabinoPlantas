#include <Arduino.h>
// Load Wi-Fi library
#include <ESP8266WiFi.h>
// Replace with your network credentials
const char* SSID = "Wifi-Casa"; 
const char* PASSWORD = "canotaje";
// Set web server port number to 80
WiFiServer server(80);
// Assign output variables to GPIO pins

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
}
// Main loop
void loop(){
  // Create a client and listen for incoming clients
  WiFiClient client = server.available();   
  
  // Do nothing if server is not available
  if (!client) {
     return;
  }
  
  // Wait a client 
  while(!client.available()){}

  incomingStr = ReceiveSerial();
  
  // A new client is connected, get the request
  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/GetString") != -1) 
  {
    Serial.write('a');
  }
  
  // Display GPIO status
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); 
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print("Incoming String: "); 
  client.println(incomingStr);
  client.println("<br><a href=\"/GetString\">get string</a><br>");
  client.println("</html>");
  Serial.println("");
  
}

String ReceiveSerial()
{
  String in = "";
  while (Serial.available())
  {
    in += (char)Serial.read();
  }
  return in;
}

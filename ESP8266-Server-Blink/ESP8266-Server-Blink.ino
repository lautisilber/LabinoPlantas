// link to video
// https://www.youtube.com/watch?v=dWM4p_KaTHY&ab_channel=RuiSantos

#include <ESP8266WiFi.h>

const char* ssid = "Wifi-Casa";
const char* password = "canotaje";

WiFiServer server(80);

String _header;

String _outputPinState = "Off";

const int _pin = 2;

void setup()
{
  Serial.begin(115200);

  pinMode(_pin, OUTPUT);

  digitalWrite(_pin, LOW);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');    
  }

  Serial.println("");
    Serial.println("WiFI connected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
}

void loop()
{
  WiFiClient _client = server.available();

  if (_client)
  {
    Serial.println("New Client");
    String currentLine = "";
    while (_client.connected())
    {
      if (_client.available())
      {
        char c = _client.read();
        Serial.write(c);
        _header += c;
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            _client.println("HTTP/1.1 200 OK");
            _client.println("Content-type:text/html");
            _client.println("Connection: close");
            _client.println();

            if (_header.indexOf("GET /2/on") >= 0)
            {
              Serial.println("Pin on");
              _outputPinState = "On";
              digitalWrite(_pin, HIGH);
            } else if (_header.indexOf("GET /2/off") >= 0) {
              Serial.println("Pin Off");
              _outputPinState = "Off";
              digitalWrite(_pin, LOW);
            }

            // Display the HTML web page
            _client.println("<!DOCTYPE html><html>");
            _client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            _client.println("<link rel=\"icon\" href=\"data:,\">");

            //CSS to style the ON/OFF buttons
            _client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            _client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            _client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            _client.println(".button2 {background-color: #77878A;}</style></head>");

            // web page heading
            _client.println("<body><h1>ESP8266 Web Server</h1>");

            // display current state, and ON/OFF buttons for pin
            _client.println("<p>GRPIO 2 - State " + _outputPinState + "</p>");
            if (_outputPinState == "Off")
            {
              _client.println("<p><a href=\"/2/on\"><button class=\button\">ON</button></a></p>");              
            } else {
              _client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            _client.println("</body></html>");

            // the HTTP response ends with another blank line
            _client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    // Clear header variable
    _header = "";
    // close the connection
    _client.stop();
    Serial.println("Client disconnected");
    Serial.println("");
  }
}

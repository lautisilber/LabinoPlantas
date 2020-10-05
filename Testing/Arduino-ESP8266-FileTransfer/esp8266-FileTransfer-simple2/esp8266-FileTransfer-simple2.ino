const byte pin = 2;

void setup() {
  Serial.begin(9600);
  pinMode(pin, OUTPUT);
  
  while (!Serial) {;}

  digitalWrite(pin, HIGH);
  delay(33);
  digitalWrite(pin, LOW);
  delay(33);
  digitalWrite(pin, HIGH);
  delay(33);
  digitalWrite(pin, LOW);
}

void loop()
{
  if (Serial.available())
  {
    digitalWrite(pin, HIGH);
    String in = Serial.readStringUntil('\n');
    if (in = "Hello from Arduino!")
    {
      Serial.println("received '" + in + "' at esp8266"); 
      digitalWrite(pin, LOW); 
    }
  }
}

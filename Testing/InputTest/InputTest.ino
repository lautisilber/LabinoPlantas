String usrIn;

void setup() {
  Serial.begin(9600);
  Serial.println("init");
  
  usrIn = ReadSerial();
  Serial.print("read: ");
  Serial.println(usrIn);
}

void loop() {
}

String ReadSerial()
{
  while (Serial.available() == 0) { }
  return Serial.readString();
}

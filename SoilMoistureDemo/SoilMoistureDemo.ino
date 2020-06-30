
const int SoilMoisturePin = A0;

int soilMoistureValue = 0;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);

}

void loop() {
  soilMoistureValue = analogRead(SoilMoisturePin);
  Serial.print("Analog Value: ");
  Serial.println(soilMoistureValue);
  delay(500);
}

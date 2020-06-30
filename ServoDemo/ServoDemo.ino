#include <Servo.h>

Servo myServo;

const int servoPin = 9;

void setup() {
  Serial.begin(9600);
  myServo.attach(9);
}

void loop() {
  myServo.write(0);
  // Esperamos 1 segundo
  delay(1000);
  
  // Desplazamos a la posición 90º
  myServo.write(90);
  // Esperamos 1 segundo
  delay(1000);
  
  // Desplazamos a la posición 180º
  myServo.write(180);
  // Esperamos 1 segundo
  delay(1000);

}

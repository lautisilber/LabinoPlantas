#include <Servo.h>

Servo myServo;

const int servoPin = 9;

bool ServoSlow(Servo servo, int degree, int secs, int steps = 1);

void setup() {
  Serial.begin(9600);
  myServo.attach(9);

  ServoSlow(myServo, 10, 100, 5);
  Serial.println(myServo.read());

  ServoSlow(myServo, 170, 100, 5);
  Serial.println(myServo.read());
}

void loop() {
  
}

bool ServoSlow(Servo servo, int degree, int secs, int steps = 1)
{
  if (degree >= 180){degree = 179;}
  else if (degree <= 0){degree = 1;}
  int stepSign;
  delay(secs);
  int current = servo.read();
  if (degree > current){stepSign = 1 * steps;}
  else if (degree < current){stepSign = -1 * steps;}
  else{return true;}
  for (int i = 0; i < abs(degree - current); i+=abs(stepSign))
  {
    int s = servo.read() + stepSign;
    if (s < 1){s = 1;}
    else if (s > 179){s = 179;}
    servo.write(s);
    delay(secs);
  }
  return true;
}

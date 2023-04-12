#include <Servo.h>

Servo servo;
int potPin = A3; 
int potVal = 0;

void setup() {
  servo.attach(8);
  servo.write(0);
  Serial.begin(9600);
}

void loop() {
  potVal = analogRead(potPin);
  potVal = (potVal / 1023.0) * 180.0;
  Serial.println(potVal);
  servo.write(potVal);
  delay(20);

}

const int buzzer = 3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
}

void loop() {
  int A = analogRead(A5);
  int B = analogRead(A4);
  int C = analogRead(A3);
  int D = analogRead(A2);
  int E = analogRead(A1);
  int F = analogRead(A0);

  Serial.print("A = ");
  Serial.print(A);
  Serial.print(" B = ");
  Serial.print(B);
  Serial.print(" C = ");
  Serial.print(C);
  Serial.print(" D = ");
  Serial.print(D);
  Serial.print(" E = ");
  Serial.print(E);
  Serial.print(" F = ");
  Serial.println(F);

  if (A > 380) { 
    tone(buzzer, 440/2, 100);
  }
  else if (B > 380) {
    tone(buzzer,  494/2, 100);
  }
  else if (C > 360) {
    tone(buzzer,  523/2, 100);
  }
  else if(D > 340) {
    tone(buzzer,  587/2, 100);
  }
  else if(E > 330) {
    tone(buzzer,  659/2, 100);
  }
  else if(F > 370) {
    tone(buzzer,  698/2, 100);
  }

  delay(50);
}

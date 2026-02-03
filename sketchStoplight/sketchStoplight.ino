const int RED = 13;
const int YELLOW = 12;
const int GREEN = 11;
const int BUTTON = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BUTTON, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(BUTTON) == LOW){
    delay(1000);
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, HIGH);
    delay(3000);
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, HIGH);
    delay(1000);
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, HIGH);
  } else {
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, HIGH);
    digitalWrite(GREEN, HIGH);
  }
}

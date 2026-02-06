const int RED = 13;
const int YELLOW = 12;
const int GREEN = 11;
const int BUTTON = 2;
const int LONG_PAUSE = 3000;
const int SHORT_PAUSE = 1000;

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
    delay(SHORT_PAUSE);
    digitalWrite(GREEN, LOW);
    digitalWrite(RED, HIGH);
    delay(LONG_PAUSE);
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, HIGH);
    delay(SHORT_PAUSE);
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, HIGH);
  } else {
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, HIGH);
    digitalWrite(GREEN, HIGH);
  }
}

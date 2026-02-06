const int LED_PIN = 13;
const int BUTTON_FAST = 2;
const int BUTTON_SLOW = 3;
int wait = 1000;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_FAST, INPUT);
  pinMode(BUTTON_SLOW, INPUT);
}

void loop() {
  if(digitalRead(BUTTON_FAST) == LOW){
    wait = 500;
  } else if (digitalRead(BUTTON_SLOW) == LOW){
    wait = 2000;
  }
  
  digitalWrite(LED_PIN, check());
}

int check(){
  if((millis()/wait)%2 == 1){
    return LOW;
  } else {
    return HIGH;
  }
}

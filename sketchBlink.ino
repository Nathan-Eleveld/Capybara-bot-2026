const int ledpin = 13;
const int fast = 2;
const int slow = 3;
int wait = 1000;

void setup() {
  // put your setup code here, to run once:
  pinMode(ledpin, OUTPUT);
  pinMode(fast, INPUT);
  pinMode(slow, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(fast) == LOW){
    wait = 500;
  } else if (digitalRead(slow) == LOW){
    wait = 2000;
  }
  
  digitalWrite(ledpin, check());
}

int check(){
  if((millis()/wait)%2 == 1){
    return LOW;
  } else {
    return HIGH;
  }
}

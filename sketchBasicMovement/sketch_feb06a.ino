const int PIN_1A = 10;
const int PIN_2A = 9;
const int PIN_1B = 6;
const int PIN_2B = 5;
void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_1A, OUTPUT);
  pinMode(PIN_2A, OUTPUT);
  pinMode(PIN_1B, OUTPUT);
  pinMode(PIN_2B, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void forward(int angle){
  analogWrite(PIN_2A, 200);
  analogWrite(PIN_2B, 200);
}

void backward(int angle){
  analogWrite(PIN_1A, 200);
  analogWrite(PIN_1B, 200);
}

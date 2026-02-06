const int PIN_1A = 10;
const int PIN_2A = 9;
const int PIN_1B = 6;
const int PIN_2B = 5;
const int PIN_1R = 13;
const int PIN_2R = 12;

void setup() {
  pinMode(PIN_1A, OUTPUT);
  pinMode(PIN_2A, OUTPUT);
  pinMode(PIN_1B, OUTPUT);
  pinMode(PIN_2B, OUTPUT);
  pinMode(PIN_2R, INPUT);
  pinMode(PIN_2R, INPUT);
  Serial.begin(9600);
}

void loop() {
  forward(20);
  backward(20);
  
}

void forward(int distance){
  analogWrite(PIN_2A, 200);
  analogWrite(PIN_2B, 200);
  while (distance > 0){
    distance = distance - 1;
    Serial.println(distance);
    delay(100);
  }
  analogWrite(PIN_2A, 0);
  analogWrite(PIN_2B, 0);
}

void backward(int distance){
  analogWrite(PIN_1A, 200);
  analogWrite(PIN_1B, 200);
  while (distance > 0){
    distance = distance - 1;
    Serial.println(distance);
    delay(100);
  }
  analogWrite(PIN_1A, 0);
  analogWrite(PIN_1B, 0);
}

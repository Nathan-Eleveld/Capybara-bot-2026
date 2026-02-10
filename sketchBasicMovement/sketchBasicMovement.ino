const int PIN_1A = 10;
const int PIN_2A = 9;
const int PIN_1B = 6;
const int PIN_2B = 5;
const int PIN_1R = 13;
const int PIN_2R = 12;
const int GRIPPER = 11;
const byte WHEEL_SPEED = 255;
int lastRead = LOW;

void setup() {
  pinMode(PIN_1A, OUTPUT);
  pinMode(PIN_2A, OUTPUT);
  pinMode(PIN_1B, OUTPUT);
  pinMode(PIN_2B, OUTPUT);
  pinMode(PIN_1R, INPUT);
  pinMode(PIN_2R, INPUT);
  Serial.begin(9600);
}

void loop() {
  forward(40);
  backward(40);
  left(5);
  right(5);
}

void forward(int distance){
  analogWrite(PIN_2A, WHEEL_SPEED);
  analogWrite(PIN_2B, WHEEL_SPEED);
  while (distance > 0){
    if (digitalRead(PIN_1R) == HIGH && lastRead == LOW){
      distance = distance - 1;
      Serial.println(distance);
    }
    lastRead = digitalRead(PIN_1R);
  }
  analogWrite(PIN_2A, 0);
  analogWrite(PIN_2B, 0);
  delay(20);
}

void backward(int distance){
  analogWrite(PIN_1A, WHEEL_SPEED);
  analogWrite(PIN_1B, WHEEL_SPEED);
  while (distance > 0){
    distance = distance - 1;
    delay(100);
  }
  analogWrite(PIN_1A, 0);
  analogWrite(PIN_1B, 0);
  delay(20);
}

void left(int distance){
  analogWrite(PIN_1A, WHEEL_SPEED);
  analogWrite(PIN_2B, WHEEL_SPEED);
  while (distance > 0){
    distance = distance - 1;
    delay(100);
  }
  analogWrite(PIN_1A, 0);
  analogWrite(PIN_2B, 0);
  delay(20);
}


void right(int distance){
  analogWrite(PIN_2A, WHEEL_SPEED);
  analogWrite(PIN_1B, WHEEL_SPEED);
  while (distance > 0){
    distance = distance - 1;
    delay(100);
  }
  analogWrite(PIN_2A, 0);
  analogWrite(PIN_1B, 0);
  delay(20);
}

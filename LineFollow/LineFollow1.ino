// Motor pins
const int RIGHT_BACKWARD = 10;
const int RIGHT_FORWARD  = 9;
const int LEFT_BACKWARD  = 6;
const int LEFT_FORWARD   = 5;

// Encoder pins
const int RIGHT_IN = 2;
const int LEFT_IN  = 3;

const byte WHEEL_SPEED = 255;

// 8 lijn sensoren
const int SENSOR_PINS[8] = {A0,A1,A2,A3,A4,A5,A6,A7};
int sensorReadings[8];

volatile int rightPulses = 0;
volatile int leftPulses  = 0;

volatile unsigned long lastInterruptRight = 0;
volatile unsigned long lastInterruptLeft  = 0;

// lees sensoren → zwart = 1
void getReadings() {
  for (int i = 0; i < 8; i++) {
    sensorReadings[i] = analogRead(SENSOR_PINS[i]) > 700;
  }
}

// motor aansturing
void drive(int left, int right) {
  digitalWrite(LEFT_BACKWARD, left < 0);
  digitalWrite(RIGHT_BACKWARD, right < 0);
  analogWrite(LEFT_FORWARD, abs(left));
  analogWrite(RIGHT_FORWARD, abs(right));
}

void setup() {
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
}

void loop() {

  getReadings();

  int sum = 0;
  int count = 0;

  // bepaal positie van de lijn
  for (int i = 0; i < 8; i++) {
    if (sensorReadings[i]) {
      sum += i;
      count++;
    }
  }

  int error = 0;

  if (count > 0) {
    int position = sum / count;   // gemiddelde positie
    error = position - 3;         // midden sensoren = 3-4
  }

  int correction = error * 40;

  int leftSpeed  = WHEEL_SPEED - correction;
  int rightSpeed = WHEEL_SPEED + correction;

  drive(leftSpeed, rightSpeed);
}

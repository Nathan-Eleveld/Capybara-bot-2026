const int RIGHT_BACKWARD = 10;
const int RIGHT_FORWARD  = 9;
const int LEFT_BACKWARD  = 6;
const int LEFT_FORWARD   = 5;

const int RIGHT_IN = 2;
const int LEFT_IN  = 3;

const byte WHEEL_SPEED = 255;

const int SENSOR_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int sensorReadings[8];

volatile int rightPulses = 0;
volatile int leftPulses  = 0;

volatile unsigned long lastInterruptRight = 0;
volatile unsigned long lastInterruptLeft  = 0;

float lastError = 0.0; // laatste afwijking t.o.v. midden (voor terugvinden lijn)

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 8; i++) {
    pinMode(SENSOR_PINS[i], INPUT);
  }

  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD,  OUTPUT);
  pinMode(LEFT_BACKWARD,  OUTPUT);
  pinMode(LEFT_FORWARD,   OUTPUT);

  digitalWrite(RIGHT_BACKWARD, LOW);
  digitalWrite(RIGHT_FORWARD,  LOW);
  digitalWrite(LEFT_BACKWARD,  LOW);
  digitalWrite(LEFT_FORWARD,   LOW);

  pinMode(RIGHT_IN, INPUT_PULLUP);
  pinMode(LEFT_IN,  INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulseRightWheelISR, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN),  countPulseLeftWheelISR,  RISING);
}

void loop() {
  getReadings();
  drive();
}

// encoder pulsen tellen (20ms debounce)
void countPulseRightWheelISR() {
  unsigned long now = millis();
  if (now >= lastInterruptRight) {
    rightPulses++;
    lastInterruptRight = now + 20;
  }
}

void countPulseLeftWheelISR() {
  unsigned long now = millis();
  if (now >= lastInterruptLeft) {
    leftPulses++;
    lastInterruptLeft = now + 20;
  }
}

// 1 = zwarte lijn, 0 = geen lijn (threshold)
void getReadings() {
  for (int i = 0; i < 8; i++) {
    int raw = analogRead(SENSOR_PINS[i]);
    sensorReadings[i] = raw > 700;
  }
}

// gemiddelde positie van actieve sensoren (1..8), anders 0
int getAverageSensorPin() {
  float sum = 0.0;
  int count = 0;

  for (int i = 0; i < 8; i++) {
    if (sensorReadings[i]) {
      sum += i + 1;
      count++;
    }
  }

  if (count == 0) return 0;

  return (int)(sum / count + 0.5);
}

// motoren sturen zodat de lijn naar het midden (sensor 4/5) terugkomt
void drive() {
  const int BASE_SPEED = 220;  // basis vooruit snelheid
  const int KP = 60;           // hoe hard hij corrigeert

  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD,  0);

  int pos = getAverageSensorPin();

  if (pos == 0) {
    // lijn kwijt: zoeken in richting van laatste afwijking
    if (lastError < 0) {
      analogWrite(RIGHT_FORWARD, BASE_SPEED);
      analogWrite(LEFT_FORWARD,  0);
    } else {
      analogWrite(RIGHT_FORWARD, 0);
      analogWrite(LEFT_FORWARD,  BASE_SPEED);
    }
    return;
  }

  float error = pos - 4.5;     // midden tussen sensor 4 en 5
  lastError = error;

  int correction = (int)(KP * error);

  int rightSpeed = BASE_SPEED - correction;
  int leftSpeed  = BASE_SPEED + correction;

  rightSpeed = constrain(rightSpeed, 0, WHEEL_SPEED);
  leftSpeed  = constrain(leftSpeed,  0, WHEEL_SPEED);

  analogWrite(RIGHT_FORWARD, rightSpeed);
  analogWrite(LEFT_FORWARD,  leftSpeed);
}

// debug sensorwaarden
void printReadings() {
  Serial.println("---------------");
  for (int i = 0; i < 8; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(sensorReadings[i]);
  }
}

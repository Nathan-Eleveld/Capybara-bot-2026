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

void setup() {
  Serial.begin(9600); // seriële debug

  for (int i = 0; i < 8; i++) {
    pinMode(SENSOR_PINS[i], INPUT); // sensors
  }

  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD,  OUTPUT);
  pinMode(LEFT_BACKWARD,  OUTPUT);
  pinMode(LEFT_FORWARD,   OUTPUT);

  digitalWrite(RIGHT_BACKWARD, LOW); // motoren uit bij start
  digitalWrite(RIGHT_FORWARD,  LOW);
  digitalWrite(LEFT_BACKWARD,  LOW);
  digitalWrite(LEFT_FORWARD,   LOW);

  pinMode(RIGHT_IN, INPUT_PULLUP); // encoder rechts
  pinMode(LEFT_IN,  INPUT_PULLUP); // encoder links

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulseRightWheelISR, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN),  countPulseLeftWheelISR,  RISING);
}

void loop() {
  getReadings(); // sensoren lezen
  drive();       // direct sturen
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

// 1 = lijn, 0 = geen lijn (threshold)
void getReadings() {
  for (int i = 0; i < 8; i++) {
    int raw = analogRead(SENSOR_PINS[i]);
    sensorReadings[i] = raw > 700;
  }
}

// sneller centreren: agressiever sturen + geen stopMotors() elke loop
void drive() {
  // achteruit altijd uit tijdens normaal vooruit rijden
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD,  0);

  switch (getAverageSensorPin()) {
    case 1: writeWheels(1.0, 0.0); break; // harder terug naar de lijn
    case 2: writeWheels(1.0, 0.3); break; // sneller corrigeren
    case 3: writeWheels(1.0, 0.7); break;
    case 4: writeWheels(1.0, 1.0); break;
    case 5: writeWheels(0.7, 1.0); break;
    case 6: writeWheels(0.3, 1.0); break; // sneller corrigeren
    case 7: writeWheels(0.0, 1.0); break; // harder terug naar de lijn

    case 0:
      // lijn kwijt: vooruit uit, achteruit aan
      analogWrite(RIGHT_FORWARD, 0);
      analogWrite(LEFT_FORWARD,  0);
      analogWrite(RIGHT_BACKWARD, WHEEL_SPEED);
      analogWrite(LEFT_BACKWARD,  WHEEL_SPEED);
      break;
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

  return (int)(sum / count + 0.5); // afronden voor stabielere position
}

// vooruit snelheid per wiel met multiplier
void writeWheels(float multiplierRight, float multiplierLeft) {
  analogWrite(RIGHT_FORWARD, (int)(WHEEL_SPEED * multiplierRight));
  analogWrite(LEFT_FORWARD,  (int)(WHEEL_SPEED * multiplierLeft));
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

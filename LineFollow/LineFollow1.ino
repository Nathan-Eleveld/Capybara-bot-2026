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
  Serial.begin(9600); // start seriële communicatie

  for (int i = 0; i < 8; i++) {
    pinMode(SENSOR_PINS[i], INPUT); // lijnsensor pins als input
  }

  pinMode(RIGHT_BACKWARD, OUTPUT); // motor rechts achteruit
  pinMode(RIGHT_FORWARD, OUTPUT);  // motor rechts vooruit
  pinMode(LEFT_BACKWARD, OUTPUT);  // motor links achteruit
  pinMode(LEFT_FORWARD, OUTPUT);   // motor links vooruit

  digitalWrite(RIGHT_BACKWARD, LOW); // motor uit bij start
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(LEFT_BACKWARD, LOW);
  digitalWrite(LEFT_FORWARD, LOW);

  pinMode(RIGHT_IN, INPUT_PULLUP); // encoder rechts input
  pinMode(LEFT_IN, INPUT_PULLUP);  // encoder links input

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulseRightWheelISR, RISING); // interrupt rechter encoder
  attachInterrupt(digitalPinToInterrupt(LEFT_IN),  countPulseLeftWheelISR,  RISING); // interrupt linker encoder
}

void loop() {
  getReadings(); // lees sensoren
  drive();       // stuur motoren
}

// telt pulsen van de rechter encoder
// debounce van 20ms voorkomt dubbele pulsen door ruis
void countPulseRightWheelISR() {
  unsigned long now = millis();
  if (now >= lastInterruptRight) {
    rightPulses++;
    lastInterruptRight = now + 20;
  }
}

// telt pulsen van de linker encoder
// debounce van 20ms voorkomt dubbele pulsen door ruis
void countPulseLeftWheelISR() {
  unsigned long now = millis();
  if (now >= lastInterruptLeft) {
    leftPulses++;
    lastInterruptLeft = now + 20;
  }
}

// leest alle sensoren en zet waarde naar 0 of 1
// waarde onder 700 wordt gezien als lijn
void getReadings() {
  for (int i = 0; i < 8; i++) {
    int raw = analogRead(SENSOR_PINS[i]); // lees analoge sensorwaarde
    sensorReadings[i] = raw < 700;        // 1 = lijn, 0 = geen lijn
  }
}

// bepaalt motorsnelheid op basis van sensorpositie
// als geen sensor lijn ziet wordt achteruit gereden
void drive() {

  stopMotors();

  switch (getAverageSensorPin()) {

    case 1:
      writeWheels(1, 0.2); // minder extreem sturen
      break;

    case 2:
      writeWheels(1, 0.5);
      break;

    case 3:
      writeWheels(1, 0.8);
      break;

    case 4:
      writeWheels(1, 1);
      break;

    case 5:
      writeWheels(0.8, 1);
      break;

    case 6:
      writeWheels(0.5, 1);
      break;

    case 7:
      writeWheels(0.2, 1); // minder extreem sturen
      break;

    case 0:
      analogWrite(RIGHT_BACKWARD, WHEEL_SPEED);
      analogWrite(LEFT_BACKWARD,  WHEEL_SPEED);
      break;
  }
}

// berekent gemiddelde sensorpositie van alle sensoren die de lijn zien
// als geen sensor actief is wordt 0 teruggegeven
int getAverageSensorPin() {

  float sum = 0.0;
  int count = 0;

  for (int i = 0; i < 8; i++) {
    if (sensorReadings[i]) {   // gebruik sensoren die de lijn zien
      sum += i + 1;            // 1..8 positie
      count++;
    }
  }

  if (count == 0) {
    return 0;
  }

  return (int)(sum / count + 0.5); // afronden voor stabieler sturen
}

// schrijft motorsnelheid vooruit met multiplier voor sturen
void writeWheels(float multiplierRight, float multiplierLeft) {
  analogWrite(RIGHT_FORWARD, (int)(WHEEL_SPEED * multiplierRight));
  analogWrite(LEFT_FORWARD,  (int)(WHEEL_SPEED * multiplierLeft));
}

// zet alle motor outputs op 0
void stopMotors() {
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
}

// print sensorwaardes voor debug
void printReadings() {
  Serial.println("---------------");
  for (int i = 0; i < 8; i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(sensorReadings[i]);
  }
}

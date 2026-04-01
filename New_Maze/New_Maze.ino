const int RIGHT_BACKWARD = 6;
const int RIGHT_FORWARD = 5;
const int LEFT_BACKWARD = 10;
const int LEFT_FORWARD = 9;

const int ULTRA_SONIC_TRIG_FRONT = 8;
const int ULTRA_SONIC_ECHO_FRONT = 7;

const int ULTRA_SONIC_TRIG_LEFT = 4;
const int ULTRA_SONIC_ECHO_LEFT = 13;

const int RIGHT_IN = 2;
const int LEFT_IN = 3;
const int GRIPPER = 11;

// Afstanden
const int FRONT_BLOCKED = 14;
const int LEFT_WALL_NEAR = 8;
const int LEFT_WALL_FAR = 12;
const int LEFT_OPENING = 25;

// Snelheden
const int LEFT_BASE_SPEED = 150;
const int RIGHT_BASE_SPEED = 160;
const int TURN_SPEED = 160;

// Tijden
const int TURN_90_TIME = 370;
const int STOP_TIME = 80;
const int FORWARD_STEP_TIME = 90;
const int AFTER_TURN_FORWARD_TIME = 220;

// Sensor timeout
const unsigned long PULSE_TIMEOUT = 25000UL;

// Debug
unsigned long lastDebug = 0;

// New code
#define WHEEL_SPEED 255
const int   SENSOR_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};

int   rightPulses = 0;
int   lastInterruptRight = 0;
int   leftPulses = 0;
int   lastInterruptLeft = 0;
int   sensorBoundry[8] = {0,0,0,0,0,0,0,0};
int   sensorReadings[8];
int servoPulseWidth = 1100;
int servoOpen = 1500;
int servoClosed = 1100; 
unsigned long lastServoPulse = 0;

void setup() {
  Serial.begin(9600);

  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);

  pinMode(ULTRA_SONIC_TRIG_FRONT, OUTPUT);
  pinMode(ULTRA_SONIC_ECHO_FRONT, INPUT);

  pinMode(ULTRA_SONIC_TRIG_LEFT, OUTPUT);
  pinMode(ULTRA_SONIC_ECHO_LEFT, INPUT);

  pinMode(RIGHT_IN, INPUT);
  pinMode(LEFT_IN, INPUT);
  pinMode(GRIPPER, OUTPUT);

  digitalWrite(GRIPPER, LOW);
  stopMotors();
  delay(500);

//  New code
  clawOpen(true);

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulserightWheelISR, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN), countPulseleftWheelISR, RISING);

  calibrateBoundrys();
  getOnTrack();
}

void loop() {
//  New code
  updateServo();

// End New code

  
  int frontDistance = readDistanceCm(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  int leftDistance = readDistanceCm(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);

  if (millis() - lastDebug > 200) {
    Serial.print("Front: ");
    Serial.print(frontDistance);
    Serial.print(" cm | Left: ");
    Serial.print(leftDistance);
    Serial.println(" cm");
    lastDebug = millis();
  }

  bool leftOpen = isLeftOpenConfirmed();
  bool frontOpen = frontDistance > FRONT_BLOCKED;

  // links open = altijd linksaf
  if (leftOpen) {
    stopBrief();
    turnLeft90();
    driveStraightTimed(AFTER_TURN_FORWARD_TIME);
    return;
  }

  // links dicht, voor open = rechtdoor
  if (frontOpen) {
    driveCorridorStep(leftDistance);
    return;
  }

  // links dicht en voor dicht = rechtsaf
  stopBrief();
  turnRight90();
  driveStraightTimed(AFTER_TURN_FORWARD_TIME);
}

bool isLeftOpenConfirmed() {
  int a = readDistanceCm(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);
  delay(15);
  int b = readDistanceCm(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);
  delay(15);
  int c = readDistanceCm(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);

  int openCount = 0;
  if (a > LEFT_OPENING) openCount++;
  if (b > LEFT_OPENING) openCount++;
  if (c > LEFT_OPENING) openCount++;

  return openCount >= 2;
}

void driveCorridorStep(int leftDistance) {
  // te ver van de muur -> iets naar links
  if (leftDistance > LEFT_WALL_FAR) {
    drive(145, 165);
    delay(FORWARD_STEP_TIME);
    stopBrief();
    return;
  }

  // te dicht op de muur -> iets naar rechts
  if (leftDistance < LEFT_WALL_NEAR) {
    drive(160, 145);
    delay(FORWARD_STEP_TIME);
    stopBrief();
    return;
  }

  // mooi in het midden
  drive(LEFT_BASE_SPEED, RIGHT_BASE_SPEED);
  delay(FORWARD_STEP_TIME);
  stopBrief();
}

int readDistanceCm(int trigPin, int echoPin) {
  int a = singleReadDistanceCm(trigPin, echoPin);
  delay(4);
  int b = singleReadDistanceCm(trigPin, echoPin);
  delay(4);
  int c = singleReadDistanceCm(trigPin, echoPin);
  return median3(a, b, c);
}

int singleReadDistanceCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, PULSE_TIMEOUT);

  if (duration == 0) return 250;

  int distance = duration * 0.0343 / 2.0;

  if (distance < 2) distance = 2;
  if (distance > 250) distance = 250;

  return distance;
}

int median3(int a, int b, int c) {
  if ((a >= b && a <= c) || (a >= c && a <= b)) return a;
  if ((b >= a && b <= c) || (b >= c && b <= a)) return b;
  return c;
}

void drive(int leftSpeed, int rightSpeed) {
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  analogWrite(LEFT_FORWARD, leftSpeed);
  analogWrite(LEFT_BACKWARD, 0);

  analogWrite(RIGHT_FORWARD, rightSpeed);
  analogWrite(RIGHT_BACKWARD, 0);
}

void rotateLeft(int speed) {
  speed = constrain(speed, 0, 255);

  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, speed);

  analogWrite(RIGHT_FORWARD, speed);
  analogWrite(RIGHT_BACKWARD, 0);
}

void rotateRight(int speed) {
  speed = constrain(speed, 0, 255);

  analogWrite(LEFT_FORWARD, speed);
  analogWrite(LEFT_BACKWARD, 0);

  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, speed);
}

void stopMotors() {
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);

//  New code
  rightPulses = 0;
  leftPulses = 0;
}

void stopBrief() {
  stopMotors();
  delay(STOP_TIME);
}

void driveStraightTimed(int ms) {
  drive(LEFT_BASE_SPEED, RIGHT_BASE_SPEED);
  delay(ms);
  stopBrief();
}

void turnLeft90() {
  rotateLeft(TURN_SPEED);
  delay(TURN_90_TIME);
  stopBrief();
}

void turnRight90() {
  rotateRight(TURN_SPEED);
  delay(TURN_90_TIME);
  stopBrief();
}


// New code
void getOnTrack(){
  stopMotors();
  while(rightPulses < 10 && leftPulses < 10){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
    Serial.println(rightPulses);
  }
  stopMotors();
  while(leftPulses < 20){
    clawOpen(false);
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    Serial.println(leftPulses);
  }
  analogWrite(RIGHT_FORWARD, 0);
}

//Function called when right wheel rotates 1/20th of a rotation
void countPulserightWheelISR(){
  if(millis() > lastInterruptRight){
    rightPulses++;
    lastInterruptRight = millis() + 20;
  }
}

//Function called when left wheel rotates 1/20th of a rotation
void countPulseleftWheelISR(){
  if(millis() > lastInterruptLeft){
    leftPulses++;
    lastInterruptLeft = millis() + 20;
  }
}

void calibrateBoundrys(){
  int temp[8];
  int readingCount = 0;
  while(rightPulses < 10){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
    for(int i = 0; i < 8; i++){
      temp[i] = sensorBoundry[i] + analogRead(SENSOR_PINS[i]);
      sensorBoundry[i] = temp[i];
      Serial.println(sensorBoundry[i]);
    }
    readingCount++;
    Serial.println("---");
  }
   
  for(int i = 0; i < 8; i++){
    sensorBoundry[i] = temp[i] / readingCount;
    Serial.println(sensorBoundry[i]);
  }
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, 0);
  delay(200);
}

int getAverageSensorPin(){
  float medianReading = 0.0;
  int readingCount = 0;
  for(int i = 0; i < 8; i++){
    if(!sensorReadings[i]){
      medianReading += i+ 1;
      readingCount++;
    }
  }  
  
  return medianReading / readingCount;
}

void updateServo() {
  if (micros() - lastServoPulse >= 20000) {
    lastServoPulse = micros();

    digitalWrite(GRIPPER, HIGH);
    delayMicroseconds(servoPulseWidth);
    digitalWrite(GRIPPER, LOW);
  }
}

void clawOpen(bool open) {
  if (open) {
    servoPulseWidth = servoOpen;
    updateServo();
  } else {
    servoPulseWidth = servoClosed;
    updateServo();
  }
}

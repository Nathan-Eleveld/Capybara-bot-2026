const int RIGHT_BACKWARD = 6;
const int RIGHT_FORWARD  = 5;
const int LEFT_BACKWARD  = 10;
const int LEFT_FORWARD   = 9;

const int ULTRA_SONIC_TRIG_FRONT = 8;
const int ULTRA_SONIC_ECHO_FRONT = 7;

const int ULTRA_SONIC_TRIG_LEFT  = 4;
const int ULTRA_SONIC_ECHO_LEFT  = 13;

const int RIGHT_IN = 2;
const int LEFT_IN  = 3;
const int GRIPPER  = 11;

// Afstanden
const int LEFT_MIN_TARGET = 12;
const int LEFT_MAX_TARGET = 15;
const int FRONT_BLOCKED   = 18;
const int LEFT_OPENING    = 20;

// Snelheden
const int BASE_SPEED      = 150;
const int TURN_SPEED      = 165;
const int CORRECT_SPEED   = 145;
const int STRONG_SPEED    = 175;

// Tijden (afhankelijk van chassis aanpassen)
const int TURN_90_TIME    = 390;
const int TURN_180_TIME   = 780;
const int BACKUP_TIME     = 220;
const int STOP_TIME       = 80;

// Sensor timeout
const unsigned long PULSE_TIMEOUT = 25000UL;

// Debug
unsigned long lastDebug = 0;

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
}

void loop() {
  int frontDistance = readDistanceCm(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  int leftDistance  = readDistanceCm(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);

  if (millis() - lastDebug > 200) {
    Serial.print("Front: ");
    Serial.print(frontDistance);
    Serial.print(" cm | Left: ");
    Serial.print(leftDistance);
    Serial.println(" cm");
    lastDebug = millis();
  }

  // 1. Als vooruit vrij is: altijd vooruit rijden
  if (frontDistance > FRONT_BLOCKED) {
    followLeftWall(leftDistance);
    return;
  }

  // 2. Voor geblokkeerd: eerst links proberen als daar opening is
  stopBrief();

  if (leftDistance > LEFT_OPENING) {
    backupShort();
    turnLeft90();
    return;
  }

  // 3. Voor dicht en links dicht:
  // probeer rechts door een kwartslag rechts te draaien
  backupShort();
  turnRight90();
  delay(60);

  frontDistance = readDistanceCm(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  if (frontDistance > FRONT_BLOCKED) {
    return;
  }

  // 4. Nog steeds vast: nog een kwartslag rechts = 180 vanaf originele richting
  backupShort();
  turnRight90();
  delay(60);

  frontDistance = readDistanceCm(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  if (frontDistance > FRONT_BLOCKED) {
    return;
  }

  // 5. Echt vast: nog stukje achteruit en opnieuw 180 proberen
  backupLong();
  turnAround180();
  delay(60);
}

int readDistanceCm(int trigPin, int echoPin) {
  int a = singleReadDistanceCm(trigPin, echoPin);
  delay(5);
  int b = singleReadDistanceCm(trigPin, echoPin);
  delay(5);
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

  // Geen echo = behandel als ver/open
  if (duration == 0) {
    return 250;
  }

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

// Houd de linkermuur tussen 12 en 15 cm
void followLeftWall(int leftDistance) {
  // Linkermuur kwijt? rustig links zoeken maar wel vooruit
  if (leftDistance > LEFT_OPENING) {
    drive(CORRECT_SPEED, STRONG_SPEED);
    delay(40);
    return;
  }

  // Te dicht op de linkermuur -> iets naar rechts sturen
  if (leftDistance < LEFT_MIN_TARGET) {
    drive(STRONG_SPEED, CORRECT_SPEED);
    delay(35);
    return;
  }

  // Te ver van de linkermuur -> iets naar links sturen
  if (leftDistance > LEFT_MAX_TARGET) {
    drive(CORRECT_SPEED, STRONG_SPEED);
    delay(35);
    return;
  }

  // Mooie afstand -> rechtdoor
  drive(BASE_SPEED, BASE_SPEED);
  delay(35);
}

void drive(int leftSpeed, int rightSpeed) {
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  analogWrite(LEFT_FORWARD, leftSpeed);
  analogWrite(LEFT_BACKWARD, 0);

  analogWrite(RIGHT_FORWARD, rightSpeed);
  analogWrite(RIGHT_BACKWARD, 0);
}

void driveBackward(int leftSpeed, int rightSpeed) {
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, leftSpeed);

  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, rightSpeed);
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
}

void stopBrief() {
  stopMotors();
  delay(STOP_TIME);
}

void backupShort() {
  driveBackward(145, 145);
  delay(BACKUP_TIME);
  stopBrief();
}

void backupLong() {
  driveBackward(155, 155);
  delay(BACKUP_TIME + 120);
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

void turnAround180() {
  rotateRight(TURN_SPEED);
  delay(TURN_180_TIME);
  stopBrief();
}

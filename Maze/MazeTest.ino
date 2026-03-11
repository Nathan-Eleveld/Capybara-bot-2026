const int RIGHT_BACKWARD = 10;
const int RIGHT_FORWARD = 9;
const int LEFT_BACKWARD = 6;
const int LEFT_FORWARD = 5;

const int ULTRA_SONIC_TRIG_FRONT = 8;
const int ULTRA_SONIC_ECHO_FRONT = 7;

const int ULTRA_SONIC_TRIG_LEFT = 4;
const int ULTRA_SONIC_ECHO_LEFT = 13;

const int ULTRA_SONIC_TRIG_RIGHT = 12;
const int ULTRA_SONIC_ECHO_RIGHT = A0;

const int RIGHT_IN = 2;
const int LEFT_IN = 3;
const int GRIPPER = 11;

const int BASE_SPEED = 160;
const int TURN_SPEED = 170;
const int BACK_SPEED = 140;

const int FRONT_OPEN_DISTANCE = 16;
const int SIDE_OPEN_DISTANCE = 18;

const int DESIRED_LEFT_DISTANCE = 10;
const int LEFT_MARGIN = 2;

unsigned long lastDebug = 0;
unsigned long actionUntil = 0;

String action = "STOP";
int correctionCount = 0;

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

  pinMode(ULTRA_SONIC_TRIG_RIGHT, OUTPUT);
  pinMode(ULTRA_SONIC_ECHO_RIGHT, INPUT);

  pinMode(RIGHT_IN, INPUT);
  pinMode(LEFT_IN, INPUT);
  pinMode(GRIPPER, OUTPUT);

  stopMotors();
}

void loop() {
  int frontDistance = getStableDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  int leftDistance = getStableDistance(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);
  int rightDistance = getStableDistance(ULTRA_SONIC_TRIG_RIGHT, ULTRA_SONIC_ECHO_RIGHT);

  bool frontOpen = frontDistance > FRONT_OPEN_DISTANCE;
  bool leftOpen = leftDistance > SIDE_OPEN_DISTANCE;
  bool rightOpen = rightDistance > SIDE_OPEN_DISTANCE;

  if (millis() < actionUntil) {
    debugSensors(frontDistance, leftDistance, rightDistance, action, correctionCount);
    return;
  }

  // Als hij te vaak heen en weer blijft corrigeren
  if (correctionCount >= 5) {
    action = "180 DOOR BLIJVEN CORRIGEREN";
    backUpAndCorrect();
    turnAround();
    correctionCount = 0;
    actionUntil = millis() + 350;
    debugSensors(frontDistance, leftDistance, rightDistance, action, correctionCount);
    return;
  }

  // Maze logica:
  // links vrij = linksaf
  // anders voor vrij = vooruit
  // anders rechts vrij = rechtsaf
  // anders 180 graden
  if (leftOpen) {
    action = "LEFT OPEN -> LINKS";
    backUpAndCorrect();
    turnLeft();
    correctionCount = 0;
    actionUntil = millis() + 250;
  }
  else if (frontOpen) {
    if (!leftOpen && leftDistance <= SIDE_OPEN_DISTANCE) {
      followLeftWall(leftDistance);
    } else {
      action = "VOORUIT";
      driveForward(BASE_SPEED, BASE_SPEED);
      correctionCount = 0;
    }
  }
  else if (rightOpen) {
    action = "RIGHT OPEN -> RECHTS";
    backUpAndCorrect();
    turnRight();
    correctionCount = 0;
    actionUntil = millis() + 250;
  }
  else {
    action = "ALLES DICHT -> 180";
    backUpAndCorrect();
    turnAround();
    correctionCount = 0;
    actionUntil = millis() + 350;
  }

  debugSensors(frontDistance, leftDistance, rightDistance, action, correctionCount);
}

int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);

  if (duration == 0) return 250;

  int distance = duration * 0.034 / 2;

  if (distance < 2 || distance > 250) return 250;

  return distance;
}

int getStableDistance(int trigPin, int echoPin) {
  int a = getDistance(trigPin, echoPin);
  delay(5);
  int b = getDistance(trigPin, echoPin);
  delay(5);
  int c = getDistance(trigPin, echoPin);

  int values[3] = {a, b, c};

  for (int i = 0; i < 2; i++) {
    for (int j = i + 1; j < 3; j++) {
      if (values[j] < values[i]) {
        int temp = values[i];
        values[i] = values[j];
        values[j] = temp;
      }
    }
  }

  return values[1];
}

// Kort achteruit voor ruimte
void backUpAndCorrect() {
  action = "BACK UP";
  driveBackward(BACK_SPEED, BACK_SPEED);
  delay(180);
  stopMotors();
  delay(60);
}

// Linkermuur volgen als links dicht is en voor open
void followLeftWall(int leftDistance) {
  if (leftDistance < DESIRED_LEFT_DISTANCE - LEFT_MARGIN) {
    action = "BIJSTUREN RECHTS";
    driveForward(BASE_SPEED - 45, BASE_SPEED + 25);
    correctionCount++;
  }
  else if (leftDistance > DESIRED_LEFT_DISTANCE + LEFT_MARGIN) {
    action = "BIJSTUREN LINKS";
    driveForward(BASE_SPEED + 25, BASE_SPEED - 45);
    correctionCount++;
  }
  else {
    action = "MOOI RECHT";
    driveForward(BASE_SPEED, BASE_SPEED);
    correctionCount = 0;
  }
}

void driveForward(int leftSpeed, int rightSpeed) {
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

void stopMotors() {
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
}

void turnLeft() {
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, TURN_SPEED);

  analogWrite(RIGHT_FORWARD, TURN_SPEED);
  analogWrite(RIGHT_BACKWARD, 0);

  delay(230);
  stopMotors();
  delay(60);
}

void turnRight() {
  analogWrite(LEFT_FORWARD, TURN_SPEED);
  analogWrite(LEFT_BACKWARD, 0);

  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, TURN_SPEED);

  delay(230);
  stopMotors();
  delay(60);
}

void turnAround() {
  analogWrite(LEFT_FORWARD, TURN_SPEED);
  analogWrite(LEFT_BACKWARD, 0);

  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, TURN_SPEED);

  delay(460);
  stopMotors();
  delay(80);
}

void debugSensors(int frontDistance, int leftDistance, int rightDistance, String currentAction, int currentCorrectionCount) {
  if (millis() - lastDebug < 200) return;
  lastDebug = millis();

  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.print(" cm | Left: ");
  Serial.print(leftDistance);
  Serial.print(" cm | Right: ");
  Serial.print(rightDistance);
  Serial.print(" cm | Action: ");
  Serial.print(currentAction);
  Serial.print(" | Corrections: ");
  Serial.println(currentCorrectionCount);
}

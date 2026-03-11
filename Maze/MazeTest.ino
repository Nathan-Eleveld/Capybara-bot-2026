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

const int FRONT_BLOCKED_DISTANCE = 14;
const int SIDE_WALL_DISTANCE = 22;

const int DESIRED_LEFT_DISTANCE = 10;
const int LEFT_MARGIN = 2;

unsigned long lastDebug = 0;
unsigned long turnUntil = 0;
String action = "STOP";

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

  bool frontBlocked = frontDistance <= FRONT_BLOCKED_DISTANCE;
  bool leftWall = leftDistance <= SIDE_WALL_DISTANCE;
  bool rightWall = rightDistance <= SIDE_WALL_DISTANCE;

  if (millis() < turnUntil) {
    debugSensors(frontDistance, leftDistance, rightDistance, action);
    return;
  }

  if (frontBlocked) {
    stopMotors();
    delay(50);

    frontDistance = getStableDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
    leftDistance = getStableDistance(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);
    rightDistance = getStableDistance(ULTRA_SONIC_TRIG_RIGHT, ULTRA_SONIC_ECHO_RIGHT);

    frontBlocked = frontDistance <= FRONT_BLOCKED_DISTANCE;
    leftWall = leftDistance <= SIDE_WALL_DISTANCE;
    rightWall = rightDistance <= SIDE_WALL_DISTANCE;

    if (frontBlocked) {
      backUpAndCorrect();

      if (leftWall) {
        action = "BACK + TURN RIGHT";
        turnRight();
        turnUntil = millis() + 250;
      } else {
        action = "BACK + TURN LEFT";
        turnLeft();
        turnUntil = millis() + 250;
      }
    } else {
      action = "FORWARD";
      driveForward(BASE_SPEED, BASE_SPEED);
    }
  } else {
    if (leftWall) {
      followLeftWall(leftDistance);
    } else {
      action = "FORWARD OPEN LEFT";
      driveForward(BASE_SPEED, BASE_SPEED);
    }
  }

  debugSensors(frontDistance, leftDistance, rightDistance, action);
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

// Kort achteruit zodat hij ruimte maakt voor de bocht
void backUpAndCorrect() {
  action = "BACK UP";
  driveBackward(BACK_SPEED, BACK_SPEED);
  delay(180);
  stopMotors();
  delay(60);
}

// Houdt de linkermuur op ongeveer dezelfde afstand
void followLeftWall(int leftDistance) {
  if (leftDistance < DESIRED_LEFT_DISTANCE - LEFT_MARGIN) {
    action = "ADJUST RIGHT";
    driveForward(BASE_SPEED - 45, BASE_SPEED + 25);
  } else if (leftDistance > DESIRED_LEFT_DISTANCE + LEFT_MARGIN) {
    action = "ADJUST LEFT";
    driveForward(BASE_SPEED + 25, BASE_SPEED - 45);
  } else {
    action = "CENTER FORWARD";
    driveForward(BASE_SPEED, BASE_SPEED);
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

void turnRight() {
  analogWrite(LEFT_FORWARD, TURN_SPEED);
  analogWrite(LEFT_BACKWARD, 0);

  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, TURN_SPEED);

  delay(240);
  stopMotors();
  delay(60);
}

void turnLeft() {
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, TURN_SPEED);

  analogWrite(RIGHT_FORWARD, TURN_SPEED);
  analogWrite(RIGHT_BACKWARD, 0);

  delay(240);
  stopMotors();
  delay(60);
}

void debugSensors(int frontDistance, int leftDistance, int rightDistance, String currentAction) {
  if (millis() - lastDebug < 200) return;
  lastDebug = millis();

  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.print(" cm | Left: ");
  Serial.print(leftDistance);
  Serial.print(" cm | Right: ");
  Serial.print(rightDistance);
  Serial.print(" cm | Action: ");
  Serial.println(currentAction);
}

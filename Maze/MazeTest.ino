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

const int BASE_SPEED = 170;
const int TURN_SPEED = 180;

const int FRONT_STOP_DISTANCE = 18;
const int SIDE_WALL_DISTANCE = 25;
const int DESIRED_LEFT_DISTANCE = 12;
const int LEFT_TOLERANCE = 3;

unsigned long lastDebugTime = 0;
String currentAction = "STOP";

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
  int frontDistance = getDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  int leftDistance = getDistance(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);
  int rightDistance = getDistance(ULTRA_SONIC_TRIG_RIGHT, ULTRA_SONIC_ECHO_RIGHT);

  bool frontBlocked = frontDistance > 0 && frontDistance < FRONT_STOP_DISTANCE;
  bool leftWall = leftDistance > 0 && leftDistance < SIDE_WALL_DISTANCE;
  bool rightWall = rightDistance > 0 && rightDistance < SIDE_WALL_DISTANCE;

  if (frontBlocked) {
    stopMotors();
    delay(80);

    frontDistance = getDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
    leftDistance = getDistance(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);
    rightDistance = getDistance(ULTRA_SONIC_TRIG_RIGHT, ULTRA_SONIC_ECHO_RIGHT);

    frontBlocked = frontDistance > 0 && frontDistance < FRONT_STOP_DISTANCE;
    leftWall = leftDistance > 0 && leftDistance < SIDE_WALL_DISTANCE;
    rightWall = rightDistance > 0 && rightDistance < SIDE_WALL_DISTANCE;

    if (frontBlocked) {
      if (leftWall) {
        currentAction = "RECHTS DRAAIEN";
        turnRight();
      } else {
        currentAction = "LINKS DRAAIEN";
        turnLeft();
      }
    }
  } else {
    if (leftWall) {
      followLeftWall(leftDistance);
    } else {
      currentAction = "VOORUIT GEEN LINKERMUUR";
      driveForward(BASE_SPEED, BASE_SPEED);
    }
  }

  debugSensors(frontDistance, leftDistance, rightDistance, frontBlocked, leftWall, rightWall);
}

int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);

  if (duration == 0) {
    return 999;
  }

  int distance = duration * 0.034 / 2;

  if (distance <= 0 || distance > 400) {
    return 999;
  }

  return distance;
}

// Houdt de linker muur op ongeveer dezelfde afstand
void followLeftWall(int leftDistance) {
  if (leftDistance == 999) {
    currentAction = "VOORUIT SENSOR LINKS GEEN DATA";
    driveForward(BASE_SPEED, BASE_SPEED);
    return;
  }

  if (leftDistance < DESIRED_LEFT_DISTANCE - LEFT_TOLERANCE) {
    currentAction = "BIJSTUREN RECHTS";
    driveForward(BASE_SPEED - 45, BASE_SPEED + 35);
  } 
  else if (leftDistance > DESIRED_LEFT_DISTANCE + LEFT_TOLERANCE) {
    currentAction = "BIJSTUREN LINKS";
    driveForward(BASE_SPEED + 35, BASE_SPEED - 45);
  } 
  else {
    currentAction = "NETJES VOORUIT";
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

  delay(350);
  stopMotors();
  delay(80);
}

void turnLeft() {
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, TURN_SPEED);

  analogWrite(RIGHT_FORWARD, TURN_SPEED);
  analogWrite(RIGHT_BACKWARD, 0);

  delay(350);
  stopMotors();
  delay(80);
}

void debugSensors(int frontDistance, int leftDistance, int rightDistance, bool frontBlocked, bool leftWall, bool rightWall) {
  if (millis() - lastDebugTime < 250) return;
  lastDebugTime = millis();

  Serial.print("VOOR: ");
  Serial.print(frontDistance);
  Serial.print(" cm | LINKS: ");
  Serial.print(leftDistance);
  Serial.print(" cm | RECHTS: ");
  Serial.print(rightDistance);
  Serial.print(" cm | frontBlocked: ");
  Serial.print(frontBlocked);
  Serial.print(" | leftWall: ");
  Serial.print(leftWall);
  Serial.print(" | rightWall: ");
  Serial.print(rightWall);
  Serial.print(" | ACTIE: ");
  Serial.println(currentAction);
}

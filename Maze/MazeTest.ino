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

const int FRONT_BLOCKED_DISTANCE = 14;
const int SIDE_WALL_MAX_DISTANCE = 25;

const int TARGET_LEFT_DISTANCE = 12;
const int TARGET_RIGHT_DISTANCE = 12;
const int TARGET_CENTER_DISTANCE = 12;

const int BASE_SPEED = 155;
const int SMALL_CORRECTION = 25;
const int MEDIUM_CORRECTION = 45;
const int STRONG_CORRECTION = 65;

const int TURN_SPEED = 170;
const int TURN_STEP_DELAY = 35;
const int MAX_TURN_TIME = 1400;

void setup() {
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

  Serial.begin(9600);
}

void loop() {
  int frontDistance = getStableDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  int leftDistance = getStableDistance(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);
  int rightDistance = getStableDistance(ULTRA_SONIC_TRIG_RIGHT, ULTRA_SONIC_ECHO_RIGHT);

  bool frontOpen = frontDistance == 999 || frontDistance > FRONT_BLOCKED_DISTANCE;
  bool leftWall = leftDistance != 999 && leftDistance <= SIDE_WALL_MAX_DISTANCE;
  bool rightWall = rightDistance != 999 && rightDistance <= SIDE_WALL_MAX_DISTANCE;

  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.print("  Left: ");
  Serial.print(leftDistance);
  Serial.print("  Right: ");
  Serial.println(rightDistance);

  if (frontOpen) {
    moveForwardWithWallControl(leftDistance, rightDistance, leftWall, rightWall);
  } else {
    stopMotors();
    delay(60);

    if (!leftWall) {
      turnLeftUntilFrontOpen();
    } else if (!rightWall) {
      turnRightUntilFrontOpen();
    } else {
      turnAroundUntilFrontOpen();
    }
  }

  delay(20);
}

int getStableDistance(int trigPin, int echoPin) {
  int a = measureDistance(trigPin, echoPin);
  delay(3);
  int b = measureDistance(trigPin, echoPin);
  delay(3);
  int c = measureDistance(trigPin, echoPin);

  if (a == 999 && b == 999 && c == 999) return 999;
  if (a == 999) a = b;
  if (b == 999) b = c;
  if (c == 999) c = a;

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

int measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) return 999;

  int distance = duration * 0.034 / 2;

  if (distance < 2 || distance > 250) return 999;

  return distance;
}

void moveForwardWithWallControl(int leftDistance, int rightDistance, bool leftWall, bool rightWall) {
  int rightSpeed = BASE_SPEED;
  int leftSpeed = BASE_SPEED;

  if (leftWall && rightWall) {
    int error = leftDistance - rightDistance;

    if (error > 6) {
      rightSpeed = BASE_SPEED - STRONG_CORRECTION;
      leftSpeed = BASE_SPEED + STRONG_CORRECTION;
    } else if (error > 3) {
      rightSpeed = BASE_SPEED - MEDIUM_CORRECTION;
      leftSpeed = BASE_SPEED + MEDIUM_CORRECTION;
    } else if (error > 1) {
      rightSpeed = BASE_SPEED - SMALL_CORRECTION;
      leftSpeed = BASE_SPEED + SMALL_CORRECTION;
    } else if (error < -6) {
      rightSpeed = BASE_SPEED + STRONG_CORRECTION;
      leftSpeed = BASE_SPEED - STRONG_CORRECTION;
    } else if (error < -3) {
      rightSpeed = BASE_SPEED + MEDIUM_CORRECTION;
      leftSpeed = BASE_SPEED - MEDIUM_CORRECTION;
    } else if (error < -1) {
      rightSpeed = BASE_SPEED + SMALL_CORRECTION;
      leftSpeed = BASE_SPEED - SMALL_CORRECTION;
    }
  } 
  else if (leftWall) {
    int error = leftDistance - TARGET_LEFT_DISTANCE;

    if (error > 6) {
      rightSpeed = BASE_SPEED - STRONG_CORRECTION;
      leftSpeed = BASE_SPEED + STRONG_CORRECTION;
    } else if (error > 3) {
      rightSpeed = BASE_SPEED - MEDIUM_CORRECTION;
      leftSpeed = BASE_SPEED + MEDIUM_CORRECTION;
    } else if (error > 1) {
      rightSpeed = BASE_SPEED - SMALL_CORRECTION;
      leftSpeed = BASE_SPEED + SMALL_CORRECTION;
    } else if (error < -6) {
      rightSpeed = BASE_SPEED + STRONG_CORRECTION;
      leftSpeed = BASE_SPEED - STRONG_CORRECTION;
    } else if (error < -3) {
      rightSpeed = BASE_SPEED + MEDIUM_CORRECTION;
      leftSpeed = BASE_SPEED - MEDIUM_CORRECTION;
    } else if (error < -1) {
      rightSpeed = BASE_SPEED + SMALL_CORRECTION;
      leftSpeed = BASE_SPEED - SMALL_CORRECTION;
    }
  } 
  else if (rightWall) {
    int error = rightDistance - TARGET_RIGHT_DISTANCE;

    if (error > 6) {
      rightSpeed = BASE_SPEED + STRONG_CORRECTION;
      leftSpeed = BASE_SPEED - STRONG_CORRECTION;
    } else if (error > 3) {
      rightSpeed = BASE_SPEED + MEDIUM_CORRECTION;
      leftSpeed = BASE_SPEED - MEDIUM_CORRECTION;
    } else if (error > 1) {
      rightSpeed = BASE_SPEED + SMALL_CORRECTION;
      leftSpeed = BASE_SPEED - SMALL_CORRECTION;
    } else if (error < -6) {
      rightSpeed = BASE_SPEED - STRONG_CORRECTION;
      leftSpeed = BASE_SPEED + STRONG_CORRECTION;
    } else if (error < -3) {
      rightSpeed = BASE_SPEED - MEDIUM_CORRECTION;
      leftSpeed = BASE_SPEED + MEDIUM_CORRECTION;
    } else if (error < -1) {
      rightSpeed = BASE_SPEED - SMALL_CORRECTION;
      leftSpeed = BASE_SPEED + SMALL_CORRECTION;
    }
  }

  driveForward(rightSpeed, leftSpeed);
}

void driveForward(int rightSpeed, int leftSpeed) {
  if (rightSpeed < 0) rightSpeed = 0;
  if (leftSpeed < 0) leftSpeed = 0;

  if (rightSpeed > 255) rightSpeed = 255;
  if (leftSpeed > 255) leftSpeed = 255;

  analogWrite(RIGHT_FORWARD, rightSpeed);
  analogWrite(LEFT_FORWARD, leftSpeed);
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
}

void stopMotors() {
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
}

void turnLeft() {
  analogWrite(RIGHT_FORWARD, TURN_SPEED);
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD, TURN_SPEED);
}

void turnRight() {
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, TURN_SPEED);
  analogWrite(RIGHT_BACKWARD, TURN_SPEED);
  analogWrite(LEFT_BACKWARD, 0);
}

void turnAround() {
  analogWrite(RIGHT_FORWARD, TURN_SPEED);
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD, TURN_SPEED);
}

void turnLeftUntilFrontOpen() {
  unsigned long startTime = millis();

  while (millis() - startTime < MAX_TURN_TIME) {
    turnLeft();
    delay(TURN_STEP_DELAY);

    int frontDistance = measureDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
    if (frontDistance == 999 || frontDistance > FRONT_BLOCKED_DISTANCE) {
      break;
    }
  }

  stopMotors();
  delay(60);
}

void turnRightUntilFrontOpen() {
  unsigned long startTime = millis();

  while (millis() - startTime < MAX_TURN_TIME) {
    turnRight();
    delay(TURN_STEP_DELAY);

    int frontDistance = measureDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
    if (frontDistance == 999 || frontDistance > FRONT_BLOCKED_DISTANCE) {
      break;
    }
  }

  stopMotors();
  delay(60);
}

void turnAroundUntilFrontOpen() {
  unsigned long startTime = millis();

  while (millis() - startTime < MAX_TURN_TIME + 600) {
    turnAround();
    delay(TURN_STEP_DELAY);

    int frontDistance = measureDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
    if (frontDistance == 999 || frontDistance > FRONT_BLOCKED_DISTANCE) {
      break;
    }
  }

  stopMotors();
  delay(60);
}

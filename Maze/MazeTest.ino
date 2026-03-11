const int RIGHT_BACKWARD = 10;
const int RIGHT_FORWARD = 9;

const int ULTRA_SONIC_TRIG_FRONT = 8;
const int ULTRA_SONIC_ECHO_FRONT = 7;

const int ULTRA_SONIC_TRIG_LEFT = 4;
const int ULTRA_SONIC_ECHO_LEFT = 13;

const int LEFT_BACKWARD = 6;
const int LEFT_FORWARD = 5;

const int RIGHT_IN = 2;
const int LEFT_IN = 3;
const int GRIPPER = 11;

const int FRONT_WALL_DISTANCE = 14;
const int LEFT_WALL_DISTANCE = 12;
const int LEFT_TOO_CLOSE = 8;
const int LEFT_TOO_FAR = 18;

const int BASE_SPEED = 160;
const int STRONG_CORRECTION = 70;
const int SMALL_CORRECTION = 35;

const int TURN_SPEED = 170;
const int TURN_TIME_LEFT = 420;
const int TURN_TIME_RIGHT = 420;

void setup() {
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

  Serial.begin(9600);
}

void loop() {
  int frontDistance = getStableDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  int leftDistance = getStableDistance(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);

  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.print("  Left: ");
  Serial.println(leftDistance);

  // Vooruit heeft altijd prioriteit
  if (frontDistance == 999 || frontDistance > FRONT_WALL_DISTANCE) {
    followLeftWall(leftDistance);
  } else {
    stopMotors();
    delay(60);

    // Voor dicht + links dicht = rechts
    if (leftDistance != 999 && leftDistance <= LEFT_TOO_FAR) {
      turnRightUntilFrontIsFree();
    } 
    // Voor dicht + links open = links
    else {
      turnLeftUntilFrontIsFree();
    }
  }

  delay(25);
}

int getStableDistance(int trigPin, int echoPin) {
  int a = measureDistance(trigPin, echoPin);
  delay(5);
  int b = measureDistance(trigPin, echoPin);
  delay(5);
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

  if (duration == 0) {
    return 999;
  }

  int distance = duration * 0.034 / 2;

  if (distance < 2 || distance > 250) {
    return 999;
  }

  return distance;
}

// Houdt ongeveer dezelfde afstand tot de linkermuur
void followLeftWall(int leftDistance) {
  int rightSpeed = BASE_SPEED;
  int leftSpeed = BASE_SPEED;

  // Geen linkermuur gezien: gewoon vooruit
  if (leftDistance == 999) {
    driveForward(BASE_SPEED, BASE_SPEED);
    return;
  }

  // Te dicht op linkermuur: stuur hard naar rechts
  if (leftDistance <= LEFT_TOO_CLOSE) {
    rightSpeed = BASE_SPEED + STRONG_CORRECTION;
    leftSpeed = BASE_SPEED - STRONG_CORRECTION;
  }
  // Te ver van linkermuur: stuur naar links
  else if (leftDistance >= LEFT_TOO_FAR) {
    rightSpeed = BASE_SPEED - STRONG_CORRECTION;
    leftSpeed = BASE_SPEED + STRONG_CORRECTION;
  }
  // Kleine correctie rond gewenste afstand
  else if (leftDistance < LEFT_WALL_DISTANCE) {
    rightSpeed = BASE_SPEED + SMALL_CORRECTION;
    leftSpeed = BASE_SPEED - SMALL_CORRECTION;
  }
  else if (leftDistance > LEFT_WALL_DISTANCE) {
    rightSpeed = BASE_SPEED - SMALL_CORRECTION;
    leftSpeed = BASE_SPEED + SMALL_CORRECTION;
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

// Draai links tot voor weer vrij is
void turnLeftUntilFrontIsFree() {
  unsigned long startTime = millis();

  while (millis() - startTime < 1200) {
    turnLeft();
    delay(40);

    int frontDistance = measureDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
    if (frontDistance == 999 || frontDistance > FRONT_WALL_DISTANCE) {
      break;
    }
  }

  stopMotors();
  delay(60);
}

// Draai rechts tot voor weer vrij is
void turnRightUntilFrontIsFree() {
  unsigned long startTime = millis();

  while (millis() - startTime < 1200) {
    turnRight();
    delay(40);

    int frontDistance = measureDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
    if (frontDistance == 999 || frontDistance > FRONT_WALL_DISTANCE) {
      break;
    }
  }

  stopMotors();
  delay(60);
}

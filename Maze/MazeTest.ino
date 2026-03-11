const int RIGHT_BACKWARD = 6;
const int RIGHT_FORWARD = 5;

const int ULTRA_SONIC_TRIG_FRONT = 8;
const int ULTRA_SONIC_ECHO_FRONT = 7;

const int ULTRA_SONIC_TRIG_LEFT = 4;
const int ULTRA_SONIC_ECHO_LEFT = 13;

const int LEFT_BACKWARD = 10;
const int LEFT_FORWARD = 9;

const int RIGHT_IN = 2;
const int LEFT_IN = 3;
const int GRIPPER = 11;

const int WALL_DISTANCE = 20;
const int SPEED = 180;
const int TURN_TIME = 450;

void setup() {

  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);

  pinMode(ULTRA_SONIC_TRIG_FRONT, OUTPUT);
  pinMode(ULTRA_SONIC_ECHO_FRONT, INPUT);

  pinMode(ULTRA_SONIC_TRIG_LEFT, OUTPUT);
  pinMode(ULTRA_SONIC_ECHO_LEFT, INPUT);

  Serial.begin(9600);
}

void loop() {

  int frontDistance = measureDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  int leftDistance = measureDistance(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);

  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.print("  Left: ");
  Serial.println(leftDistance);

  if (leftDistance > WALL_DISTANCE) {
    stopMotors();
    delay(100);
    turnLeft();
  }

  else if (frontDistance > WALL_DISTANCE) {
    moveForward();
  }

  else {
    stopMotors();
    delay(100);
    turnRight();
  }
}

int measureDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 40000);

  if (duration == 0) {
    return 999;
  }

  return duration * 0.034 / 2;
}

void moveForward() {

  analogWrite(RIGHT_FORWARD, SPEED);
  analogWrite(LEFT_FORWARD, SPEED);

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

  analogWrite(RIGHT_FORWARD, SPEED);
  analogWrite(LEFT_BACKWARD, SPEED);

  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_FORWARD, 0);

  delay(TURN_TIME);

  stopMotors();
}

void turnRight() {

  analogWrite(LEFT_FORWARD, SPEED);
  analogWrite(RIGHT_BACKWARD, SPEED);

  analogWrite(LEFT_BACKWARD, 0);
  analogWrite(RIGHT_FORWARD, 0);

  delay(TURN_TIME);

  stopMotors();
}

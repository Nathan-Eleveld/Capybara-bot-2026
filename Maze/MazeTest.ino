const int RIGHT_BACKWARD = 10;
const int RIGHT_FORWARD = 9;

const int ULTRA_SONIC_TRIG_FRONT = 8;
const int ULTRA_SONIC_ECHO_FRONT = 7;

const int ULTRA_SONIC_TRIG_LEFT = 12;
const int ULTRA_SONIC_ECHO_LEFT = 4;

const int LEFT_BACKWARD = 6;
const int LEFT_FORWARD = 5;

const int RIGHT_IN = 2;
const int LEFT_IN = 3;

const int GRIPPER = 11;

const int WALL_DISTANCE = 20;
const int TURN_TIME = 450;
const int FORWARD_SPEED = 180;

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
  int frontDistance = measureDistance(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
  int leftDistance = measureDistance(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);

  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.print(" | Left: ");
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
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    return 999;
  }

  return duration * 0.034 / 2;
}

void moveForward() {
  analogWrite(RIGHT_FORWARD, FORWARD_SPEED);
  analogWrite(LEFT_FORWARD, FORWARD_SPEED);
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
  analogWrite(RIGHT_FORWARD, FORWARD_SPEED);
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD, FORWARD_SPEED);
  delay(TURN_TIME);
  stopMotors();
}

void turnRight() {
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, FORWARD_SPEED);
  analogWrite(RIGHT_BACKWARD, FORWARD_SPEED);
  analogWrite(LEFT_BACKWARD, 0);
  delay(TURN_TIME);
  stopMotors();
}

const int RIGHT_BACKWARD = 10;
const int RIGHT_FORWARD  = 9;
const int LEFT_BACKWARD  = 6;
const int LEFT_FORWARD   = 5;

const int ULTRA_SONIC_TRIG_FRONT = 8;
const int ULTRA_SONIC_ECHO_FRONT = 7;

const int ULTRA_SONIC_TRIG_LEFT  = 4;
const int ULTRA_SONIC_ECHO_LEFT  = 13;

const int ULTRA_SONIC_TRIG_RIGHT = 12;
const int ULTRA_SONIC_ECHO_RIGHT = A0;

const int RIGHT_IN = 2;
const int LEFT_IN  = 3;
const int GRIPPER  = 11;

const int BASE_SPEED      = 155;
const int TURN_SPEED      = 170;
const int BACK_SPEED      = 140;
const int MAX_STEER       = 50;

const int OPEN_FRONT_CM   = 24;
const int OPEN_SIDE_CM    = 24;
const int WALL_SEEN_CM    = 30;
const int TARGET_SIDE_CM  = 12;

const int STEER_GAIN      = 3;
const int STEER_DEADBAND  = 6;

const int MAX_DISTANCE_CM   = 150;
const unsigned long PULSE_TIMEOUT_US = 10000UL;

const unsigned long PING_INTERVAL_MS = 30;
const unsigned long DEBUG_INTERVAL_MS = 200;

const int REVERSE_MS     = 130;
const int TURN_90_MS     = 235;
const int TURN_180_MS    = 470;
const int COMMIT_MS      = 120;
const int STOP_PAUSE_MS  = 50;
const int POST_TURN_MS   = 180;

const int OSCILLATION_LIMIT = 5;

int frontCm = MAX_DISTANCE_CM;
int leftCm  = MAX_DISTANCE_CM;
int rightCm = MAX_DISTANCE_CM;

bool frontReady = false;
bool leftReady  = false;
bool rightReady = false;

byte nextSensor = 0;

unsigned long lastPingMs = 0;
unsigned long lastDebugMs = 0;
unsigned long ignoreTurnsUntil = 0;

int oscillationCount = 0;
int lastSteerDir = 0;

const char* actionText = "START";

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

  digitalWrite(ULTRA_SONIC_TRIG_FRONT, LOW);
  digitalWrite(ULTRA_SONIC_TRIG_LEFT, LOW);
  digitalWrite(ULTRA_SONIC_TRIG_RIGHT, LOW);

  stopMotors();
}

void loop() {
  updateSensors();

  if (!allSensorsReady()) {
    actionText = "WACHT OP SENSOREN";
    stopMotors();
    debugPrint();
    return;
  }

  if (millis() < ignoreTurnsUntil) {
    actionText = "RECHT NA BOCHT";
    driveForward(BASE_SPEED, BASE_SPEED);
    debugPrint();
    return;
  }

  if (oscillationCount >= OSCILLATION_LIMIT) {
    doTurnAround("RECOVERY 180");
    debugPrint();
    return;
  }

  bool leftOpen  = leftCm  >= OPEN_SIDE_CM;
  bool rightOpen = rightCm >= OPEN_SIDE_CM;
  bool frontOpen = frontCm >= OPEN_FRONT_CM;

  // Bochten gaan voor rechtdoor:
  // links open = links
  // anders rechts open = rechts
  // anders voor open = vooruit
  // anders 180
  if (leftOpen) {
    doTurnLeft();
  } else if (rightOpen) {
    doTurnRight();
  } else if (frontOpen) {
    handleForward();
  } else {
    doTurnAround("DOODLOPEND 180");
  }

  debugPrint();
}

void updateSensors() {
  if (millis() - lastPingMs < PING_INTERVAL_MS) return;
  lastPingMs = millis();

  int measuredCm = 0;

  if (nextSensor == 0) {
    measuredCm = readUltrasonicCm(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
    frontCm = smoothDistance(frontCm, measuredCm);
    frontReady = true;
  } else if (nextSensor == 1) {
    measuredCm = readUltrasonicCm(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);
    leftCm = smoothDistance(leftCm, measuredCm);
    leftReady = true;
  } else {
    measuredCm = readUltrasonicCm(ULTRA_SONIC_TRIG_RIGHT, ULTRA_SONIC_ECHO_RIGHT);
    rightCm = smoothDistance(rightCm, measuredCm);
    rightReady = true;
  }

  nextSensor++;
  if (nextSensor > 2) nextSensor = 0;
}

bool allSensorsReady() {
  return frontReady && leftReady && rightReady;
}

int readUltrasonicCm(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, PULSE_TIMEOUT_US);

  if (duration == 0) return MAX_DISTANCE_CM;

  int cm = duration / 58;

  if (cm < 2) cm = 2;
  if (cm > MAX_DISTANCE_CM) cm = MAX_DISTANCE_CM;

  return cm;
}

int smoothDistance(int previousCm, int currentCm) {
  if (previousCm <= 0 || previousCm > MAX_DISTANCE_CM) return currentCm;
  return (previousCm + (currentCm * 2)) / 3;
}

void handleForward() {
  bool leftWall  = leftCm  < WALL_SEEN_CM;
  bool rightWall = rightCm < WALL_SEEN_CM;

  int errorCm = 0;

  if (leftWall && rightWall) {
    errorCm = rightCm - leftCm;
  } else if (leftWall) {
    errorCm = TARGET_SIDE_CM - leftCm;
  } else if (rightWall) {
    errorCm = rightCm - TARGET_SIDE_CM;
  } else {
    errorCm = 0;
  }

  int steer = constrain(errorCm * STEER_GAIN, -MAX_STEER, MAX_STEER);

  updateOscillation(steer);

  if (steer > STEER_DEADBAND) {
    actionText = "VOORUIT CORRECTIE RECHTS";
  } else if (steer < -STEER_DEADBAND) {
    actionText = "VOORUIT CORRECTIE LINKS";
  } else {
    actionText = "VOORUIT";
    lastSteerDir = 0;
    if (oscillationCount > 0) oscillationCount--;
  }

  int leftSpeed  = BASE_SPEED + steer;
  int rightSpeed = BASE_SPEED - steer;

  driveForward(leftSpeed, rightSpeed);
}

void updateOscillation(int steer) {
  int steerDir = 0;

  if (steer > STEER_DEADBAND) {
    steerDir = 1;
  } else if (steer < -STEER_DEADBAND) {
    steerDir = -1;
  }

  if (steerDir == 0) {
    lastSteerDir = 0;
    return;
  }

  if (lastSteerDir != 0 && steerDir != lastSteerDir) {
    oscillationCount++;
  } else if (lastSteerDir == steerDir && oscillationCount > 0) {
    oscillationCount--;
  }

  lastSteerDir = steerDir;
}

void resetOscillation() {
  oscillationCount = 0;
  lastSteerDir = 0;
}

void doTurnLeft() {
  actionText = "LINKS OPEN -> LINKS";

  driveBackward(BACK_SPEED, BACK_SPEED);
  delay(REVERSE_MS);

  stopMotors();
  delay(STOP_PAUSE_MS);

  spinLeft(TURN_SPEED);
  delay(TURN_90_MS);

  stopMotors();
  delay(STOP_PAUSE_MS);

  driveForward(BASE_SPEED, BASE_SPEED);
  delay(COMMIT_MS);

  ignoreTurnsUntil = millis() + POST_TURN_MS;
  resetOscillation();
}

void doTurnRight() {
  actionText = "RECHTS OPEN -> RECHTS";

  driveBackward(BACK_SPEED, BACK_SPEED);
  delay(REVERSE_MS);

  stopMotors();
  delay(STOP_PAUSE_MS);

  spinRight(TURN_SPEED);
  delay(TURN_90_MS);

  stopMotors();
  delay(STOP_PAUSE_MS);

  driveForward(BASE_SPEED, BASE_SPEED);
  delay(COMMIT_MS);

  ignoreTurnsUntil = millis() + POST_TURN_MS;
  resetOscillation();
}

void doTurnAround(const char* reason) {
  actionText = reason;

  driveBackward(BACK_SPEED, BACK_SPEED);
  delay(REVERSE_MS + 20);

  stopMotors();
  delay(STOP_PAUSE_MS);

  spinRight(TURN_SPEED);
  delay(TURN_180_MS);

  stopMotors();
  delay(STOP_PAUSE_MS);

  driveForward(BASE_SPEED, BASE_SPEED);
  delay(COMMIT_MS);

  ignoreTurnsUntil = millis() + POST_TURN_MS;
  resetOscillation();
}

void driveForward(int leftSpeed, int rightSpeed) {
  leftSpeed  = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  analogWrite(LEFT_FORWARD, leftSpeed);
  analogWrite(LEFT_BACKWARD, 0);

  analogWrite(RIGHT_FORWARD, rightSpeed);
  analogWrite(RIGHT_BACKWARD, 0);
}

void driveBackward(int leftSpeed, int rightSpeed) {
  leftSpeed  = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, leftSpeed);

  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, rightSpeed);
}

void spinLeft(int speedValue) {
  speedValue = constrain(speedValue, 0, 255);

  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, speedValue);

  analogWrite(RIGHT_FORWARD, speedValue);
  analogWrite(RIGHT_BACKWARD, 0);
}

void spinRight(int speedValue) {
  speedValue = constrain(speedValue, 0, 255);

  analogWrite(LEFT_FORWARD, speedValue);
  analogWrite(LEFT_BACKWARD, 0);

  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, speedValue);
}

void stopMotors() {
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
}

void debugPrint() {
  if (millis() - lastDebugMs < DEBUG_INTERVAL_MS) return;
  lastDebugMs = millis();

  Serial.print("Front: ");
  Serial.print(frontCm);
  Serial.print(" cm | Left: ");
  Serial.print(leftCm);
  Serial.print(" cm | Right: ");
  Serial.print(rightCm);
  Serial.print(" cm | Osc: ");
  Serial.print(oscillationCount);
  Serial.print(" | Action: ");
  Serial.println(actionText);
}

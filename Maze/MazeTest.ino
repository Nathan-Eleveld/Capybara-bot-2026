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
const int FRONT_BLOCKED_DISTANCE = 14;
const int FRONT_OPEN_DISTANCE    = 20;
const int LEFT_OPEN_DISTANCE     = 22;
const int MAX_VALID_DISTANCE     = 180;
const int INVALID_DISTANCE       = 999;

// Sensor timeout
const unsigned long SONAR_TIMEOUT_US = 25000UL;

// Snelheden
const int DRIVE_SPEED   = 150;
const int TURN_SPEED    = 165;
const int REVERSE_SPEED = 130;

// Tijden
const unsigned long LOOP_DELAY_MS        = 40;
const unsigned long STOP_BEFORE_TURN_MS  = 120;
const unsigned long STOP_AFTER_TURN_MS   = 80;
const unsigned long LEFT_TURN_TIME_MS    = 320;
const unsigned long RIGHT_TURN_TIME_MS   = 320;
const unsigned long FORWARD_STEP_MS      = 140;
const unsigned long REVERSE_TIME_MS      = 180;
const unsigned long DEBUG_INTERVAL_MS    = 250;
const unsigned long ACTION_COOLDOWN_MS   = 250;

// Anti-vastloop
const int MAX_SAME_ACTION_COUNT = 8;
const int MAX_FLIP_COUNT        = 8;
const unsigned long FLIP_WINDOW_MS = 2500;

enum Action {
  ACTION_STOP,
  ACTION_FORWARD,
  ACTION_TURN_LEFT,
  ACTION_TURN_RIGHT,
  ACTION_REVERSE_STUCK
};

int frontDistance = INVALID_DISTANCE;
int leftDistance  = INVALID_DISTANCE;

int lastFrontDistance = 40;
int lastLeftDistance  = 20;

Action currentAction = ACTION_STOP;
Action lastAction    = ACTION_STOP;

unsigned long lastDebugTime = 0;
unsigned long lastActionTime = 0;
unsigned long flipWindowStart = 0;

int sameActionCount = 0;
int flipCount = 0;
bool antiStuckActive = false;

int medianOf3(int a, int b, int c) {
  if (a > b) { int t = a; a = b; b = t; }
  if (b > c) { int t = b; b = c; c = t; }
  if (a > b) { int t = a; a = b; b = t; }
  return b;
}

const char* actionToText(Action action) {
  switch (action) {
    case ACTION_STOP:          return "STOP";
    case ACTION_FORWARD:       return "FORWARD";
    case ACTION_TURN_LEFT:     return "TURN_LEFT";
    case ACTION_TURN_RIGHT:    return "TURN_RIGHT";
    case ACTION_REVERSE_STUCK: return "REVERSE_STUCK";
    default:                   return "UNKNOWN";
  }
}

// Motor aansturing
void applyMotor(int forwardPin, int backwardPin, int speedValue) {
  speedValue = constrain(speedValue, -255, 255);

  if (speedValue > 0) {
    analogWrite(forwardPin, speedValue);
    analogWrite(backwardPin, 0);
  } else if (speedValue < 0) {
    analogWrite(forwardPin, 0);
    analogWrite(backwardPin, -speedValue);
  } else {
    analogWrite(forwardPin, 0);
    analogWrite(backwardPin, 0);
  }
}

void setMotorSpeeds(int leftSpeed, int rightSpeed) {
  applyMotor(LEFT_FORWARD, LEFT_BACKWARD, leftSpeed);
  applyMotor(RIGHT_FORWARD, RIGHT_BACKWARD, rightSpeed);
}

void stopMotors() {
  setMotorSpeeds(0, 0);
}

void driveForward() {
  setMotorSpeeds(DRIVE_SPEED, DRIVE_SPEED);
}

void reverseShort() {
  setMotorSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
}

void turnLeftInPlace() {
  setMotorSpeeds(-TURN_SPEED, TURN_SPEED);
}

void turnRightInPlace() {
  setMotorSpeeds(TURN_SPEED, -TURN_SPEED);
}

// Sensor meting
int measureDistanceRaw(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, SONAR_TIMEOUT_US);

  if (duration == 0) return INVALID_DISTANCE;

  int distance = duration / 58;

  if (distance <= 0 || distance > MAX_VALID_DISTANCE) {
    return INVALID_DISTANCE;
  }

  return distance;
}

int readFilteredDistance(int trigPin, int echoPin, int lastValid) {
  int d1 = measureDistanceRaw(trigPin, echoPin);
  delayMicroseconds(700);
  int d2 = measureDistanceRaw(trigPin, echoPin);
  delayMicroseconds(700);
  int d3 = measureDistanceRaw(trigPin, echoPin);

  if (d1 == INVALID_DISTANCE) d1 = lastValid;
  if (d2 == INVALID_DISTANCE) d2 = lastValid;
  if (d3 == INVALID_DISTANCE) d3 = lastValid;

  int result = medianOf3(d1, d2, d3);

  if (lastValid != INVALID_DISTANCE && abs(result - lastValid) > 40) {
    result = (result + lastValid) / 2;
  }

  return result;
}

void updateDistances() {
  frontDistance = readFilteredDistance(
    ULTRA_SONIC_TRIG_FRONT,
    ULTRA_SONIC_ECHO_FRONT,
    lastFrontDistance
  );

  leftDistance = readFilteredDistance(
    ULTRA_SONIC_TRIG_LEFT,
    ULTRA_SONIC_ECHO_LEFT,
    lastLeftDistance
  );

  lastFrontDistance = frontDistance;
  lastLeftDistance  = leftDistance;
}

bool isFrontBlocked() {
  return frontDistance <= FRONT_BLOCKED_DISTANCE;
}

bool isFrontOpen() {
  return frontDistance >= FRONT_OPEN_DISTANCE;
}

bool isLeftOpen() {
  return leftDistance >= LEFT_OPEN_DISTANCE;
}

// Debug
void printDebug() {
  if (millis() - lastDebugTime < DEBUG_INTERVAL_MS) return;
  lastDebugTime = millis();

  Serial.print("front distance: ");
  Serial.print(frontDistance);
  Serial.print(" cm | left distance: ");
  Serial.print(leftDistance);
  Serial.print(" cm | actie: ");
  Serial.print(actionToText(currentAction));
  Serial.print(" | anti-vastloop: ");
  Serial.println(antiStuckActive ? "JA" : "NEE");
}

// Anti-vastloop tracking
void registerAction(Action action) {
  currentAction = action;

  if (action == lastAction) {
    sameActionCount++;
  } else {
    sameActionCount = 0;
  }

  if (flipWindowStart == 0 || millis() - flipWindowStart > FLIP_WINDOW_MS) {
    flipWindowStart = millis();
    flipCount = 0;
  }

  if (action != lastAction) {
    flipCount++;
  }

  lastAction = action;
  lastActionTime = millis();
}

bool shouldRunAntiStuck() {
  if (sameActionCount >= MAX_SAME_ACTION_COUNT) return true;
  if (flipCount >= MAX_FLIP_COUNT) return true;
  return false;
}

void runAntiStuck() {
  antiStuckActive = true;
  registerAction(ACTION_REVERSE_STUCK);
  printDebug();

  stopMotors();
  delay(100);

  reverseShort();
  delay(REVERSE_TIME_MS);

  stopMotors();
  delay(100);

  turnRightInPlace();
  delay(220);

  stopMotors();
  delay(100);

  sameActionCount = 0;
  flipCount = 0;
  flipWindowStart = millis();
  antiStuckActive = false;
}

// Bewegingen
void doForwardStep() {
  registerAction(ACTION_FORWARD);
  driveForward();
  delay(FORWARD_STEP_MS);
  stopMotors();
  delay(40);
}

void doLeftTurn() {
  registerAction(ACTION_TURN_LEFT);

  stopMotors();
  delay(STOP_BEFORE_TURN_MS);

  turnLeftInPlace();
  delay(LEFT_TURN_TIME_MS);

  stopMotors();
  delay(STOP_AFTER_TURN_MS);

  driveForward();
  delay(FORWARD_STEP_MS);

  stopMotors();
  delay(40);
}

void doRightTurn() {
  registerAction(ACTION_TURN_RIGHT);

  stopMotors();
  delay(STOP_BEFORE_TURN_MS);

  turnRightInPlace();
  delay(RIGHT_TURN_TIME_MS);

  stopMotors();
  delay(STOP_AFTER_TURN_MS);

  driveForward();
  delay(FORWARD_STEP_MS);

  stopMotors();
  delay(40);
}

// Beslissing zonder zigzag-correcties
void decideNextMove() {
  stopMotors();
  registerAction(ACTION_STOP);
  delay(50);

  updateDistances();

  bool leftOpen  = isLeftOpen();
  bool frontOpen = isFrontOpen();
  bool frontBlocked = isFrontBlocked();

  // Linkerhandregel
  if (leftOpen) {
    doLeftTurn();
    return;
  }

  if (frontOpen && !frontBlocked) {
    doForwardStep();
    return;
  }

  doRightTurn();
}

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

  pinMode(RIGHT_IN, INPUT_PULLUP);
  pinMode(LEFT_IN, INPUT_PULLUP);

  pinMode(GRIPPER, OUTPUT);
  digitalWrite(GRIPPER, LOW);

  stopMotors();
  delay(300);

  updateDistances();
  lastFrontDistance = frontDistance;
  lastLeftDistance  = leftDistance;

  Serial.println("Maze solver gestart");
}

void loop() {
  updateDistances();
  printDebug();

  if (millis() - lastActionTime >= ACTION_COOLDOWN_MS) {
    decideNextMove();
  }

  if (shouldRunAntiStuck()) {
    runAntiStuck();
  }

  delay(LOOP_DELAY_MS);
}
  }

  delay(LOOP_DELAY_MS);
}

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
const int TARGET_LEFT_DISTANCE   = 14;
const int LEFT_DEAD_ZONE         = 3;
const int FRONT_STOP_DISTANCE    = 16;
const int FRONT_SAFE_DISTANCE    = 22;
const int LEFT_OPEN_DISTANCE     = 24;
const int LEFT_WALL_MAX_DISTANCE = 22;

// Sensor filtering
const int MAX_VALID_DISTANCE     = 180;
const int INVALID_DISTANCE       = 999;
const unsigned long SONAR_TIMEOUT_US = 25000UL;

// Snelheden
const int BASE_SPEED             = 150;
const int CORRECTION_FAST_SPEED  = 165;
const int CORRECTION_SLOW_SPEED  = 105;
const int TURN_SPEED             = 165;
const int REVERSE_SPEED          = 135;

// Tijden
const unsigned long LOOP_DELAY_MS           = 35;
const unsigned long SHORT_STOP_MS           = 90;
const unsigned long SHARP_TURN_STOP_MS      = 120;
const unsigned long LEFT_TURN_STEP_MS       = 280;
const unsigned long RIGHT_TURN_STEP_MS      = 300;
const unsigned long FORWARD_AFTER_TURN_MS   = 120;
const unsigned long REVERSE_TIME_MS         = 180;
const unsigned long TURN_COOLDOWN_MS        = 450;
const unsigned long DEBUG_INTERVAL_MS       = 220;

// Anti-vastloop
const int MAX_REPEAT_SAME_DECISION = 7;
const int MAX_ACTION_FLIPS         = 8;
const unsigned long FLIP_WINDOW_MS = 2500;

enum Action {
  ACTION_STOP,
  ACTION_FORWARD,
  ACTION_CORRECT_LEFT,
  ACTION_CORRECT_RIGHT,
  ACTION_TURN_LEFT,
  ACTION_TURN_RIGHT,
  ACTION_REVERSE_STUCK
};

int frontDistance = INVALID_DISTANCE;
int leftDistance  = INVALID_DISTANCE;

int lastFrontDistance = 40;
int lastLeftDistance  = TARGET_LEFT_DISTANCE;

Action currentAction = ACTION_STOP;
Action lastAction = ACTION_STOP;

unsigned long lastTurnTime = 0;
unsigned long lastDebugTime = 0;
unsigned long actionFlipWindowStart = 0;

int repeatedDecisionCount = 0;
int actionFlipCount = 0;
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
    case ACTION_CORRECT_LEFT:  return "CORRECT_LEFT";
    case ACTION_CORRECT_RIGHT: return "CORRECT_RIGHT";
    case ACTION_TURN_LEFT:     return "TURN_LEFT";
    case ACTION_TURN_RIGHT:    return "TURN_RIGHT";
    case ACTION_REVERSE_STUCK: return "REVERSE_STUCK";
    default:                   return "UNKNOWN";
  }
}

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
  setMotorSpeeds(BASE_SPEED, BASE_SPEED);
}

void correctLeft() {
  setMotorSpeeds(CORRECTION_SLOW_SPEED, CORRECTION_FAST_SPEED);
}

void correctRight() {
  setMotorSpeeds(CORRECTION_FAST_SPEED, CORRECTION_SLOW_SPEED);
}

void turnLeftInPlace() {
  setMotorSpeeds(-TURN_SPEED, TURN_SPEED);
}

void turnRightInPlace() {
  setMotorSpeeds(TURN_SPEED, -TURN_SPEED);
}

void reverseShort() {
  setMotorSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
}

int measureDistanceCmRaw(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, SONAR_TIMEOUT_US);

  if (duration == 0) {
    return INVALID_DISTANCE;
  }

  int distance = duration / 58;

  if (distance <= 0 || distance > MAX_VALID_DISTANCE) {
    return INVALID_DISTANCE;
  }

  return distance;
}

int readFilteredDistance(int trigPin, int echoPin, int lastValidDistance) {
  int d1 = measureDistanceCmRaw(trigPin, echoPin);
  delayMicroseconds(800);
  int d2 = measureDistanceCmRaw(trigPin, echoPin);
  delayMicroseconds(800);
  int d3 = measureDistanceCmRaw(trigPin, echoPin);

  if (d1 == INVALID_DISTANCE) d1 = lastValidDistance;
  if (d2 == INVALID_DISTANCE) d2 = lastValidDistance;
  if (d3 == INVALID_DISTANCE) d3 = lastValidDistance;

  int median = medianOf3(d1, d2, d3);

  if (lastValidDistance != INVALID_DISTANCE) {
    int maxJump = 35;
    if (abs(median - lastValidDistance) > maxJump) {
      median = (lastValidDistance * 2 + median) / 3;
    }
  }

  return median;
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
  lastLeftDistance = leftDistance;
}

bool isFrontBlocked() {
  return frontDistance <= FRONT_STOP_DISTANCE;
}

bool isLeftOpen() {
  return leftDistance >= LEFT_OPEN_DISTANCE;
}

bool hasUsableLeftWall() {
  return leftDistance < LEFT_OPEN_DISTANCE && leftDistance <= LEFT_WALL_MAX_DISTANCE;
}

void printDebug() {
  if (millis() - lastDebugTime < DEBUG_INTERVAL_MS) return;
  lastDebugTime = millis();

  Serial.print("front distance: ");
  Serial.print(frontDistance);
  Serial.print(" cm | left distance: ");
  Serial.print(leftDistance);
  Serial.print(" cm | action: ");
  Serial.print(actionToText(currentAction));
  Serial.print(" | anti-vastloop: ");
  Serial.println(antiStuckActive ? "JA" : "NEE");
}

void registerAction(Action action) {
  currentAction = action;

  if (action == lastAction) {
    repeatedDecisionCount++;
  } else {
    repeatedDecisionCount = 0;
  }

  if (actionFlipWindowStart == 0 || millis() - actionFlipWindowStart > FLIP_WINDOW_MS) {
    actionFlipWindowStart = millis();
    actionFlipCount = 0;
  }

  if (action != lastAction) {
    actionFlipCount++;
  }

  lastAction = action;
}

bool shouldRunAntiStuck() {
  if (repeatedDecisionCount >= MAX_REPEAT_SAME_DECISION) return true;
  if (actionFlipCount >= MAX_ACTION_FLIPS) return true;
  return false;
}

void runAntiStuck() {
  antiStuckActive = true;
  registerAction(ACTION_REVERSE_STUCK);
  printDebug();

  stopMotors();
  delay(SHORT_STOP_MS);

  reverseShort();
  delay(REVERSE_TIME_MS);

  stopMotors();
  delay(SHORT_STOP_MS);

  turnRightInPlace();
  delay(220);

  stopMotors();
  delay(SHORT_STOP_MS);

  repeatedDecisionCount = 0;
  actionFlipCount = 0;
  actionFlipWindowStart = millis();
  antiStuckActive = false;
}

void executeLeftTurn() {
  registerAction(ACTION_TURN_LEFT);
  stopMotors();
  delay(SHARP_TURN_STOP_MS);

  turnLeftInPlace();
  delay(LEFT_TURN_STEP_MS);

  stopMotors();
  delay(SHORT_STOP_MS);

  driveForward();
  delay(FORWARD_AFTER_TURN_MS);

  lastTurnTime = millis();
}

void executeRightTurn() {
  registerAction(ACTION_TURN_RIGHT);
  stopMotors();
  delay(SHARP_TURN_STOP_MS);

  turnRightInPlace();
  delay(RIGHT_TURN_STEP_MS);

  stopMotors();
  delay(SHORT_STOP_MS);

  lastTurnTime = millis();
}

void followLeftWall() {
  int error = leftDistance - TARGET_LEFT_DISTANCE;

  if (abs(error) <= LEFT_DEAD_ZONE) {
    registerAction(ACTION_FORWARD);
    driveForward();
    return;
  }

  if (error > LEFT_DEAD_ZONE) {
    registerAction(ACTION_CORRECT_LEFT);
    correctLeft();
  } else {
    registerAction(ACTION_CORRECT_RIGHT);
    correctRight();
  }
}

void moveForwardWithoutWallCorrection() {
  registerAction(ACTION_FORWARD);
  driveForward();
}

void decideAndDrive() {
  bool frontBlocked = isFrontBlocked();
  bool leftOpen = isLeftOpen();
  bool leftWallUsable = hasUsableLeftWall();
  bool turnCooldownActive = millis() - lastTurnTime < TURN_COOLDOWN_MS;

  if (frontBlocked) {
    registerAction(ACTION_STOP);
    stopMotors();
    delay(SHORT_STOP_MS);

    updateDistances();
    frontBlocked = isFrontBlocked();
    leftOpen = isLeftOpen();

    if (!frontBlocked) {
      return;
    }

    if (leftOpen) {
      executeLeftTurn();
    } else {
      executeRightTurn();
    }

    return;
  }

  if (leftOpen && !turnCooldownActive) {
    executeLeftTurn();
    return;
  }

  if (leftWallUsable) {
    followLeftWall();
  } else {
    moveForwardWithoutWallCorrection();
  }
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
  lastLeftDistance = leftDistance;

  Serial.println("Maze solver gestart");
}

void loop() {
  updateDistances();
  decideAndDrive();
  printDebug();

  if (shouldRunAntiStuck()) {
    runAntiStuck();
  }

  delay(LOOP_DELAY_MS);
}

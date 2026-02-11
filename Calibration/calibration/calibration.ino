// THIS FILE IS FOR CALIBRATION ONLY. USE THIS FILE ONLY FOR TESTING
const int DELAY = 2000;
const int RANGE = 200;

const int RIGHT_WHEEL_FORWARD = 5;
const int RIGHT_WHEEL_BACKWARD = 6;
const int LEFT_WHEEL_FORWARD = 9;
const int LEFT_WHEEL_BACKWARD = 10;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RIGHT_WHEEL_FORWARD, OUTPUT);
  pinMode(RIGHT_WHEEL_BACKWARD, OUTPUT);
  pinMode(LEFT_WHEEL_FORWARD, OUTPUT);
  pinMode(LEFT_WHEEL_BACKWARD, OUTPUT);

//  calibrateWheels();

  Serial.begin(9600);
  Serial.println(calibratePulse());
  
}

void loop() {
  
}

void calibrateWheels() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(DELAY);
  digitalWrite(LED_BUILTIN, LOW);
  delay(DELAY);

  analogWrite(RIGHT_WHEEL_FORWARD, RANGE);
  delay(DELAY);
  analogWrite(RIGHT_WHEEL_FORWARD, 0);
  analogWrite(RIGHT_WHEEL_BACKWARD, RANGE);
  delay(DELAY);
  analogWrite(RIGHT_WHEEL_BACKWARD, 0);
  analogWrite(LEFT_WHEEL_FORWARD,RANGE);
  delay(DELAY);
  analogWrite(LEFT_WHEEL_FORWARD,0);
  analogWrite(LEFT_WHEEL_BACKWARD
  
  , RANGE);
  delay(DELAY);
  analogWrite(LEFT_WHEEL_BACKWARD, 0);
}

int calibratePulse(){
  analogWrite(RIGHT_WHEEL_FORWARD, RANGE);
  delay(DELAY);
  analogWrite(RIGHT_WHEEL_FORWARD, 0);
  return digitalRead(13);
}

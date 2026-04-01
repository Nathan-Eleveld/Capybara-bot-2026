#define RIGHT_BACKWARD 10
#define RIGHT_FORWARD 9
#define LEFT_BACKWARD 6
#define LEFT_FORWARD 5
#define RIGHT_IN 2
#define LEFT_IN 3
#define WHEEL_SPEED 255
const int   SENSOR_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
const int   GRIPPER = 11;
int   sensorBoundry[8] = {0,0,0,0,0,0,0,0};
int   sensorReadings[8];
int   rightPulses = 0;
int   lastInterruptRight = 0;
int   leftPulses = 0;
int   lastInterruptLeft = 0;
int   rightWheel;
int   leftWheel;
int   winFrames;
int servoPulseWidth = 1100;
int servoOpen = 1500;
int servoClosed = 1100; 
unsigned long lastServoPulse = 0;
bool  finished = false;

//#define DEBUG

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Battlebot has Started");
  #endif

  Serial.begin(9600);
  
  for(int pin : SENSOR_PINS){
    pinMode(pin, INPUT);   
    digitalWrite(pin, LOW); 
  }
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(RIGHT_IN, INPUT);
  pinMode(LEFT_IN, INPUT);
  pinMode(GRIPPER, OUTPUT);

  clawOpen(true);
  
  stopWheels();

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulserightWheelISR, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN), countPulseleftWheelISR, RISING);

  calibrateBoundrys();
  getOnTrack();
  stopWheels();
}

void loop(){
  updateServo();
  
  if(!winDetect()){
    getReadings(); 

//  Maze Logic
    
    clawOpen(false);
  } else if(!finished) {
    finishingMove();
  }
}

void calibrateBoundrys(){
  int temp[8];
  int readingCount = 0;
  while(rightPulses < 10){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
    for(int i = 0; i < 8; i++){
      temp[i] = sensorBoundry[i] + analogRead(SENSOR_PINS[i]);
      sensorBoundry[i] = temp[i];
//      Serial.println(sensorBoundry[i]);
    }
    readingCount++;
//    Serial.println("---");
  }
   
  for(int i = 0; i < 8; i++){
    sensorBoundry[i] = temp[i] / readingCount;
//    Serial.println(sensorBoundry[i]);
  }
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, 0);
  delay(200);
}

//Function called when right wheel rotates 1/20th of a rotation
void countPulserightWheelISR(){
  if(millis() > lastInterruptRight){
    rightPulses++;
    lastInterruptRight = millis() + 20;
  }
}

//Function called when left wheel rotates 1/20th of a rotation
void countPulseleftWheelISR(){
  if(millis() > lastInterruptLeft){
    leftPulses++;
    lastInterruptLeft = millis() + 20;
  }
}

void getReadings(){
  int i = 0;
  for(int pin : SENSOR_PINS){
    sensorReadings[i] = (analogRead(pin) < sensorBoundry[i]);
    i++;
  }
}

int getAverageSensorPin(){
  float medianReading = 0.0;
  int readingCount = 0;
  for(int i = 0; i < 8; i++){
    if(!sensorReadings[i]){
      medianReading += i+ 1;
      readingCount++;
    }
  }  
  
  return medianReading / readingCount;
}

void writeWheels(float multiplierRight, float multiplierLeft){
  analogWrite(RIGHT_FORWARD, WHEEL_SPEED * multiplierRight);
  analogWrite(LEFT_FORWARD, WHEEL_SPEED * multiplierLeft);
}

void writeWheelsBackwards(float multiplierRight, float multiplierLeft){
  analogWrite(RIGHT_BACKWARD, WHEEL_SPEED * multiplierRight);
  analogWrite(LEFT_BACKWARD, WHEEL_SPEED * multiplierLeft);
}

void getOnTrack(){
  stopWheels();
  while(rightPulses < 10 && leftPulses < 10){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
//    Serial.println(rightPulses);
  }
  stopWheels();
  while(leftPulses < 20){
     clawOpen(false);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
//    Serial.println(rightPulses);
  }
  analogWrite(LEFT_FORWARD, 0);
}

void stopWheels(){
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
  rightPulses = 0;
  leftPulses = 0;
}

bool winDetect(){
  int sensorsActive = 0;
  for(int reading : sensorReadings){
    sensorsActive += reading;
  }
  if(sensorsActive < 2){
    winFrames++;
  }else{
    winFrames = 0;
  }
  #ifdef DEBUG
    Serial.print(winFrames);
    Serial.print(" --- ");
  #endif
  if(winFrames > 20){
    return true;
  } else {
    return false;
  }
}


void updateServo() {
  if (micros() - lastServoPulse >= 20000) {
    lastServoPulse = micros();

    digitalWrite(GRIPPER, HIGH);
    delayMicroseconds(servoPulseWidth);
    digitalWrite(GRIPPER, LOW);
  }
}

void clawOpen(bool open) {
  if (open) {
    servoPulseWidth = servoOpen;
    updateServo();
  } else {
    servoPulseWidth = servoClosed;
    updateServo();
  }
}

void finishingMove(){
  stopWheels();
   while(rightPulses < 5 || leftPulses < 5){
    writeWheelsBackwards(1, 1);
    Serial.println(rightPulses);
    Serial.println(leftPulses);
  }
  stopWheels();
  clawOpen(true);
  finished = true;
}

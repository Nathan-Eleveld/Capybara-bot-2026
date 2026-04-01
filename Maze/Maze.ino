#define RIGHT_BACKWARD 10
#define RIGHT_FORWARD 9
#define LEFT_BACKWARD 6
#define LEFT_FORWARD 5
#define ULTRA_SONIC_TRIG_FRONT 8
#define ULTRA_SONIC_ECHO_FRONT 7
#define ULTRA_SONIC_TRIG_LEFT 4
#define ULTRA_SONIC_ECHO_LEFT 13
#define RIGHT_IN 2
#define LEFT_IN 3
#define WHEEL_SPEED 255
#define GRIPPER 11
#define NEO_PIXEL_PIN 12
#define WISH_LEFT 8
const int SENSOR_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int sensorBoundry[8] = {0,0,0,0,0,0,0,0};
int sensorReadings[8];
int rightPulses = 0;
unsigned long lastInterruptRight = 0;
int leftPulses = 0;
unsigned long lastInterruptLeft = 0;
unsigned long lastActiveSonicFront = 0;
unsigned long lastActiveSonicLeft = 0;
int distance;
int distanceFront;
int distanceLeft;
long duration;
int   winFrames;
unsigned long lastActiveSonic;
int servoPulseWidth = 1100;
int servoOpen = 1500;
int servoClosed = 1100; 
unsigned long lastServoPulse = 0;
bool  finished = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Battlebot has Started");

  for(int pin : SENSOR_PINS){
    pinMode(pin, INPUT);   
    digitalWrite(pin, LOW); 
  }

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

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulserightWheelISR, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN), countPulseleftWheelISR, RISING);

  clawOpen(true);
  stopWheels();
  digitalWrite(GRIPPER, LOW);

  delay(500);
  calibrateBoundrys();
  getOnTrack();
  stopWheels();
}

void loop() {
  if(!winDetect()){
    getReadings(); 

    updateServo();
    // put your main code here, to run repeatedly:
    distanceLeft = ultraSonicSensor(ULTRA_SONIC_TRIG_LEFT, ULTRA_SONIC_ECHO_LEFT);
    distanceFront = ultraSonicSensor(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
    //  Serial.println(distanceLeft);
  
    stopWheels();
    if(distanceLeft + 1 < WISH_LEFT){
      writeWheels(1, 0.2);
    } else if(distanceLeft - 1 > WISH_LEFT){
      writeWheels(0.3, 1);
    } else {
      writeWheels(1, 1);
    }
  
    if(distanceFront < 10 && distanceLeft > 4){
      stopWheels();
      delay(100);
      while(distanceFront < 30){
        analogWrite(RIGHT_FORWARD, WHEEL_SPEED * 0.8);
        analogWrite(LEFT_BACKWARD, WHEEL_SPEED);
        distanceFront = ultraSonicSensor(ULTRA_SONIC_TRIG_FRONT, ULTRA_SONIC_ECHO_FRONT);
      }
      stopWheels(); 
      while(rightPulses < 4){
        analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
        analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
      }
      stopWheels();
      delay(100);
    }

    clawOpen(false);
  } else if(!finished) {
    finishingMove();
  }
  
  
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

void stopWheels(){
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
  leftPulses = 0;
  rightPulses = 0;
}

int ultraSonicSensor(int trig, int echo){
    digitalWrite(trig, LOW);
    delayMicroseconds(2);

    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    duration = pulseIn(echo, HIGH);

    lastActiveSonic = millis() + 250;
    
    distance = duration * 0.034 / 2;
    
    if (distance != 0){
      return distance;
    } else {
      return ultraSonicSensor(trig, echo);
    }
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

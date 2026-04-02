#include <Adafruit_NeoPixel.h>

#define RIGHT_BACKWARD 10
#define RIGHT_FORWARD 9
#define LEFT_BACKWARD 6
#define LEFT_FORWARD 5
#define RIGHT_IN 2
#define LEFT_IN 3
#define WHEEL_SPEED 255
#define ULTRA_SONIC_TRIG 8
#define ULTRA_SONIC_ECHO 7
#define SERVO_OPEN 1500
#define SERVO_CLOSED 1000
#define GRIPPER 11
#define NEO_PIXEL_PIN 12

const int SENSOR_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int sensorBoundry[8] = {0,0,0,0,0,0,0,0};
int sensorReadings[8];
int rightPulses = 0;
unsigned long lastInterruptRight = 0;
int leftPulses = 0;
unsigned long lastInterruptLeft = 0;
int rightWheel;
int leftWheel;
int winFrames;
long duration;
int distance;
unsigned long lastActiveSonic = 0;
int servoPulseWidth = 1100;
unsigned long lastServoPulse = 0;
int detectCount = 0;
bool waiting = true;

#define DEBUG

Adafruit_NeoPixel pixels(4, NEO_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
  
void setup(){
  #ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Battlebot has Started");
  #endif

  pixels.begin();
  pixels.clear();
  setPixelsGreenToRed(0,0,0,0);
  
  for(int pin : SENSOR_PINS){
    pinMode(pin, INPUT);   
    digitalWrite(pin, LOW); 
  }
  pinMode(ULTRA_SONIC_TRIG, OUTPUT);
  pinMode(ULTRA_SONIC_ECHO, INPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(RIGHT_IN, INPUT);
  pinMode(LEFT_IN, INPUT);

  stopWheels();

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulserightWheelISR, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN), countPulseleftWheelISR, RISING);

  waitForRobotStart();

  calibrateBoundrys();
  getOnTrack();
}

void loop(){
  getReadings();
  updateServo();
  if(!sensorReadings[0] && !sensorReadings[1] && (!sensorReadings[5] || !sensorReadings[4])){
    rightTurn();
  } else{
    drive(); 
  }
  if(lastActiveSonic < millis()){
    if(ultraSonicSensor() < 15)
    {
      stopWheels(); 
      while(rightPulses < 20){
        analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
        analogWrite(LEFT_BACKWARD, WHEEL_SPEED);
      }
    }
  }
}

bool isRobotDetected(){
  detectCount = 0;

  for(int count = 0; count < 3; count++){
    int d = ultraSonicSensor();

    if(distance > 0 && distance < 30){
      detectCount++;
    }

    delay(50);
  }

  return detectCount >= 3;
}

void waitForRobotStart(){
  stopWheels();
  waiting = true;

  while(waiting){
    if(isRobotDetected()){
      delay(3000);
      waiting = false;
    }
  }
}

void calibrateBoundrys(){
  int temp[8];
  int readingCount = 0;
  while(rightPulses < 10){
    clawOpen(true);
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
    for(int i = 0; i < 8; i++){
      temp[i] = sensorBoundry[i] + analogRead(SENSOR_PINS[i]);
      sensorBoundry[i] = temp[i];
      Serial.println(sensorBoundry[i]);
    }
    readingCount++;
    Serial.println("---");
  }
   
  for(int i = 0; i < 8; i++){
    sensorBoundry[i] = temp[i] / readingCount;
    Serial.println(sensorBoundry[i]);
  }
  Serial.println("done calibrating");
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

void drive(){
  stopWheels();
  
  switch(getAverageSensorPin()){
    case 1:
      writeWheels(1, 0);
      setPixelsGreenToRed(0,0,0,0.5);
      break;
    case 2:
      writeWheels(1, 0);
      setPixelsGreenToRed(0,0,0,0.3);
      break;
    case 3:
      writeWheels(1, 0.5);
      break;
    case 4:
      writeWheels(1, 1);
      break;
    case 5:
      writeWheels(0.5, 1);
      break;
    case 6:
      writeWheels(0, 1);
      setPixelsGreenToRed(0,0,0.3,0);
      break;
    case 7:
    case 8:
      writeWheels(0, 0.8);
      setPixelsGreenToRed(0,0,0.5,0);
      break;
    case 0:
      stopWheels();
      while(sensorReadings[7] && sensorReadings[6] && sensorReadings[5]){
        analogWrite(RIGHT_BACKWARD, WHEEL_SPEED * 0.8);
        analogWrite(LEFT_FORWARD, WHEEL_SPEED * 0.8);
        getReadings();
      }
      setPixelsGreenToRed(0,0,0,1);
      stopWheels();
      break;
  }
}

int getAverageSensorPin(){
  float medianReading = 0.0;
  int readingCount = 0;
  for(int i = 0; i < 8; i++){
    if(!sensorReadings[i]){
      medianReading += i + 1;
      readingCount++;
    }
  }  
  
  return medianReading / readingCount;
}

void writeWheels(float multiplierRight, float multiplierLeft){
  analogWrite(RIGHT_FORWARD, WHEEL_SPEED * multiplierRight);
  analogWrite(LEFT_FORWARD, WHEEL_SPEED * multiplierLeft);
}

void getOnTrack(){
  stopWheels();
  while(rightPulses < 10 && leftPulses < 10){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
  }
  stopWheels();
  while(rightPulses < 20){
    clawOpen(false);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
  }
  analogWrite(LEFT_FORWARD, 0);
}

void stopWheels(){
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, 0);
  analogWrite(RIGHT_BACKWARD, 0);
  analogWrite(LEFT_BACKWARD, 0);
  leftPulses = 0;
  rightPulses = 0;
}

void rightTurn(){
  setPixelsGreenToRed(0,0,1,1);
  checkForFinish();
  Serial.print("turning Right");
  setPixelsGreenToRed(0,0,1,0);
  stopWheels();
  while(leftPulses < 12){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_BACKWARD, WHEEL_SPEED);
    if(leftPulses > 4){
      analogWrite(RIGHT_FORWARD, WHEEL_SPEED * 0.8);
      analogWrite(LEFT_BACKWARD, WHEEL_SPEED * 0.8);
    }
  }
  setPixelsGreenToRed(0,0,0,0);
  stopWheels();
  getReadings();
}

void checkForFinish(){
  updateServo();
  stopWheels();
  while(leftPulses < 8){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
    if (leftPulses < 2){
      analogWrite(RIGHT_FORWARD, WHEEL_SPEED * 0.8);
      analogWrite(LEFT_FORWARD, WHEEL_SPEED * 0.8);
    }
  }
  stopWheels();
  if(mostlyBlack()){
    while (leftPulses < 25){
      analogWrite(RIGHT_BACKWARD, WHEEL_SPEED);
      analogWrite(LEFT_BACKWARD, WHEEL_SPEED);
      if (leftPulses > 5){
        clawOpen(true);
      }
    }
    stopWheels();
    setPixelsGreenToRed(1,1,1,1);
    while(true){
      updateServo();  
      delay(100);
      setPixelsGreenToRed(0,0,1,0.7);  
      delay(100);
      setPixelsGreenToRed(0,0,0.7,1);  
      delay(100);
    }
  }
  stopWheels();
  getReadings();
  delay(50);
}

boolean mostlyBlack(){
  getReadings();
  return (!sensorReadings[6] && !sensorReadings[2]);
}

int ultraSonicSensor(){
  digitalWrite(ULTRA_SONIC_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(ULTRA_SONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRA_SONIC_TRIG, LOW);

  duration = pulseIn(ULTRA_SONIC_ECHO, HIGH);

  distance = duration * 0.034 / 2;
  lastActiveSonic = millis() + 250;
  
  if (distance != 0)
  {
    return distance;
  } else {
    return ultraSonicSensor();
  }
}

void clawOpen(bool open){
  if (open) {
    servoPulseWidth = SERVO_OPEN;
    updateServo();
  } else {
    servoPulseWidth = SERVO_CLOSED;
    updateServo();
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

void setPixelsGreenToRed(double colorZero, double colorOne, double colorTwo, double colorThree){
  pixels.setPixelColor(0, pixels.Color(0, 200, 110));
  pixels.setPixelColor(1, pixels.Color(0, 200, 110));
  pixels.setPixelColor(2, pixels.Color(255 - colorTwo * 255, colorTwo * 255, 0));
  pixels.setPixelColor(3, pixels.Color(255 - colorThree * 255, colorThree * 255, 0));
  pixels.show();
}

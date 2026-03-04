#define RIGHT_BACKWARD 10
#define RIGHT_FORWARD 9
#define LEFT_BACKWARD 6
#define LEFT_FORWARD 5
#define RIGHT_IN 2
#define LEFT_IN 3
#define WHEEL_SPEED 255
const int   SENSOR_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int   SENSOR_BOUNDRY[8] = {0,0,0,0,0,0,0,0};
int   sensorReadings[8];
int   rightPulses = 0;
int   lastInterruptRight = 0;
int   leftPulses = 0;
int   lastInterruptLeft = 0;
int   rightWheel;
int   leftWheel;
int   winFrames;

#define DEBUG

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Battlebot has Started");
  #endif
  
  for(int pin : SENSOR_PINS){
    pinMode(pin, INPUT);    
  }
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(RIGHT_IN, INPUT);
  pinMode(LEFT_IN, INPUT);

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulserightWheelISR, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN), countPulseleftWheelISR, RISING);

  calibrateBoundrys();
  getOnTrack();
}

void loop(){
  if(!winDetect()){
    getReadings(); 
    drive();
  } else {
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, 0);
    analogWrite(LEFT_BACKWARD, 0);
  }
}

void calibrateBoundrys(){
  int temp[8];
  int readingCount = 0;
  while(millis() < 500){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
    for(int i = 0; i < 8; i++){
      temp[i] = SENSOR_BOUNDRY[i] + analogRead(SENSOR_PINS[i]);
      SENSOR_BOUNDRY[i] = temp[i];
      Serial.println(SENSOR_BOUNDRY[i]);
    }
    readingCount++;
    Serial.println("---");
  }
   
  for(int i = 0; i < 8; i++){
    SENSOR_BOUNDRY[i] = temp[i] / readingCount;
    Serial.println(SENSOR_BOUNDRY[i]);
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
    sensorReadings[i] = (analogRead(pin) < SENSOR_BOUNDRY[i]);
    i++;
  }
}

void drive(){
    analogWrite(RIGHT_FORWARD, 0);
    analogWrite(LEFT_FORWARD, 0);
    analogWrite(RIGHT_BACKWARD, 0);
    analogWrite(LEFT_BACKWARD, 0);
  
  switch(getAverageSensorPin()){
    case 1:
      writeWheels(1, 0);
      break;
    case 2:
      writeWheels(1, 0.5);
      break;
    case 3:
      writeWheels(1, 0.8);
      break;
    case 4:
      writeWheels(1, 1);
      break;
    case 5:
      writeWheels(0.8, 1);
      break;
    case 6:
      writeWheels(0.5, 1);
      break;
    case 7:
      writeWheels(0, 1);
      break;
    case 0:
      analogWrite(RIGHT_BACKWARD, WHEEL_SPEED);
      analogWrite(LEFT_BACKWARD, WHEEL_SPEED);
      break;
  }
  #ifdef DEBUG
  Serial.println(getAverageSensorPin());
  #endif
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

void getOnTrack(){
  while(millis() < 1000){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
  }
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, 0);
  leftPulses = 0;
  rightPulses = 0;
  while(millis() < 1300){
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
  }
  analogWrite(LEFT_FORWARD, 0);
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

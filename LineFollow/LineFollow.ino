#define RIGHT_BACKWARD 10
#define RIGHT_FORWARD 9
#define LEFT_BACKWARD 6
#define LEFT_FORWARD 5
#define RIGHT_IN 2
#define LEFT_IN 3
#define WHEEL_SPEED 255
const int   SENSOR_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int sensorBoundry[8] = {0,0,0,0,0,0,0,0};
int sensorReadings[8];
int rightPulses = 0;
unsigned long lastInterruptRight = 0;
int leftPulses = 0;
unsigned long lastInterruptLeft = 0;
int rightWheel;
int leftWheel;
int winFrames;

#define DEBUG

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Battlebot has Started");
  #endif
  
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

  stopWheels();

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulserightWheelISR, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN), countPulseleftWheelISR, RISING);

  calibrateBoundrys();
  getOnTrack();
}

void loop(){
  getReadings();
  if(!sensorReadings[1] && (!sensorReadings[5] || !sensorReadings[4])){
    rightTurn();
  } else{
    drive(); 
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
      Serial.println(sensorBoundry[i]);
    }
    readingCount++;
    Serial.println("---");
  }
   
  for(int i = 0; i < 8; i++){
    sensorBoundry[i] = temp[i] / readingCount;
    Serial.println(sensorBoundry[i]);
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

void drive(){
    stopWheels();
  
  switch(getAverageSensorPin()){
    case 1:
      writeWheels(1, 0);
      break;
    case 2:
      writeWheels(1, 0.1);
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
      writeWheels(0.1, 1);
      break;
    case 7:
      writeWheels(0, 1);
      break;
    case 0:
      analogWrite(RIGHT_BACKWARD, WHEEL_SPEED * 0.8);
      analogWrite(LEFT_FORWARD, WHEEL_SPEED * 0.8);
      Serial.print("turning Left");
      break;
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

void getOnTrack(){
  stopWheels();
  while(rightPulses < 10 && leftPulses < 10){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
    Serial.println(rightPulses);
  }
  stopWheels();
  while(rightPulses < 15){
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
    Serial.println(rightPulses);
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
  checkForFinish();
  Serial.print("turning Right");
  while(leftPulses < 15){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    Serial.println(leftPulses);
    Serial.print("leftpulses: ");
  }
  stopWheels();
}

void checkForFinish(){
  stopWheels();
  while(leftPulses < 5){
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
    analogWrite(LEFT_FORWARD, WHEEL_SPEED);
  }
  stopWheels();
  delay(50);
  if(mostlyBlack()){
    while(true){}
  }
  while(leftPulses < 6){
    analogWrite(RIGHT_BACKWARD, WHEEL_SPEED);
    analogWrite(LEFT_BACKWARD, WHEEL_SPEED);
  }
  stopWheels();
  delay(50);
}

boolean mostlyBlack(){
  getReadings();
  int count = 0;
  for(int reading : sensorReadings){
    count += reading;
  }
  return (count < 6);
}

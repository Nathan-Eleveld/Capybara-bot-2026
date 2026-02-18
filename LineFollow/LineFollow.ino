const int RIGHT_BACKWARD = 10;
const int RIGHT_FORWARD = 9;
const int LEFT_BACKWARD = 6;
const int LEFT_FORWARD = 5;
const int RIGHT_IN = 2;
const int LEFT_IN = 3;
const byte WHEEL_SPEED = 255;
const int SENSOR_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int sensorReadings[8];
int RightPulses = 0;
int LastInterruptRight = 0;
int LeftPulses = 0;
int LastInterruptLeft = 0;
int RightWheel;
int LeftWheel;
bool turn;

void setup() {
  Serial.begin(9600);
  for(int pin : SENSOR_PINS){
    pinMode(pin, INPUT);    
  }
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(RIGHT_IN, INPUT);
  pinMode(LEFT_IN, INPUT);

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulseRightWheel, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN), countPulseLeftWheel, RISING);
}

void loop() {
  getReadings(); 
  drive();
}

//Function called when right wheel rotates 1/20th of a rotation
void countPulseRightWheel()
{
  if(millis() > LastInterruptRight)
  {
    RightPulses++;
    LastInterruptRight = millis() + 20;
  }
}

//Function called when left wheel rotates 1/20th of a rotation
void countPulseLeftWheel()
{
  if(millis() > LastInterruptLeft)
  {
    LeftPulses++;
    LastInterruptLeft = millis() + 20;
  }
}

void getReadings(){
  int i = 0;
  for(int pin : SENSOR_PINS){
    sensorReadings[i] = (analogRead(pin) < 700);
    //will be calibrated per sensor later
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

void printReadings(){
  int i = 0;
  Serial.println("---------------");
  for(int reading : sensorReadings){
    Serial.print(i);
    Serial.print(": ");
    Serial.println(reading);
    i++;
  }
}

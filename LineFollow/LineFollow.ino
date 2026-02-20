const int   RIGHT_BACKWARD = 10;
const int   RIGHT_FORWARD = 9;
const int   LEFT_BACKWARD = 6;
const int   LEFT_FORWARD = 5;
const int   RIGHT_IN = 2;
const int   LEFT_IN = 3;
const byte  WHEEL_SPEED = 255;
const int   SENSOR_PINS[8] = {A0, A1, A2, A3, A4, A5, A6, A7};
int   sensorReadings[8];
int   rightPulses = 0;
int   lastInterruptRight = 0;
int   leftPulses = 0;
int   lastInterruptLeft = 0;
int   rightWheel;
int   leftWheel;
bool  turn;

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

  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulserightWheelISR, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN), countPulseleftWheelISR, RISING);
}

void loop() {
  getReadings(); 
  drive();
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
  for(int pin : SENSOR_PINS){RightWheel
    
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
  
  switch(getMedianReading()){
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

int getMedianReading(){
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

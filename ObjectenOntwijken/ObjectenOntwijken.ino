long duration;
int distance;
const int RIGHT_BACKWARD = 10;
const int RIGHT_FORWARD = 9;
const int LEFT_BACKWARD = 6;
const int LEFT_FORWARD = 5;
const int RIGHT_IN = 2;
const int LEFT_IN = 3;
const byte WHEEL_SPEED = 255;
const int ULTRA_SONIC_TRIG = 8;
const int ULTRA_SONIC_ECHO = 7;
int RightPulses = 0;
int LastInterruptRight = 0;
int LeftPulses = 0;
int LastInterruptLeft = 0;
int RightWheel;
int LeftWheel;
bool turn;

void setup() 
{
  // put your setup code here, to run once:
  pinMode(ULTRA_SONIC_TRIG, OUTPUT);
  pinMode(ULTRA_SONIC_ECHO, INPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(RIGHT_IN, INPUT);
  pinMode(LEFT_IN, INPUT);
  Serial.begin(9600);

  //initiates pins 2 and 3 to activate functions when going from LOW to HIGH
  attachInterrupt(digitalPinToInterrupt(RIGHT_IN), countPulseRightWheel, RISING);
  attachInterrupt(digitalPinToInterrupt(LEFT_IN), countPulseLeftWheel, RISING);
}

void loop() 
{
  Serial.println(ultraSonicSensor());
  driveNoDelay();
  if (detectObstacle())
  {
    avoidObstacle();
  }
  
}

int ultraSonicSensor()
{
    static unsigned long lastActive = 0;

    if(millis() > lastActive)
    {
      return distance;
    }
    digitalWrite(ULTRA_SONIC_TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(ULTRA_SONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRA_SONIC_TRIG, LOW);

    duration = pulseIn(ULTRA_SONIC_ECHO, HIGH);

    distance = duration * 0.034 / 2;
    lastActive = millis() + 500;
    
    if (distance != 0)
    {
      return distance;
    } else {
      return ultraSonicSensor();
    }
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

void drive(int RightDistance, int LeftDistance) { 
  RightPulses = 0;
  LeftPulses = 0;
  
  turn = false;
  //Handles giong backward 
  if(RightDistance > 0) {
    RightWheel = RIGHT_FORWARD;
  } else {
    turn = true;
    RightWheel = RIGHT_BACKWARD;
  }
  if(LeftDistance > 0) {
    LeftWheel = LEFT_FORWARD;
  } else {
    if(!turn)
    {
      turn = true;
    } else {
      turn = false;
    }
    LeftWheel = LEFT_BACKWARD;
  }

  RightDistance = abs(RightDistance);
  LeftDistance = abs(LeftDistance);

  // keeps driving until the set distance is reached for each wheel
  while(RightPulses < RightDistance || LeftPulses < LeftDistance)
  {
    analogWrite(RightWheel, WHEEL_SPEED);
    analogWrite(LeftWheel, WHEEL_SPEED);

  // Compensates if one wheel is going faster than the other
    if(!turn)
    {
      if(LeftPulses > RightPulses)
      {
        analogWrite(RightWheel, WHEEL_SPEED / 1.5);
      }
      if(RightPulses > LeftPulses)
      {
        analogWrite(LeftWheel, WHEEL_SPEED / 1.5);
      }
    }
  // Stops wheel if destination is reached
    if(RightPulses >= RightDistance)
    {
      analogWrite(LeftWheel, 0);
    }
    if(LeftPulses >= LeftDistance)
    {  
      analogWrite(RightWheel, 0);
    }
  }
  analogWrite(RightWheel, 0);
  analogWrite(LeftWheel, 0);
  RightPulses = 0;
  LeftPulses = 0;
  delay(500);
}

void driveNoDelay()
{
  analogWrite(RIGHT_FORWARD, WHEEL_SPEED);
  analogWrite(LEFT_FORWARD, WHEEL_SPEED);
  
  // Compensates if one wheel is going faster than the other
  if(LeftPulses > RightPulses)
  {
    analogWrite(RIGHT_FORWARD, WHEEL_SPEED / 1.5);
  }
  if(RightPulses > LeftPulses)
  {
    analogWrite(LEFT_FORWARD, WHEEL_SPEED / 1.5);
  }
}

void avoidObstacle()
{
  analogWrite(RIGHT_FORWARD, 0);
  analogWrite(LEFT_FORWARD, 0);
  delay(200);
  drive(7, -7);
  drive(25,25);
  drive(-7,7);
  drive(30,30);
  drive(-7, 7);
  drive(25,25);
  drive(7, -7);
}

bool detectObstacle()
{
  if (ultraSonicSensor() < 15){
    return true;
  }   
  else
  {
    return false;
  }
}

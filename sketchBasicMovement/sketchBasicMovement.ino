const int RIGHT_BACKWARD = 10;
const int RIGHT_FORWARD = 9;
const int LEFT_BACKWARD = 6;
const int LEFT_FORWARD = 5;
const int RIGHT_IN = 2;
const int LEFT_IN = 3;
const int GRIPPER = 11;
const byte WHEEL_SPEED = 255;
int RightPulses = 0;
int LastInterruptRight = 0;
int LeftPulses = 0;
int LastInterruptLeft = 0;
int RightWheel;
int LeftWheel;

void setup() 
  {
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

  //Forward 20 is een rotatie
  //elke rotatie is ~20 cm
  drive(80, 80);
  drive(-80, -80);
  drive(1, 20);
  drive(20, 1);
}
  
void loop() 
{
  
}

//Function called when right wheel rotates 1/20th of a rotation
void countPulseRightWheel()
{
  if(millis() - LastInterruptRight > 20)
  {
    RightPulses++;
    LastInterruptRight = millis();
  }
}

//Function called when left wheel rotates 1/20th of a rotation
void countPulseLeftWheel()
{
  if(millis() - LastInterruptLeft > 20)
  {
    LeftPulses++;
    LastInterruptLeft = millis();
  }
}

void drive(int RightDistance, int LeftDistance)
{ 
  //Handles giong backward 
  if(RightDistance > 0)
  {
    RightWheel = RIGHT_FORWARD;
  } else {
    RightWheel = RIGHT_BACKWARD;
  }
  if(LeftDistance > 0)
  {
    LeftWheel = LEFT_FORWARD;
  } else {
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
    if(LeftPulses > RightPulses)
    {
      analogWrite(RightWheel, WHEEL_SPEED / 1.5);
    }
    if(RightPulses > LeftPulses)
    {
      analogWrite(LeftWheel, WHEEL_SPEED / 1.5);
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
    debug(LeftPulses, LeftDistance, RightPulses, RightDistance);
  }
  analogWrite(RightWheel, 0);
  analogWrite(LeftWheel, 0);
  RightPulses = 0;
  LeftPulses = 0;
  delay(500);
}

void debug(int a, int b, int c, int d)
{
  Serial.print(a);
  Serial.print("/");
  Serial.print(b);
  Serial.print(" --- ");
  Serial.print(c);
  Serial.print("/");
  Serial.println(d);
}

const int PIN_1A = 10;
const int PIN_2A = 9;
const int PIN_1B = 6;
const int PIN_2B = 5;
const int PIN_1R = 13;
const int PIN_2R = 12;
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
  pinMode(PIN_1A, OUTPUT);
  pinMode(PIN_2A, OUTPUT);
  pinMode(PIN_1B, OUTPUT);
  pinMode(PIN_2B, OUTPUT);
  pinMode(PIN_1R, INPUT);
  pinMode(PIN_2R, INPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2), countPulseRightWheel, RISING);
  attachInterrupt(digitalPinToInterrupt(3), countPulseLeftWheel, RISING);

  //Forward 20 is een rotatie
  // elke rotatie is ~20 cm
  drive(80, 80);
  drive(-80, -80);
//  drive(-10, 10);
//  drive(10, -10);
}
  
void loop() 
{
  
}

void countPulseRightWheel()
{
  if(millis() - LastInterruptRight > 20)
  {
    RightPulses++;
    LastInterruptRight = millis();
  }
}

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
  if(RightDistance > 0)
  {
    RightWheel = PIN_2A;
  } else {
    RightWheel = PIN_1A;
  }

  if(LeftDistance > 0)
  {
    LeftWheel = PIN_2B;
  } else {
    LeftWheel = PIN_1B;
  }
  
  while(RightPulses <= abs(RightDistance) || LeftPulses <= abs(LeftDistance)){
    analogWrite(RightWheel, WHEEL_SPEED);
    analogWrite(LeftWheel, WHEEL_SPEED);

    if(LeftPulses > RightPulses)
    {
      analogWrite(RightWheel, WHEEL_SPEED / 1.5);
    }
    if(RightPulses > LeftPulses)
    {
      analogWrite(LeftWheel, WHEEL_SPEED / 1.5);
    }
    
    if(RightPulses >= abs(RightDistance))
    {
      analogWrite(RightWheel, 0);
    }
    if(LeftPulses >= abs(LeftDistance))
    {  
      analogWrite(LeftWheel, 0);
    } 
    debug(LeftPulses, abs(LeftDistance), RightPulses, abs(RightDistance));
  }
  analogWrite(PIN_2A, 0);
  analogWrite(PIN_2B, 0);
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

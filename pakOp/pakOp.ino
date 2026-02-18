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

void setup() {
  // put your setup code here, to run once:
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(RIGHT_IN, INPUT);
  pinMode(LEFT_IN, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

}

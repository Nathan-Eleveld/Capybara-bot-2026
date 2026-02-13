const int SENSOR_8 = A7;

void setup() {
  // put your setup code here, to run once:
  pinMode(SENSOR_8, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(analogRead(SENSOR_8));
}

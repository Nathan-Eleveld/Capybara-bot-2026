

void setup() {
  attachInterrupt(digitalPinToInterrupt(2), countPulse, RISING);
  Serial.begin(9600);
}

void loop() {
  

}

void countPulse(){
  Serial.println(millis());
}

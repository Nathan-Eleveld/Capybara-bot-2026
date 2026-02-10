int pulses = 0;

void setup() {
  pinMode(9, OUTPUT);
  pinMode(13, INPUT);
  Serial.begin(9600);
  
  // put your setup code here, to run once:
  digitalWrite(9, 200);
  while (millis() < 1000){
    if (digitalRead(13) == LOW){
      pulses++;
      Serial.println(pulses);
    }
    Serial.print("-");
  }
  digitalWrite(9, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

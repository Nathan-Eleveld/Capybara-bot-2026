long duration;
int distance;

const int ULTRA_SONIC_TRIG = 8;
const int ULTRA_SONIC_ECHO = 7;

void setup() {
  // put your setup code here, to run once:
  pinMode(ULTRA_SONIC_TRIG, OUTPUT);
  pinMode(ULTRA_SONIC_ECHO, INPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  ultraSonicSensor();
}

void ultraSonicSensor() {
    digitalWrite(ULTRA_SONIC_TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(ULTRA_SONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRA_SONIC_TRIG, LOW);

    duration = pulseIn(ULTRA_SONIC_ECHO, HIGH);

    distance = duration * 0.034 / 2;

    Serial.println("Distance: ");
    Serial.println(distance);
}

#include <SoftwareSerial.h>

//Create software serial object to communicate with HC-05
SoftwareSerial mySerial(12, 13); //HC-05 Tx & Rx is connected to Arduino #3 & #2

void setup()
{
  mySerial.begin(9600);
}

void loop()
{
  mySerial.println("YES");
}

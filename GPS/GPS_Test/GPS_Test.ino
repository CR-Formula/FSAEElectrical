#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
/*
   This sample sketch demonstrates the normal use of a TinyGPSPlus (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = 3, TXPin = 4;
static const uint32_t GPSBaud = 115200;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial GPS(RXPin, TXPin);

void setup()
{
  Serial.begin(115200);
  GPS.begin(GPSBaud);

  interrupts();
  attachInterrupt(digitalPinToInterrupt(RXPin), displayInfo, CHANGE);

  Serial.println(F("A simple demonstration of TinyGPSPlus with an attached GPS module"));
  Serial.println(F("by Mikal Hart"));
  Serial.println();
}

void loop()
{

}

void displayInfo()
{
  Serial.print(F("Location: ")); 
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);


  Serial.println();
}

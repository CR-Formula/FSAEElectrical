#include <SPI.h>
#include <RH_RF95.h>

#if defined (__AVR_ATmega32U4__)  // Mega
  #define RFM95_CS    8
  #define RFM95_INT   7
  #define RFM95_RST   4

#elif defined (__AVR_ATmega328P__)  // Uno
  #define RFM95_CS    4  //
  #define RFM95_INT   3  //
  #define RFM95_RST   2  // "A"

#endif

#define RF95_FREQ 915.0

typedef struct data_struct
{
  double RPM;        // RPM
  double TPS;        // TPS
  double FOT;        // Fuel Open Time
  double IA;         // Ignition Angle
  double Lam;        // Lambda
  double AirT;       // Air Temp
  double CoolT;      // Coolant Temp
  double Speed;      // Vehicle Speed
  double OilP;       // Oil Pressure
  double FuelP;      // Fuel Pressure
  double FLTemp;     // Front Left Brake Temp
  double FRTemp;     // Front Right Brake Temp
  double RLTemp;     // Rear Left Brake Temp
  double RRTemp;     // Rear Right Brake Temp
  double FRPot;      // Front Right Suspension Damper
  double FLPot;      // Front Left Suspension Damper
  double RRPot;      // Rear Right Suspension Damper
  double RLPot;      // Rear Left Suspension Damper
  double BrakeFront; // Front Brake Pressure
  double BrakeRear;  // Rear Brake Pressure
  double AccX;       // Accelerometer X Axis
  double AccY;       // Accelerometer Y Axis
  double AccZ;       // Accelerometer Z Axis
} data_struct;
data_struct telemetry;

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  while (!Serial) delay(1);
  delay(100);

  Serial.println("LoRa RX Test!");

  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);
}

void loop() {
  if (rf95.available()) { // Check for new message
    uint8_t buf[sizeof(telemetry)]; // buffer for message
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      memcpy(&telemetry, buf, sizeof(telemetry)); // Copy the message into the data struct
      // Print out the data struct
      Serial.println("Data:");
      Serial.println(telemetry.RPM);
      Serial.println(telemetry.TPS);
      Serial.println(telemetry.FOT);
      Serial.println(telemetry.IA);
      Serial.println(telemetry.Lam);
      Serial.println(telemetry.AirT);
      Serial.println(telemetry.CoolT);
      Serial.println(telemetry.Speed);
      Serial.println(telemetry.OilP);
      Serial.println(telemetry.FuelP);
      Serial.println(telemetry.FLTemp);
      Serial.println(telemetry.FRTemp);
      Serial.println(telemetry.RLTemp);
      Serial.println(telemetry.RRTemp);
      Serial.println(telemetry.FRPot);
      Serial.println(telemetry.FLPot);
      Serial.println(telemetry.RRPot);
      Serial.println(telemetry.RLPot);
      Serial.println(telemetry.BrakeFront);
      Serial.println(telemetry.BrakeRear);
      Serial.println(telemetry.AccX);
      Serial.println(telemetry.AccY);
      Serial.println(telemetry.AccZ);
      Serial.println("End of Data");
    }
  }
}
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

RH_RF95 rf95(RFM95_CS, RFM95_INT);

typedef struct data_struct
{
  uint8_t RPM;        // RPM
  uint32_t TPS;        // TPS
  uint32_t FOT;        // Fuel Open Time
  uint32_t IA;         // Ignition Angle
  uint32_t Lam;        // Lambda
  uint32_t AirT;       // Air Temp
  uint32_t CoolT;      // Coolant Temp
  uint32_t Speed;      // Vehicle Speed
  uint32_t OilP;       // Oil Pressure
  uint32_t FuelP;      // Fuel Pressure
  uint32_t FLTemp;     // Front Left Brake Temp
  uint32_t FRTemp;     // Front Right Brake Temp
  uint32_t RLTemp;     // Rear Left Brake Temp
  uint32_t RRTemp;     // Rear Right Brake Temp
  uint32_t FRPot;      // Front Right Suspension Damper
  uint32_t FLPot;      // Front Left Suspension Damper
  uint32_t RRPot;      // Rear Right Suspension Damper
  uint32_t RLPot;      // Rear Left Suspension Damper
  uint32_t BrakeFront; // Front Brake Pressure
  uint32_t BrakeRear;  // Rear Brake Pressure
  uint32_t AccX;       // Accelerometer X Axis
  uint32_t AccY;       // Accelerometer Y Axis
  uint32_t AccZ;       // Accelerometer Z Axis
} data_struct;
data_struct telemetry;

void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  while (!Serial) delay(1);
  delay(100);

  Serial.println("LoRa TX Test!");

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

  telemetry.RPM = 1;
  telemetry.TPS = 2;
  telemetry.FOT = 3;
  telemetry.IA = 4;
  telemetry.Lam = 5;
  telemetry.AirT = 6;
  telemetry.CoolT = 7;
  telemetry.Speed = 8;
  telemetry.OilP = 9;
  telemetry.FuelP = 10;
  telemetry.FLTemp = 11;
  telemetry.FRTemp = 12;
  telemetry.RLTemp = 13;
  telemetry.RRTemp = 14;
  telemetry.FRPot = 15;
  telemetry.FLPot = 16;
  telemetry.RRPot = 17;
  telemetry.RLPot = 18;
  telemetry.BrakeFront = 19;
  telemetry.BrakeRear = 20;
  telemetry.AccX = 21;
  telemetry.AccY = 22;
  telemetry.AccZ = 23;
}

void loop() {
  char radiopacket[sizeof(telemetry)]; // Buffer for data transmission
  memcpy(radiopacket, &telemetry, sizeof(telemetry)); // Copy data from struct

  Serial.println("Sending...");
  rf95.send((uint8_t *)radiopacket, sizeof(radiopacket));

  Serial.println("Waiting for packet to complete...");
  rf95.waitPacketSent();

  telemetry.RPM += 1;
  telemetry.TPS += 1;
  telemetry.FOT += 1;
  telemetry.IA += 1;
  telemetry.Lam += 1;
  telemetry.AirT += 1;
  telemetry.CoolT += 1;
  telemetry.Speed += 1;
  telemetry.OilP += 1;
  telemetry.FuelP += 1;
  telemetry.FLTemp += 1;
  telemetry.FRTemp += 1;
  telemetry.RLTemp += 1;
  telemetry.RRTemp += 1;
  telemetry.FRPot += 1;
  telemetry.FLPot += 1;
  telemetry.RRPot += 1;
  telemetry.RLPot += 1;
  telemetry.BrakeFront += 1;
  telemetry.BrakeRear += 1;
  telemetry.AccX += 1;
  telemetry.AccY += 1;
  telemetry.AccZ += 1;
}

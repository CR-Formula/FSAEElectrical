#include <SPI.h>
#include <RH_RF95.h>

// Uno
#define RFM95_CS    4
#define RFM95_INT   3
#define RFM95_RST   2
#define RF95_FREQ 915.0

typedef struct data_struct
{
  uint8_t RPM;            // RPM
  uint32_t TPS;           // TPS
  uint32_t FOT;           // Fuel Open Time
  uint32_t IA;            // Ignition Angle
  uint32_t Lam;           // Lambda
  uint32_t AirT;          // Air Temp
  uint32_t CoolT;         // Coolant Temp
  uint32_t Speed;         // Vehicle Speed
  uint32_t OilP;          // Oil Pressure
  uint32_t FuelP;         // Fuel Pressure
  uint32_t FLTemp;        // Front Left Brake Temp
  uint32_t FRTemp;        // Front Right Brake Temp
  uint32_t RLTemp;        // Rear Left Brake Temp
  uint32_t RRTemp;        // Rear Right Brake Temp
  uint32_t FRPot;         // Front Right Suspension Damper
  uint32_t FLPot;         // Front Left Suspension Damper
  uint32_t RRPot;         // Rear Right Suspension Damper
  uint32_t RLPot;         // Rear Left Suspension Damper
  uint32_t BrakeFront;    // Front Brake Pressure
  uint32_t BrakeRear;     // Rear Brake Pressure
  uint32_t AccX;          // Accelerometer X Axis
  uint32_t AccY;          // Accelerometer Y Axis
  uint32_t AccZ;          // Accelerometer Z Axis
} data_struct;
data_struct telemetry;

uint32_t message[256]; // buffer for message

RH_RF95 rf95(RFM95_CS, RFM95_INT);

/**
 * @brief Setup the LoRa module and Serial
 * @note The LoRa configuration must match the transmitter's configuration
 */
void setup() {
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  delay(5);

  // Reset LoRa Module
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Must match the transmitter's settings in CAN_Filter.ino
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

/**
 * @brief Super Loop to print out the received message over COM port
 */
void loop() {
  if (rf95.available()) { // Check for new message
    uint8_t buf[sizeof(telemetry)]; // buffer for message
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      memcpy(&telemetry, buf, sizeof(telemetry)); // Copy the message into the data struct
      // Print out the data struct
      Serial.print(telemetry.RPM);
      Serial.print(", ");
      Serial.print(telemetry.TPS);
      Serial.print(", ");
      Serial.print(telemetry.FOT);
      Serial.print(", ");
      Serial.print(telemetry.IA);
      Serial.print(", ");
      Serial.print(telemetry.Lam);
      Serial.print(", ");
      Serial.print(telemetry.AirT);
      Serial.print(", ");
      Serial.print(telemetry.CoolT);
      Serial.print(", ");
      Serial.print(telemetry.Speed);
      Serial.print(", ");
      Serial.print(telemetry.OilP);
      Serial.print(", ");
      Serial.print(telemetry.FuelP);
      Serial.print(", ");
      Serial.print(telemetry.FLTemp);
      Serial.print(", ");
      Serial.print(telemetry.FRTemp);
      Serial.print(", ");
      Serial.print(telemetry.RLTemp);
      Serial.print(", ");
      Serial.print(telemetry.RRTemp);
      Serial.print(", ");
      Serial.print(telemetry.FRPot);
      Serial.print(", ");
      Serial.print(telemetry.FLPot);
      Serial.print(", ");
      Serial.print(telemetry.RRPot);
      Serial.print(", ");
      Serial.print(telemetry.RLPot);
      Serial.print(", ");
      Serial.print(telemetry.BrakeFront);
      Serial.print(", ");
      Serial.print(telemetry.BrakeRear);
      Serial.print(", ");
      Serial.print((float)telemetry.AccX, );
      Serial.print(", ");
      Serial.print((float)telemetry.AccY);
      Serial.print(", ");
      Serial.print((float)telemetry.AccZ);
      Serial.println();
    }
  }
}
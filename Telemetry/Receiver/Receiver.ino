#include <SPI.h>
#include <RH_RF95.h>

// Uno
#define RFM95_CS    4
#define RFM95_INT   3
#define RFM95_RST   2
#define RF95_FREQ 915.0

typedef struct data_struct
{
  float RPM;           // RPM
  float TPS;           // TPS
  float FOT;           // Fuel Open Time
  float IA;            // Ignition Angle
  float Lam;           // Lambda
  float AirT;          // Air Temp
  float CoolT;         // Coolant Temp
  float Speed;         // Vehicle Speed
  float OilP;          // Oil Pressure
  float FRTemp;        // Front Right Brake Temp
  float RRTemp;        // Rear Right Brake Temp
  float FRPot;         // Front Right Suspension Damper
  float RRPot;         // Rear Right Suspension Damper
  float AccX;          // Accelerometer X Axis
  float AccZ;          // Accelerometer Z Axis
  float AccY;          // Accelerometer Y Axis
} data_struct;
data_struct telemetry;

uint32_t message[256]; // buffer for message

uint8_t buf[sizeof(telemetry)]; // buffer for message

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
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      memcpy(&telemetry, buf, sizeof(telemetry)); // Copy the message into the data struct
      // Print out the data struct
      Serial.print((int)telemetry.RPM);
      Serial.print(", ");
      Serial.print((int)telemetry.TPS);
      Serial.print(", ");
      Serial.print((double)telemetry.FOT);
      Serial.print(", ");
      Serial.print((double)telemetry.IA);
      Serial.print(", ");
      Serial.print((double)telemetry.Lam);
      Serial.print(", ");
      Serial.print((double)telemetry.AirT);
      Serial.print(", ");
      Serial.print((double)telemetry.CoolT);
      Serial.print(", ");
      Serial.print((double)telemetry.Speed);
      Serial.print(", ");
      Serial.print((double)telemetry.OilP);
      Serial.print(", ");
      Serial.print((double)telemetry.FRTemp);
      Serial.print(", ");
      Serial.print((double)telemetry.RRTemp);
      Serial.print(", ");
      Serial.print((double)telemetry.FRPot);
      Serial.print(", ");
      Serial.print((double)telemetry.RRPot);
      Serial.print(", ");
      Serial.print((double)telemetry.AccX);
      Serial.print(", ");
      Serial.print((double)telemetry.AccY);
      Serial.print(", ");
      Serial.print((double)telemetry.AccZ);
      Serial.println();
    }
  }
}
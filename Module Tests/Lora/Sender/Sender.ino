#include <SPI.h>
#include <RH_RF95.h>

#if defined(__AVR_ATmega32U4__) // Feather 32u4 w/Radio
#define RFM95_CS 8
#define RFM95_INT 7
#define RFM95_RST 4
#elif defined(__AVR_ATmega328P__) // Feather 328P w/wing
#define RFM95_CS 4                //
#define RFM95_INT 3               //
#define RFM95_RST 2               // "A"
#endif

#define RF95_FREQ 915.0

typedef struct data_struct
{
  float RPM;        // RPM
  float TPS;        // TPS
  float FOT;        // Fuel Open Time
  float IA;         // Ignition Angle
  float Lam;        // Lambda
  float AirT;       // Air Temp
  float CoolT;      // Coolant Temp
  float Speed;      // Vehicle Speed
  float OilP;       // Oil Pressure
  float FuelP;      // Fuel Pressure
  float FLTemp;     // Front Left Brake Temp
  float FRTemp;     // Front Right Brake Temp
  float RLTemp;     // Rear Left Brake Temp
  float RRTemp;     // Rear Right Brake Temp
  float FRPot;      // Front Right Suspension Damper
  float FLPot;      // Front Left Suspension Damper
  float RRPot;      // Rear Right Suspension Damper
  float RLPot;      // Rear Left Suspension Damper
  float BrakeFront; // Front Brake Pressure
  float BrakeRear;  // Rear Brake Pressure
  float AccX;       // Accelerometer X Axis
  float AccY;       // Accelerometer Y Axis
  float AccZ;       // Accelerometer Z Axis
} data_struct;
data_struct telemetry;

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup()
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  while (!Serial)
    delay(1);
  delay(100);

  Serial.println("LoRa TX Test!");

  // Manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init())
  {
    Serial.println("LoRa radio init failed");
    while (1)
      ;
  }
  Serial.println("LoRa radio init OK!");

  if (!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while (1)
      ;
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);
  rf95.setTxPower(23, false);
}

int16_t packetnum = 0; // packet counter, we increment per xmission

void loop()
{
  telemetry.RPM++;
  telemetry.TPS++;
  delay(1000); // Delay for debugging
  Serial.println("Transmitting...");
  delay(10);
  rf95.send((uint8_t *)&telemetry, sizeof(telemetry));
  Serial.println(telemetry.RPM);

  Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply...");
  if (rf95.waitAvailableTimeout(1000))
  {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len))
    {
      Serial.print("Got reply: ");
      Serial.println((char *)buf);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
  }
}

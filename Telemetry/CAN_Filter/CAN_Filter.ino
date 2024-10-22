/*Iowa State Formula SAE Electrical Subsystem*/

#include <mcp_can.h> // CAN Bus Library
#include <SPI.h> // SPI Library
#include <Adafruit_LSM9DS1.h> // IMU Library
#include <Adafruit_Sensor.h> // Sensor Library
#include <RH_RF95.h> // LoRa Library
#include <math.h> // Math Library
#include <stdio.h> // Standard I/O Library
#include <stdlib.h> // Standard Library

#define FR_THERMO A1
#define ACC_X A15
#define ACC_Y A13
#define ACC_Z A14
#define FR_POT A12
#define RR_POT A12

void Accel_Cal(); // Calibrate Accelerometer
void Accel_Read(); // Read Accelerometer
void CAN_Data(); // Read in CAN Data
void Telemetry_Filter(); // Filter out extraneous values
void Send_Dash(); // Send values to update the dash display
void Suspension_Pot(); // Read in analog Suspension Damper Potentiometers
void Brake_Pressure(); // Read in analog Brake Pressure values
void Lora_Send(); // Send data over LoRa
void Print_Test_Data(); // Print test data to validate connections

// Stores the CAN Packet ID
long unsigned int rxId;
unsigned char len = 0;
// Length of the Buffer for Packet Data
unsigned char rxBuf[8];
// Stores the CAN packet serial message
char msgString[128];
static const int RxPin = 3;
static const int TxPin = 4;

// Sets INT to pin 2
#define CAN0_INT    2
// Sets CS
// MCP_CAN CAN0(10); // Uno
MCP_CAN CAN0(53); // Mega

// Defines for the LoRa Module Pins
#define RFM95_CS    49
#define RFM95_INT   3
#define RFM95_RST   7
#define RF95_FREQ   915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT); // LoRa Module

// Holds all calculated Telemetry Data
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

// Stores Calculated Brake Bias Value
double brakeBias = 0;

// Variables used for filtering out extraneous values
int RPMLast;
float TPSLast;
float FOTLast;
float IALast;
float LamLast;
float AirTLast;
float CoolTLast;
float OilPLast;

// Variables for Accelerometer
int     XYZ_RAW[3];
int     XYZ_Cal_Offset[3];
double  XYZ_G[3];
uint8_t CALIBRATION_DONE;

char message[64]; // Buffer for Dash Message

char buf[sizeof(telemetry)]; // Buffer for data transmission

/**
 * @brief Initialize Peripherials and Sensors
 */
void setup() {
  Serial2.begin(115200); // Serial port 2 for Nextion

  // Init LoRa Module reset pin
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // Initializes MCP2515 running at 16MHz with a baudrate of 250kb/s and the masks and filters disabled.
  if (CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");

  // Set MCP2515 operation to normal
  CAN0.setMode(MCP_NORMAL);

  // Configure the INT input pin
  pinMode(CAN0_INT, INPUT);

  // Reset LoRa Module
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Configure the LoRa Module
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
  // TODO: Look into Spreading Factor and bandwidth settings
  // SF11/500kHz  bitrate: 1760 Max   Payload Size: 109

  // Initialize Accelerometer
  CALIBRATION_DONE = 0;
  Serial.begin(115200);
  Accel_Cal();
}

/**
 * @brief Function to read Data fron PE3 ECU
 * @note All 2 byte data is stored [LowByte, HighByte]
 * @note Num = (LowByte + HighByte * 256) * resolution
 */
void CAN_Data() {
  if (!digitalRead(CAN0_INT))
  {
    // Read Id, length, and message data
    CAN0.readMsgBuf(&rxId, &len, rxBuf);

    // check for specific CAN ID. pull needed data, and repackage the data
    // Change if added Sensors
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF048) {
      telemetry.RPM = (rxBuf[0] + rxBuf[1] * 256); // RPM
      telemetry.TPS = (rxBuf[2] + rxBuf[3] * 256) * 0.1; // Throttle Position Sensor
      telemetry.FOT = (rxBuf[4] + rxBuf[5] * 256) * 0.1; // Fuel Open Time
      telemetry.IA = (rxBuf[6] + rxBuf[7] * 256) * 0.1; // Igition Angle
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF148) {
      telemetry.Lam = (rxBuf[4] + rxBuf[5] * 256) * 0.001; // Lambda
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF548) {
      telemetry.AirT = (rxBuf[2] + rxBuf[3] * 256) * 0.1; // Air Temp
      telemetry.CoolT = (rxBuf[4] + rxBuf[5] * 256) * 0.1; // Coolant Temp
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF348) { 
      telemetry.OilP = (((rxBuf[6] + rxBuf[7] * 256) * 0.001) - .5) / 4.0 * 150.0; // Oil Pressure PSI
    }
  }
}

/**
 * @brief Reads data from the Analog Accelerometer
 * @note Applies calibration offset
 * 
 */
void Accel_Read() {
  telemetry.AccX = ((float)(analogRead(ACC_X) - XYZ_Cal_Offset[0]) / 170.667); /* Acceleration in g units */
  telemetry.AccY = ((float)(analogRead(ACC_Y) - XYZ_Cal_Offset[1]) / 170.667);
  telemetry.AccZ = ((float)(analogRead(ACC_Z) - XYZ_Cal_Offset[2]) / 170.667);
}

/**
 * @brief Calibrate Accelerometer
 * @note Reads in 5 values and averages them
 * 
 */
void Accel_Cal() {

  for (int i = 0; i < 5; i++) {
    XYZ_Cal_Offset[0] += analogRead(ACC_X);
    XYZ_Cal_Offset[1] += analogRead(ACC_Y);
    XYZ_Cal_Offset[2] += analogRead(ACC_Z);
  }

  XYZ_Cal_Offset[0] = XYZ_Cal_Offset[0] / 5;
  XYZ_Cal_Offset[1] = XYZ_Cal_Offset[1] / 5;
  XYZ_Cal_Offset[2] = XYZ_Cal_Offset[2] / 5;

  delay(1);
  CALIBRATION_DONE = 1;
}

/**
 * @brief Function to filter out extraneous values
 * @note Originaly written to reduce Time Slicing
 */
void Telemetry_Filter() {
  if (telemetry.RPM > 20000) {
      telemetry.RPM = RPMLast;
    }
    if (abs(TPSLast - telemetry.TPS) > 90 && TPSLast > 0) {
      telemetry.TPS = TPSLast;
    }
    if (abs(FOTLast - telemetry.FOT) > 10 && FOTLast > 0) {
      telemetry.FOT = FOTLast;
    }
    if (abs(IALast - telemetry.IA) > 50 && IALast > 0 || telemetry.IA > 40) {
      telemetry.IA = IALast;
    }
    if (abs(LamLast - telemetry.Lam) > 1 && LamLast > 0) {
      telemetry.Lam = LamLast;
    }
    if (abs(AirTLast - telemetry.AirT) > 25 && AirTLast > 0) {
      telemetry.AirT = AirTLast;
    }
    if (abs(CoolTLast - telemetry.CoolT) > 50 && CoolTLast > 0) {
      telemetry.CoolT = CoolTLast;
    }

  // Save last filtered values
  RPMLast = telemetry.RPM;
  TPSLast = telemetry.TPS;
  FOTLast = telemetry.FOT;
  IALast = telemetry.IA;
  LamLast = telemetry.Lam;
  AirTLast = telemetry.AirT;
  CoolTLast = telemetry.CoolT;
  OilPLast = telemetry.OilP;
}

/**
 * @brief Writes End Command to the Nextion Display
 */
void Nextion_CMD() {
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
}

/**
 * @brief Send Data to the Dash
 * @note The telemetry values have to match Nextion Object names
 * @todo Add shift light functionality
 */
void Send_Dash() {
  // Send dash values as text objects

  sprintf(message, "%d\"", (int)telemetry.RPM);
  Serial2.print("rpm.txt=\"");
  Serial2.print(message); 
  Nextion_CMD();

  sprintf(message, "%d\"", (int)telemetry.CoolT);
  Serial2.print("waterTemp.txt=\"");
  Serial2.print(message);
  Nextion_CMD();

  sprintf(message, "%d\"", (int)telemetry.OilP);
  Serial2.print("oilPress.txt=\"");
  Serial2.print(message);
  Nextion_CMD();

  sprintf(message, "%d\"", (int)telemetry.FRTemp);
  Serial2.print("brakeFR.txt=\"");
  Serial2.print(message);
  Nextion_CMD();

  sprintf(message, "%d\"", (int)telemetry.RRTemp);
  Serial2.print("brakeRR.txt=\"");
  Serial2.print(message);
  Nextion_CMD();

  int rpmBar = telemetry.RPM / 160;
  Serial2.print("rpmBar.val=");
  Serial2.print(rpmBar);
  Nextion_CMD();

  // Send value for shift lights
  // Shift light range from 8525 - 15500
  if (telemetry.RPM > 7750) {
    int shiftlights = ((telemetry.RPM - 7750) * 80) / 7750;
    digitalWrite(5, shiftlights);
  }
}

/**
 * @brief Reads in suspension potentiometer values
 * @todo verify conversions
 */
void Suspension_Pot() {
  // telemetry.FLPot = ((double)analogRead(A0) * 50.0) / 1023.0;
  telemetry.FRPot = ((double)analogRead(FR_POT) * 50.0) / 1023.0;
  // telemetry.RLPot = ((double)analogRead(A2) * 50.0) / 1023.0;
  telemetry.RRPot = ((double)analogRead(RR_POT) * 50.0) / 1023.0;
}

/**
 * @brief Send the LoRa Packet
 * 
 */
void Lora_Send() {
  memcpy(buf, &telemetry, sizeof(telemetry)); // Copy data from struct
  rf95.send((uint8_t *)buf, sizeof(buf)); // Send Data
}

/**
 * @brief Caclulate Brake Temperature
 * @note Uses a thermocouple to calculate temperature
 * 
 */
void Brake_Temp() {
  int FRTemp = analogRead(FR_THERMO);

  // T = (V - 1.25) / .005
  telemetry.FRTemp = ((((double)FRTemp * 5) / 1023.0) - 1.25) / .005;
}

/**
 * @brief Function to print test data for debug
 * @note Comment this out when you upload to the car
 */
void Print_Test_Data() {
  Serial.println();
  Serial.println(telemetry.TPS);
  Serial.println(telemetry.FRTemp);
  Serial.println(brakeBias);
  Serial.println();
}

/**
 * @brief Super loop that calls all the telemetry functions
 * 
 */
void loop() {
  // function calls for each sensor/module
  CAN_Data();
  Brake_Temp();
  // Telemetry_Filter();
  // Suspension_Pot();
  Accel_Read();
  // Send_Dash();
  Lora_Send();

  Print_Test_Data();
  
  delay(1); // Delay for stability
}
/*2021-22 Iowa State Formula SAE Electrical Subsystem*/

#include <mcp_can.h>        //TODO: Need this library for the time being -- may change with the MEGA switch
#include <SPI.h>            //TODO: May not need this Library
#include <TinyGPSPlus.h>    //TODO: See Below
#include <SoftwareSerial.h> //TODO: Need to transfer GPS to I2C
#include <EasyTransfer.h>

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
#define CAN0_INT 2
// Sets CS to pin 10
MCP_CAN CAN0(10);

// Create EasyTransfer Object
EasyTransfer ET;

// Holds all calculated Telemetry Data
typedef struct data_struct {
  int RPM;     // Holds RPM value
  float TPS;   // Holds TPS value
  float FOT;   // holds Fuel Open Time value
  float IA;    // Holds Ignition Angle value
  float Lam;   // Holds Lambda value
  float AirT;  // Holds Air Temp value
  float CoolT; // Holds Coolent Temp value
  float Lat;   // Holds Latitude
  float Lng;   // Holds Longitude
  float Speed; // Holds GPS Speed
  float OilP;  // Holds Oil Pressure
} data_struct;
data_struct telemetry;

// Variables used for filtering out extraneous values
int RPMLast;
float TPSLast;
float FOTLast;
float IALast;
float LamLast;
float AirTLast;
float CoolTLast;
float OilPLast;

void setup() {
  Serial.begin(115200);

  // Initializes MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if (CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");

  // Set MCP2515 operation to normal
  CAN0.setMode(MCP_NORMAL);

  // Configure the INT input pin
  pinMode(CAN0_INT, INPUT);

  // Start EasyTransfer
  ET.begin(details(telemetry), &Serial);
}

void loop() {
  // Checks for CAN0_INT pin to be low, then reads the recieve buffer
  //variables for the PE3 ECU CAN data
  //All 2 byte data is stored [LowByte, HighByte]
  //Num = (LowByte + HighByte * 256) * resolution
  if (!digitalRead(CAN0_INT))
  {
    // Read Data to data buffer array
    CAN0.readMsgBuf(&rxId, &len, rxBuf);

    // check for specific CAN ID. pull need data, and repackage the data
    // Change if added Sensors
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF048) {
      telemetry.RPM = (rxBuf[0] + rxBuf[1] * 256); //RPM
      telemetry.TPS = (rxBuf[2] + rxBuf[3] * 256) * 0.1; //Throttle Position Sensor
      telemetry.FOT = (rxBuf[4] + rxBuf[5] * 256) * 0.1; //Fuel Open Time
      telemetry.IA = (rxBuf[6] + rxBuf[7] * 256) * 0.1; //Igition Angle
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF148) {
      telemetry.Lam = (rxBuf[4] + rxBuf[5] * 256) * 0.01; //Lambda
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF548) {
      telemetry.AirT = (rxBuf[2] + rxBuf[3] * 256) * 0.1; //Air Temp
      telemetry.CoolT = (rxBuf[4] + rxBuf[5] * 256) * 0.1; //Coolant Temp

      /*//Code to test values
      Serial.print(rxBuf[4], DEC);
      Serial.print(rxBuf[5], DEC);
      delay(50);*/
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF348) { 
      telemetry.OilP = (rxBuf[6] + rxBuf[7] * 256) * 0.001; //Oil Pressure
    }
  }

  // Will add Sensor reads here

  //Filters out extraneous values for each CAN Value
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

    // Send the data over Serial using EasyTransfer
    ET.sendData();

    //delay for stability
    delay(2);

    //Save last correct values
    RPMLast = telemetry.RPM;
    TPSLast = telemetry.TPS;
    FOTLast = telemetry.FOT;
    IALast = telemetry.IA;
    LamLast = telemetry.Lam;
    AirTLast = telemetry.AirT;
    CoolTLast = telemetry.CoolT;
    OilPLast = telemetry.OilP;
}

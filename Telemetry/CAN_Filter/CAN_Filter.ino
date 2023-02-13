/*Iowa State Formula SAE Electrical Subsystem*/

#include <mcp_can.h>
#include <SPI.h>
#include <EasyTransfer.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>

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
// Sets CS
//MCP_CAN CAN0(10); //Uno
MCP_CAN CAN0(53); //Mega


//Objects for brake temp sensors
Adafruit_MLX90614 FLB = Adafruit_MLX90614(); //Front Left Brake Temp
Adafruit_MLX90614 FRB = Adafruit_MLX90614(); //Front Right Brake Temp
Adafruit_MLX90614 RLB = Adafruit_MLX90614(); //Rear Left Brake Temp
Adafruit_MLX90614 RRB = Adafruit_MLX90614(); //Rear Right Brake Temp


// Create EasyTransfer Object
EasyTransfer ET;

// Holds all calculated Telemetry Data
typedef struct data_struct {
  float RPM;     // Holds RPM value
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
  float FLTemp; //Holds Front Left Brake Temp
  float FRTemp; //Holds Front Right Brake Temp
  float RLTemp; //Holds Rear Left Brake Temp
  float RRTemp; //Holds Rear Right Brake Temp
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

//Function to update Nextion display
void Nextion_CMD() {
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
}

void setup() {
  Serial.begin(115200); //Serial Port 0 for ESP
  Serial2.begin(115200); //Serial port 2 for Nextion
  Wire.begin(); //I2C for Sensors

  // Initializes MCP2515 running at 16MHz with a baudrate of 250kb/s and the masks and filters disabled.
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

  //initalize brake temp sensors
  FLB.begin(0x5A, &Wire);
  FRB.begin(0x5B, &Wire);
  RLB.begin(0x5C, &Wire);
  RRB.begin(0x5D, &Wire);
}

void loop() {
  // Checks for CAN0_INT pin to be low, then reads the recieve buffer
  //variables for the PE3 ECU CAN data
  //All 2 byte data is stored [LowByte, HighByte]
  //Num = (LowByte + HighByte * 256) * resolution
  if (!digitalRead(CAN0_INT))
  {
    // Read Id, length, and message data
    CAN0.readMsgBuf(&rxId, &len, rxBuf);

    // check for specific CAN ID. pull needed data, and repackage the data
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

      /*//Code to test bit CAN values
      Serial.print(rxBuf[4], DEC);
      Serial.print(rxBuf[5], DEC);
      delay(50);*/
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF348) { 
      telemetry.OilP = (rxBuf[6] + rxBuf[7] * 256) * 0.001; //Oil Pressure
    }
  }

  //Function calls to read brake temp sensor values
  telemetry.FLTemp = FLB.readObjectTempC();
  telemetry.FRTemp = FRB.readObjectTempC();
  telemetry.RLTemp = RLB.readObjectTempC();
  telemetry.RRTemp = RRB.readObjectTempC();

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
    ET.sendData(); //Writes a bunch of junk to serial monitor, this is normal as it uses .write()

    //Send dash values as text objects
    char message[32];
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
    int rpmBar = telemetry.RPM / 160;
    Serial2.print("rpmBar.val=");
    Serial2.print(rpmBar);
    Nextion_CMD();
    //TODO: Add gear and Laptimes

    //Send value for shift lights
    //Shift light range from 8525 - 15500
    if (telemetry.rpm > 7750) {
      int shiftlights ((telemetry.rpm - 7750) * 80) / 7750;
      digitalWrite(5, shiftlights);
    }

    //delay for stability
    delay(5);

    //Sample Data for testing
    Serial.println();
    Serial.println(telemetry.TPS);
    Serial.println(telemetry.FRTemp);
    Serial.println(rpmBar);
    Serial.println(shift_lights);
    Serial.println();


    //Save last filtered values
    RPMLast = telemetry.RPM;
    TPSLast = telemetry.TPS;
    FOTLast = telemetry.FOT;
    IALast = telemetry.IA;
    LamLast = telemetry.Lam;
    AirTLast = telemetry.AirT;
    CoolTLast = telemetry.CoolT;
    OilPLast = telemetry.OilP;
}

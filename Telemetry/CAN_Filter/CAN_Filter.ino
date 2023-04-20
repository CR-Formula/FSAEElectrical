/*Iowa State Formula SAE Electrical Subsystem*/

#include <mcp_can.h>
#include <SPI.h>
#include <EasyTransfer.h>
#include <Adafruit_MLX90614.h>
#include <ICM_20948.h>
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
// MCP_CAN CAN0(10); // Uno
MCP_CAN CAN0(53); // Mega

// Create Object for Accel/Gyro
ICM_20948_I2C myICM;

//Objects for brake temp sensors
Adafruit_MLX90614 FLB = Adafruit_MLX90614(); // Front Left Brake Temp
Adafruit_MLX90614 FRB = Adafruit_MLX90614(); // Front Right Brake Temp
Adafruit_MLX90614 RLB = Adafruit_MLX90614(); // Rear Left Brake Temp
Adafruit_MLX90614 RRB = Adafruit_MLX90614(); // Rear Right Brake Temp


// Create EasyTransfer Object
EasyTransfer ET;

// Code to setup transmission speed interrupts
#define transSpeedINT 3 // Interrupt Pin
long startTime = millis();
long endTime;
long totalTime = 0; // Difference in start and end
long outputRPM = 0; // Calculated output shaft RPM
const int outputTeeth = 24; // number of gear teeth
char Gear = 'N'; // Holds temp gear value

// Holds all calculated Telemetry Data
typedef struct data_struct {
  float RPM;        // RPM value
  float TPS;        // TPS value
  float FOT;        // Fuel Open Time value
  float IA;         // Ignition Angle value
  float Lam;        // Lambda value
  float AirT;       // Air Temp value
  float CoolT;      // Coolent Temp value
  float Lat;        // Latitude
  float Lng;        // Longitude
  float Speed;      // GPS Speed
  float OilP;       // Oil Pressure
  float FuelP;      // Fuel Pressure
  float FLTemp;     // Front Left Brake Temp
  float FRTemp;     // Front Right Brake Temp
  float RLTemp;     // Rear Left Brake Temp
  float RRTemp;     // Rear Right Brake Temp
  float FRPot;      // Front Right suspension damper
  float FLPot;      // Front Left suspension damper
  float RRPot;      // Rear Right suspension damper
  float RLPot;      // Rear Left suspension damper
  float BrakeFront; // Front Brake Pressure
  float BrakeRear;  // Rear Brake Pressure
  float AccX;       // Acclerometer X Axis
  float AccY;       // Acclerometer Y Axis
  float AccZ;       // Acclerometer Z Axis
  float GyrX;       // Gyroscope X Axis
  float GyrY;       // Gyroscope Y Axis
  float GyrZ;       // Gyroscope Z Axis
  float MagX;       // Magnometer X Axis
  float MagY;       // Magnometer Y Axis
  float MagZ;       // Magnometer Z Axis
  float GearRatio;  // Stores the current gear ratio
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

// Function to initialize sensors, connections, and serial ports
void setup() {
  Serial.begin(115200); // Serial Port 0 for ESP
  Serial2.begin(115200); // Serial port 2 for Nextion
  Wire.begin(); // I2C for Sensors

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

  // initalize brake temp sensors
  FLB.begin(0x5A, &Wire);
  FRB.begin(0x5B, &Wire);
  RLB.begin(0x5C, &Wire);
  RRB.begin(0x5D, &Wire);

  // Initialize ICM
  myICM.begin(Wire);

  // Initialize Transmission Speed Interrupts
  pinMode(transSpeedINT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(transSpeedINT), outputCount, CHANGE);
}

// Function to read in CAN data from the ECU
void CAN_Data() {
  // Checks for CAN0_INT pin to be low, then reads the recieve buffer
  // variables for the PE3 ECU CAN data
  // All 2 byte data is stored [LowByte, HighByte]
  // Num = (LowByte + HighByte * 256) * resolution
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
      telemetry.Lam = (rxBuf[4] + rxBuf[5] * 256) * 0.01; // Lambda
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF548) {
      telemetry.AirT = (rxBuf[2] + rxBuf[3] * 256) * 0.1; // Air Temp
      telemetry.CoolT = (rxBuf[4] + rxBuf[5] * 256) * 0.1; // Coolant Temp
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF348) { 
      telemetry.FuelP = (rxBuf[0] + rxBuf[1] * 256) * 0.001; // Fuel Pressure
      telemetry.FuelP = (rxBuf[0] + rxBuf[1] * 256) * 0.001; // Fuel Pressure
      telemetry.OilP = (rxBuf[6] + rxBuf[7] * 256) * 0.001; // Oil Pressure
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFFC48) {
      double wheel1 = (rxBuf[4] + rxBuf[5] * 256) * 0.1; // Non driven wheel speed 1 (ft/s)
      double wheel2 = (rxBuf[6] + rxBuf[7] * 256) * 0.1; // Non driven wheel speed 2 (ft/s)
      if (wheel1 > wheel2) { // Take the higher of the two wheel speeds
        telemetry.Speed = wheel1 / 1.467;
      }
      else {
        telemetry.Speed = wheel2 / 1.467;
      }
    }
  }
}

// Function calls to read brake temp sensor values
void Brake_Temp() {
  telemetry.FLTemp = FLB.readObjectTempC();
  telemetry.FRTemp = FRB.readObjectTempC();
  telemetry.RLTemp = RLB.readObjectTempC();
  telemetry.RRTemp = RRB.readObjectTempC();
}

// Function to filter out extraneous values
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

// write function to update Nextion display
void Nextion_CMD() {
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
}

// Function to send values to updated the dash display and shift lights
void Send_Dash() {
  // Send dash values as text objects
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
  Serial2.print("Gear.txt=\"");
  Serial2.print(Gear);
  Nextion_CMD();
  // TODO: Laptimes

  // Send value for shift lights
  // Shift light range from 8525 - 15500
  if (telemetry.RPM > 7750) {
    int shiftlights = ((telemetry.RPM - 7750) * 80) / 7750;
    digitalWrite(5, shiftlights);
  }
}

// Read in analog Suspention Damper Potentiometers
void Suspension_Pot() {
  // Values are 0-1023 and map to 0-50mm
  int FLPot = analogRead(A0);
  int FRPot = analogRead(A1);
  int RLPot = analogRead(A2);
  int RRPot = analogRead(A3);

  // AiM 50mm Suspension Pots
  // Prints in mm
  telemetry.FLPot = ((double)FLPot * 50.0) / 1023.0;
  telemetry.FRPot = ((double)FRPot * 50.0) / 1023.0;
  telemetry.RLPot = ((double)RLPot * 50.0) / 1023.0;
  telemetry.RRPot = ((double)RRPot * 50.0) / 1023.0;
}

// Read in analog break pressure values
void Brake_Pressure() {
  int FrontPres = analogRead(A4);
  int RearPres = analogRead(A5);

  // .5V - 4.5V --> 0 Bar - 100 Bar
  // Print in PSI --> * 14.504
  // Brake Bias (Front / Rear) * 100
  telemetry.BrakeFront = (((double)FrontPres * 100.0) / 1023.0) * 14.504;
  telemetry.BrakeRear = (((double)RearPres * 100.0) / 1023.0) * 14.504;
}

// Function to print test data to validate connections
void Print_Test_Data() {
  Serial.println();
  Serial.println(telemetry.TPS);
  Serial.println(telemetry.FRTemp);
  Serial.println(telemetry.FRTemp);
  Serial.println();
}

// Function that collects ICM Data from the Accel/Gyro
void ICM_Data(ICM_20948_I2C *sensor) {
  myICM.getAGMT();

  // Accelerometer Values
  telemetry.AccX = sensor->accX();
  telemetry.AccY = sensor->accY();
  telemetry.AccZ = sensor->accZ();

  // Gyroscope Values
  telemetry.GyrX = sensor->gyrX();
  telemetry.GyrY = sensor->gyrY();
  telemetry.GyrZ = sensor->gyrZ();

  // Mag Values
  telemetry.MagX = sensor->magX();
  telemetry.MagY = sensor->magY();
  telemetry.MagZ = sensor->magZ();
}

// Calculate Transmission speed based off of interrupts
void outputCount() {
  toothCount++;
}

// Calculate the Gear Ratio given Trans Speed and Engine RPM
void gear_Ratio() {
  // Define Trans Speed Sensor Pin
  // Engine Speed / Trans Speed = Gear Ratio
  if (toothCount >= outputTeeth) {
    endTime = millis();
    totalTime = endTime - startTime;
    outputRPM = 1.0 / (totalTime / 60000.0);
  }
  telemetry.GearRatio = telemetry.RPM / outputRPM;
  // Gear Ratio (1/2/3/4/5/6) -->	2.583/2/1.667/1.444/1.286/1.15
  if (telemetry.GearRatio >= 2.292 && telemetry.GearRatio < 2.583 * 1.11) { // 2.583
    Gear = '1';
  }
  else if (telemetry.GearRatio >= 1.883 && telemetry.GearRatio < 2.292) { // 2
    Gear = '2';
  }
  else if (telemetry.GearRatio >= 1.556 && telemetry.GearRatio < 1.883) { // 1.667
    Gear = '3';
  }
  else if (telemetry.GearRatio >= 1.365 && telemetry.GearRatio < 1.556) { // 1.444
    Gear = '4';
  }
  else if (telemetry.GearRatio >= 1.218 && telemetry.GearRatio < 1.365) { // 1.286
    Gear = '5';
  }
  else if (telemetry.GearRatio >= 1.15 * 0.94 && telemetry.GearRatio < 1.218) { // 1.15
    Gear = '6';
  }
  else {
    Gear = 'N';
  }
}

void loop() {
  // function calls for each sensor/module
  CAN_Data();
  Brake_Temp();
  Telemetry_Filter();
  Suspension_Pot();
  ICM_Data(&myICM);
  Brake_Pressure();
  Send_Dash();
  gear_Ratio();

  // Send the data over Serial using EasyTransfer library
  ET.sendData(); // Writes a bunch of junk to serial monitor, this is normal as it uses .write()

  // delay for stability
  delay(5);

  // Print_Test_Data();
}

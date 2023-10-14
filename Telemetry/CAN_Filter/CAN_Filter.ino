/*Iowa State Formula SAE Electrical Subsystem*/

#include <mcp_can.h>
#include <SPI.h>
#include <EasyTransfer.h>
#include <Adafruit_MLX90614.h>

#include <Wire.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>

// Create Object for Adafruit IMU
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

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

byte sendBuffer[8];

// Variables for Moving Average 
#define WINDOW_SIZE 10
int INDEX_X, INDEX_Y, INDEX_Z = 0;
double VALUE_X, VALUE_Y, VALUE_Z = 0;
double SUM_X, SUM_Y, SUM_Z = 0;
double READINGS_X[WINDOW_SIZE];
double READINGS_Y[WINDOW_SIZE];
double READINGS_Z[WINDOW_SIZE];
double AVERAGED_X, AVERAGED_Y, AVERAGED_Z = 0;

int XCalibration = 0;
int YCalibration = 0;
int ZCalibration = 0;
int imuCalibratedFlag = 0;

int IMUDone = 0;

//Objects for brake temp sensors
Adafruit_MLX90614 FLB = Adafruit_MLX90614(); // Front Left Brake Temp
Adafruit_MLX90614 FRB = Adafruit_MLX90614(); // Front Right Brake Temp
Adafruit_MLX90614 RLB = Adafruit_MLX90614(); // Rear Left Brake Temp
Adafruit_MLX90614 RRB = Adafruit_MLX90614(); // Rear Right Brake Temp


// Create EasyTransfer Object
EasyTransfer ET;

// Holds all calculated Telemetry Data
typedef struct data_struct {
  float RPM;        // RPM
  float TPS;        // TPS
  float FOT;        // Fuel Open Time
  float IA;         // Ignition Angle
  float Lam;        // Lambda
  float AirT;       // Air Temp
  float CoolT;      // Coolant Temp
  float Lat;        // Latitude
  float Lng;        // Longitude
  float Speed;      // GPS Speed
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
  float BrakeBias;  // Brake Bias
  float AccX;       // Accelerometer X Axis
  float AccY;       // Accelerometer Y Axis
  float AccZ;       // Accelerometer Z Axis
  float GyrX;       // Gyroscope X Axis
  float GyrY;       // Gyroscope Y Axis
  float GyrZ;       // Gyroscope Z Axis
  float MagX;       // Magnetometer X Axis
  float MagY;       // Magnetometer Y Axis
  float MagZ;       // Magnetometer Z Axis
} data_struct;
data_struct telemetry;

// Stores Calculated Brake Bias Value
double brakeBias = 0;
double oilPressure = 0;

// Variables used for filtering out extraneous values
int RPMLast;
float TPSLast;
float FOTLast;
float IALast;
float LamLast;
float AirTLast;
float CoolTLast;
float OilPLast;

void setupSensor() {
  // 1.) Set the accelerometer range
  Serial.println("Test");
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_4G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_8G);
  //lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
  
  // 2.) Set the magnetometer sensitivity
  Serial.println("Test2");
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_8GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_12GAUSS);
  //lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_500DPS);
  //lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}

void adafruit_IMU() {
  lsm.read();

  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp);

  SUM_X = SUM_X - READINGS_X[INDEX_X]; // Remove the oldest entry from the sum
  VALUE_X = a.acceleration.x + 0.3; // Read the next sensor value, needs offset
  READINGS_X[INDEX_X] = VALUE_X; // Add the newest reading to the window
  SUM_X = SUM_X + VALUE_X; // Add the newest reading to the sum
  INDEX_X = (INDEX_X+1) % WINDOW_SIZE; // Increment the index, and wrap to 0 if it exceeds the window size

  AVERAGED_X = SUM_X / WINDOW_SIZE * 0.102; // Divide the sum of the window by the window size for the result

  SUM_Y = SUM_Y - READINGS_Y[INDEX_Y];
  VALUE_Y = a.acceleration.y - 1.35;
  READINGS_Y[INDEX_Y] = VALUE_Y;
  SUM_Y = SUM_Y + VALUE_Y;
  INDEX_Y = (INDEX_Y+1) % WINDOW_SIZE;

  AVERAGED_Y = SUM_Y / WINDOW_SIZE * 0.102;

  SUM_Z = SUM_Z - READINGS_Z[INDEX_Z];
  VALUE_Z = a.acceleration.z - 9.6;
  READINGS_Z[INDEX_Z] = VALUE_Z;
  SUM_Z = SUM_Z + VALUE_Z;
  INDEX_Z = (INDEX_Z+1) % WINDOW_SIZE;

  AVERAGED_Z = SUM_Z / WINDOW_SIZE * 0.102;

  // Assign Values to Struct Variables
  telemetry.AccX = AVERAGED_X - XCalibration;
  telemetry.AccY = AVERAGED_Y - YCalibration;
  telemetry.AccZ = AVERAGED_Z - ZCalibration;
  IMUDone = 1;
}

void init_imu(){
  for(int i = 0; i < 11; i++){
    adafruit_IMU();
  }
  XCalibration = telemetry.AccX;
  YCalibration = telemetry.AccY;
  ZCalibration = telemetry.AccZ;
}

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

  // Start the adafruit IMU
  lsm.begin();

  // Start EasyTransfer
  ET.begin(details(telemetry), &Serial);

  // initalize brake temp sensors
  // FLB.begin(0x5A, &Wire);
  // FRB.begin(0x5B, &Wire);
  // RLB.begin(0x5C, &Wire);
  // RRB.begin(0x5D, &Wire);

  // Initialize ICM
  // myICM.begin(Wire);

  // Setup Adafruit IMU
  setupSensor();
  Serial.println("Sensor Setup");
  init_imu();
  Serial.println("IMU Calibrated");
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
      telemetry.Lam = (rxBuf[4] + rxBuf[5] * 256) * 0.001; // Lambda
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF548) {
      telemetry.AirT = (rxBuf[2] + rxBuf[3] * 256) * 0.1; // Air Temp
      telemetry.CoolT = (rxBuf[4] + rxBuf[5] * 256) * 0.1; // Coolant Temp
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF348) { 
      oilPressure = (rxBuf[6] + rxBuf[7] * 256) * 0.001; // Oil Pressure Analog
      oilPressure = (oilPressure -.5) / 4 * 150; // Convert to PSI
      telemetry.OilP = oilPressure; // Store in struct
    }
  }
}

void CAN_Data_send() {
  // Checks for CAN0_INT pin to be low, then reads the recieve buffer
  // variables for the PE3 ECU CAN data
  // All 2 byte data is stored [LowByte, HighByte]
  // Num = (LowByte + HighByte * 256) * resolution
  int calibratedX = telemetry.AccX - XCalibration;
  int calibratedY = telemetry.AccY - YCalibration;
  int calibratedZ = telemetry.AccZ - ZCalibration;
  
  sendBuffer[0] = (byte)abs(((calibratedX) / 9.8) * 64);
  sendBuffer[1] = (byte)abs(((calibratedY) / 9.8) * 64);
  sendBuffer[2] = (byte)abs(((calibratedZ) / 9.8) * 64);
  sendBuffer[3] = 0;
  sendBuffer[4] = 0;
  sendBuffer[5] = 0;

  if(calibratedX < 0){
    sendBuffer[3] = 1;
  }
  if(calibratedY < 0){
    sendBuffer[4] = 1;
  }
  if(calibratedZ < 0){
    sendBuffer[5] = 1;
  }
  
  Serial.println(sendBuffer[0]);
  Serial.println(sendBuffer[1]);
  Serial.println(sendBuffer[2]);
  Serial.println(sendBuffer[3]);
  Serial.println(sendBuffer[4]);
  Serial.println(sendBuffer[5]);
  
  byte sndStat = CAN0.sendMsgBuf(0xabc, 0, 8, sendBuffer);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
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
  char message[64];
  sprintf(message, "%d\"", (int)telemetry.RPM);
  Serial2.print("rpm.txt=\"");
  Serial2.print(message); 
  Nextion_CMD();

  sprintf(message, "%d\"", (int)telemetry.CoolT);
  Serial2.print("waterTemp.txt=\"");
  Serial2.print(message);
  Nextion_CMD();

  sprintf(message, "%d\"", (int)oilPressure);
  Serial2.print("oilPress.txt=\"");
  Serial2.print(message);
  Nextion_CMD();

  sprintf(message, "%d\"", (int)brakeBias);
  Serial2.print("bias.txt=\"");
  Serial2.print(message);
  Nextion_CMD();

  sprintf(message, "%d\"", (int)telemetry.FLTemp);
  Serial2.print("brakeFL.txt=\"");
  Serial2.print(message);
  Nextion_CMD();

  sprintf(message, "%d\"", (int)telemetry.FLTemp);
  Serial2.print("brakeFR.txt=\"");
  Serial2.print(message);
  Nextion_CMD();

  sprintf(message, "%d\"", (int)telemetry.RLTemp);
  Serial2.print("brakeRL.txt=\"");
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
  // TODO: Add gear and Laptimes

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

  telemetry.FLPot = ((double)FLPot * 50.0) / 1023.0;
  telemetry.FRPot = ((double)FRPot * 50.0) / 1023.0;
  telemetry.RLPot = ((double)RLPot * 50.0) / 1023.0;
  telemetry.RRPot = ((double)RRPot * 50.0) / 1023.0;
}

// Read in analog break pressure values
void Brake_Pressure() {
  int FrontPres = analogRead(A4);
  int RearPres = analogRead(A5);
  
  // .5v - 4.5V --> 0 - 100 bar
  // bar --> psi = bar * 14.504
  double fPSI = (((double)FrontPres * 112.5) / 1023.0) * 14.504;
  double rPSI = (((double)RearPres * 112.5) / 1023.0) * 14.504;

  telemetry.BrakeFront = fPSI;
  telemetry.BrakeRear = rPSI;

  brakeBias = (0.99 * fPSI) / ((0.99 * fPSI) + (0.79 * rPSI)) * 100;
  telemetry.BrakeBias = brakeBias;
}

// Function to print test data to validate connections
void Print_Test_Data() {
  Serial.println();
  Serial.println(telemetry.TPS);
  Serial.println(telemetry.FRTemp);
  Serial.println(brakeBias);
  // Serial.println("Accel Data Below");
  // Serial.println(telemetry.AccX);
  // Serial.println(telemetry.AccY);
  // Serial.println(telemetry.AccZ);
  /* Serial.println(telemetry.GyrX);
  Serial.println(telemetry.GyrY);
  Serial.println(telemetry.GyrZ); */
  Serial.println();
}

void loop() {
  // function calls for each sensor/module
  CAN_Data();
  // Brake_Temp();
  // Telemetry_Filter();
  // Suspension_Pot();
  // ICM_Data(&myICM);
  // Brake_Pressure();
  Send_Dash();
  // adafruit_IMU();

  /* telemetry.AccX = -20.3;
  telemetry.AccY = -9.8;
  telemetry.AccZ = 6.7; */

  // Wait for IMU Signal
  // while(!IMUDone);
  
  // Send the data over Serial using EasyTransfer library
  ET.sendData(); // Writes a bunch of junk to serial monitor, this is normal as it uses .write()
  
  // Print_Test_Data();
  // int elapsed = millis() - init_time;
  // if (elapsed % 10 == 0) {
  //     CAN_Data_send();
  // }
  
  delay(7); // Delay for stability
}
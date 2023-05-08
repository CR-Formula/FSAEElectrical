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
} data_struct;
data_struct telemetry;

void setup() {
    Serial.begin(115200);
    telemetry.RPM = 0;
    telemetry.TPS = 0;
    telemetry.FOT = 0;
    telemetry.IA = 0;
    telemetry.Lam = 0;
    telemetry.AirT = 0;
    telemetry.CoolT = 0;
    telemetry.Lat = 0;
    telemetry.Lng = 0;
    telemetry.Speed = 0;
    telemetry.OilP = 0;
    telemetry.FuelP = 0;
    telemetry.FLTemp = 0;
    telemetry.FRTemp = 0;
    telemetry.RLTemp = 0;
    telemetry.RRTemp = 0;
    telemetry.FRPot = 0;
    telemetry.FLPot = 0;
    telemetry.RRPot = 0;
    telemetry.RLPot = 0;
    telemetry.BrakeFront = 0;
    telemetry.BrakeRear = 0;
    telemetry.AccX = 0;
    telemetry.AccY = 0;
    telemetry.AccZ = 0;
    telemetry.GyrX = 0;
    telemetry.GyrY = 0;
    telemetry.GyrZ = 0;
}

void loop() {
    // Convert to byte array
    byte *dataBytes = (byte *)&telemetry;
    int structSize = sizeof(telemetry);
    // send the byte data
    Serial.write(dataBytes, structSize);

    // Increase the values
    telemetry.RPM ++;
    telemetry.TPS ++;
    telemetry.FOT ++;
    telemetry.IA ++;
    telemetry.Lam ++;
    telemetry.AirT ++;
    telemetry.CoolT ++;
    telemetry.Lat ++;
    telemetry.Lng ++;
    telemetry.Speed ++;
    telemetry.OilP ++;
    telemetry.FuelP ++;
    telemetry.FLTemp ++;
    telemetry.FRTemp ++;
    telemetry.RLTemp ++;
    telemetry.RRTemp ++;
    telemetry.FRPot ++;
    telemetry.FLPot ++;
    telemetry.RRPot ++;
    telemetry.RLPot ++;
    telemetry.BrakeFront ++;
    telemetry.BrakeRear ++;
    telemetry.AccX ++;
    telemetry.AccY ++;
    telemetry.AccZ ++;
    telemetry.GyrX ++;
    telemetry.GyrY ++;
    telemetry.GyrZ ++;
    delay(10);
}
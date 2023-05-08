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
    Serial2.begin(115200);
}

void loop() {
    byte *receivedDataBytes = (byte *)&telemetry;
    Serial2.readBytes(receivedDataBytes, sizeof(telemetry));

    Serial.println(telemetry.RPM);
    Serial.println(telemetry.TPS);
    Serial.println(telemetry.FOT);
    Serial.println(telemetry.IA);
    Serial.println(telemetry.Lam);
    Serial.println(telemetry.AirT);
    Serial.println(telemetry.CoolT);
    Serial.println(telemetry.Lat);
    Serial.println(telemetry.Lng);
    Serial.println(telemetry.Speed);
    Serial.println(telemetry.OilP);
    Serial.println(telemetry.FuelP);
    Serial.println(telemetry.FLTemp);
    Serial.println(telemetry.FRTemp);
    Serial.println(telemetry.RLTemp);
    Serial.println(telemetry.RRTemp);
    Serial.println(telemetry.FRPot);
    Serial.println(telemetry.FLPot);
    Serial.println(telemetry.RRPot);
    Serial.println(telemetry.RLPot);
    Serial.println(telemetry.BrakeFront);
    Serial.println(telemetry.BrakeRear);
    Serial.println(telemetry.AccX);
    Serial.println(telemetry.AccY);
    Serial.println(telemetry.AccZ);
    Serial.println(telemetry.GyrX);
    Serial.println(telemetry.GyrY);
    Serial.println(telemetry.GyrZ);
    delay(10);
}
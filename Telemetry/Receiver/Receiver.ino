/*Iowa State Formula SAE Electrical Subsystem*/

#include <esp_now.h>
#include <WiFi.h>
#include<stdio.h>
#include<stdlib.h>
#include <iostream>

// Structure example to receive data
// Must match the sender structure
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

// When the board recieves data:
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  // Tracks the size of the data structure
  memcpy(&telemetry, incomingData, sizeof(telemetry)); // Memory Allocation for data
}
 
void setup() {
  // Serial monitor printout for debug
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // init the ESP connection
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register Reciever with the Sender
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  // CSV format Serial Print
  Serial.printf("%f, %f, %f, %f, %f, %f, %f, ", telemetry.RPM, telemetry.TPS, telemetry.FOT, telemetry.IA, telemetry.Lam, telemetry.AirT, telemetry.CoolT);
  Serial.printf("%f, %f, %f, %f, %f, %f, %f, %f, ", telemetry.Lat, telemetry.Lng, telemetry.Speed, telemetry.OilP, telemetry.FuelP, telemetry.FLTemp, telemetry.FRTemp, telemetry.RLTemp);
  Serial.printf("%f, %f, %f, %f, %f, %f, %f, ", telemetry.RRTemp, telemetry.FRPot, telemetry.FLPot, telemetry.RRPot, telemetry.RLPot, telemetry.BrakeFront, telemetry.BrakeRear);
  Serial.printf("%f, %f, %f, %f, %f, %f, ", telemetry.AccX, telemetry.AccY, telemetry.AccZ, telemetry.GyrX, telemetry.GyrY, telemetry.GyrZ);
  Serial.printf("%f, %f, %f\n", telemetry.MagX, telemetry.MagY, telemetry.MagZ);

  // delay for stability
  delay(1);
}

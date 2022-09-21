/* 2021 Iowa State Formula SAE Electrical Subsystem*/

#include <esp_now.h>
#include <WiFi.h>
#include<stdio.h>
#include<stdlib.h>
#include <iostream>

//Structure example to receive data
//Must match the sender structure
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

//When the board recieves data:
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  //Tracks the size of the data structure
  memcpy(&telemetry, incomingData, sizeof(telemetry)); //Memory Allocation for data
  //Serial.println(len);
}
 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  //init the ESP connection
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register Reciever with the Sender
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  //CSV format Serial Print
  Serial.printf("%d, %f, %f, %f, %f, %f, %f, %f, %f, %d, %f\n", telemetry.RPM, telemetry.TPS, telemetry.FOT, telemetry.IA, telemetry.Lam, telemetry.AirT, telemetry.CoolT, telemetry.Lat, telemetry.Lng, telemetry.Speed, telemetry.OilP);
  
  //delay for stability
  delay(1);
}
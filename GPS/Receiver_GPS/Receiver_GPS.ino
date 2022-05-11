/* 2021 Iowa State Formula SAE Electrical Subsystem*/

#include <esp_now.h>
#include <WiFi.h>
#include<stdio.h>
#include<stdlib.h>
#include <iostream>
#include <string>
#include <Math.h>

unsigned int RPMLast;
  double TPSLast;
  double FOTLast;
  double IALast;
  double LamLast;
  double AirTLast;
  double CoolTLast;

//Structure example to receive data
//Must match the sender structure
typedef struct test_struct {

  //Array to store CAN information
  int nums[17]; //Change if added Sensors
  //[rpm, rpm, tps, tps, fot, fot, ing, ing, lam, lam, air, air, cool, cool, Lat, Lng, Speed]
  
} test_struct;

test_struct myData;

//When the board recieves data:
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  //Tracks the size of the data structure
  memcpy(&myData, incomingData, sizeof(myData)); //Memory Allocation for data
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
  //Print the data stored in the data structure array
  int len = sizeof(myData.nums) / sizeof(myData.nums[0]); //Stores the number of elements in the array
  //variables for the PE3 ECU CAN data
  //All 2 byte data is stored [LowByte, HighByte]
  //Num = (HighByte*256 + LowByte) * resolution


  unsigned int RPM = (myData.nums[0] + myData.nums[1] * 256) * 1; //Holds RPM value
  double TPS = (myData.nums[2] + myData.nums[3] * 256) * 0.1; //Holds TPS value
  double FOT = (myData.nums[4] + myData.nums[5] * 256) * 0.1; //holds Fuel Open Time value
  double IA = (myData.nums[6] + myData.nums[7] * 256) * 0.1; //Holds Ignition Angle value
  double Lam = ((myData.nums[8] + myData.nums[9] * 256) * 0.01) * 0.1; //Holds Lambda value
  double AirT = (myData.nums[10] + myData.nums[11] * 256) * 0.1; //Holds Air Temp value
  double CoolT = (myData.nums[12] + myData.nums[13] * 256) * 0.1; //Holds Coolent Temp value
  double Lat = myData.nums[14];
  double Lng = myData.nums[15];
  double Speed = myData.nums[16];
  
  if (RPM > 20000) {
    RPM = RPMLast;
  }
  if (abs(TPSLast - TPS) > 90 && TPSLast > 0) {
    TPS = TPSLast;
  }
  if (abs(FOTLast - FOT) > 10 && FOTLast > 0) {
    FOT = FOTLast;
  }
  if (abs(IALast - IA) > 50 && IALast > 0 || IA > 40) {
    IA = IALast;
  }
  if (abs(LamLast - Lam) > 1 && LamLast > 0) {
    Lam = LamLast;
  }
  if (abs(AirTLast - AirT) > 25 && AirTLast > 0) {
    AirT = AirTLast;
  }
  if (abs(CoolTLast - CoolT) > 50 && CoolTLast > 0) {
    CoolT = CoolTLast;
  }
  
  //CSV format Serial Print
  Serial.printf("%d, %f, %f, %f, %f, %f, %f, %f, %f, %d\n", RPM, TPS, FOT, IA, Lam, AirT, CoolT, Lat, Lng, Speed);

  RPMLast = RPM;
  TPSLast = TPS;
  FOTLast = FOT;
  IALast = IA;
  LamLast = Lam;
  AirTLast = AirT;
  CoolTLast = CoolT;
  
  //Test code to print the data buffer
  /*int k = 0;
  for (k = 0; k < 14; k++) {
    Serial.printf("%d, ", myData.nums[k]);
  }
  Serial.printf("\n");*/
}

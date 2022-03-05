 /* 2021 Iowa State Formula SAE Electrical Subsystem*/

#include <esp_now.h>
#include <WiFi.h>
#include<stdio.h>
#include<stdlib.h>
#include <iostream>
#include <string>


//Structure example to receive data
//Must match the sender structure
typedef struct test_struct {

  //Array to store CAN information
  int nums[14]; //Change if added Sensors
  
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

  //May need to add some statements for possible negative values

  

  unsigned int RPM = (myData.nums[0] + myData.nums[1] * 256) * 1; //Holds RPM value
  signed double TPS = (myData.nums[2] + myData.nums[3] * 256) * 0.1; //Holds TPS value
  signed double FOT = (myData.nums[4] + myData.nums[5] * 256) * 0.1; //holds Fuel Open Time value
  signed double IA = (myData.nums[6] + myData.nums[7] * 256) * 0.1; //Holds Ignition Angle value
  signed double Lam = (myData.nums[8] + myData.nums[9] * 256) * 0.01; //Holds Lambda value
  signed double AirT = (myData.nums[10] + myData.nums[11] * 256) * 0.1; //Holds Air Temp value
  signed double CoolT = (myData.nums[12] + myData.nums[13] * 256) * 0.1; //Holds Coolent Temp value
  
  //CSV format Serial Print
  Serial.printf("%d, %d, %d, %d, %d, %d, %d\n", RPM, TPS, FOT, IA, Lam, AirT, CoolT);


  //Test code to print the data buffer
  /*int k = 0;
  for (k = 0; k < 14; k++) {
    Serial.printf("%d, ", myData.nums[k]);
  }
  Serial.printf("\n");*/
}

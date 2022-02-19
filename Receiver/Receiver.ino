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
  int nums[5];
  
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
  int temp = myData.nums[0] + (myData.nums[1] * 256);
  //Serial.println("%d", temp);

  //will have to add the code to print variables as CSV for GUI, need to know how many variables we will have
  
  for (int i = 0; i < 5; i++) {
    Serial.print(myData.nums[i]);
    Serial.print(", ");
  }
  Serial.println("");
}

/* 2021 Iowa State Formula SAE Electrical Subsystem*/

#include <esp_now.h>
#include <WiFi.h>
#include <stdio.h>
#include <string>
#include <iostream>

//MAC_Addresses for the different ESP32 Boards
uint8_t broadcastAddress1[] = {0x24, 0x0A, 0xC4, 0xEE, 0x13, 0xDC}; //Short Antenna
//uint8_t broadcastAddress1[] = {0x24, 0x0A, 0xC4, 0xEE, 0x7D, 0x04}; //Long Antenna
//uint8_t broadcastAddress1[] = {0x9C, 0x9C, 0x1F, 0xC7, 0x03, 0x54}; //No Antenna

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];
boolean newData = false;
int integerFromPC = 0;

//Data Structure to store variables to be sent
typedef struct test_struct {
  
  //Array to store CAN information
  int nums[14]; //Change if added Sensors
  
} test_struct;
test_struct test;

//Function to check if data send was successful
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "" : "Delivery Fail");
}
 
void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  WiFi.mode(WIFI_STA);
 
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
   
  //Set Receiver information
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  //Check if Reciever was connected successfully
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  //get data from CAN bus
  recWithStartEndMarkers();
  //Print data for testing
  showNewData();

  /*Loop for copying values into data structure array
  * 
  * Look into using Serial.read() as it may prevent conversion issues
  */

  // receivedChars -> CAN PACKET: 1.,.0.,.0.,.2.,.0.,.D.6.,.1.,.6.4.,.0.,

  int tempVal[numChars];
  char base[32] = "0x";
  char tempNums[32] = "0x"; //Hold the current number from received chars
  int k = 0; //index for main nums buffer
  int q = 0; //index for tracking tempNums index
  //add to struct array -> test.nums[index]

  for (int i = 0; receivedChars[i] != '\0'; i++) {
    if (receivedChars[i] != ',') {
      tempNums[q] = receivedChars[i];
      q++;  
    }
    else if (receivedChars[i] == ',') {
      tempVal[k] = (int)strtol(tempNums, NULL, 16);
      k++;
      q = 0;
      strcpy(tempNums, base);
    }
  }
  
  
  if (receivedChars[0] == '1') {
      int i;
      for (i = 0; i < 8; i++) { //RPM, TPS, Fuel Open Time, Ignition Angle
          test.nums[i] = tempVal[i + 1];
      }
  }
  if (receivedChars[0] == '2') { //Lambda
      test.nums[8] = tempVal[1];
      test.nums[9] = tempVal[2];
  }
  if (receivedChars[0] == '3') { //Air temp, Coolant Temp
      test.nums[10] = tempVal[1];
      test.nums[11] = tempVal[2];
      test.nums[12] = tempVal[3];
      test.nums[13] = tempVal[4];
  }

  
  //sends the specified data
  esp_err_t result = esp_now_send(0, (uint8_t *) &test, sizeof(test_struct)); 

  //Tests that the data was sent successfully
  if (result == ESP_OK) {
    //Serial.println("Sent with success");
    int a = 0;
    Serial.print("Buffer: ");
    for (a = 0; a < 14; a++) {
      Serial.print(test.nums[a], HEX);
      Serial.print(',');
    }
    Serial.println(' ');
  }
  else {
    //Serial.println("Error sending the data");
  }
}

//Gets data from the CAN filter
void recWithStartEndMarkers() {
  static boolean recInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial2.available() && newData == false) {
    rc = Serial2.read();

    if (recInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0';
        recInProgress = false;
        ndx = 0;
        newData = true;
      }
    }
    else if (rc == startMarker) {
      recInProgress = true;
    }
 }
}

//Shows data recieved from CAN filter
void showNewData() {
  if (newData == true) {
    Serial.print("CAN PACKET: ");
    Serial.println(receivedChars);
    newData = false;
  }
}

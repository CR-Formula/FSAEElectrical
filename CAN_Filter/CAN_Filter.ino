// CAN Bus Filter

#include <mcp_can.h>
#include <SPI.h>

//Stores the CAN Packet ID
long unsigned int rxId;
unsigned char len = 0;
//Length of the Buffer for Packet Data
unsigned char rxBuf[8];
//Stores the CAN packet serial message
char msgString[128];

//Sets INT to pin 2
#define CAN0_INT 2
//Sets CS to pin 10
MCP_CAN CAN0(10);


void setup()
{
  Serial.begin(115200);
  
  // Initializes MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");

  //Set MCP2515 operation to normal
  CAN0.setMode(MCP_NORMAL);

  //Configure the INT input pin
  pinMode(CAN0_INT, INPUT);
  
  Serial.println("MCP2515 Library Receive Example...");   //---------> Can most likely delete in final revision
}

/**
 * Method for sending the data from the CAN Packets
 * Passes a data array and a length value
 * 
 * Look into Serial.send() instead, may lead to less need for conversion
 * 
 */
void sendTheData(unsigned char data[], int n)
{
   Serial.print('<'); //Marker to seperate values in serial output
   for(int i = 0; i < n; i++)
   {
      Serial.print(data[i], HEX);
      Serial.print(',');
   }
   Serial.println('>'); //end marker
}

void loop()
{
  //Checks for CAN0_INT pin to be low, then reads the recieve buffer
  if(!digitalRead(CAN0_INT))
  {
    //Read Data to data buffer array
    CAN0.readMsgBuf(&rxId, &len, rxBuf);

    //Old Example Code
    /*if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
    if((rxId & 0x1FFFFFFF) == 0x0CFFF548){
      Serial.print(rxBuf[4]);
    }
    Serial.println();
  }
  */

    //check for specific CAN ID. pull need data, and repackage the data
    //Change if added Sensors
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF048) { //RPM, TPS, Fuel Open Time, Igniton Angle
        unsigned char newID = 0x1;
        unsigned char data[9];
        data[0] = newID;
        int i = 0;
        for (i = 1; i < 9; i++) {
            data[i] = rxBuf[i-1];
        }

        sendTheData(data, 9);
        //delay(50); //Will have to tune these for each package
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF148) { //Lambda
      unsigned char newID = 0x2;
      unsigned char data[3];
      data[0] = newID;
      data[1] = rxBuf[2];
      data[2] = rxBuf[3];

      sendTheData(data, 3);
      //delay(50);
    }
    if ((rxId & 0x1FFFFFFF) == 0x0CFFF548) { //Air Temp, Coolant Temp
       unsigned char newID = 0x3;
       unsigned char data[5];
       //add the needed data to custom CAN packet with new ID
       data[0] = newID;
       data[1] = rxBuf[2];
       data[2] = rxBuf[3];
       data[3] = rxBuf[4];
       data[4] = rxBuf[5];

       //Send new CAN Packet to the sender ESP board
       sendTheData(data, 5);
       //Serial.print(rxBuf[4], DEC);
       //Serial.print(rxBuf[5], DEC);
       //delay(50);
    }
    
    
  }
}

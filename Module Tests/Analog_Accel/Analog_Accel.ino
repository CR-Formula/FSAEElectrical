#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int     XYZ_RAW[3];
int     XYZ_Cal_Offset[3];
double  XYZ_G[3];
boolean CALIBRATION_DONE;

void accelRead(int XYZ_RAW[], double XYZ_G[], int XYZ_Cal_Offset[]);
void accelCalibrate(int XYZ_RAW[], double XYZ_G[], int XYZ_Cal_Offset[]);

void setup() 
{
  CALIBRATION_DONE = false;
  Serial.begin(115200);
  accelCalibrate(XYZ_RAW, XYZ_G, XYZ_Cal_Offset);
  
}

void loop() 
{
  accelRead(XYZ_RAW, XYZ_G, XYZ_Cal_Offset);
  Serial.print("X:");
  Serial.print(XYZ_G[0]);
  Serial.print(",");
  Serial.print("Y:");
  Serial.print(XYZ_G[1]);
  Serial.print(",");
  Serial.print("Z:");
  Serial.println(XYZ_G[2]);
  delay(10);
}

void accelRead(int XYZ_RAW[], double XYZ_G[], int XYZ_Cal_Offset[])
{
  int i;
  
  for(i = 0; i < 3; i++)
  {
    XYZ_RAW[i] = analogRead(i); //Read analog pins 0-2
    if(CALIBRATION_DONE) 
    {
      XYZ_RAW[i] = XYZ_RAW[i] - XYZ_Cal_Offset[i];
    }
  }

 
  XYZ_G[0] = ((double)(XYZ_RAW[0] - 512) / 170.667); /* Acceleration in g units */
  XYZ_G[1] = ((double)(XYZ_RAW[1] - 506) / 170.667);
  XYZ_G[2] = ((double)(XYZ_RAW[2] - 615) / 170.667);;

}

void accelCalibrate(int XYZ_RAW[], double XYZ_G[], int XYZ_Cal_Offset[])
{
  int i;
  accelRead(XYZ_RAW, XYZ_G, XYZ_Cal_Offset);
  XYZ_Cal_Offset[0] = XYZ_RAW[0];
  XYZ_Cal_Offset[1] = XYZ_RAW[1];
  XYZ_Cal_Offset[2] = XYZ_RAW[2];
  for(i = 0; i < 9; i++)
  {
    accelRead(XYZ_RAW, XYZ_G, XYZ_Cal_Offset);
    XYZ_Cal_Offset[0] += XYZ_RAW[0];
    XYZ_Cal_Offset[1] += XYZ_RAW[1];
    XYZ_Cal_Offset[2] += XYZ_RAW[2];

  }

  for(i = 0; i < 3; i++)
  {
    XYZ_Cal_Offset[i] = XYZ_RAW[i] - XYZ_Cal_Offset[i] / 10;
  }
  delay(1);
  CALIBRATION_DONE = true;
}

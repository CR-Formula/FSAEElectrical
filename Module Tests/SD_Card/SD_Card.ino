#include <SPI.h>
#include <SD.h>

const int chipSelect = 10;  

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to output
  pinMode(chipSelect, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

void loop()
{
  // make a string for assembling the data to log:
  String dataString = "";

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  // this opens the file and appends to the end of file
  // if the file does not exist, this will create a new file.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile)   {  
    int timeStamp = millis();
    //write to uSD card
    dataFile.print(timeStamp);
    dataFile.print(" ms");
    dataFile.print(", ");
    //output also on Serial monitor for debugging
    Serial.print(timeStamp);
    Serial.print(",");

    // read three sensors on A0, A1, and A2 while appending to the string:
    for (int analogPin = 0; analogPin < 3; analogPin++) 
    {
      int sensorVal = analogRead(analogPin);
      //write analog sensor data to uSD card
      dataFile.print(" Analog Pin A");
      dataFile.print(analogPin);
      dataFile.print(" = ");
      dataFile.print(sensorVal);
      //output also on Serial monitor for debugging
      Serial.print(" Analog Pin A");
      Serial.print(analogPin);
      Serial.print(" = ");
      Serial.print(sensorVal);
      //place comma between the analog sensor data
      if (analogPin < 3) 
      {
        dataString += ","; 
      }
    }
    dataFile.println(); //create a new row to read data more clearly
    dataFile.close();   //close file
    Serial.println();   //print to the serial port too:

  }  
  // if the file isn't open, pop up an error:
  else
  {
    Serial.println("error opening datalog.txt");

  } 
}

// #include "Nextion.h"
// #include "NexProgressBar.h"

// NexText gear = NexText(0,1,"gear");
// NexNumber rpm = NexNumber(0,1,"rpm");
// NexProgressBar rpmBar = NexProgressBar(0,1,"rpmBar");
#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

char message[9];

void setup() 
{
    //Serial.begin(115200);
    Serial1.begin(115200); //Nextion Serial Port
    Wire.begin();
    uint8_t Sensor_Address = 0x5B; //Default 0x5A
    mlx.begin(Sensor_Address, &Wire);
}

void loop() 
{
    // gear.setText("P");
    // rpm.setValue(rpm_val);
    // rpm_val += 100;
    // rpmBar.setValue(rpm_val/160);
    // //delay(1);
    // gear.setText("N");
    // rpm.setValue(rpm_val);
    // rpm_val += 100;
    // rpmBar.setValue(rpm_val/160);
    // //delay(1);
    uint8_t temp = mlx.readObjectTempC();
    sprintf(message, "Temp: %d\"", temp);
    Serial1.print("lastLap.txt=\"");
    Serial1.print(message);     
    Serial1.write(0xff);
    Serial1.write(0xff);
    Serial1.write(0xff);
    delay(1);

}

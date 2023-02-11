
#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

char message[9];

void setup() 
{
    //Serial.begin(115200);
    Serial2.begin(115200); //Nextion Serial Port
    Wire.begin();
    uint8_t Sensor_Address = 0x5B; //Default 0x5A -- AB Sensor changed to 0x5B
    mlx.begin(Sensor_Address, &Wire);
}

void loop() 
{
    uint8_t temp = mlx.readObjectTempC();
    int count = 10000;
    
    //rpm
    sprintf(message, "%d\"", temp);
    Serial2.print("rpm.txt=\"");
    Serial2.print(message);     
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    delay(1);

    //gear
    char Gear = '1';
    sprintf(message, "%c\"", Gear);
    Serial2.print("gear.txt=\"");
    Serial2.print(message);     
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    delay(1);

    //waterTemp
    sprintf(message, "%d\"", temp);
    Serial2.print("waterTemp.txt=\"");
    Serial2.print(message);     
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    delay(1);

    //oilPress
    int oilP = 9;
    sprintf(message, "%d\"", oilP);
    Serial2.print("oilPress.txt=\"");
    Serial2.print(message);     
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    delay(1);

    //lap
    int lap = 5;
    sprintf(message, "%d\"", lap);
    Serial2.print("lap.txt=\"");
    Serial2.print(message);     
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    delay(1);

    //lastLap
    int min = 1;
    int sec = 32;
    int ms = 135;
    sprintf(message, "%d.%d:%d\"", min, sec, ms);
    Serial2.print("lastLap.txt=\"");
    Serial2.print(message);     
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    delay(1);

    //bestLap
    sprintf(message, "%d.%d:%d\"", min, sec, ms);
    Serial2.print("bestLap.txt=\"");
    Serial2.print(message);     
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    delay(1);

    //rpmBar
    Serial2.print("rpmBar.val=");
    Serial2.print(temp + 50);
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);
    delay(1);
}

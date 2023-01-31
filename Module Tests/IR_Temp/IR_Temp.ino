#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(115200);
  Wire.begin();
  uint8_t Sensor_Address = 0x5B; //Default 0x5A
  mlx.begin(Sensor_Address, &Wire);
}

void loop() {
 Serial.print("Ambient: ");
 Serial.print(mlx.readAmbientTempC());
 Serial.println(" C");
 
 Serial.print("Target:  ");
 Serial.print(mlx.readObjectTempC());
 Serial.println(" C");

 delay(500);

}

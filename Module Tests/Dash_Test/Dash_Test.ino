// Code to test Dash values
void setup() {
  Serial.begin(9600);
}

void loop() {
    /*float rpm = 0;
    for (rpm = 0; rpm <= 15500; rpm+=50) {
        Serial.write("rpm.val=");
        Serial.write(rpm);
        Serial.print("rpm.val=");
        Serial.println(rpm);
        delay(50);
    }
    for (rpm = 15500; rpm >=0; rpm-=50) {
        Serial.write("rpm.val=");
        Serial.write(rpm);
        Serial.print("rpm.val=");
        Serial.println(rpm);
        delay(50);
    }*/
    delay(2500);
    Serial.print("rpm.val=69420");
    Serial.print(0xff);  // We always have to send this three lines after each command sent to the nextion display.
    Serial.print(0xff);
    Serial.print(0xff);
    delay(2500);
}
int sensorPin = 2;
int filter = 5; //Change this to increase/decrease accuracy
float startTime = micros();
int rpm = 0;
int count = 0;

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT);
  Serial.println("Wheelspeed test code, Please connect sensor to pin 2");
  attachInterrupt(digitalPinToInterrupt(sensorPin), wheelSpeed, RISING);

}

void loop() {
  Serial.println(rpm);
  delay(500); //should still interrupt in the delay, if not you can remove
}

void wheelSpeed() {
  count++;

  if (count >= filter) {
    float endTime = micros();
    float timePassed = (endTime - startTime) / 1000000.0;
    int rpm = count / timePassed * 60;
    startTime = micros();
  }
}

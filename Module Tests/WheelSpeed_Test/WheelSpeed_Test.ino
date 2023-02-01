/**
 * @brief Code to track RPM of a wheel
 * Speed calculations will change depending on tire and nummber of teeth
 * Code only tracks one sensor
 * 
 */
int sensorPin = 2;
int cutoff = 100; //Check number of rotations every 100 milliseconds
int rpm = 0;
int count = 0;
long startTime = millis();
long endTime;

void setup() {
  Serial.begin(115200);
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), wheelSpeed, RISING);
  Serial.println("Wheelspeed test code, Please connect sensor to pin 2");
}

void loop() {
  endTime = millis();
  if (endTime - startTime >= cutoff || startTime > endTime) { //checks if the correct amount of time has passed or if the timer overflowed
    int time = endTime - startTime; //store time passed
    rpm = count / time; //calculate rpm of the wheel
    count = 0; //reset count
    Serial.println(rpm); //print rpm
  }
}

void wheelSpeed() {
  count++; //add to wheelspeed count
}

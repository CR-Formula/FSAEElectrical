// Speed = (Tire circumference / # of teeth per revolution) / time between teeth


const int interruptPin = 2;
long startTime = millis();
long endTime;
long timeDif = 0; // Holds time difference in ms
long mph = 0;
const long tireSize = 47.124; // Tire size in in
const int teeth = 23;
float seconds; // Difference in seconds

void setup() {
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), event, CHANGE);
}

void loop() {
  // Serial.print("Count: ");
  Serial.print("MPH: ");
  Serial.println(mph);
  Serial.print("timeDif: ");
  Serial.println(timeDif);
  // Serial.print("Hours: ");
  // Serial.println(seconds);
  // Serial.println(startTime);
  // Serial.println(endTime);
}

void event() {
  endTime = millis();
  timeDif = endTime - startTime;
  startTime = millis();
  seconds = timeDif / (float)1000;
  mph = (tireSize / teeth) / seconds; // Speed in in/s
  mph = mph / 17.6;
}

const int interruptPin = 2;
int count = 0;
long startTime = millis();
int cutoff = 1000;
double rpm = 0;

void setup() {
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), event, CHANGE);
}

void loop() {
  // Serial.print("Count: ");
  // Serial.println(count);
  long endTime = millis();
  // Serial.println(startTime);
  // Serial.println(endTime);

  if (endTime - startTime > cutoff) {
    rpm = ((double)count * 60.0) / (endTime - startTime);
    Serial.print("RPM: ");
    Serial.println(rpm);
    startTime = millis();
    count = 0;
  }
}

void event() {
  count++;
}

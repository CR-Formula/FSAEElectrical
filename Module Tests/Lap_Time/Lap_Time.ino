long startTime = millis();
long endTime;
long lap = 0;
long bestLap = 0;
int lapCounter = 0;
int irPin = 18; // interupt pin

void setup() {
    Serial.begin(115200);
    pinMode(irPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(irPin), lapTime, CHANGE);
}

void loop() {
    Serial.println(bestLap); // print the best lap
}

void lapTime() {
    endTime = millis();
    lap = endTime - startTime;
    startTime = millis();
    lapCounter++;
    lap = lap / 1000.0; // Lap time in seconds
    if (bestLap > lap) {
        bestLap = lap;
    }
}
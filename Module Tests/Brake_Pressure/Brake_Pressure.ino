void setup() {
    Serial.begin(115200);
}

void loop() {
    // Val from 0-1023 for 0-5v
    int pressure = analogRead(A0);
    // do Math depending on sensor range
}
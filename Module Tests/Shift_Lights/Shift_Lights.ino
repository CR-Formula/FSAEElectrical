int pin = 5;

void setup() {
  pinMode(pin, OUTPUT);
  Serial.begin(115200);
  Serial.println("This is to test PWM on Arduino");
}

void loop() {
  delay(50);
  for(int i=0; i<255; i++){
    analogWrite(pin, i);
    delay(5);
    Serial.println(i);
  }
  delay(50);
  for(int i=255; i>0; i--){
    analogWrite(pin, i);
    delay(5);
    Serial.println(i);
  }
}
char message[32];
unsigned long long count = 0;
int tempC = 0;
int oil = 0;
int barVal = 0;
char gear = 'N';
long long laptime = 0;
long long bestlap = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  sprintf(message, "%i\"", count);
  Serial.print("rpm.txt=\"");
  Serial.print(message);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  count++;
  
  sprintf(message, "%i\"", tempC);
  Serial.print("waterTemp.txt=\"");
  Serial.print(message);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  tempC++;
  
  sprintf(message, "%i\"", oil);
  Serial.print("oilPress.txt=\"");
  Serial.print(message);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  oil++;

  sprintf(message, "%i\"", barVal);
  Serial.print("rpmBar.txt=\"");
  Serial.print(message);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  if (barVal <= 100) {
    barVal++;
  }
  else {
    barVal = 0;
  }

  sprintf(message, "%c\"", gear);
  Serial.print("gear.txt=\"");
  Serial.print(message);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  if (gear == 'T') {
    gear = 'E';
  }
  else if (gear == 'E') {
    gear = 'S';
  }
  else if (gear == 'S') {
    gear = '!';
  }
  else {
    gear = '';
  }

  sprintf(message, "%i\"", laptime);
  Serial.print("lastLap.txt=\"");
  Serial.print(message);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  laptime++;

  sprintf(message, "%i\"", bestlap);
  Serial.print("bestLap.txt=\"");
  Serial.print(message);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  bestlap++;
  //delay(1);
}

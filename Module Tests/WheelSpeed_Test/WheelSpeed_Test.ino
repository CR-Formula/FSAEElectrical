// Speed = (Tire circumference / # of teeth per revolution) / time between teeth


const int interruptPin = 2; // Interrupt pin on Arduino
long startTime = millis(); // Starts the timer
long endTime; // Holds the value for the ending time
long timeDif = 0; // Holds time difference in ms

long mph = 0;
const long tireSize = 47.124; // Tire size in in
const int teeth = 23; // Number of teeth on the reluctor wheel

float seconds; // Difference in seconds
int trigger_count = 0; // Number of times the interrupt has been triggered

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
  Serial.print("Count: ");
  Serial.println(trigger_count);
  // Serial.print("Hours: ");
  // Serial.println(seconds);
  // Serial.println(startTime);
  // Serial.println(endTime);
}

void event() {
  endTime = millis();
  timeDif = endTime - startTime;
  startTime = millis();
  trigger_count++;
  seconds = timeDif / (float)1000;
  mph = (tireSize / teeth) / seconds; // Speed in in/s
  mph = mph / 17.6;
}

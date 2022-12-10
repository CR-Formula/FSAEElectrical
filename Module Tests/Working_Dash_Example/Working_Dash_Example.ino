#include "Nextion.h"
#include "NexProgressBar.h"

NexText gear = NexText(0,1,"gear");
NexNumber rpm = NexNumber(0,1,"rpm");
NexProgressBar rpmBar = NexProgressBar(0,1,"rpmBar");
int rpm_val = 0;

void setup() 
{
    nexSerial.begin(115200);
}

void loop() 
{
    gear.setText("P");
    rpm.setValue(rpm_val);
    rpm_val += 100;
    rpmBar.setValue(rpm_val/160);
    //delay(1);
    gear.setText("N");
    rpm.setValue(rpm_val);
    rpm_val += 100;
    rpmBar.setValue(rpm_val/160);
    //delay(1);
}

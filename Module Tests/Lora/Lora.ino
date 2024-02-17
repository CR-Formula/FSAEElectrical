#include "stdio.h"
#include "RadioHead.h"

// Code Setup https://learn.adafruit.com/adafruit-rfm69hcw-and-rfm96-rfm95-rfm98-lora-packet-padio-breakouts/using-the-rfm69-radio

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 915.0

// TODO: Set pinouts for Mega
#elif defined(__AVR_ATmega328P__)
#define RFM69_CS 4  //
#define RFM69_INT 3 //
#define RFM69_RST 2 // "A"
#define LED 13

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

void setup()
{
    Serial.begin(115200);

    pinMode(LED, OUTPUT);
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, LOW);

    Serial.println("LoRa RX Test!");
    Serial.println();

    // manual reset
    digitalWrite(RFM69_RST, HIGH);
    delay(10);
    digitalWrite(RFM69_RST, LOW);
    delay(10);

    if (!rf69.init()) {
        Serial.println("RFM69 radio init failed");
        while (1)
            ;
    }
    Serial.println("RFM69 radio init OK!");

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
    // No encryption
    if (!rf69.setFrequency(RF69_FREQ)) {
        Serial.println("setFrequency failed");
    }

    // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
    // ishighpowermodule flag set like this:
    rf69.setTxPower(20, true); // range from 14-20 for power, 2nd arg must be true for 69HCW

    // The encryption key has to be the same as the one in the server
    uint8_t key[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                     0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    rf69.setEncryptionKey(key);
}

void loop()
{
    delay(1000); // Wait 1 second between transmits, could also 'sleep' here!

    char radiopacket[20] = "Hello World #";
    itoa(packetnum++, radiopacket + 13, 10);
    Serial.print("Sending ");
    Serial.println(radiopacket);

    // Send a message!
    rf69.send((uint8_t *)radiopacket, strlen(radiopacket));
    rf69.waitPacketSent();

    // Now wait for a reply
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf69.waitAvailableTimeout(500)) {
        // Should be a reply message for us now
        if (rf69.recv(buf, &len)) {
            Serial.print("Got a reply: ");
            Serial.println((char *)buf);
            Blink(LED, 50, 3); // blink LED 3 times, 50ms between blinks
        }
        else {
            Serial.println("Receive failed");
        }
    }
    else {
        Serial.println("No reply, is another RFM69 listening?");
    }
}
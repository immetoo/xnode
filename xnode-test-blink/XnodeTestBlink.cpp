/*
 * Xnode Test Blink
 */

#include <Arduino.h>

#define DELAY_TIME	100
#define ERROR_LED	A2 // 15
#define RADIO_LED	A3 // 14
#define NETWORK_LED	7



void setup(void) {
	Serial.begin(115200);
	Serial.println("[XnodeTestBlink]");
	pinMode(ERROR_LED, OUTPUT);
	pinMode(RADIO_LED, OUTPUT);
	pinMode(NETWORK_LED, OUTPUT);
}

void loop(void) {
	Serial.print("OUTPUT: ");
	Serial.println(ERROR_LED);
	digitalWrite(ERROR_LED, HIGH);
	delay(DELAY_TIME * 1);
	digitalWrite(ERROR_LED, LOW);
	delay(DELAY_TIME);

	Serial.print("OUTPUT: ");
	Serial.println(RADIO_LED);
	digitalWrite(RADIO_LED, HIGH);
	delay(DELAY_TIME * 3);
	digitalWrite(RADIO_LED, LOW);
	delay(DELAY_TIME);

	Serial.print("OUTPUT: ");
	Serial.println(NETWORK_LED);
	digitalWrite(NETWORK_LED, HIGH);
	delay(DELAY_TIME * 9);
	digitalWrite(NETWORK_LED, LOW);
	delay(DELAY_TIME);
}

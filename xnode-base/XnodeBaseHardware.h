#ifndef XnodeBaseHardware_h
#define XnodeBaseHardware_h

#include <XnodeConstants.h>
#include <XnodeSystemHardware.h>
#include <XnodeBaseConfig.h>
#include <XnodeSerial.h>
#include <avr/wdt.h>
#include <Arduino.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#define HW_PIN_ERROR_LED	7
#define HW_PIN_RADIO_LED	A2
#define HW_PIN_NETWORK_LED	A3
#define HW_PIN_SPI_ETH0_CS	9

#define SYS_LED_TEST_TIME	666	// ms of on time in boot to see if all leds are on.
#define SYS_LED_BLINK_TIME	100	// ms delay of blink freq.

enum ShieldLED {
	SYS_LED_NONE, // = 0 is used for blink led idx
	SYS_LED_ERROR,
	SYS_LED_RADIO,
	SYS_LED_NETWORK
};

enum ShieldLEDStatus {
	SYS_LED_STATUS_OFF, SYS_LED_STATUS_ON, SYS_LED_STATUS_BLINK
};

class XnodeBaseHardware: public XnodeSystemHardware {
private:
	void loopBlink();
	void writeLed(uint8_t led, uint8_t status);
public:
	void begin();
	void loop();
	void changeLed(uint8_t led, uint8_t status);
	
	// from XnodeSystemHardware
	xnode_base_config_t config;
	const char*   getSystemHardwareTypeP();
	const char*   getSystemHardwareVersionP();
	unsigned long getSystemHardwareRebootCount();
	bool          systemHardwareConfigBegin();
	void          systemHardwareConfigSave();
	void          systemHardwareConfigReset();
};

extern XnodeBaseHardware XBHardware;

#endif

#include <XnodeBaseHardware.h>

const char pmSystemHardwareType[] PROGMEM = "xnode-base";
const char pmSystemHardwareVersion[] PROGMEM = XNODE_VERSION;
const char pmSysTestLeds[] PROGMEM = "System leds test";

byte CONFIG_DEFAULT_NET_MAC[] = NET_XNODE_MAC_DEFAULT;
unsigned long blink_timer = ZERO;
uint8_t blink_led = ZERO;
uint8_t blink_status = ZERO;

xnode_base_config_t config;
xnode_base_config_t EEMEM eeprom;
XnodeBaseHardware XBHardware;

void XnodeBaseHardware::begin() {
	pinMode(HW_PIN_ERROR_LED, OUTPUT);
	pinMode(HW_PIN_RADIO_LED, OUTPUT);
	pinMode(HW_PIN_NETWORK_LED, OUTPUT);
	
	changeLed(SYS_LED_ERROR, SYS_LED_STATUS_ON);
	changeLed(SYS_LED_RADIO, SYS_LED_STATUS_ON);
	changeLed(SYS_LED_NETWORK, SYS_LED_STATUS_ON);
	
	XSerial.printCommentLineP(pmSysTestLeds);
	delay(SYS_LED_TEST_TIME); // let users see all leds
	
	changeLed(SYS_LED_ERROR, SYS_LED_STATUS_OFF);
	changeLed(SYS_LED_RADIO, SYS_LED_STATUS_OFF);
	changeLed(SYS_LED_NETWORK, SYS_LED_STATUS_OFF);
}

void XnodeBaseHardware::loop() {
	loopBlink();
}

bool XnodeBaseHardware::systemHardwareConfigBegin() {
	eeprom_read_block((void*) &config, (void*) &eeprom, sizeof(xnode_base_config_t));
	if (config.eeprom_struct_size != sizeof(xnode_base_config_t)) {
		return true;
	}
	config.sys_boot++;
	return false;
}

void XnodeBaseHardware::systemHardwareConfigSave() {
	config.eeprom_struct_size = sizeof(xnode_base_config_t);
	eeprom_write_block((const void*) &config, (void*) &eeprom, sizeof(xnode_base_config_t));
}

void XnodeBaseHardware::systemHardwareConfigReset() {
	// Temp code way
	config.net_mac[0] = ZERO;
	config.net_mac[1] = ZERO;
	config.net_mac[2] = ZERO;
	config.net_mac[3] = ZERO;
	config.net_mac[4] = ZERO;
	config.net_mac[5] = ZERO;
	
	config.net_ip[0] = ZERO;
	config.net_ip[1] = ZERO;
	config.net_ip[2] = ZERO;
	config.net_ip[3] = ZERO;
	
	config.net_mask[0] = ZERO;
	config.net_mask[1] = ZERO;
	config.net_mask[2] = ZERO;
	config.net_mask[3] = ZERO;
	
	config.net_gate[0] = ZERO;
	config.net_gate[1] = ZERO;
	config.net_gate[2] = ZERO;
	config.net_gate[3] = ZERO;
	
	config.net_dns[0] = ZERO;
	config.net_dns[1] = ZERO;
	config.net_dns[2] = ZERO;
	config.net_dns[3] = ZERO;
	
	config.sys_boot = ZERO;
	// end Temp
	
	// TODO: Temp fix cast~~ so clear config zering all in one go.
	//for (uint16_t i=ZERO;i<sizeof(xnode_base_config_t);i++) {
	//((void *)config)+i = ZERO;
	//}
	
	// Copy some defaults into config
	for (byte i = ZERO; i < NET_BYTE_MAC_SIZE; ++i) {
		config.net_mac[i] = CONFIG_DEFAULT_NET_MAC[i];
	}
	
	// Add default mask
	config.net_mask[0] = 255;
	config.net_mask[1] = 255;
	config.net_mask[2] = 255;
	config.net_mask[3] = ZERO;
	
	config.rf_next_node_id = ONE + ONE;
	for (byte i = ZERO; i < RF_KEY_SIZE; ++i) {
		config.rf_key[i] = ZERO;
	}
	for (byte i = ZERO; i < NET_BYTE_NET_KEY_SIZE; ++i) {
		config.net_key.u8[i] = ZERO;
	}
	for (byte i = ZERO; i < NET_BYTE_NET_ID_SIZE; ++i) {
		config.net_id[i] = ZERO;
	}
}

unsigned long XnodeBaseHardware::getSystemHardwareRebootCount() {
	return config.sys_boot;
}

const char* XnodeBaseHardware::getSystemHardwareTypeP() {
	return pmSystemHardwareType;
}

const char* XnodeBaseHardware::getSystemHardwareVersionP() {
	return pmSystemHardwareVersion;
}


void XnodeBaseHardware::writeLed(uint8_t led, uint8_t status) {
	int out = LOW;
	if (status != SYS_LED_STATUS_OFF) {
		out = HIGH;
	}
	
	if (led == SYS_LED_ERROR) {
		digitalWrite(HW_PIN_ERROR_LED, out);
	} else if (led == SYS_LED_RADIO) {
		digitalWrite(HW_PIN_RADIO_LED, out);
	} else {
		digitalWrite(HW_PIN_NETWORK_LED, out);
	}
}

void XnodeBaseHardware::changeLed(uint8_t led, uint8_t status) {
#ifdef DEBUG_SYSTEM
	Serial.print(F("#D XBHardware.changeLed led="));
	Serial.print(led);
	Serial.print(F(",status="));
	Serial.print(status);
	Serial.println();
#endif
	if (status == SYS_LED_STATUS_BLINK) {
		blink_led = led; // start blinking this led
	}
	if (blink_led == led && status != SYS_LED_STATUS_BLINK) {
		blink_led = SYS_LED_NONE; // clear blinking
	}
	writeLed(led, status);
}

// blink led, note only single led can blink at once.
void XnodeBaseHardware::loopBlink() {
	if (blink_led == SYS_LED_NONE) {
		return; // no led to blink
	}
	unsigned long time = millis();
	if (time < blink_timer) {
		return; // wait until time is passed
	}
	blink_timer = time + SYS_LED_BLINK_TIME;
	if (blink_status == ZERO) {
		blink_status = ONE;
	} else {
		blink_status = ZERO;
	}
	writeLed(blink_led, blink_status);
}

#include <XnodeSatelliteHardware.h>

const char pmSystemHardwareType[] PROGMEM = "xnode-satellite";
const char pmSystemHardwareVersion[] PROGMEM = XNODE_VERSION;

xnode_satellite_config_t config;
xnode_satellite_config_t EEMEM eeprom;
XnodeSatelliteHardware XSHardware;

const char* XnodeSatelliteHardware::getSystemHardwareTypeP() {
	return pmSystemHardwareType;
}

const char* XnodeSatelliteHardware::getSystemHardwareVersionP() {
	return pmSystemHardwareVersion;
}

unsigned long XnodeSatelliteHardware::getSystemHardwareRebootCount() {
	return config.sys_boot;
}

bool XnodeSatelliteHardware::systemHardwareConfigBegin() {
	eeprom_read_block((void*) &config, (void*) &eeprom, sizeof(xnode_satellite_config_t));
	if (config.eeprom_struct_size != sizeof(xnode_satellite_config_t)) {
		return true;
	}
	config.sys_boot++;
	return false;
}

void XnodeSatelliteHardware::systemHardwareConfigSave() {
	config.eeprom_struct_size = sizeof(xnode_satellite_config_t);
	eeprom_write_block((const void*) &config, (void*) &eeprom, sizeof(xnode_satellite_config_t));
}

void XnodeSatelliteHardware::systemHardwareConfigReset() {
	config.sys_boot = ZERO;
	config.node_id = ZERO;
	for (byte i = ZERO; i < RF_KEY_SIZE; ++i) {
		config.rf_key[i] = ZERO;
	}
}

#ifndef XnodeSatelliteHardware_h
#define XnodeSatelliteHardware_h

#include <XnodeSystem.h>
#include <XnodeConstants.h>
#include <XnodeSatelliteConfig.h>
#include <avr/eeprom.h>

#define HW_PIN_DHT_WIRE		4

class XnodeSatelliteHardware: public XnodeSystemHardware {
public:
	// from XnodeSystemHardware
	xnode_satellite_config_t config;
	const char*   getSystemHardwareTypeP();
	const char*   getSystemHardwareVersionP();
	unsigned long getSystemHardwareRebootCount();
	bool          systemHardwareConfigBegin();
	void          systemHardwareConfigSave();
	void          systemHardwareConfigReset();
};

extern XnodeSatelliteHardware XSHardware;

#endif


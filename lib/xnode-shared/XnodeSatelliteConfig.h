#ifndef XnodeSatelliteConfig_h
#define XnodeSatelliteConfig_h

// Config data
typedef struct {
	uint16_t eeprom_struct_size; // Config size changes it data defaults to zeros
	byte rf_key[RF_KEY_SIZE];    // rf key to encrypt radio
	byte node_id;                // this node id to transmit to base
	unsigned long sys_boot;      // boot counter
	
} xnode_satellite_config_t;

#endif


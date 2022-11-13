#ifndef XnodeBaseConfig_h
#define XnodeBaseConfig_h

#include <Arduino.h>

union NetKey_t {
	unsigned long int u32[NET_BYTE_NET_KEY_SIZE / 4];
	unsigned char     u8[NET_BYTE_NET_KEY_SIZE];
};
typedef union NetKey_t NetKey;

// Config data
typedef struct {
	uint16_t eeprom_struct_size; // Config size changes it data defaults to zeros
	
	byte net_mac[NET_BYTE_MAC_SIZE];
	byte net_ip[NET_BYTE_IP_SIZE];
	byte net_mask[NET_BYTE_IP_SIZE];
	byte net_gate[NET_BYTE_IP_SIZE];
	byte net_dns[NET_BYTE_IP_SIZE];
	NetKey net_key;
	byte net_id[NET_BYTE_NET_ID_SIZE];

	byte rf_key[RF_KEY_SIZE];
	byte rf_next_node_id;

	unsigned long sys_boot; // boot counter
	
} xnode_base_config_t;

#endif


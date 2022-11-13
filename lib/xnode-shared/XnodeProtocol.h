#ifndef XnodeProtocol_h
#define XnodeProtocol_h

#include <Arduino.h>
#include <XnodeConstants.h>

#define RF_STRUCT_SUPER_SIZE 52
#define RF_STRUCT_FLASH_SIZE 32
#define RF_STRUCT_CHAR_SIZE 48

enum RFMessageType {
	RF_MSG_NODE_INIT,     // sat tx+rx   init_t
	RF_MSG_NODE_FLASH,    // sat tx+rx   flash_t
	RF_MSG_NODE_COMMAND,  // sat rx      char_t
	RF_MSG_NODE_DATA      // sat tx      char_t
};

typedef struct {
	byte msg_type;
	byte msg_data[RF_STRUCT_SUPER_SIZE];
} xp_super_t;

typedef struct {
	byte rf_key[RF_KEY_SIZE];
	byte node_id;
} xp_msg_init_t;

typedef struct {
	uint16_t offset;
	byte flash_data[RF_STRUCT_FLASH_SIZE];
} xp_msg_flash_t;

typedef struct {
	char char_data[RF_STRUCT_CHAR_SIZE];
} xp_msg_char_t;

#endif


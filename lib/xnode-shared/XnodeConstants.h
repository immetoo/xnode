#ifndef XnodeBaseConstants_h
#define XnodeBaseConstants_h

// Temp version here, todo move to makefile and use git --version
#define XNODE_VERSION "1.3"

// Default serial baudrate for all devices
#define SERIAL_SPEED 115200

// Default watchdog timeout
#define WDT_TIMEOUT WDTO_8S

// RF Settings
#define RF_BASE_NODE_ID      1
#define RF_NETWORK_ID        98
#define RF_FREQUENCY         RF69_868MHZ
#define RF_KEY_SIZE          16
#define RF_KEY_FETCH_TIME    1000*5
#define RF_KEY_ENCRYPT_TIME  1000*60*2
#define RF_NO_DATA_FAIL_TIME RF_KEY_ENCRYPT_TIME*2

// Needed for total magic number free code
#define ZERO 0
#define ONE  1 

// Net related sizes
#define NET_BYTE_IP_SIZE 4
#define NET_BYTE_MAC_SIZE 6
#define NET_BYTE_NET_KEY_SIZE 16
#define NET_BYTE_NET_ID_SIZE 6

// Net config
//define NET_XNODE_HOST "xng.server.local" // XNodeGateway
#define NET_XNODE_HOST "www.forwardfire.net"
#define NET_XNODE_MAC_DEFAULT {0x74,0x69,0x69,0x4D,0x33,0xB1}
#define NET_PING_BOOT_TIME  1000*3
#define NET_PING_HOST_TIME  1000*60*90
#define NET_DHCP_RETRY_TIME 1000*60*3
#define NET_DNS_LOOKUP_TIME 1000*3600*1
// TODO: make net time to secs as 1h is ~max for dns

// Net config for url; Input Xnode Version A
#define NET_URL_POST             "_a"
#define NET_URL_PARA_POST_TYPE   "pt="
#define NET_URL_PARA_REQ_CNT     "&rc="
#define NET_URL_PARA_NET_ID      "&ni="
#define NET_URL_PARA_NODE_DATA   "&nd="
#define NET_URL_PARA_NODE_NUMBER "&nn="

// The different post types of the url
#define NET_URL_PT_PING        'p'
#define NET_URL_PT_INIT        'i'
#define NET_URL_PT_DATA        'd'
#define NET_URL_RESULT_OK      'X'

// Some characters for printing.
#define CHAR_NEWLINE   '\n'
#define CHAR_SPACE     ' '
#define CHAR_DOT       '.'
#define CHAR_PROMT     '$'
#define CHAR_COMMENT   '#'
#define CHAR_EQUALS    '='

// Full ascii messages
#define MSG_RF_CHIP_INIT         "Init radio"
#define MSG_RF_CHIP_ERROR        "ERR: No chip"
#define MSG_RF_INFO_FREQ         "rf_freq"
#define MSG_RF_INFO_NETWORK      "rf_network"
#define MSG_RF_INFO_NODE         "rf_node"
#define MSG_RF_INFO_KEY          "rf_key"
#define MSG_RF_INFO_TX           "rf_tx"
#define MSG_RF_INFO_RX           "rf_rx"
#define MSG_RF_INFO_TXE          "rf_txe"
#define MSG_RF_INFO              "rf_info"
#define MSG_RF_ENCRYPT           "rf_encrypt"

#endif

#include <XnodeBaseRadio.h> 

const char pmRadioChipInit[] PROGMEM = MSG_RF_CHIP_INIT;
const char pmRadioChipError[] PROGMEM = MSG_RF_CHIP_ERROR;

const char pmRadioInfoFreq[] PROGMEM = MSG_RF_INFO_FREQ;
const char pmRadioInfoNetwork[] PROGMEM = MSG_RF_INFO_NETWORK;
const char pmRadioInfoNode[] PROGMEM = MSG_RF_INFO_NODE;
const char pmRadioInfoKey[] PROGMEM = MSG_RF_INFO_KEY;
const char pmRadioInfoTx[] PROGMEM = MSG_RF_INFO_TX;
const char pmRadioInfoRx[] PROGMEM = MSG_RF_INFO_RX;
const char pmRadioInfoTxe[] PROGMEM = MSG_RF_INFO_TXE;
const char pmRadioInfoNextNode[] PROGMEM = "rf_next_node";

//const char pmRadioInfoNextNode[] PROGMEM = "rf_sat";

const char pmRadioEncrypt[] PROGMEM = MSG_RF_ENCRYPT;
const char pmRadioInfo[] PROGMEM = MSG_RF_INFO;

const char pmRadioSendFailRf[] PROGMEM = "rf_fail=1";

volatile unsigned long key_encrypt_timer = ZERO;
volatile unsigned long rf_no_data_timer = ZERO;

unsigned long rf_tx = ZERO;
unsigned long rf_rx = ZERO;
unsigned long rf_txe = ZERO;

uint16_t rf_ping_count = ZERO;
uint8_t rf_status_data = ZERO;
uint8_t rf_status_ping = ZERO;
uint8_t rf_status_chip = ZERO;

byte rf_data_sender_id;
xp_super_t rf_data;
xp_msg_init_t* rf_msg_init;
xp_msg_flash_t* rf_msg_flash;
xp_msg_char_t* rf_msg_char;

RFM69 radio;
XnodeBaseRadio XBRadio;

void XnodeBaseRadio::begin() {
	XBHardware.changeLed(SYS_LED_RADIO, SYS_LED_STATUS_ON);
	
	// Init radio chip
	XSerial.printCommentLineP(pmRadioChipInit);
	if (!radio.initialize(RF_FREQUENCY, RF_BASE_NODE_ID, RF_NETWORK_ID)) {
		XSerial.printCommentLineP(pmRadioChipError);
		XBHardware.changeLed(SYS_LED_ERROR, SYS_LED_STATUS_BLINK);
		rf_status_chip = ONE;
		return;
	}
	radio.promiscuous(true);
	
	XSerial.executeCommandP(pmRadioInfo);
	
	// init of time window until rf is encrypted and idle timeout
	key_encrypt_timer = millis() + (unsigned long) RF_KEY_ENCRYPT_TIME;
	rf_no_data_timer = millis() + (unsigned long) RF_KEY_ENCRYPT_TIME + (unsigned long) RF_NO_DATA_FAIL_TIME;
}

void XnodeBaseRadio::loop() {
	if (rf_status_chip != ZERO) {
		return; // no chip
	}
	if (key_encrypt_timer != ZERO && millis() > key_encrypt_timer) {
		cmdClose();
	}
	checkNoDataTimeout();
	
	if (!radio.receiveDone()) {
		return;
	}
	XBHardware.changeLed(SYS_LED_RADIO, SYS_LED_STATUS_ON);
	rf_no_data_timer = millis() + (unsigned long) RF_NO_DATA_FAIL_TIME;
	rf_rx++;
	rf_status_data = ZERO;
	rf_data = *(xp_super_t*) radio.DATA;
	rf_data_sender_id = radio.SENDERID;
	
#ifdef DEBUG_RADIO
	Serial.print(F("#D rf_rx "));
	Serial.print(rf_data_sender_id,DEC);
	Serial.print(CHAR_SPACE);
	Serial.print(rf_data.msg_type,DEC);
	Serial.print(CHAR_NEWLINE);
#endif
	// always reply ack if requested, see sendWithRetry impl (before reply with sendWithRetry)
	if (radio.ACK_REQUESTED) {
#ifdef DEBUG_RADIO
	Serial.println(F("#D rf_sendACK"));
#endif
		radio.sendACK();
	}
	
	if (rf_data.msg_type == RF_MSG_NODE_DATA) {
		rf_msg_char = (xp_msg_char_t*) rf_data.msg_data;
		XBNetwork.sendNodeData(rf_data_sender_id, rf_msg_char->char_data);
	} else if (rf_data.msg_type == RF_MSG_NODE_INIT) {
		rf_msg_init = (xp_msg_init_t*) rf_data.msg_data;
		handleCmdInit();
	} else if (rf_data.msg_type == RF_MSG_NODE_FLASH) {
		rf_msg_flash = (xp_msg_flash_t*) rf_data.msg_data;
		handleCmdFlash();
	}
	checkPing(); // sets radio.SENDERID to zero because it sends data
	

	XBHardware.changeLed(SYS_LED_RADIO, SYS_LED_STATUS_OFF);
}

void XnodeBaseRadio::checkPing() {
	rf_ping_count++;
	if (rf_ping_count % 10 != ZERO) {
		return;
	}
	rf_status_ping = ZERO; // reset good
#ifdef DEBUG_RADIO
	Serial.print(F("#D rf_ping "));
	Serial.print(radio.SENDERID);
	Serial.print(CHAR_NEWLINE);
#endif
	
	delay(3); //need this when sending right after reception .. ?
	rf_tx++;
	if (!radio.sendWithRetry(radio.SENDERID, "ACK TEST", 8, 0)) { // 0 = only 1 attempt, no retries
		rf_txe++;
		rf_status_ping = ONE; // bad
	}
}

void XnodeBaseRadio::checkNoDataTimeout() {
	if (millis() < rf_no_data_timer) {
		return; // wait until time is passed
	}
	rf_no_data_timer = millis() + (unsigned long) RF_NO_DATA_FAIL_TIME;
	
	rf_msg_char = (xp_msg_char_t*) rf_data.msg_data; // hacky reuse sensor buffer to send data
	strcpy(rf_msg_char->char_data, XUtil.UNPSTR(pmRadioSendFailRf)); // because send also used unpstr buffer
	XBNetwork.sendNodeData(RF_BASE_NODE_ID, rf_msg_char->char_data);
}

void XnodeBaseRadio::sendReply() {
	rf_tx++;
	if (!radio.sendWithRetry(rf_data_sender_id, (const void*) &rf_data, sizeof(xp_super_t))) {
		rf_txe++;
	}
}

void XnodeBaseRadio::handleCmdFlash() {
	
	sendReply();
}

void XnodeBaseRadio::handleCmdInit() {
	for (byte i = ZERO; i < RF_KEY_SIZE; ++i) {
		rf_msg_init->rf_key[i] = XBHardware.config.rf_key[i];
	}
#ifdef DEBUG_RADIO
	Serial.print(F("#D rf_init "));
	Serial.print(rf_msg_init->node_id);
	Serial.print(CHAR_NEWLINE);
#endif
	
	if (rf_msg_init->node_id == ZERO) {
		rf_msg_init->node_id = XBHardware.config.rf_next_node_id;
		
		// Save next node id
		XBHardware.config.rf_next_node_id++;
		XSystem.configSave();
	}
	sendReply();
	
	// update open encrypt timer so user can init multiple devices one at the time
	key_encrypt_timer = millis() + (unsigned long) RF_KEY_ENCRYPT_TIME;
}

void XnodeBaseRadio::cmdClose() {
	XSystem.buildReplyPValue(pmRadioEncrypt, ONE);
	radio.encrypt((const char*) XBHardware.config.rf_key);
	key_encrypt_timer = ZERO; // flag encrypted
	XBHardware.changeLed(SYS_LED_RADIO, SYS_LED_STATUS_OFF);
}

void XnodeBaseRadio::cmdOpen() {
	if (key_encrypt_timer != ZERO) {
		return; // only open if it is encrypted already
	}
	radio.encrypt(ZERO);
	key_encrypt_timer = millis() + (unsigned long) RF_KEY_ENCRYPT_TIME;
	XSystem.buildReplyPValue(pmRadioEncrypt, ZERO);
	XBHardware.changeLed(SYS_LED_RADIO, SYS_LED_STATUS_BLINK); // rf blink = open for key requests
}

void XnodeBaseRadio::cmdInfo() {
	// Print key
	XSystem.buildReplyPValueByteA(pmRadioInfoKey,XBHardware.config.rf_key, RF_KEY_SIZE);
	
	// Print radio config
	XSystem.buildReplyPValue(pmRadioInfoFreq,(int) RF_FREQUENCY == RF69_433MHZ ? 433 :	RF_FREQUENCY == RF69_868MHZ ? 868 : 915);
	XSystem.buildReplyPValue(pmRadioInfoNetwork,(int) RF_NETWORK_ID);
	XSystem.buildReplyPValue(pmRadioInfoNode,(int) RF_BASE_NODE_ID);
	XSystem.buildReplyPValue(pmRadioEncrypt,(int) key_encrypt_timer == ZERO ? ONE : ZERO);// if timer==0 then encrypted=1

	// Print base radio stuff
	XSystem.buildReplyPValue(pmRadioInfoRx, rf_rx);
	XSystem.buildReplyPValue(pmRadioInfoTx, rf_tx);
	XSystem.buildReplyPValue(pmRadioInfoTxe, rf_txe);
	XSystem.buildReplyPValue(pmRadioInfoNextNode, (int) XBHardware.config.rf_next_node_id);
}

void XnodeBaseRadio::systemModuleCommandList() {
	XSystem.buildReplyCommandListP(pmRadioInfoKey);
	XSystem.buildReplyCommandListP(pmRadioInfo);
	XSystem.buildReplyCommandListP(pmRadioEncrypt);
}

bool XnodeBaseRadio::systemModuleCommandExecute(char* cmd, char** args) {
	if (XUtil.strcmpP(cmd, pmRadioInfo) == ZERO) {
		cmdInfo();
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmRadioInfoKey) == ZERO) {
		if (XSystem.buildReplyCommandArgumentError(cmd, args)) {
			return true;
		}
		XUtil.charsToByteA(args[ZERO],XBHardware.config.rf_key, RF_KEY_SIZE);
		XSystem.buildReplyPValueByteA(pmRadioInfoKey,XBHardware.config.rf_key, RF_KEY_SIZE);
		XSystem.configSave();
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmRadioEncrypt) == ZERO) {
		if (key_encrypt_timer == ZERO) {
			cmdOpen();
		} else {
			cmdClose();
		}
		return true;
	}
	return false;
}


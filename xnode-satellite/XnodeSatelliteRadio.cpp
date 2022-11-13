#include <XnodeSatelliteRadio.h> 

const char pmRadioChipInit[] PROGMEM = MSG_RF_CHIP_INIT;
const char pmRadioChipError[] PROGMEM = MSG_RF_CHIP_ERROR;
const char pmRadioExecutePrefix[] PROGMEM = "@";//MSG_REMOTE_PREFIX; // FIXME

const char pmRadioInfoFreq[] PROGMEM = MSG_RF_INFO_FREQ;
const char pmRadioInfoNetwork[] PROGMEM = MSG_RF_INFO_NETWORK;
const char pmRadioInfoNode[] PROGMEM = MSG_RF_INFO_NODE;
const char pmRadioInfoKey[] PROGMEM = MSG_RF_INFO_KEY;
const char pmRadioEncrypt[] PROGMEM = MSG_RF_ENCRYPT;
const char pmRadioInfo[] PROGMEM = MSG_RF_INFO;

uint8_t rf_status_chip = ZERO;
uint8_t rf_status_key = ZERO;

volatile unsigned long key_fetch_timer = ZERO;
volatile unsigned long key_encrypt_timer = ZERO;

xp_super_t rf_data;
xp_msg_init_t* rf_msg_init;
xp_msg_flash_t* rf_msg_flash;
xp_msg_char_t* rf_msg_char;

RFM69 radio;
XnodeSatelliteRadio XSRadio;

void XnodeSatelliteRadio::begin() {
	XSerial.printCommentLineP(pmRadioChipInit);
	if (!radio.initialize(RF_FREQUENCY, XSHardware.config.node_id, RF_NETWORK_ID)) {
		XSerial.printCommentLineP(pmRadioChipError);
		rf_status_chip = ONE;
		return;
	}
	key_encrypt_timer = millis() + (unsigned long) RF_KEY_ENCRYPT_TIME;
	key_fetch_timer = millis() + (unsigned long) RF_KEY_FETCH_TIME;
	
	// Print serial info
	XSerial.executeCommandP(pmRadioInfo);
}

void XnodeSatelliteRadio::loop() {
	if (rf_status_chip != ZERO) {
		return; // no chip
	}
	if (key_encrypt_timer != ZERO) {
		initEncryptionKey();
	}
	if (!radio.receiveDone()) {
		return;
	}
	//rf_status_data = ZERO;
	rf_data = *(xp_super_t*) radio.DATA;
#ifdef DEBUG_RADIO
	Serial.print(F("#D rf_rx "));
	Serial.print(radio.SENDERID,DEC);
	Serial.print(CHAR_SPACE);
	Serial.print(rf_data.msg_type,DEC);
	Serial.print(CHAR_NEWLINE);
#endif
	// always reply ack if requested, see sendWithRetry impl
	if (radio.ACK_REQUESTED) {
#ifdef DEBUG_RADIO
	Serial.println(F("#D rf_sendACK"));
#endif
		radio.sendACK();
	}
	if (rf_data.msg_type == RF_MSG_NODE_INIT) {
		handleNodeInit();
	} else if (rf_data.msg_type == RF_MSG_NODE_COMMAND) {
		handleNodeCommand();
	}
}

void XnodeSatelliteRadio::handleNodeInit() {
	rf_msg_init = (xp_msg_init_t*) rf_data.msg_data;
	if (XSHardware.config.node_id != rf_msg_init->node_id) {
#ifdef DEBUG_RADIO
		Serial.print(F("#D init "));
		Serial.print(rf_msg_init->node_id,DEC);
		Serial.print(CHAR_NEWLINE);
#endif
		XSHardware.config.node_id = rf_msg_init->node_id; // allow new node_id
	}
	for (byte i = ZERO; i < RF_KEY_SIZE; ++i) {
		XSHardware.config.rf_key[i] = rf_msg_init->rf_key[i];
	}
	XSystem.configSave();
	key_fetch_timer = ZERO; // stop sending because we have reply
	if (XSHardware.config.node_id != rf_msg_init->node_id) {
		radio.initialize(RF_FREQUENCY, XSHardware.config.node_id, RF_NETWORK_ID); // re-init
	}
}

void XnodeSatelliteRadio::handleNodeCommand() {
	rf_msg_char = (xp_msg_char_t*) rf_data.msg_data;
	XSerial.print(CHAR_NEWLINE);
	XSerial.printCharP(pmRadioExecutePrefix);
	//XSerial.printPromtLine();
	//XSerial.processCommand(rf_msg_char->char_data);
}

void XnodeSatelliteRadio::initEncryptionKey() {
	if (millis() > key_encrypt_timer) {
		cmdClose(); // do once after timeout
		return;
	}
	if (key_fetch_timer==ZERO) {
		return; // we are done
	}
	if (millis() < key_fetch_timer) {
		return; // wait until next fetch
	}
	key_fetch_timer = millis() + (unsigned long) RF_KEY_FETCH_TIME;
	
	// Setup first data package for fetching key rf_keys.
	rf_data.msg_type = RF_MSG_NODE_INIT;
	rf_msg_init = (xp_msg_init_t*) rf_data.msg_data;
	rf_msg_init->node_id = XSHardware.config.node_id; // send our id, so we fetch only once new id.
	send();
}

byte XnodeSatelliteRadio::readTemperature() {
	return radio.readTemperature();
}

bool XnodeSatelliteRadio::isSecure() {
	return key_encrypt_timer == ZERO;
}

void XnodeSatelliteRadio::cmdClose() {
	XSystem.buildReplyPValue(pmRadioEncrypt, ONE);
	radio.encrypt((const char*) XSHardware.config.rf_key);
	key_encrypt_timer = ZERO; // disable calling initEncrytion and this
}

void XnodeSatelliteRadio::cmdOpen() {
	if (key_encrypt_timer != ZERO) {
		return; // only open if it is encrypted already
	}
	radio.encrypt(ZERO);
	key_encrypt_timer = millis() + (unsigned long) RF_KEY_ENCRYPT_TIME;
	XSystem.buildReplyPValue(pmRadioEncrypt, ZERO);
}

void XnodeSatelliteRadio::send() {
	//rf_tx++;
	if (radio.sendWithRetry(RF_BASE_NODE_ID, (const void*) &rf_data, sizeof(xp_super_t)), 5, 100) {
		// TODO make cnt
		Serial.println("# send ok");
	} else {
		Serial.println("# ERR: send failed");
	}
}

void XnodeSatelliteRadio::sendNodeData(char* data) {
	rf_data.msg_type = RF_MSG_NODE_DATA;
	rf_msg_char = (xp_msg_char_t*) rf_data.msg_data;
	
	// TODO: add loop + cnt over data
	strcpy(rf_msg_char->char_data, data);
	send();
}

void XnodeSatelliteRadio::sendCommandP(const char* cmdName) {
	if (!isSecure()) {
		return; // wait until line is secure  
	}
	char sendCommandBuff[32];
	strcpy(sendCommandBuff, XUtil.UNPSTR(cmdName)); // free buffer as gets used in strcmpP again..
	XSystem.executeCommand(sendCommandBuff);
	sendNodeData(XSystem.replyBuffer); // print result or error
}

void XnodeSatelliteRadio::cmdInfo() {
	// Print key
	XSystem.buildReplyPValueByteA(pmRadioInfoKey,XSHardware.config.rf_key,RF_KEY_SIZE);
	
	// Print radio config
	XSystem.buildReplyPValue(pmRadioInfoFreq,(int) RF_FREQUENCY == RF69_433MHZ ? 433 :	RF_FREQUENCY == RF69_868MHZ ? 868 : 915);
	XSystem.buildReplyPValue(pmRadioInfoNetwork,(int) RF_NETWORK_ID);
	XSystem.buildReplyPValue(pmRadioInfoNode,(int) XSHardware.config.node_id);
	XSystem.buildReplyPValue(pmRadioEncrypt,(int) key_encrypt_timer == ZERO ? ONE : ZERO);// if timer==0 then encrypted=1
}

void XnodeSatelliteRadio::systemModuleCommandList() {
	XSystem.buildReplyCommandListP(pmRadioInfo);
	XSystem.buildReplyCommandListP(pmRadioEncrypt);
}

bool XnodeSatelliteRadio::systemModuleCommandExecute(char* cmd, char** args) {
	if (XUtil.strcmpP(cmd, pmRadioInfo) == ZERO) {
		cmdInfo();
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

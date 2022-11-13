#include <XnodeSystem.h>

#define CMD_MAX_ARGS                8    // max 8 arguments to command
#define CMD_WHITE_SPACE       " \r\t\n"  // All diffent white space chars to split commands on

const char pmCmdHelp[] PROGMEM = "help";
const char pmCmdReboot[] PROGMEM = "reboot";
const char pmCmdSysCnfRst[] PROGMEM = "sys__cnf__rst"; // (internal)
const char pmCmdSysInfo[] PROGMEM = "sys_info";

const char pmSysConfigSave[] PROGMEM = "Config saved";
const char pmSysConfigLoad[] PROGMEM = "Config loaded";
const char pmSysConfigReset[] PROGMEM = "Config reset";

const char pmSysInfoVNumber[] PROGMEM = "sys_vnum";
const char pmSysInfoVDate[] PROGMEM = "sys_vdate";
const char pmSysInfoVDateValue[] PROGMEM = __DATE__" "__TIME__; // Print compile date like; "Apr 22 2012 16:36:10"
const char pmSysInfoType[] PROGMEM = "sys_type";
const char pmSysInfoBoot[] PROGMEM = "sys_boot";
const char pmSysInfoDebugLevel[] PROGMEM = "sys_debug";
const char pmSysInfoFreeRam[] PROGMEM = "sys_sram";

const char pmSysBootDebugHeader[] PROGMEM = "==== DEBUGGING ENABLED ====";
const char pmSysBootStart[] PROGMEM = "Booting ";
const char pmSysBootDone[] PROGMEM = "boot done";
const char pmSysBootDoneHelp[] PROGMEM = "Available commands;";

const char pmCmdErrArgument[] PROGMEM = "#ERR Missing argument: ";
const char pmCmdErrUnknown[] PROGMEM = "#ERR Unknown command: ";

const char pmRemoteExecutePrefix[] PROGMEM = "remote@";

uint8_t reboot_requested = ZERO;
uint8_t debug_level = ZERO;
uint8_t replyBufferIndex = ZERO;
uint8_t systemModuleIndex = ZERO;
XnodeSystemModule* systemModules[SYSTEM_MODULE_ARRAY_SIZE];
XnodeSystemHardware* hardware;
XnodeSystem XSystem;

void XnodeSystem::begin(XnodeSystemHardware* hardwareNode) {
	hardware = hardwareNode;
	
	// Print first line of boot process
	XSerial.print(CHAR_COMMENT);
	XSerial.print(CHAR_SPACE);
	XSerial.printCharP (pmSysBootStart); // has space
	XSerial.printCharP(hardware->getSystemHardwareTypeP());
	XSerial.print(CHAR_NEWLINE);
	
	// Start debug asp 
	beginDebug();
	
	// Fixup config
	XSerial.printCommentLineP(pmSysConfigLoad);
	if (hardware->systemHardwareConfigBegin()) {
		hardware->systemHardwareConfigReset();
		XSerial.printCommentLineP(pmSysConfigReset);
	}
	hardware->systemHardwareConfigSave();
	
	// Print system info
	XSerial.executeCommandP(pmCmdSysInfo);
	
	// Enable the watchdog
	wdt_enable(WDT_TIMEOUT);
}

void XnodeSystem::loop() {
	// Tickle the watchdog
	wdt_reset();
	
	// Reboot countdown and action so reply can be send.
	if (reboot_requested > ONE) {
		reboot_requested--;
	} else if (reboot_requested > ZERO) {
		wdt_enable (WDTO_15MS); // reboot in 15ms.
		delay(30);
	}
}

void XnodeSystem::beginDebug() {
	// Make sure human and machine are notified when debugging is turned on.
#ifdef DEBUG_NETWORK
	// shift every flag on its on bit so machine can readout which flags is enabled.
	debug_level+=ONE << 0; // 1
#endif
#ifdef DEBUG_RADIO
	debug_level+=ONE << 1; // 2
#endif
#ifdef DEBUG_SYSTEM
	debug_level+=ONE << 2; // 4
#endif
#ifdef DEBUG_SERIAL
	debug_level+=ONE << 3; // 8
#endif
#ifdef DEBUG_SENSOR
	debug_level+=ONE << 4; // 16, all = 31 
#endif
	if (debug_level > ZERO) {
		XSerial.printCommentLineP(pmSysBootDebugHeader); // print line for human,machine version is in serialPrintInfo.
	}
}

void XnodeSystem::bootDone() {
	XSerial.printCommentLineP(pmSysBootDoneHelp);
	XSerial.executeCommandP(pmCmdHelp);
	XSerial.printCommentLineP(pmSysBootDone);
	XSerial.printPromt();
}

void XnodeSystem::configSave() {
#ifdef DEBUG_SYSTEM
	Serial.println(F("#D XSystem.configSave"));
#endif
	wdt_disable();
	hardware->systemHardwareConfigSave();
	wdt_enable (WDT_TIMEOUT);
}

void XnodeSystem::cmdSysInfo() {
	buildReplyPValueP(pmSysInfoVNumber,hardware->getSystemHardwareVersionP());
	buildReplyPValueP(pmSysInfoVDate,pmSysInfoVDateValue);
	buildReplyPValue(pmSysInfoBoot,hardware->getSystemHardwareRebootCount());
	buildReplyPValueP(pmSysInfoType,hardware->getSystemHardwareTypeP());
	buildReplyPValue(pmSysInfoDebugLevel,debug_level);
	buildReplyPValue(pmSysInfoFreeRam,XUtil.freeRam());
}

void XnodeSystem::cmdHelp() {
	for (uint8_t i = ZERO; i < SYSTEM_MODULE_ARRAY_SIZE; i++) {
		if (systemModules[i] == ZERO) {
			continue;
		}
		systemModules[i]->systemModuleCommandList();
	}
	buildReplyCommandListP(pmCmdSysInfo);
	buildReplyCommandListP(pmCmdReboot);
	buildReplyCommandListP(pmCmdHelp);
}

void XnodeSystem::registrateSystemModule(XnodeSystemModule* module) {
	systemModules[systemModuleIndex++] = module;
}

// execute cmd with the supplied argument
bool XnodeSystem::executeCommandModule(char* cmd, char** args) {
	
	// Check for system build in commands.
	if (XUtil.strcmpP(cmd, pmCmdHelp) == ZERO) {
		cmdHelp();
		return true;
	}
	if (XUtil.strcmpP(cmd, pmCmdReboot) == ZERO) {
		buildReplyPValue(pmCmdReboot,ONE);
		reboot_requested = 100; // let reply be send out/back
		return true;
	}
	if (XUtil.strcmpP(cmd, pmCmdSysInfo) == ZERO) {
		cmdSysInfo();
		return true;
	}
	if (XUtil.strcmpP(cmd, pmCmdSysCnfRst) == ZERO) {
		hardware->systemHardwareConfigReset();
		buildReplyPValue(pmCmdSysCnfRst,ONE);
		configSave();
		buildReplyPValue(pmCmdReboot,ONE);
		reboot_requested = 100;
		return true;
	}
	
	// Check registated parses.
	for (uint8_t i = ZERO; i < SYSTEM_MODULE_ARRAY_SIZE; i++) {
		if (systemModules[i] == ZERO) {
			continue;
		}
		if (systemModules[i]->systemModuleCommandExecute(cmd, args)) {
			return true;
		}
	}
	return false; // no command found
}

// Parse the cmd from a buffer
void XnodeSystem::executeCommand(char* input,bool isRemote) {
#ifdef DEBUG_SYSTEM
	Serial.print(F("#D XSystem.executeCommand input="));
	Serial.print(input);
	Serial.println();
#endif
	if (isRemote) {
		XSerial.printCharP(pmRemoteExecutePrefix);
		XSerial.printPromt();
		XSerial.printChar(input);
		XSerial.print(CHAR_NEWLINE);
	}
	replyBufferIndex = ZERO;
	uint8_t idx = ZERO;
	char *cmd, *ptr, *args[CMD_MAX_ARGS];
	if (strtok((char *) input, CMD_WHITE_SPACE) == NULL) {
		// no command given so just print new promt.
		buildReplyCharP(pmCmdErrUnknown); // TODO: replace with key+value
		buildReply(CHAR_NEWLINE);
		buildReply('\0'); 
		return;
	}
	cmd = (char *) input;
	while ((ptr = strtok(NULL, CMD_WHITE_SPACE)) != NULL) {
		args[idx] = ptr;
		if (++idx == (CMD_MAX_ARGS - ONE)) {
			break;
		}
	}
	args[idx] = NULL;
	bool parsed = executeCommandModule(cmd, args);
	if (!parsed) {
		buildReplyCharP(pmCmdErrUnknown);
		buildReplyChar(cmd); // TODO: add args
		buildReply(CHAR_NEWLINE);
	}
	buildReply('\0');
}

void XnodeSystem::buildReply(unsigned long value, int base) {
	char buf[8 * sizeof(long) + ONE]; // Assumes 8-bit chars plus zero byte.
	char *str = &buf[sizeof(buf) - ONE];
	*str = ZERO; // reverse printing so start with termination.
	if (base < 2) {
		base = 10;// fix base 1 or 0 default to decimals
	}
	do {
		unsigned long m = value;
		value /= base;
		char c = m - base * value;
		*--str = c < 10 ? c + '0' : c + 'A' - 10;
	} while(value);
	return buildReplyChar(str);
}
void XnodeSystem::buildReply(float value,int digitsFull,int digitsDot) {
	dtostrf(value, digitsFull, digitsDot, (char*)replyBuffer+replyBufferIndex);
	replyBufferIndex += (digitsFull + 1 + digitsDot) - 1; // "999.9" "-99.9" and remove terminating zero.
}
void XnodeSystem::buildReply(char value) {
	replyBuffer[replyBufferIndex++] = value;
	if (replyBufferIndex > SYSTEM_REPLY_ARRAY_SIZE) {
		replyBufferIndex--;
		replyBuffer[replyBufferIndex] = ZERO; // terminate last char
#ifdef DEBUG_SYSTEM
		Serial.print("#D+");
		Serial.print(value); // print overflow chars
		Serial.print("-");
#endif
	}
}
void XnodeSystem::buildReplyChar(char* value) {
	while (*value != ZERO) {
		buildReply(*value);
		value++;
	}
}
void XnodeSystem::buildReplyCharP(const char* value) {
	buildReplyChar(XUtil.UNPSTR(value));
}
void XnodeSystem::buildReplyCommandListP(const char* cmdName) {
	buildReplyCharP(cmdName);
	buildReply(CHAR_NEWLINE);
}
bool XnodeSystem::buildReplyCommandArgumentError(char* cmd, char** args) {
	if (args[ZERO] != NULL) {
		return false; // nothing printed
	}
	buildReplyCharP(pmCmdErrArgument);
	buildReplyChar(cmd);
	buildReply(CHAR_NEWLINE);
	return true;
}
void XnodeSystem::buildReplyPValueIP(const char* valueName,uint8_t ip[4]) {
	buildReplyCharP(valueName);
	buildReply(CHAR_EQUALS);
	buildReply(ip[0], DEC);
	buildReply(CHAR_DOT);
	buildReply(ip[1], DEC);
	buildReply(CHAR_DOT);
	buildReply(ip[2], DEC);
	buildReply(CHAR_DOT);
	buildReply(ip[3], DEC);
	buildReply(CHAR_NEWLINE);
}

void XnodeSystem::buildReplyPValue(const char* valueName,unsigned long value) {
	buildReplyCharP(valueName);
	buildReply(CHAR_EQUALS);
	buildReply(value);
	buildReply(CHAR_NEWLINE);
}
void XnodeSystem::buildReplyPValue(const char* valueName,int value) {
	buildReplyPValue(valueName,(unsigned long)value);
}
void XnodeSystem::buildReplyPValue(const char* valueName,float value,int digitsFull,int digitsDot) {
	buildReplyCharP(valueName);
	buildReply(CHAR_EQUALS);
	buildReply(value,digitsFull,digitsDot);
	buildReply(CHAR_NEWLINE);	
}
void XnodeSystem::buildReplyPValueP(const char* valueName,const char* valueP) {
	buildReplyCharP(valueName);
	buildReply(CHAR_EQUALS);
	buildReplyCharP(valueP);
	buildReply(CHAR_NEWLINE);
}
void XnodeSystem::buildReplyPValueByteA(const char* valueName,byte* value, byte data_len) {
	buildReplyCharP(valueName);
	buildReply(CHAR_EQUALS);
	for (byte i = ZERO; i < data_len; ++i) {
		byte d = value[i];
		if (d<0xF) {
			buildReply('0'); // exta zero to have two chars
		}
		buildReply(d, HEX);
	}
	buildReply(CHAR_NEWLINE);
}


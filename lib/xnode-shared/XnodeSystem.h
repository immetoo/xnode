#ifndef XnodeSystem_h
#define XnodeSystem_h

#include <XnodeSystemHardware.h>
#include <XnodeSystemModule.h>
#include <XnodeConstants.h>
#include <XnodeProtocol.h>
#include <XnodeSerial.h>
#include <XnodeUtil.h>
#include <avr/wdt.h>

#define SYSTEM_MODULE_ARRAY_SIZE 4
#define SYSTEM_REPLY_ARRAY_SIZE 250

class XnodeSystem {
private:
	void beginDebug();
	bool executeCommandModule(char* cmd, char** args);
	void cmdHelp();
	void cmdSysInfo();
	void buildReply(char value);
	void buildReplyChar(char* value);
	void buildReply(unsigned long value, int = DEC);
	void buildReply(float value,int digitsFull,int digitsDot);
public:
	char replyBuffer[SYSTEM_REPLY_ARRAY_SIZE];
	XnodeSystemHardware* hardware;
	void begin(XnodeSystemHardware* hardware);
	void loop();
	void bootDone();
	void configSave();
	void executeCommand(char* input,bool isRemote=false);
	void registrateSystemModule(XnodeSystemModule* module);
	
	void buildReplyCommandListP(const char* cmdName);
	bool buildReplyCommandArgumentError(char* cmd, char** args);
	void buildReplyCharP(const char* value);
	void buildReplyPValueIP(const char* valueName,uint8_t ip[4]);
	void buildReplyPValue(const char* valueName,unsigned long value);
	void buildReplyPValue(const char* valueName,int value);
	void buildReplyPValue(const char* valueName,float value,int digitsFull = 3,int digitsDot = 1);
	void buildReplyPValueP(const char* valueName,const char* valueP);
	void buildReplyPValueByteA(const char* valueName,byte* value, byte data_len);
};

extern XnodeSystem XSystem;

#endif


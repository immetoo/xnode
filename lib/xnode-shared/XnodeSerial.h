#ifndef XnodeSerial_h
#define XnodeSerial_h

#define CMD_BUFF_SIZE 40 // max command length (40 is rf_key=9A2EE3A293486E9FE73D77EFC8087D2F)

#include <XnodeSystemModule.h>
#include <XnodeSystemHardware.h>
#include <XnodeConstants.h>
#include <XnodeUtil.h>
#include <XnodeSystem.h>

class XnodeSerial {
private:
	void executeCommand(char* buff,bool echoBuff = true,bool echoPromt = true);
	void processSerialByte(uint8_t c);
	
public:
	void begin();
	void loop();
	
	void executeCommandP(const char* cmd);
	void printPromt();
	void printCommentLineP(const char* argu);
	void printCharP(const char* argu);
	void printChar(char* argu);
	void print(char value);
};

extern XnodeSerial XSerial;

#endif


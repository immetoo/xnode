#ifndef XnodeBaseNetwork_h
#define XnodeBaseNetwork_h

#include <XnodeBaseHardware.h>
#include <XnodeSerial.h>
#include <XnodeSystem.h>
#include <XnodeSystemModule.h>
#include <XnodeBaseConfig.h>
#include <XnodeUtil.h>
#include <avr/wdt.h>
#include <Arduino.h>
#include <EtherCard.h>

#define XXTEA_NUM_ROUNDS 32

class XnodeBaseNetwork: public XnodeSystemModule {
private:
	int parseReplyData(word off);
	
	void sendStart(char postType);
	void loopAutoSetup();
	void loopDataTX();
	void loopDataRX();
	void loopPingBoot();
	void loopPingHost();
	void handleServerResult();
	void setupChip();
	void setupIp();
	void sendPing();
	void sendInit();
	void parseCommandIp(const char* pmName, char** args, byte* ip_conf);
	void xxteaEncrypt(unsigned long v[2]);
	void xxteaDecrypt(unsigned long v[2]);
	
	void cmdDnsLookup();
	void cmdInfo();
	void cmdInfoEth0();
public:
	void begin();
	void loop();
	bool isUp();
	
	void sendNodeData(byte senderId, char* data);
	
	// from XnodeSystemModule
	bool systemModuleCommandExecute(char* cmd, char** args);
	void systemModuleCommandList();
};

extern XnodeBaseNetwork XBNetwork;

#endif


#ifndef XnodeBaseRadio_h
#define XnodeBaseRadio_h

#include <XnodeBaseHardware.h>
#include <XnodeSerial.h>
#include <XnodeSystem.h>
#include <XnodeSystemModule.h>
#include <XnodeBaseNetwork.h>
#include <XnodeProtocol.h>
#include <RFM69.h>

class XnodeBaseRadio: public XnodeSystemModule {
private:
	void checkNoDataTimeout();
	void checkPing();
	void cmdOpen();
	void cmdClose();
	void cmdInfo();
	void handleCmdInit();
	void handleCmdFlash();
	void sendReply();
public:
	void begin();
	void loop();
	
	// from XnodeSystemModule
	bool systemModuleCommandExecute(char* cmd, char** args);
	void systemModuleCommandList();
};

extern XnodeBaseRadio XBRadio;

#endif 

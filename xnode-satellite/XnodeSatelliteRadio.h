#ifndef XnodeSatelliteRadio_h
#define XnodeSatelliteRadio_h

#include <XnodeSatelliteHardware.h>
#include <XnodeSystemModule.h>
#include <XnodeProtocol.h>
#include <XnodeSystem.h>
#include <RFM69.h>

class XnodeSatelliteRadio: public XnodeSystemModule {
public:
	void begin();
	void loop();
	void sendCommandP(const char* cmdName);
	void sendNodeData(char* data);
	byte readTemperature();
private:
	bool isSecure();
	void cmdClose();
	void cmdOpen();
	void initEncryptionKey();
	void send();
	void handleNodeInit();
	void handleNodeCommand();
	void cmdInfo();
	
	// from XnodeSystemModule
	virtual bool systemModuleCommandExecute(char* cmd, char** args);
	virtual void systemModuleCommandList();
};

extern XnodeSatelliteRadio XSRadio;

#endif 

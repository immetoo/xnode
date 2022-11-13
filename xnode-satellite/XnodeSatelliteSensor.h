#ifndef XnodeSatelliteSensor_h
#define XnodeSatelliteSensor_h

#include <XnodeSatelliteRadio.h>
#include <XnodeSatelliteHardware.h>
#include <XnodeSystemModule.h>
#include <DHT.h>
#include <stdlib.h>

class XnodeSatelliteSensor: public XnodeSystemModule {
private:
	void loopA();
	void loopB();
	void loopC();
	void loopD();
public:
	void begin();
	void loop();
	
	// from XnodeSystemModule
	virtual bool systemModuleCommandExecute(char* cmd, char** args);
	virtual void systemModuleCommandList();
};

extern XnodeSatelliteSensor XSSensor;

#endif


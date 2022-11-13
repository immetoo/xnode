#ifndef XnodeSystemHardware_h
#define XnodeSystemHardware_h

class XnodeSystemHardware {
public:
	virtual const char*   getSystemHardwareTypeP();
	virtual const char*   getSystemHardwareVersionP();
	virtual unsigned long getSystemHardwareRebootCount();
	virtual bool          systemHardwareConfigBegin();
	virtual void          systemHardwareConfigSave();
	virtual void          systemHardwareConfigReset();
};

#endif


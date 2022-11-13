#ifndef XnodeSystemModule_h
#define XnodeSystemModule_h

class XnodeSystemModule {
public:
	virtual bool systemModuleCommandExecute(char* cmd, char** args);
	virtual void systemModuleCommandList();
};

#endif


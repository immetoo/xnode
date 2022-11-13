/*
 * Xnode Base
 */

#include <XnodeSerial.h>
#include <XnodeSystem.h>
#include <XnodeBaseHardware.h>
#include <XnodeBaseNetwork.h>
#include <XnodeBaseRadio.h>

void setup() {
	// Build system modules
	XSystem.registrateSystemModule(&XBNetwork);
	XSystem.registrateSystemModule(&XBRadio);
	
	// Init system modules
	XSerial.begin();
	XSystem.begin(&XBHardware);
	XBHardware.begin();
	XBNetwork.begin();
	XBRadio.begin();
	
	// Mark device booted
	XSystem.bootDone();
}

void loop() {
	XSerial.loop();
	XSystem.loop();
	XBHardware.loop();
	XBNetwork.loop();
	XBRadio.loop();
}


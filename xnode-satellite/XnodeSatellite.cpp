/*
 * Xnode Satellite
 */

#include <XnodeSerial.h>
#include <XnodeSystem.h>
#include <XnodeSatelliteHardware.h>
#include <XnodeSatelliteRadio.h>
#include <XnodeSatelliteSensor.h>

void setup() {
	// Build system modules
	XSystem.registrateSystemModule(&XSRadio);
	XSystem.registrateSystemModule(&XSSensor);
	
	// Start system modules
	XSerial.begin();
	XSystem.begin(&XSHardware);
	XSRadio.begin();
	XSSensor.begin();
	
	// Mark device booted
	XSystem.bootDone();
}

void loop() {
	XSerial.loop();
	XSystem.loop();
	XSRadio.loop();
	XSSensor.loop();
}

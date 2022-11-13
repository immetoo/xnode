/*
 * Xnode Satellite Flash
 */

#include <Arduino.h>
#include <MegaIspRepair.h>
#include <xnode-flashdata.generated>

MegaIspRepair megaIspRepair;

void setup() {
}

void loop() {
	megaIspRepair.run("XnodeMegaFlash", sections);
}


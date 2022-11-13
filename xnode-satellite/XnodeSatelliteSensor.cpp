#include <XnodeSatelliteSensor.h>

const char pmSensorInit[] PROGMEM = "Init sensors";

const char pmSensorRadioTemp[] PROGMEM = "s_rt";
const char pmSensorDHTTemp[] PROGMEM = "s_dt";
const char pmSensorDHTWater[] PROGMEM = "s_dh";
const char pmSensorSolarVoltage[] PROGMEM = "s_sv";
const char pmSensorRotoCupsPPM[] PROGMEM = "s_rp";

#define READ_SENSOR_TIME 15000
unsigned long read_sensor_timerA = ZERO;
unsigned long read_sensor_timerB = ZERO;
unsigned long read_sensor_timerC = ZERO;
unsigned long read_sensor_timerD = ZERO;
DHT dht(HW_PIN_DHT_WIRE,DHT22,6);
XnodeSatelliteSensor XSSensor;

/*
int valueRadioTemp = ZERO;
int valueDHTTemp = ZERO;
int valueDHTWater = ZERO;
int valueSolarVoltage = ZERO;
int rotoCupsPPM = ZERO;

byte valueRadioTemp = ZERO;
float valueDHTTemp = ZEROf;
float valueDHTWater = ZEROf;
int valueSolarVoltage = ZERO;
int rotoCupsPPM = ZERO;
*/
void XnodeSatelliteSensor::begin() {
	XSerial.printCommentLineP(pmSensorInit);
	
	
	//delay(1100); // chip needs 1 sec init time, else we get NAN returned. 
	read_sensor_timerA = millis() + (unsigned long) READ_SENSOR_TIME; // temp redo with deep sleep
	read_sensor_timerB = millis() + (unsigned long) READ_SENSOR_TIME + 2000;
	read_sensor_timerC = millis() + (unsigned long) READ_SENSOR_TIME + 3000;
	read_sensor_timerD = millis() + (unsigned long) READ_SENSOR_TIME + 4000;
	XSerial.executeCommandP(pmSensorRadioTemp);
	XSerial.executeCommandP(pmSensorDHTTemp);
	XSerial.executeCommandP(pmSensorSolarVoltage);
	XSerial.executeCommandP(pmSensorDHTWater);
}

void XnodeSatelliteSensor::loop() {
	loopA();
	loopB();
	loopC();
	loopD();
}

void XnodeSatelliteSensor::loopA() {
	if (millis() < read_sensor_timerA) {
			return;
	}
	read_sensor_timerA = millis() + (unsigned long) READ_SENSOR_TIME;
	XSRadio.sendCommandP(pmSensorDHTTemp);
}

void XnodeSatelliteSensor::loopB() {
	if (millis() < read_sensor_timerB) {
			return;
	}
	read_sensor_timerB = millis() + (unsigned long) READ_SENSOR_TIME;
	XSRadio.sendCommandP(pmSensorDHTWater);
}

void XnodeSatelliteSensor::loopC() {
	if (millis() < read_sensor_timerC) {
			return;
	}
	read_sensor_timerC = millis() + (unsigned long) READ_SENSOR_TIME;
	XSRadio.sendCommandP(pmSensorSolarVoltage);
}

void XnodeSatelliteSensor::loopD() {
	if (millis() < read_sensor_timerD) {
			return;
	}
	read_sensor_timerD = millis() + (unsigned long) READ_SENSOR_TIME;
	XSRadio.sendCommandP(pmSensorRadioTemp);
}

void XnodeSatelliteSensor::systemModuleCommandList() {
	XSystem.buildReplyCommandListP(pmSensorRadioTemp);
	XSystem.buildReplyCommandListP(pmSensorDHTTemp);
	XSystem.buildReplyCommandListP(pmSensorDHTWater);
	XSystem.buildReplyCommandListP(pmSensorSolarVoltage);
	//XSystem.buildReplyCommandListP(pmSensorRotoCupsPPM);	
}

bool XnodeSatelliteSensor::systemModuleCommandExecute(char* cmd, char** args) {
	if (XUtil.strcmpP(cmd, pmSensorRadioTemp) == ZERO) {
		XSystem.buildReplyPValue(pmSensorRadioTemp,XSRadio.readTemperature());
		return true;
	}
	if (XUtil.strcmpP(cmd, pmSensorDHTTemp) == ZERO) {
		XSystem.buildReplyPValue(pmSensorDHTTemp,dht.readTemperature());// TODO: fixme 
		return true;
	}
	if (XUtil.strcmpP(cmd, pmSensorDHTWater) == ZERO) {
		XSystem.buildReplyPValue(pmSensorDHTWater,dht.readHumidity());
		return true;
	}
	if (XUtil.strcmpP(cmd, pmSensorSolarVoltage) == ZERO) {
		XSystem.buildReplyPValue(pmSensorSolarVoltage,analogRead(A1));
		return true;
	}
	if (XUtil.strcmpP(cmd, pmSensorRotoCupsPPM) == ZERO) {
		//XSystem.buildReplyPValue(pmSensorRotoCupsPPM,readAnalog(0));
		return true;
	}
	return false;
}


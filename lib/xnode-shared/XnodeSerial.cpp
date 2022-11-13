#include <XnodeSerial.h>

char cmd_buff[CMD_BUFF_SIZE];
volatile uint8_t cmd_process = ONE;
volatile uint8_t cmd_buff_idx = ZERO;
XnodeSerial XSerial;

void XnodeSerial::begin() {
	Serial.begin(SERIAL_SPEED);
	print(CHAR_NEWLINE);
}

void XnodeSerial::loop() {
	if (cmd_process == ZERO) {
		executeCommand(cmd_buff);
		cmd_process = ONE;
	}
	while (Serial.available() > ZERO) {
		processSerialByte(Serial.read());
	}
}

// Parse the cmd from a buffer
void XnodeSerial::executeCommand(char* buff,bool echoBuff,bool echoPromt) {
#ifdef DEBUG_SERIAL
	Serial.print(F("#D XSerial.processCommand buff="));
	Serial.print(buff);
	Serial.print(F(",echoBuff="));
	Serial.print(echoBuff);
	Serial.print(F(",echoPromt="));
	Serial.print(echoPromt);
	Serial.println();
#endif
	if (echoBuff) {
		printChar(buff);
		print(CHAR_NEWLINE);
	}
	if (buff[ZERO] > ZERO) {
		XSystem.executeCommand(buff);   // exe cmd
		printChar(XSystem.replyBuffer); // print result or error
	}
	if (echoPromt) {
		printPromt();
	}
}

void XnodeSerial::processSerialByte(uint8_t c) {
	if (c < 0x07 || c > 0x7E) {
		return; // only process ascii chars
	}
	if (cmd_process == ZERO) {
		return; // skip serial data
	}
	if (cmd_buff_idx > CMD_BUFF_SIZE) {
		cmd_buff_idx = ZERO; // protect against to long input
	}
	if (c == '\b') {
		cmd_buff[cmd_buff_idx] = '\0'; // backspace
		cmd_buff_idx--;
		print(' ');
		print(c); // reply the backspace char for console like experience
	} else if (c == '\n') {
		cmd_buff[cmd_buff_idx] = '\0'; // newline
		cmd_buff_idx = ZERO;
		cmd_process = ZERO; // Start processing line
	} else {
		cmd_buff[cmd_buff_idx] = c;   // store in buffer
		cmd_buff_idx++;
	}
}

void XnodeSerial::executeCommandP(const char* cmd) {
	strcpy(cmd_buff, XUtil.UNPSTR(cmd)); // free buffer as gets used in strcmpP again..
	cmd_buff_idx = ZERO;
	executeCommand(cmd_buff,false,false);
}

void XnodeSerial::printPromt() {
	printCharP(XSystem.hardware->getSystemHardwareTypeP());
	print(CHAR_PROMT);
	print(CHAR_SPACE);
}

void XnodeSerial::printCommentLineP(const char* argu) {
	print(CHAR_COMMENT);
	print(CHAR_SPACE);
	printCharP(argu);
	print(CHAR_NEWLINE);
}

void XnodeSerial::printCharP(const char* argu) {
	printChar(XUtil.UNPSTR(argu));
}

void XnodeSerial::printChar(char* dstring) {
	while (*dstring != ZERO) {
		print(*dstring);
		dstring++;
	}
}

void XnodeSerial::print(char value) {
	Serial.print(value);
}

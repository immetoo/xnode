#include <XnodeUtil.h> 

char unpstr_buff[UNPSTR_BUFF_SIZE]; // buffer to copy progmem data into

// Uncopy from program/flash memory to sram
char* XnodeUtil::UNPSTR(const char* ptr) {
	for (uint8_t i = ZERO; i < UNPSTR_BUFF_SIZE; i++) {
		unpstr_buff[i] = '\0'; // clean buffer
	}
	uint8_t i = ZERO;
	uint8_t c = ZERO;
	do {
		c = pgm_read_byte(ptr++);
		unpstr_buff[i++] = c;
	} while (c != ZERO);
	return unpstr_buff;
}

// Fill pstr_buff from pointer
char* XnodeUtil::UNPSTRA(const uint16_t* argu) {
	// rm readByte use word which auto size ptr
	uint8_t msb = pgm_read_byte((const char*) argu + 1);
	uint8_t lsb = pgm_read_byte((const char*) argu);
	const char*p = (const char*) ((msb * 256) + lsb);
	return UNPSTR(p);
}

uint8_t XnodeUtil::strcmp(char *s1, char *s2) {
	while (*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}
	if (*s1 == *s2) {
		return 0;
	}
	if (*s1 < *s2) {
		return -1;
	}
	return 1;
}

uint8_t XnodeUtil::strcmpP(char *s1, const char* s2) {
	return strcmp(s1, UNPSTR(s2));
}

void XnodeUtil::charsToByteA(char* input,byte* data, byte data_len) {
	for (byte i = ZERO; i < data_len; i++) {
		data[i] = ZERO; // clear array
	}
	byte charIdx = ZERO;
	for (byte i = ZERO; i < data_len; i++) {
		char c1 = input[charIdx];
		char c2 = input[charIdx + ONE];
		if (c1 == ZERO || c2 == ZERO) {
			break;
		}
		data[i] = charsToByte(c1, c2);
		charIdx++;
		charIdx++;
	}
}

byte XnodeUtil::charsToByte(char c1, char c2) {
	byte result = ZERO;
	result += charToNibble(c1) * 16;
	result += charToNibble(c2);
	return result;
}

byte XnodeUtil::charToNibble(char c) {
	uint8_t result = ZERO;
	if (c > '0' && c <= '9') {
		result = c - '0';
	} else if (c >= 'A' && c <= 'F') {
		result = c - 'A' + 10;
	}
	return result;
}

int XnodeUtil::freeRam() {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


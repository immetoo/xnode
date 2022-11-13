#ifndef XnodeUtil_h
#define XnodeUtil_h

#include <XnodeConstants.h>
#include <avr/pgmspace.h>
#include <Arduino.h>

#define UNPSTR_BUFF_SIZE 32

class XnodeUtil {
public:
	static char* UNPSTR(const char* ptr);
	static char* UNPSTRA(const uint16_t* argu);
	static uint8_t strcmp(char *s1, char *s2);
	static uint8_t strcmpP(char *s1, const char* s2);

	static void charsToByteA(char* input,byte* data, byte data_len);
	static byte charsToByte(char c1, char c2);
	static byte charToNibble(char s);

	static int freeRam();
};

extern XnodeUtil XUtil;

#endif


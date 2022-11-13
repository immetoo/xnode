#ifndef IspRepair_h
#define IspRepair_h

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <stdbool.h>

typedef struct {
	const char* title;
	const char* mdate;
	const unsigned char* progdata;
	unsigned start;
	unsigned off;
	unsigned count;
} mega_flash_data_struct;

class MegaIspRepair {
private:
	static byte XferByte(byte v);
	static byte Send_ISP(word v01, byte v2 = 0, byte v3 = 0);
	static void Send_ISP_wait(word v01, byte v2 = 0, byte v3 = 0);
	static void Reset_Target();
	static void Read_Signature();
	static byte Read_Fuses(byte flo, byte fhi);
	static word addr2page(word addr);
	static void LoadPage(word addr, const byte* ptr);
	static void WritePage(word page);
	static void WriteData(word start, const byte* data, word count);
	static byte EnableProgramming();
	static void blink();
	static byte readConfig();
	static byte programSection(byte index, mega_flash_data_struct sections[]);
public:
	void run(const char* typeTitle, mega_flash_data_struct sections[]);
};

#endif

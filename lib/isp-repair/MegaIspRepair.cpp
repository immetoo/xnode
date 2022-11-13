/*
 * Reflash a boot loader and a sketch an a second ATmega.
 * MegaIspRepair
 *   2014, Jan 21 <w.cazander@gmail.com> Rewrote to class and converted to boot pair with lcd.
 * IspRepair
 *   2010-05-29 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
 * BootCloner
 *  adapted from http://www.arduino.cc/playground/BootCloner/BootCloner
 *  original copyright notice: 2007 by Amplificar <mailto:amplificar@gmail.com>
 */

#include <MegaIspRepair.h>
#include <MegaIspRepairConfig.h>

bool fastSPI = false; // don't start in fast mode right away

// transfer a byte using software SPI, using a faster mode when possible
byte MegaIspRepair::XferByte(byte v) {
	byte result = 0;
	if (fastSPI)
		for (byte i = 0; i < 8; ++i) {
			bitWrite(PORTC, 3, v & 0x80);
			v <<= 1;
			bitClear(PORTC, 0);
			result <<= 1;
			bitSet(PORTC, 0);
			result |= bitRead(PIND, 4);
		}
	else
		for (byte i = 0; i < 8; ++i) {
			digitalWrite(PIN_MOSI, v & 0x80);
			digitalWrite(PIN_SCK, 0); // slow pulse, max 60KHz
			digitalWrite(PIN_SCK, 1);
			v <<= 1;
			result = (result << 1) | digitalRead(PIN_MISO);
		}
	return result;
}

// send 4 bytes to target microcontroller, returns the fourth MISO byte
byte MegaIspRepair::Send_ISP(word v01, byte v2, byte v3) {
	XferByte(v01 >> 8);
	XferByte(v01);
	XferByte(v2);
	return XferByte(v3);
}

// send 4 bytes to target microcontroller and wait for completion
void MegaIspRepair::Send_ISP_wait(word v01, byte v2, byte v3) {
	Send_ISP(v01, v2, v3);
	while (Send_ISP(CMD_Poll) & 1)
		;
}

// reset the target microcontroller
void MegaIspRepair::Reset_Target() {
	digitalWrite(RESET, 1);
	digitalWrite(PIN_SCK, 0); // has to be set LOW at startup, or PE fails
	delay(30);
	digitalWrite(RESET, 0);
	delay(30); // minimum delay here is 20ms for the ATmega8
}

// print the 16 signature bytes (device codes)
void MegaIspRepair::Read_Signature() {
	Serial.print("Signatures:");
	for (byte x = 0; x < 8; ++x) {
		Serial.print(" ");
		Serial.print(Send_ISP(CMD_Read_Signature, x), HEX);
	}
	Serial.println("");
}

// prints the lock and fuse bits (no leading zeros)
byte MegaIspRepair::Read_Fuses(byte flo, byte fhi) {
	Serial.print("Lock Bits: ");
	Serial.println(Send_ISP(CMD_Read_Lock), HEX);
	Serial.print("Fuses: low ");
	Serial.print(Send_ISP(CMD_Read_Fuse_Low), HEX);
	Serial.print(", high ");
	Serial.print(Send_ISP(CMD_Read_Fuse_High), HEX);
	Serial.print(", extended ");
	Serial.println(Send_ISP(CMD_Read_Fuse_Extended), HEX);
	return Send_ISP(CMD_Read_Lock) == LOCK_BITS && Send_ISP(CMD_Read_Fuse_Low) == flo && Send_ISP(CMD_Read_Fuse_High) == fhi && Send_ISP(CMD_Read_Fuse_Extended) == FUSE_EXTENDED;
}

word MegaIspRepair::addr2page(word addr) {
	return (word)(addr & ~(PAGE_BYTES - 1)) >> 1;
}

void MegaIspRepair::LoadPage(word addr, const byte* ptr) {
	word cmd = addr & 1 ? CMD_Load_Page_High : CMD_Load_Page_Low;
	Send_ISP(cmd | (addr >> 9), addr >> 1, pgm_read_byte(ptr));
}

void MegaIspRepair::WritePage(word page) {
	Send_ISP_wait(CMD_Write_Page | (page >> 8), page);
}

void MegaIspRepair::WriteData(word start, const byte* data, word count) {
	word page = addr2page(start);
	for (word i = 0; i < count; i += 2) {
		if (page != addr2page(start)) {
			WritePage(page);
			Serial.print('.');
			page = addr2page(start);
		}
		LoadPage(start++, data + i);
		LoadPage(start++, data + i + 1);
	}
	WritePage(page);
	Serial.println();
}

byte MegaIspRepair::EnableProgramming() {
	Reset_Target();
	if (Send_ISP(CMD_Program_Enable, 0x22, 0x22) != 0x22) {
		Serial.println("Program Enable FAILED");
		return 0;
	}
	return 1;
}

void MegaIspRepair::blink() {
	pinMode(DONE_LED, OUTPUT);
	digitalWrite(DONE_LED, 0); // inverted logic
	delay(100); // blink briefly
	pinMode(DONE_LED, INPUT);
}

byte MegaIspRepair::readConfig() {
	static byte pins[] = { CONFIG1, CONFIG2, CONFIG3, CONFIG4 };
	byte switches = 0;
	for (byte i = 0; i < 4; ++i) {
		pinMode(pins[i], INPUT);
		digitalWrite(pins[i], 1); // enable pull-up
		bitWrite(switches, i, digitalRead(pins[i]));
		digitalWrite(pins[i], 0); // disable pull-up
	}
	return switches; // a 4-bit value, i.e. 0..15
}

byte MegaIspRepair::programSection(byte index, mega_flash_data_struct sections[]) {
	Serial.print(index, DEC);
	byte f = EnableProgramming();
	if (f) {
		fastSPI = FAST_SPI && PIN_SCK == 14 && PIN_MISO == 4 && PIN_MOSI == 17;
		WriteData(sections[index].start, sections[index].progdata + sections[index].off, sections[index].count);
		fastSPI = false;
	}
	return f;
}

void MegaIspRepair::run(const char* typeTitle, mega_flash_data_struct sections[]) {
	Serial.begin(57600);
	Serial.println();
	Serial.println("# Booting mega_isp_repair");
	Serial.println("# Version=3.0");
	Serial.print("# Type=");
	Serial.print(typeTitle);
	Serial.println();
	blink();
	
	digitalWrite(PIN_SCK, 1);
	digitalWrite(PIN_MOSI, 1);
	digitalWrite(RESET, 1);
	
	pinMode(PIN_SCK, OUTPUT);
	pinMode(PIN_MOSI, OUTPUT);
	pinMode(RESET, OUTPUT);
	
	byte config = readConfig();
	byte xspeed = 0;
	
	// Always burn a boot pair
	byte bootld = (config * 2) + 0;
	byte sketch = (config * 2) + 1;
	
	Serial.print("Configuration: ");
	Serial.print(config, HEX);
	Serial.println(xspeed ? " (resonator)" : " (crystal)");
	Serial.println();
	Serial.println(sections[sketch].title);
	Serial.println(sections[bootld].title);
	Serial.println();
	
	if (EnableProgramming()) {
		Serial.println("Erasing Flash");
		Send_ISP_wait(CMD_Erase_Flash, 0x22, 0x22);
		
		if (EnableProgramming()) {
			byte fuseLo = xspeed ? FUSE_LOW_FAST : FUSE_LOW_XTAL;
			// derive the boot size from its starting address
			byte fuseHi = FUSE_HIGH_2048;
			switch (sections[bootld].start & 0x0FFF) {
				case 0x0E00:
					fuseHi = FUSE_HIGH_512;
					break;
				case 0x0C00:
					fuseHi = FUSE_HIGH_1024;
					break;
				case 0x0800:
					fuseHi = FUSE_HIGH_2048;
					break;
				case 0x0000:
					fuseHi = FUSE_HIGH_4096;
					break;
			}
			
			// set the fuses and lock bits
			Serial.println("Setting Fuses");
			Send_ISP_wait(CMD_Write_Fuse_Low, 0, fuseLo);
			Send_ISP_wait(CMD_Write_Fuse_High, 0, fuseHi);
			Send_ISP_wait(CMD_Write_Fuse_Extended, 0, FUSE_EXTENDED);
			Send_ISP_wait(CMD_Write_Lock, 0, LOCK_BITS);
			
			// burn the sketch and bootstrap code
			if (programSection(sketch, sections) && programSection(bootld, sections)) {
				Read_Signature();
				if (Read_Fuses(fuseLo, fuseHi)) {
					Serial.println("\nDone.");
					blink();
				} else
					Serial.println("Fuses NOT OK!");
			}
		}
	}
	
	pinMode(PIN_SCK, INPUT);
	pinMode(PIN_MOSI, INPUT);
	pinMode(RESET, INPUT);
	
	digitalWrite(PIN_SCK, 0);
	digitalWrite(PIN_MOSI, 0);
	digitalWrite(RESET, 0);
	
#if ARDUINO >= 100
	Serial.flush();
#endif
	delay(10); // let the serial port finish
	cli(); // stop responding to interrupts
	ADCSRA &= ~bit(ADEN); // disable the ADC
	//PRR = 0xFF; // disable all subsystems
	set_sleep_mode (SLEEP_MODE_PWR_DOWN);
	sleep_mode();
	// total power down, can only wake up with a hardware reset
}


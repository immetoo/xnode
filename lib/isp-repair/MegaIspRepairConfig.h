#define FAST_SPI      1   // comment out to revert to digitalWrite() calls

// pin definitions
#define PIN_SCK     14  // PC0 - AIO1 - serial clock to target avr
#define PIN_MISO    4   // PD4 - DIO1 - input from target avr
#define PIN_MOSI    17  // PC3 - AIO4 - output to target avr
#define RESET       7   // PD7 - DIO4 - reset pin of the target avr
#define DONE_LED    9   // B1 - blue LED on JN USB, blinks on start and when ok

// pins used for the optional config switches
#define CONFIG1     5   // DIO2
#define CONFIG2     15  // AIO2
#define CONFIG3     16  // AIO3
#define CONFIG4     6   // DIO3

// MPU-specific values
#define PAGE_BYTES      128  // ATmega168 and ATmega328
#define LOCK_BITS       0xCF
#define FUSE_LOW_XTAL   0xFF
#define FUSE_LOW_FAST   0xDE
#define FUSE_HIGH_512   0xDE
#define FUSE_HIGH_1024  0xDC
#define FUSE_HIGH_2048  0xDA
#define FUSE_HIGH_4096  0xD8 // not in ATmega168
#define FUSE_EXTENDED   0xFD

// ISP Command Words
#define CMD_Program_Enable      0xAC53
#define CMD_Erase_Flash         0xAC80
#define CMD_Poll                0xF000
#define CMD_Read_Flash_Low      0x2000
#define CMD_Read_Flash_High     0x2800
#define CMD_Load_Page_Low       0x4000
#define CMD_Load_Page_High      0x4800
#define CMD_Write_Page          0x4C00
#define CMD_Read_EEPROM         0xA000
#define CMD_Write_EEPROM        0xC000
#define CMD_Read_Lock           0x5800
#define CMD_Write_Lock          0xACE0
#define CMD_Read_Signature      0x3000
#define CMD_Write_Fuse_Low      0xACA0
#define CMD_Write_Fuse_High     0xACA8
#define CMD_Write_Fuse_Extended 0xACA4
#define CMD_Read_Fuse_Low       0x5000
#define CMD_Read_Fuse_High      0x5808
#define CMD_Read_Fuse_Extended  0x5008
#define CMD_Read_Fuse_High      0x5808
#define CMD_Read_Calibration    0x3800


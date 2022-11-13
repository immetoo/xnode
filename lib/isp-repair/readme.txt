
// This code is adapted from isp_prepare. It omits the button and run LED,
// and starts right away. To use, open a serial console and wait until done.
//
// The 6 ISP pins of the target board need to be connected to the board running
// this sketch as follows (using Arduino pin naming):
//
//   ISP pin 1  <->  digital 4  (MISO)          ISP CONNECTOR
//   ISP pin 2  <->  VCC                          +---+---+
//   ISP pin 3  <->  analog 0   (SCK)             | 1 | 2 |
//   ISP pin 4  <->  analog 3   (MOSI)            | 3 | 4 |
//   ISP pin 5  <->  digital 7  (RESET)           | 5 | 6 |
//   ISP pin 6  <->  ground                       +---+---+
//
// The same hookup, using JeeNode port/pin names:
//
//   ISP pin 1  <->  DIO1
//   ISP pin 2  <->  +3V
//   ISP pin 3  <->  AIO1
//   ISP pin 4  <->  AIO4
//   ISP pin 5  <->  DIO4
//   ISP pin 6  <->  GND
//



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/* Include code using a file generated in isp_prepare/ dir with this cmd:

        ./hex2c.tcl Blink.cpp.hex \
                    RF12demo.cpp.hex \
                    optiboot_atmega328.hex \
                    ATmegaBOOT_168_atmega328.hex \
                    optiboot_atmega328_1s.hex \
                    optiboot_atmega328.hex >../isp_repair2/data.h

    Code choices are fixed: section 0 is RF12demo, section 1 is blink
    Boot choices are entries 2..5 in the sections[] array in data.h
*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

 

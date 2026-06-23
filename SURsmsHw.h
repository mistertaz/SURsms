#ifndef _sursmshw_h
#define _sursmshw_h

// configuration items for PIC24 processor
// some differences occur for resident loader vs. application
#include <24FJ128GA204.h>
#device ADC=12
#device PASS_STRINGS=IN_RAM

#if !defined(SONO____BOOT)
//#build(stack=0x2700:0x27ff)  // change from 2750:27ff
#build(stack=256)
//#build(stack=384)
#endif


#if !defined(SONO____BOOT)
#if __USING_ICD != 0
#device ICSP=3  // selects the 3rd set of debugger Rx/Tx pin pairs
#device ICD=TRUE
#endif
#endif

#use delay(crystal=4915200)

//!#use profile( ICD)
//!#profile functions

// processor fuse definitions
#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOJTAG                   //JTAG disabled
#FUSES CKSFSM                   //Clock Switching is enabled, fail Safe clock monitor is enabled
#FUSES NOIESO                   //Internal External Switch Over mode disabled
#FUSES NOWPFP                   //Write/Erase Protect Page Start/End Location, set to page 0
#FUSES NOBROWNOUT               //No brownout reset
#FUSES SOSC_DIG                 //To use the SOSC pins as normal i/o 

// processor port pin assignments
#use FAST_IO(C)

#define iRTC      PIN_A7  // re-introduced for v4.X
#define TOUCH_PIN iRTC  // re-introduced for v4.X
#define Rst       PIN_C0  // Xbee RESET/ signal
#define PS        PIN_C7  // XBee 'Pin Sleep' input: 0 is sleeping
#define bRT       PIN_C9
#define bRS       PIN_C4
#define PwKy     PIN_C1  // can't call this 'Key', conflicts with stdlib.h. 'PwKy' is signal name on schematic. 'Sleep Control' output: 0 is wake
#define LED       PIN_A10
//#define sw1       PIN_C5
#define PUSH_BUTTON PIN_C5
#define LCD_V     PIN_C2  // LCD operating power
#define cCT    PIN_C8  // clear-to-send from the XBEE modem

//!// pin assignments used in the LCD operating firmware: LCD_SMS.C
//!//#define LCD_ENABLE_PIN  PIN_A0
//!#define LCD_ENABLE_PIN  PIN_A8      // changed from A0 for product version
//!#define LCD_RS_PIN      PIN_A1
//!////     #define LCD_RW_PIN      PIN_E2
//!#define LCD_DATA4       PIN_B0
//!#define LCD_DATA5       PIN_B1
//!#define LCD_DATA6       PIN_B2
//!#define LCD_DATA7       PIN_B3
//!

// pin assignments used in the LCD operating firmware: Flex_LCD420_fast.c
#define LCD_DB4   PIN_B0
#define LCD_DB5   PIN_B1
#define LCD_DB6   PIN_B2
#define LCD_DB7   PIN_B3
#define LCD_E     PIN_A8
#define LCD_RS    PIN_A1
//#define LCD_RW    PIN_XX  // not used


// serial uart definitions
// application uses named streams and interrupt-driven character reception
// resident loader uses same streams but no interrupts

// UART1 is the comm port to access the SUR, and also for debug output in development
// biggest input will be 15 tag lines, that could be around 560 bytes
// so 640 would hold all of it.
// need to see whether we read faster than that, could make the buffer smaller
#pin_select U1TX=PIN_B14
#pin_select U1RX=PIN_B15
#if !defined(SONO____BOOT)
#use rs232(STREAM=SURport,BAUD=38400,UART1,errors, RECEIVE_BUFFER=640)
#else
#use rs232(STREAM=SURport,BAUD=38400,UART1,errors)
#endif

// UART2 is the comm port to access the XBee cellular phone modem
#pin_select U2TX=PIN_B12
#pin_select U2RX=PIN_B13
#if !defined(SONO____BOOT)  // don't define this uart for the loader

#if __USING_SMSISR  // different definitions used for ISR and runtime buffering
// this is UART 2, so the interrupt-related prefix is RDA2
#use rs232(Stream=SMSport,BAUD=9600,UART2,errors)  // we provide receive interrupt handler
#else
//#use rs232(Stream=SMSport,BAUD=9600,UART2,errors, RECEIVE_BUFFER=640)  // runtime provides receive interrupt handler. space for three SMS packets plus a little more
#use rs232(Stream=SMSport,BAUD=9600,UART2,errors, RECEIVE_BUFFER=256)  // runtime provides receive interrupt handler. we don't get SMS packets so do not need space for three of them anymore
#endif

//#use rs232(Stream=SMSport,BAUD=9600,UART2,errors, RECEIVE_BUFFER=640)  // runtime provides receive interrupt handler. space for three SMS packets plus a little more

#endif  // ???!defined(SONO____BOOT)

// UART3 is the comm port to access the the RN4871 Bluetooth Controller
// this will consume 256 bytes of buffer space one way or another.
// if the runtime provides receive interrupt handler, space is defined in the 'use rs232' item.
// if we service the interrupt ourselves, in order to filter the controller in-band messages,
// then we define some RAM in which to buffer.  256 bytes is a SWAG
#pin_select U3TX=PIN_B10
#pin_select U3RX=PIN_B11
#if !defined(SONO____BOOT)  // app uses this uart as below. varies whether using interrupt driven receive or not

#if __USING_BTLEISR  // different definitions used for ISR and runtime buffering
// this is UART 3, so the interrupt-related prefix is RDA3
#use rs232(STREAM=BTport,BAUD=38400,UART3,errors)  // we provide receive interrupt handler
#else
#use rs232(STREAM=BTport,BAUD=38400,UART3,errors, RECEIVE_BUFFER=256)  // runtime provides receive interrupt handler.
#endif

#else  // bootloader uses this uart as shown below
#use rs232(STREAM=BTport,BAUD=38400,UART3,errors)
#endif


// this had been defined first, before any other rs232 device
// moved it here so we can use the 'first' device (SURport) without a stream name
#use rs232(ICD, DISABLE_INTS, stream=DBGport)


// external EEPROM definitions
#define EEPROM_SDA  PIN_B8
#define EEPROM_SCL  PIN_B7

// I2C addresses for RV-3129-C3 RTC routines

//!#define  RTC_W    0b10101100              // write addr for RV-3129-C3
//!#define  RTC_R    0b10101101              // read addr for RV-3129-C3
//!#define  RTC_C1   0x00                    // RV-3129-C3 Control Reg 1
//!#define  RTC_INT  0x01                    // RV-3129-C3 Control_INT
//!#define  RTC_INF  0x02                    // RV-3129-C3 Control_INT_Flag
//!#define  RTC_CS   0x03                    // RV-3129-C3 Control_Status
//!#define  RTC_CR   0x04                    // RV-3129-C3 Control_Reset (bit 4)
//!#define  RTC_EE   0x30


//#use i2c(master, sda=EEPROM_SDA, scl=EEPROM_SCL)  // this duplicates a line in the vendor driver file '24128.c'
//#use i2c(master, sda=EEPROM_SDA, scl=EEPROM_SCL, nofloat_high)


#endif


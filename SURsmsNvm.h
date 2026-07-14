
#ifndef _sursmsnvm_h
#define _sursmsnvm_h

// this header file describes the memory layout for the external non-volatile memory used in the PIC24 app.

// PIC C definition of type 'char' is inconsistent between 8-bit and 16-bit architectures.
// PIC18 is 8-bit, and a char is unsigned 8-bit integer.
// PIC24 is 16-bit, and a char is signed 8-bit integer.
// device library supplied for these devices provides a consistent
// definition for 'BYTE' which will always be unsigned 8-bit integer.
// we will use this type in those situations where the ordinary 'char'
// definition is inappropriate.


//
// this external EEPROM (24C128 serial EEPROM) is at least 16Kb in size
// we will use it lavishly, with no thought of space conservation.
// oops, that is 16K BITS. 2K Bytes. x800. watch out how we use it after all.
//

// the first section of NVRAM is 128 bytes. this is a result of the original
// PIC18 version of this product that had only 128 bytes.
// this holds app configuration values, and is protected with a CRC
//
// second section is also 128 bytes, and is not particularly organized; nor is it
// protected with a CRC. here we store less-critical items such as the initial
// values to use in setting the clock/calendar when its battery backup loses power.
// the values are set when the configuration command is used to set time and date.
// so they are always wrong, but always in the past and reasonable.
//
// third section holds the configuration items for the tag filtering process.
// the length of this secion can vary depending on compilation options. since it
// is the last region we define, the length is not critical. it will always fit.
//
// the first section (config) and the third section (filters) are both shadowed
// in processor RAM while the app runs. Updating occurs only when some contents change.
// the second section is directly read and/or written at point of use. no shadow.
//

#define CFG_NVM_BASE 0x0000
#define CFG_NVM_SIZE 0x80  // 128

#define XXX_NVM_BASE 0x0080
#define XXX_NVM_SIZE 0x80  // 128

// define enumerated type to name the bytes in CFG_NVM and corresponding shadow.
// enum types are ascending zero-based sequence of integers.

enum eepr {
   VERS ,        // addr 00(0x00)  Software version number stored here
   EMPTY01 ,     // addr 01(0x01)
   USETMATCH,    // addr 02(0x02)  Use tag list matching criteria for filtering response selection
   USEDMATCH,    // addr 03(0x03)  Use duplicate tag matching criteria for filtering response selection
   SECONDARY ,   // addr 04(0x04)  Replicate SMS messages to secondary phone if this is non-zero
   DETREPINT ,   // addr 05(0x05)  Detection report interval selection code
   HLTHREPINT ,  // addr 06(0x06)  Health report interval selection code
   USERAWLOGS ,  // addr 07(0x07)  Use old-style format of tag data if non-zero (more/less strict checking??)
   ALLTAGS ,     // addr 08(0x08)  Pass tags normally suppressed: 1-3-5-7 (promiscuous mode)
   PBLPRTHR ,    // addr 09(0x09)  Pushbutton long-press threshold in seconds.
   EMPTY0A ,     // addr 10(0x0A)
   EMPTY0B ,     // addr 11(0x0B)
   PRPHLEN ,     // addr 12(0x0C)  length of primary phone number string
   SEPHLEN ,     // addr 13(0x0D)  length of secondary phone number string
   PRCCLEN ,     // addr 14(0x0E)  length of primary phone number country code string
   SECCLEN ,     // addr 15(0x0F)  length of secondary phone number country code string
   PRPHNUM ,     // addr 16(0x10)  Primary phone number field, size 16 bytes
   RSRVD11 ,     // addr 17(0x11)  Primary phone number field
   RSRVD12 ,     // addr 18(0x12)  Primary phone number field
   RSRVD13 ,     // addr 19(0x13)  Primary phone number field
   RSRVD14 ,     // addr 20(0x14)  Primary phone number field
   RSRVD15 ,     // addr 21(0x15)  Primary phone number field
   RSRVD16 ,     // addr 22(0x16)  Primary phone number field
   RSRVD17 ,     // addr 23(0x17)  Primary phone number field
   RSRVD18 ,     // addr 24(0x18)  Primary phone number field
   RSRVD19 ,     // addr 25(0x19)  Primary phone number field
   RSRVD1A ,     // addr 26(0x1A)  Primary phone number field 
   RSRVD1B ,     // addr 27(0x1B)  Primary phone number field 
   RSRVD1C ,     // addr 28(0x1C)  Primary phone number field 
   RSRVD1D ,     // addr 29(0x1D)  Primary phone number field 
   RSRVD1E ,     // addr 30(0x1E)  Primary phone number field 
   RSRVD1F ,     // addr 31(0x1F)  Primary phone number field 

   SEPHNUM ,     // addr 32(0x20)  Secondary phone number field, size 16 bytes
   RSRVD21 ,     // addr 33(0x21)  Secondary phone number field
   RSRVD22 ,     // addr 34(0x22)  Secondary phone number field
   RSRVD23 ,     // addr 35(0x23)  Secondary phone number field
   RSRVD24 ,     // addr 36(0x24)  Secondary phone number field
   RSRVD25 ,     // addr 37(0x25)  Secondary phone number field
   RSRVD26 ,     // addr 38(0x26)  Secondary phone number field
   RSRVD27 ,     // addr 39(0x27)  Secondary phone number field
   RSRVD28 ,     // addr 40(0x28)  Secondary phone number field
   RSRVD29 ,     // addr 41(0x29)  Secondary phone number field
   RSRVD2A ,     // addr 42(0x2A)  Secondary phone number field 
   RSRVD2B ,     // addr 43(0x2B)  Secondary phone number field 
   RSRVD2C ,     // addr 44(0x2C)  Secondary phone number field 
   RSRVD2D ,     // addr 45(0x2D)  Secondary phone number field 
   RSRVD2E ,     // addr 46(0x2E)  Secondary phone number field 
   RSRVD2F ,     // addr 47(0x2F)  Secondary phone number field 
   EMPTY30 ,     // addr 48(0x30)
   EMPTY31 ,     // addr 49(0x31)
   EMPTY32 ,     // addr 50(0x32)
   EMPTY33 ,     // addr 51(0x33)
   EMPTY34 ,     // addr 52(0x34)
   EMPTY35 ,     // addr 53(0x35)
   EMPTY36 ,     // addr 54(0x36)
   EMPTY37 ,     // addr 55(0x37)
   EMPTY38 ,     // addr 56(0x38)
   EMPTY39 ,     // addr 57(0x39)
   EMPTY3A ,     // addr 58(0x3A)
   EMPTY3B ,     // addr 59(0x3B)
   EMPTY3C ,     // addr 60(0x3C)
   EMPTY3D ,     // addr 61(0x3D)
   EMPTY3E ,     // addr 62(0x3E)
   EMPTY3F ,     // addr 63(0x3F)
   EMPTY40 ,     // addr 64(0x40)
   EMPTY41 ,     // addr 65(0x41)
   EMPTY42 ,     // addr 66(0x42)
   EMPTY43 ,     // addr 67(0x43)
   EMPTY44 ,     // addr 68(0x44)
   EMPTY45 ,     // addr 69(0x45)
   EMPTY46 ,     // addr 70(0x46)
   EMPTY47 ,     // addr 71(0x47)
   EMPTY48 ,     // addr 72(0x48)
   EMPTY49 ,     // addr 73(0x49)
   EMPTY4A ,     // addr 74(0x4A)
   EMPTY4B ,     // addr 75(0x4B)
   EMPTY4C ,     // addr 76(0x4C)
   EMPTY4D ,     // addr 77(0x4D)
   EMPTY4E ,     // addr 78(0x4E)
   EMPTY4F ,     // addr 79(0x4F)
   PRPHCCD ,     // addr 80(0x50)  Primary phone number country code field, size 8 bytes
   RSRVD51 ,     // addr 81(0x51)  Primary phone number country code field
   RSRVD52 ,     // addr 82(0x52)  Primary phone number country code field
   RSRVD53 ,     // addr 83(0x53)  Primary phone number country code field
   RSRVD54 ,     // addr 84(0x54)  Primary phone number country code field
   RSRVD55 ,     // addr 85(0x55)  Primary phone number country code field
   RSRVD56 ,     // addr 86(0x56)  Primary phone number country code field
   RSRVD57 ,     // addr 87(0x57)  Primary phone number country code field
   SEPHCCD ,     // addr 88(0x58)  Secondary phone number country code field, size 8 bytes
   RSRVD59 ,     // addr 89(0x59)  Secondary phone number country code field
   RSRVD5A ,     // addr 90(0x5A)  Secondary phone number country code field
   RSRVD5B ,     // addr 91(0x5B)  Secondary phone number country code field
   RSRVD5C ,     // addr 92(0x5C)  Secondary phone number country code field
   RSRVD5D ,     // addr 93(0x5D)  Secondary phone number country code field
   RSRVD5E ,     // addr 94(0x5E)  Secondary phone number country code field
   RSRVD5F ,     // addr 95(0x5F)  Secondary phone number country code field
   MATCH_T0 ,    // addr 96(0x60)  Detection Type 0 match action
   MATCH_T1 ,    // addr 97(0x61)  Detection Type 1 match action
   MATCH_T2 ,    // addr 98(0x62)  Detection Type 2 match action
   MATCH_T3 ,    // addr 99(0x63)  Detection Type 3 match action
   MATCH_T4 ,    // addr 100(0x64)  Detection Type 4 match action
   MATCH_T5 ,    // addr 101(0x65)  Detection Type 5 match action
   MATCH_T6 ,    // addr 102(0x66)  Detection Type 6 match action
   MATCH_T7 ,    // addr 103(0x67)  Detection Type 7 match action
   MATCH_T8 ,    // addr 104(0x68)  Detection Type 8 match action
   MATCH_T9 ,    // addr 105(0x69)  Detection Type 9 match action
   MATCH_TA ,    // addr 106(0x6A)  Detection Type 10 (0xA) match action
   MATCH_TB ,    // addr 107(0x6B)  Detection Type 11 (0xB) match action
   MATCH_TC ,    // addr 108(0x6C)  Detection Type 12 (0xC) match action
   MATCH_TD ,    // addr 109(0x6D)  Detection Type 13 (0xD) match action
   MATCH_TE ,    // addr 110(0x6E)  Detection Type 14 (0xE) match action
   MATCH_TF ,    // addr 111(0x6F)  Detection Type 15 (0xF) match action
   EMPTY70 ,     // addr 112(0x70)
   EMPTY71 ,     // addr 113(0x71)
   EMPTY72 ,     // addr 114(0x72)
   EMPTY73 ,     // addr 115(0x73)
   EMPTY74 ,     // addr 116(0x74)
   EMPTY75 ,     // addr 117(0x75)  
   MDM_EARLY ,   // addr 118(0x76)  // not presently used, retained in code for future. XBee modem started a minute early if non-zero, default = 0 for no.
   XBEE_LOPWR ,  // addr 119(0x77)  // not presently used, retained in code for future. XBee modem low power mode, [0 for none, 1 for airplane, 2 for pin sleep] default = 0 for full power all the time
   EMPTY78 ,     // addr 120(0x78)  
   EMPTY79 ,     // addr 121(0x79)
   EMPTY7A ,     // addr 122(0x7A)  
   EMPTY7B ,     // addr 123(0x7B)
   EMPTY7C ,     // addr 124(0x7C)
   DBG_LEV ,     // addr 125(0x7D)  debug print enabled level
   CRC_LSB ,     // addr 126(0x7E)  ls byte of 16-bit CCITT CRC for CFG_NVM
   CRC_MSB       // addr 127(0x7F)  ms byte of 16-bit CCITT CRC for CFG_NVM
   } ;



// compute manually the size of the virtual eeprom byte array.
// it is substantially less than the 1024-byte page used for virtual EEPROM
//
// size = 2 + 16 * 2 + 250 * 2 + 2 = 536 [0x218]

#define FLTR_NVM_BASE 0x0100

#define MATCH_LIST_SIZE 250  // maximum numbers of IDs in the list for matching, for all types
#define FLTR_NVM_SIZE 536
#define FLTR_NVM_ISIZE 268


// only three explicit cells are written directly (not as part of shadow copy)
// software version, crc least significant byte, and crc most significant byte.
// these are placed in the first byte, and the last two bytes, of the shadow EEPROM.
// define offsets for these (subscripts in the 'FLTR_NVMshadow' array as well).

#define FLTR_NVM_VERSION 0
#define FLTR_NVM_LISTSIZE 1
#define FLTR_NVM_CRC_LSB (FLTR_NVM_SIZE - 2)
#define FLTR_NVM_CRC_MSB (FLTR_NVM_SIZE - 1)

   

#endif  // _sursms24nvm_h


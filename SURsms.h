
#ifndef _sursms_h
#define _sursms_h

// this is the specific header file suitable for the SURSMS app.

// leave breadcrumb trail for debugging
#define BCRUMB printf(dpo, "*** [%s:%Lu]\r\n", __FILENAME__, (unsigned int16)__LINE__)

// PIC C definition of type 'char' is inconsistent between 8-bit and 16-bit architectures.
// PIC18 is 8-bit, and a char is unsigned 8-bit integer.
// PIC24 is 16-bit, and a char is signed 8-bit integer.
// device library supplied for these devices provides a consistent
// definition for 'BYTE' which will always be unsigned 8-bit integer.
// we will use this type in those situations where the ordinary 'char'
// definition is inappropriate.

// handy short definitions for numeric items
typedef unsigned int8 UB;
typedef unsigned int16 UW;
typedef signed int16 SW;
typedef signed int32 S32;
typedef unsigned int32 U32;

// items for support of Digi XBee 3G Global cellular modem (SMS modem)

#define XBEE_DEAD_TIME_DEFAULT 3*1000L  // default three seconds between successive modem commands

#define XBEE_API_FRAME_DELIMITER (0x7E)

#define XBEE_API_STALE_PKT_DELTA (1500L)  // transmit failed if no status response in 1.5s (1500ms)

//
// we are prepared to receive these API message packet types only,
// ftFMATCR, ftFMTXST, ftFMMST, ftFMRXSMS.
// the definitions of other packet types such as ftFMTXSMS are included in the enum for convenience.
//
enum ft {
   ftFMTXSMS=0x1F,  // SMS transmit message frame 0x1F
   ftFMATC=0x08,  // AT command frame 0x08
   ftFMATCR=0x88,  // AT command response frame 0x88
   ftFMTXST,  // SMS transmit status response 0x89
   ftFMMST,  // modem status response 0x8A
   ftFMRXSMS=0x9F,  // SMS received message frame 0x9F
   ftPH_SLEEP=0xDD  // phony value, unused by XBEE, to command pin sleep mode
};


//
// enumeration of state codes for XBee API packet reception parsing
//

typedef enum {
      psIDLE,  // looking for frame delimiter
      psLENGTH_MSB,  // looking for frame length msb
      psLENGTH_LSB,  // looking for frame length lsb
      psFRAMETYPE,  // looking for API frame type value
      psBODY,  // storing input bytes for unwanted frame type
      psCKSM,  // looking for received message checksum value
  } prs_states ;

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&



// API frame types for XBEE modem

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//
// need a largest-structure size for buffer allocation minimum
// largest structure seems to be SMS transmit
// common header is 3 bytes always (delimiter, len msb, len lsb)
// remaining structure has:
//    frametype 1 byte
//    frameID 1 byte
//    options 1 byte
//    phone number 20 bytes
//    message text 160 bytes max, may be less
//    checksum 1 byte
//
// so length is 3 + 1 + 1 + 1 + 20 + 160 + 1 = 187 bytes
// 
// round up to 192 0xC0  <<--- can't assume this
//#define MAX_API_BUF 192
// allowing for the fixed three bytes at the start,
// the varialbe part of the message is 184 bytes
#define MAX_API_FRM 184



// API frame common buffer beginning
struct FM_cbfr {
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
};

typedef struct __attribute__((packed)) FM_cbfr FMBFR;



// API frame assembly buffer (used for message reception)
struct FM_bfrasm {
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
   BYTE body[MAX_API_FRM];  // API message body
   //BYTE put_cksm;  // putative checksum, appears here for length determination
};

typedef struct __attribute__((packed)) FM_bfrasm FMBFRASM;


// AT command 0x08
//
// there are two different forms of the AT command API packet.
// one is used when a value is queried, (no_parm)
// the other when a value is set (parm)

struct FM_atcmdset {  // AT command frame for set-style commands
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
   BYTE frametype;  // 0x08
   BYTE frameid;
   BYTE command[2];  // two-character command
   BYTE cmdvalue;  // optional set value if present, otherwise it's a query
   //BYTE cksm;  // frame checksum (not used, never explicitly referenced)
} ;

typedef struct __attribute__((packed)) FM_atcmdset FMATCP;  // command with parameter. ususally a 'set', might be a 'query'


struct FM_atcmdqry {  // AT command frame for query-style commands
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
   BYTE frametype;  // 0x08
   BYTE frameid;
   BYTE command[2];  // two-character command
   //BYTE cksm;  // frame checksum (not used, never explicitly referenced)
} ;

typedef struct __attribute__((packed)) FM_atcmdqry FMATCNP;  // command with no parameter. usually a 'query'


// AT command response 0x88
//
// there are two different forms of the AT command response API packet.
// one is used when a value is queried, the other when a value is set.
// 'set' commands return a status, but no returned parameter
// 'query' commands return both a status and a returned parameter value

struct FM_atcmdsetrsp {  // AT command response frame for set-style commands
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
   BYTE frametype;  // 0x88
   BYTE frameid;
   BYTE commandCh1;  // two-character command
   BYTE commandCh2;
   BYTE cmdstat;  // status of the set command
   //BYTE cksm;  // frame checksum (not used, never explicitly referenced)
} ;

typedef struct __attribute__((packed)) FM_atcmdsetrsp FMATCNR;  // response with no return value other than status


struct FM_atcmdqryrsp {  // AT command response frame for query-style commands
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
   BYTE frametype;  // 0x88
   BYTE frameid;
   BYTE commandCh1;  // two-character command
   BYTE commandCh2;
   BYTE cmdstat;  // status of the query command
   BYTE cmdparam[1];  // result of the query. this may be longer than one byte.
   //BYTE cksm;  // frame checksum (not used, never explicitly referenced)
} ;

typedef struct __attribute__((packed)) FM_atcmdqryrsp FMATCR;  // command with both status and return value


// AT command query-style response buffer. overlays allocatable small buffer most of the time. may be large buffer however
struct BF_atcmdqryrsp {  
   BYTE respbl;  // length of response
   BYTE respb[31];  // response
} ;

typedef struct __attribute__((packed)) BF_atcmdqryrsp BFATCR;  // command with both status and return value


// Modem status 0x8A

struct FM_modemstat {
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
   BYTE frametype;  // 0x8A
   // fields above this point are identical for every frame. fields below vary according to the frame type
   BYTE mdmstat;
   //BYTE cksm;  // frame checksum (not used, never explicitly referenced)
} ;

typedef struct __attribute__((packed)) FM_modemstat FMMST;


// Transmit status 0x89

struct FM_txstat {
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
   BYTE frametype;  // 0x89
   // fields above this point are identical for every frame. fields below vary according to the frame type
   BYTE frameid;
   BYTE txstat;
   //BYTE cksm;  // frame checksum (not used, never explicitly referenced)
} ;

typedef struct __attribute__((packed)) FM_txstat FMTXST;


// Transmit SMS 0x1F

struct FM_txsms {
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
   BYTE frametype;  // 0x1F
   // fields above this point are identical for every frame. fields below vary according to the frame type
   BYTE frameid;
   BYTE options;  // reserved for future usage
   BYTE phnum[20];  // ASCII null-terminated string, destination phone number
   BYTE mtxt[160];  // text body of message
   //BYTE put_cksm;  // putative checksum, appears here for length determination
} ;

typedef struct __attribute__((packed)) FM_txsms FMTXSMS;


// Receive SMS 0x9F

struct FM_rxsms {
   U32 tStamp;  // 32-bit uptime value in milliseconds
   U32 deadTime;  // 32-bit no-transmit interval value in milliseconds
   BYTE delim;  // frame start delimiter, always 0x7E
   BYTE flength_msb;  // upper 8 bytes of frame length, big-endian format
   BYTE flength_lsb;  // lower 8 bytes of frame length, big-endian format
   BYTE frametype;  // 0x9F
   // fields above this point are identical for every frame. fields below vary according to the frame type
   BYTE phnum[20];  // ASCII null-terminated string, source phone number
   BYTE mtxt[160];  // text body of message
   //BYTE put_cksm;  // putative checksum, appears here for length determination
} ;

typedef struct __attribute__((packed)) FM_rxsms FMRXSMS;


// define union of structs for management of packets under API
// via sharable pool of buffers

typedef union APIbuf {
   FMBFR fmbfr;  // API frame prototype for serial receives
   FMBFRASM fmbfrasm;  // API frame assembly buffer for serial receives
   FMATCP fmatcp;  // AT command frame for set-style commands
   FMATCNP fmatcnp;  // AT command frame for query-style commands
   FMATCNR fmatcnr;  // AT command response frame for status-only (set-style) commands
   FMATCR fmatcr;  // AT command response frame for status-plus-response (query-style) commands
   FMMST fmmst;  // modem status response
   FMTXST fmtxst;  // SMS transmit status response
   FMRXSMS fmrxsms;  // SMS received message frame
   FMTXSMS fmtxsms;  // SMS transmit message frame
} APBU;

#define PACKET_BUFFER_SIZE (sizeof(APBU))  // this value was 188 (xBC). adding new 32-bit field makes it 192 (xC0)
#if (PACKET_BUFFER_SIZE & 1)  // don't want an odd number here
#error PACKET BUFFER SIZE IS ODD
#endif


// define similar union of structs for management of packets under API
// via sharable pool of buffers, excluding the large SMS message buffers

typedef union APIbufNoSMS {
   //FMBFR fmbfr;  // API frame prototype for serial receives
   FMATCP fmatcp;  // AT command frame for set-style commands
   FMATCNP fmatcnp;  // AT command frame for query-style commands
   FMATCNR fmatcnr;  // AT command response frame for status-only (set-style) commands
   FMATCR fmatcr;  // AT command response frame for status-plus-response (query-style) commands
   FMMST fmmst;  // modem status response
   FMTXST fmtxst;  // SMS transmit status response
   //FMRXSMS fmrxsms;  // SMS received message frame
   //FMTXSMS fmtxsms;  // SMS transmit message frame
} APBUNoSMS;

#define PACKET_BUFFER_SIZE_NOSMS (sizeof(APBUNoSMS))  // we can use a 'small buffer' for any of these that don't have to hold an SMS message
#if (PACKET_BUFFER_SIZE_NOSMS & 1)  // don't want an odd number here
#error PACKET BUFFER SIZE NOSMS IS ODD
#endif

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// the former scheme of fixed buffer layout is replaced by a pool of buffers.
// two sizes will be supported, 'small' and 'large'
//
// there used to be a third kind of buffer, 'packet buffer'.
// however the size of that element grew until it is the same as the
// 'large buffer' element. so we eliminate the third category and use
// the large buffer for the packet. 
//
// add some preprocessor check so the declared large buffer size will
// not be less than the current 192 bytes, but will grow if the
// packet length is increased.

#define NOMINAL_LARGE_BUFFER_SIZE 192
#if NOMINAL_LARGE_BUFFER_SIZE >= PACKET_BUFFER_SIZE
#define LARGE_BUFFER_SIZE NOMINAL_LARGE_BUFFER_SIZE
#else
#define LARGE_BUFFER_SIZE PACKET_BUFFER_SIZE
#endif

//
// packet data buffer quantity items
//
// we think we need fewer of these
//#define NOMINAL_PACKET_BUFFER_POOL_SIZE 5
//#define NOMINAL_PACKET_BUFFER_POOL_SIZE 4
// we think we need still fewer of these, because we can use small buffers for some of them
#define NOMINAL_PACKET_BUFFER_POOL_SIZE 3


//
// large data buffers
//
// moved earlier in the code #define LARGE_BUFFER_SIZE 192
#define NOMINAL_LARGE_BUFFER_POOL_SIZE 4

// now that we have combined the large data buffer pool with the'
// packet pool, add the nominal values, plus one more for luck

#define LARGE_BUFFER_POOL_SIZE (NOMINAL_LARGE_BUFFER_POOL_SIZE + NOMINAL_PACKET_BUFFER_POOL_SIZE + 1)

//
// small data buffers
//
#define SMALL_BUFFER_SIZE 32
//#define SMALL_BUFFER_POOL_SIZE 5
//#define SMALL_BUFFER_POOL_SIZE 4
// we will need more of these now, since we can use them for short packets
#define SMALL_BUFFER_POOL_SIZE 9


#define SUR_TAG_MSG_BUFFER_SIZE 640


// String parsing buffer sizes for various usages

#define XBEE_STRING_ASSEMBLY_BUFFER_SIZE 512
#define BT_STRING_ASSEMBLY_BUFFER_SIZE 192
#define CFG_COMMAND_BUFFER_SIZE 64  // a SWAG

//
// Message queue sizes
//

#define   API_RCV_SLOTS 10  // queue forreceiving API messages
#define   SMS_PKT_DISP_SLOTS 7  // queue for SMS text message packet disposition
#define   SMS_STAT_MTCH_SLOTS 7  // queue for matching sms transmit status responsed with packet for disposal
#define   API_XMIT_SLOTS 6  // queue for transmitting API packets
//#define   SMS_RCV_SLOTS 7  // queue for received SMS text messages
//#define   SMS_RSP_RCV_SLOTS 7  // queue for responses to transmitted SMS messages
//#define   ATQ_RCV_SLOTS 7  // queue for AT query response messages
//#define   ATS_RCV_SLOTS 7  // queue for AT set response messages
//#define   ATC_RCV_SLOTS 7  // queue for AT command response messages


// definitions for message queue data structures.
// using the good old first-on-out-limit circular buffer. 
//


union klu_pv {  // allow easy way to store -1 in a pointer
   void *p;  // access as pointer
   signed int16 v;  // access as signed value
} ;

typedef union klu_pv KLPV;


struct Q_byte {
   UB *first;  // 'first' pointer for byte queue
   UB *in;  // 'in' pointer for byte queue
   UB *out;  // 'out' pointer for byte queue
   UB *limit;  // 'limit' pointer for byte queue
   //UW count;
   //char *descr;
   BYTE *descr;
};

typedef struct Q_byte BYTEQ;

struct Q_word {
   UW *first;  // 'first' pointer for word queue
   UW *in;  // 'in' pointer for word queue
   UW *out;  // 'out' pointer for word queue
   UW *limit;  // 'limit' pointer for word queue
   //UW count;
   //char *descr;
   BYTE *descr;
};

typedef struct Q_word WORDQ;



// define an enum for battery power management

enum pwrstat {
   POWER_GOOD, POWER_LOW, POWER_RECOVERING
};



// define an enum for memory dump format control

enum dmpadr {
   DMP_NOADDR, DMP_OFST, DMP_ADDR, DMP_BOTH
};


// define enumerated type to name the heirarchical debug print levels.
// enum types are ascending zero-based sequence of integers.

typedef enum dbpr {
   ANYDB=1, LEV2, LEV3, LEV4, LEV5
   } dbpr_code ;


//
// enums and structures used to hold tag matching preference data


enum tagtype {
   IGNORE_ALL, INCLUDE_ALL, USE_DETAIL
} ;

// complex definition for the tag matching control structure

// this structure will overlay the 'FLTR_NVMshadow' byte array.
// we plan to access using a pointer.

// control entry used to store version in first byte, then pad out to even word

typedef struct __attribute__((packed)) versentry {
   char m_vers;  // software version, must be the first byte of the FLTR_NVM field
   char m_evenup;  // unused byte used to pad length to even word
} VNTRY, *P_VNTRY ;

// control entry used to store CRC in two bytes, as even-length word

typedef struct __attribute__((packed)) crcentry {
   char m_crc_lsb;  // ls byte of 16-bit CCITT CRC for FLTR_NVM. CRC must be the last two bytes in the field.
   char m_crc_msb;  // ms byte of 16-bit CCITT CRC for FLTR_NVM
} CNTRY, *P_CNTRY ;

// control entries denoting which types have lists of ids, (hpow many list items) and where the list starts for a particular type.
// there is an array of 16 of these structures, and the array subscript is the type associated with the structure.
// the list has a maximum length of 250, so an 8-bit integer is large enough to contain the count or the array subscript.

typedef struct __attribute__((packed)) typeentry {
   char m_entrycount;
   char m_firstentry;  // a subscript into the 'Fdata' array. 
} TNTRY, *P_TNTRY ;


// just an integer, expressed as a structure

typedef struct __attribute__((packed)) matchentry {
   unsigned int16 m_data;  // holds 16-bit tag ID value for matching
} MNTRY, *P_MNTRY;

typedef struct __attribute__((packed)) matchcontrol {
   VNTRY V;  // software version number
   TNTRY Ctrl[16];  // type value is the offset into this array
   unsigned int16 Fdata[MATCH_LIST_SIZE];  // holds list of 16-bit tag ID values for matching
   //MNTRY Fdata[MATCH_LIST_SIZE];  // holds list of 16-bit tag ID values for matching
   CNTRY C;  // 16-bit CCITT CRC for FLTR_NVM. CRC must be the last two bytes in the field.
} MCTRL, *P_MCTRL ;




//
// other miscellaneous defines
//
#define AsciiESC 0x1B  // ASCII ESCAPE CHARACTER

#define HealthReportMinuteMatch  4  // health report at XX:04 for any hour in which it is sent


//
// switch pressed duration codes
//
typedef enum  {
   swpNONE,  // 0: no key press
   swpSHORT,  // 1: short key press
   swpLONG  // 2: long key press
} swp_code ;


//
// XBEE cell modem low-power operating mode
//
typedef enum  {
   mlpFULL,  // 0: full power always
   mlpAIRPL,  // 1: airplane mode
   mlpSLEEP  // 2: pin-sleep 
} mlp_code ;

//
// items below for pushbutton switch debouncing
//

#define PB_CHECK_MSEC    10    // prescale for one-millisecond timer cycle
#define PB_PRESS_MSEC    100    // Stable time before registering pressed
#define PB_RELEASE_MSEC    300    // Stable time before registering released


//
// items below for periodic RTC values and LCD image updating
//

#define RTC_LCD_CHECK_MSEC    125    // prescale for one-millisecond timer cycle



//
// items below for periodic LCD image updating
//

//!#define LCD_CHECK_MSEC    250    // prescale for one-millisecond timer cycle


//!struct PVctr {
//!   int1 v[16];
//!};
//!
//!typedef struct PVctr PVEC;



// structure used in filtration of detected tag data
   
struct TokenTag {
   unsigned int16 tagId;
   BYTE tagType;
   BYTE numOfDups;  // how many duplicates of this ID
   int1 locOfDups[16];  // used to note tag ID duplications in record set (subscripts in the SURstrings.rstrPtr[] array where dups occur)
   BOOLEAN knockedOut;  // track whether this record has been filtered out
   BOOLEAN lockedIn;  // track whether this record has been selected for certain
   BYTE timeString[9];  // 8 characters and a null
   BYTE freqString[5];  // 4 characters and a null
};

typedef struct TokenTag TTAG;
   
// state machine codes for BTLE port activation process

#define __BTLEQ_LOOK_CSI_OR_PCT 0
#define __BTLEQ_LOOK_2NDPCT 1
#define __BTLEQ_DONE 99

// BTLE port spy definitions

//!#define __BTLE_SPY_UNDEFINED 99
//!#define __BTLE_SPY_READING 1
//!#define __BTLE_SPY_WRITING 2


#endif  // _sursms24_h


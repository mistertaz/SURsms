
#ifndef _GLOBALS_C
#define _GLOBALS_C

// global storage definitions for PIC24 Application

// values saved at startup for subsequent display
BOOLEAN PowerLost ;  // TRUE means PIC24 reset cause indicated this
BOOLEAN InferPowerLost ;  // TRUE means cold start, FALSE means warm restart
BOOLEAN InferPowerMaintained ;  // TRUE means warm restart, FALSE means cold restart
unsigned int16 resetCause;  // the 'restart reason'


// XBEE modem items specific to PIC24 version

U32 modemDeadTimeEnds = 0;  // can't transmit to xbee unless uptime is greater than this quantity
BOOLEAN XbeeShutDownModem = FALSE;  // deferred modem shutdown request
BOOLEAN XbeeInCommandMode = FALSE;  // track whether or not XBEE modem is in command mode
BOOLEAN XbeeSwallowedFrameDelimiter = FALSE;  // track whether new API packet input detected in AT command response processing

// anticipating modem usage appears to be disused
//!// used to anticipate modem usage
//!int NextMinuteBin;
//!BOOLEAN NeedTheHealthReportNextMinute = FALSE;
//!BOOLEAN NeedTheDetectionReportNextMinute = FALSE;
//!BOOLEAN NeedModemNextMinute = FALSE;

BOOLEAN ButtonEnabled = FALSE;
U32 ButtonWaitMatch = 0;

// state variables to keep track of things
// used in big Loop, set here and there
BOOLEAN NeedTheHealthReport = FALSE;
BOOLEAN ConditionalNeedTheHealthReport = FALSE;  // used when the health report minute matches the detection report minute
BOOLEAN PBNeedTheHealthReport = FALSE;  // used when short button press initiates a health report
BOOLEAN NeedTheDetectionReport = FALSE;
//BOOLEAN NeedTheMinuteReport = FALSE;
swp_code buttonWasPushed;  // pushbutton activity tracked here
// state variables to keep track of things
BOOLEAN XbeeModemIsActive = FALSE;  // track actual condition in this variable
//BOOLEAN XBeeApiIsActive = FALSE;  // reflect whether or not XBee 3G cellular modem initialization has completed
BOOLEAN xbeeUsingAirplaneMode = FALSE;  // low power manangement does not use 'Airplane Mode'
BOOLEAN xbeeUsingPinSleep = FALSE;  // low power management does not use 'Sleep Mode' controlled by pin signal
// cells below are used to determine whether the XBEE modem is available for use
BOOLEAN xbeeFailedReset = FALSE;  // modem CTS/ came up in wrong state
BOOLEAN xbeeFailedConfiguration = FALSE;  // any command that fails to provide a response causes this

// XBEE AT command response buffer
//!BFATCR xbAtRsp;

// most recently fetched RSSI value, and uptime when that occurred
BYTE savedRSSI = 0x99;  // x99 is an invalid value
U32 savedRSSI_At = 0;  // timestamp of saved RSSI number, allows determination of staleness

// string handling buffers and control structures

BYTE xbeeStringAssembly[XBEE_STRING_ASSEMBLY_BUFFER_SIZE];  // string buffer for modem interaction responses

SPARSE XBEEstrings = { "XBEE", xbeeStringAssembly, XBEE_STRING_ASSEMBLY_BUFFER_SIZE };

BYTE surTagAssembly[SUR_TAG_MSG_BUFFER_SIZE];  

SPARSE SURstrings = { "SUR", surTagAssembly, SUR_TAG_MSG_BUFFER_SIZE };

BYTE cfgCommandBuffer[CFG_COMMAND_BUFFER_SIZE];  // configuration command buffer

BYTE *cfgCmdPtr;  // pointer for cfgCommandBuffer[]

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//!// storage used for XBee API packet data buffers
//!//
//!APBU *pbptrs[PACKET_BUFFER_POOL_SIZE+1];  // receive buffer pointers, used as stack of free buffers
//!//APBU packetPool[PACKET_BUFFER_POOL_SIZE][PACKET_BUFFER_SIZE];  // packet buffer storage pool. pool-size count of buffer-size buffers
//!APBU packetPool[PACKET_BUFFER_POOL_SIZE];  // packet buffer storage pool. pool-size count of packet buffers
//!int pbptrDepth = 0;  // depth of free receive buffer pointer stack, empty if zero
//!int pbptrLeastDepth;  // tracked value indicating lowest free buffer count
//!

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// storage used for XBee API packet parsing
prs_states ppInputState = psIDLE;
U32 ppFrameCounter = 0;
APBU *ppFptr = 0;
FMBFR tempFptr;
unsigned int16 ppCurrentFrameLength;
unsigned int16 ppCurrentChecksum;
BYTE ppCurrentSubscript;  // subscripting into message body


BYTE reportedModemStatus = 0x99;  // impossible value, indicating not yet set via protocol
U32 reportedModemStatusTstamp;
BOOLEAN RegisteredToCellNetwork;

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


// message queueing facility storage, label strings,
// and the actual queues

BYTE lblApiRcv[] = {"API RCV PKT"};  // label for API informational packet queue
BYTE lblApiXmit[] = {"API XMIT PKT"};  // label for API informational packet queue
BYTE lblSmsPktDisp[] = {"SMS PKT DISP"};  // label for SMS packet disposition queue
BYTE lblSmsStatMatch[] = {"SMS STAT MATCH"};  // label for SMS status response matching queue
//!BYTE lblAtqHdr[] = {"ATQ RCV PKT"};  // label for AT query response packet queue
//!BYTE lblAtsHdr[] = {"ATS RCV PKT"};  // label for AT set response packet queue
//!BYTE lblAtcHdr[] = {"ATC RCV PKT"};  // label for AT command response packet queue
//!BYTE lblSmsHdr[] = {"SMS RCV PKT"};  // label for SMS message received packet queue
//!BYTE lblSmsRspHdr[] = {"SMS RSP PKT"};  // label for SMS message transmit response packet queue


// XBee API received message queue structure
// queue stores pointers, so it's a 'word queue'

UW apiRcvQueueData[API_RCV_SLOTS+1];  // add one, so queue can always hold the maximum count of items
WORDQ apiRcvQueueCtrl;
WORDQ *apiRcvQueue = &apiRcvQueueCtrl;  // kludge for expediency. fix later

// XBee API transmit message queue structure
// queue stores pointers, so it's a 'word queue'

UW apiXmitQueueData[API_XMIT_SLOTS+1];  // add one, so queue can always hold the maximum count of items
WORDQ apiXmitQueueCtrl;
WORDQ *apiXmitQueue = &apiXmitQueueCtrl;  // kludge for expediency. fix later

// SMS transmit message packet disposition queue structure
// queue stores pointers, so it's a 'word queue'

UW smsPktDispQueueData[SMS_PKT_DISP_SLOTS+1];  // add one, so queue can always hold the maximum count of items
WORDQ smsPktDispQueueCtrl;
WORDQ *smsPktDispQueue = &smsPktDispQueueCtrl;  // kludge for expediency. fix later

// SMS transmit status match queue structure
// queue stores pointers, so it's a 'word queue'

UW smsStatMatchQueueData[SMS_STAT_MTCH_SLOTS+1];  // add one, so queue can always hold the maximum count of items
WORDQ smsStatMatchQueueCtrl;
WORDQ *smsStatMatchQueue = &smsStatMatchQueueCtrl;  // kludge for expediency. fix later
//!
// SMS received message queue structure
// queue stores pointers, so it's a 'word queue'

//!UW smsRcvQueueData[SMS_RCV_SLOTS+1];  // add one, so queue can always hold the maximum count of items
//!WORDQ smsRcvQueueCtrl;
//!WORDQ *smsRcvQueue = &smsRcvQueueCtrl;  // kludge for expediency. fix later

// SMS received message response queue structure
// queue stores pointers, so it's a 'word queue'

//!UW smsrspRcvQueueData[SMS_RSP_RCV_SLOTS+1];  // add one, so queue can always hold the maximum count of items
//!WORDQ smsrspRcvQueueCtrl;
//!WORDQ *smsrspRcvQueue = &smsrspRcvQueueCtrl;  // kludge for expediency. fix later

// AT query received message queue structure
// queue stores pointers, so it's a 'word queue'

//!UW atqRcvQueueData[ATQ_RCV_SLOTS+1];  // add one, so queue can always hold the maximum count of items
//!WORDQ atqRcvQueueCtrl;
//!WORDQ *atqRcvQueue = &atqRcvQueueCtrl;  // kludge for expediency. fix later

// AT set received message queue structure
// queue stores pointers, so it's a 'word queue'

//!UW atsRcvQueueData[ATS_RCV_SLOTS+1];  // add one, so queue can always hold the maximum count of items
//!WORDQ atsRcvQueueCtrl;
//!WORDQ *atsRcvQueue = &atsRcvQueueCtrl;  // kludge for expediency. fix later

// AT command received message queue structure
// queue stores pointers, so it's a 'word queue'

//!UW atcRcvQueueData[ATC_RCV_SLOTS+1];  // add one, so queue can always hold the maximum count of items
//!WORDQ atcRcvQueueCtrl;
//!WORDQ *atcRcvQueue = &atcRcvQueueCtrl;  // kludge for expediency. fix later

// used to emphasize errors
BYTE fortyBucks[] = { "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" };

BYTE fortyBucksBoom[] = { "$$$$ BOOM! $$$$$$$$$$$$$$$$$$$$$$$$$$$$$" };

// common global storage definitions for all SURsms Applications

// volatile counters used to note the system up-time, in milliseconds and seconds.
// updated by the respective TIMER2 interrupt servicing routines in the corresponding processors.

volatile U32 uptime_milliseconds = 0;

volatile U32 uptime_seconds = 0;

volatile BOOLEAN needTickTask = FALSE;

// these booleans will toggle at the rate indicated

volatile BOOLEAN FlagHalfHz;  // toggles every second

volatile BOOLEAN FlagOneHz;  // toggles every half-second

// big loop instrumentation counters and values

   unsigned int16 cycleCounter = 0;
   unsigned int16 loopCycleCounter = 0;
   U32 thisCycleStartTime;
   U32 thisCycleEndTime;
   U32 thisCycleTimeSpan;


BOOLEAN updateLcdNow = FALSE;  // set in ISR when display needs updating

rtc_time_t datetime;  // automatically-maintained date-and-time structure using binary numbers

BOOLEAN dateTimeNeedsUpdate = FALSE;  // per-sample structure update flag

// automatically-maintained time-and-date variables, in BCD representation.
// these are used in unexpected places like setting the touch-time clock, so be careful.
// also keep hour-min-sec as binary values for convenient comparisons
int HourBCD;
int HourBin;
int MinuteBCD;
int MinuteBin;
int SecondBCD;
int SecondBin;
int DayBCD;
int MonthBCD;
int YearBCD = 0x61;

// values used to track hour and minute and second rollover to zero
int LastHourBin = 99;  // init to bogus value
int LastMinuteBin = 99;  // init to bogus value
int LastSecondBin = 99;  // init to bogus value
BOOLEAN SecondZeroCrossing = FALSE;
BOOLEAN MinuteZeroCrossing = FALSE;
BOOLEAN HourZeroCrossing = FALSE;


int longPressThresholdSecs = 3;  // this is a configured value, 3 is acceptable default
int16 longPressThresholdMsecs = 3*1000UL;  // computed later from longPressDurationSecs
int16 longPressThresholdMsecVector[] = { 2*1000UL, 3*1000UL, 4*1000UL, 5*1000UL};  // allowable range is 2-5


//
// characters used in LCD to indicate run mode and cell modem state
//
BYTE LcdActivityIdle = ' ';  // space character indicating app is idle
BYTE LcdActivityRun = '*';  // splat character indicating app is active
//BYTE LcdActivityIdle = 0xEB;  // upper-half display character indicating app is idle (upper small 'x')
//BYTE LcdActivityRun = 0xDF;  // upper-half display character indicating app is active (upper small square)

BYTE LcdModemIdle = ' ';  // space character indicating modem low-power sleep mode
BYTE LcdModemActive = 'M';  // 'M' character indicating modem active
//#define LcdModemBTWait '?'  // question mark character indicating waiting for BT connect
//#define LcdModemActiveBT 'B'  // 'B' character indicating Bluetooth connected. modem and BT are mutually exclusive
//#define LcdModemIdle 0xEB  // upper-half display character indicating modem low-power sleep mode (upper small 'x')
//#define LcdModemActive 0xDF  // upper-half display character indicating modem active (upper small square)
//#define LcdModemBTWait 0xFC  // upper-half display character indicating waiting for BT connect (sideways Beta)
//#define LcdModemActiveBT 0xE2  // upper-half display character indicating Bluetooth connected (it's a Beta). modem and BT are mutually exclusive


BOOLEAN RunningNow;
BOOLEAN relaxedSurTiming = TRUE;  // timing is relaxed if using simulated SUR

BYTE CFG_NVMshadow[2][CFG_NVM_SIZE] ;  // contents of CFG_NVM here to avoid continual reading

// subscripts permit swapping of config nvm shadow images
BYTE cfg_cur = 0;
BYTE cfg_fut = 1;

// create a word-aligned union which will permit accessing of the FLTR_NVM data
// as unsigned bytes, unsigned ints, or signed ints as required.
union ve_vec {
   unsigned int16 Iv[FLTR_NVM_SIZE/2];
   BYTE Bv[FLTR_NVM_SIZE];
   };
   
#define FILTER_NVM_STRUCT_SIZE sizeof(union ve_vec)

union ve_vec __attribute__((aligned(2))) FLTR_NVMshadow[2] ;  // contents of entire FLTR_NVM page here to avoid continual reading

// subscripts permit swapping of filter nvm shadow images
BYTE flt_cur = 0;
BYTE flt_fut = 1;

// the FLTR_NVM data maps like this:
// [0000] version number in byte
// [0001] size of tag matching list in byte (max 100 until we get larger CFG_NVM)
// [0002]-[0021] tag-subtype matching controls in two bytes. 16 X (count of entries in byte), (starting subscript for subtype list in byte)
// [0022]-[00E9] lists of tag subtype integers used to match tag detections for tag types. these are two-byte little-endian integers.
// [00EA] word holding crc for this data
//
// the arrays for control items and subtype lists are subscripted as INTEGERS, and are zero-based.
// therefore we need offsetting subscripts in the total array to access these quantities.
//
// i'm just going to hardcode these for now until I am certain of the new approach being successful
//
// these are SUBSCRIPTS in the array of INTEGERS
#define VeCtrlBase 0x0001
#define VeFdataBase 0x0011
#define VeCrcBase 0x0075


BYTE DisplayLine1String[21];
BYTE DisplayLine2String[21];
BYTE DisplayLine3String[21];
BYTE DisplayLine4String[21];

BYTE SavedDisplayLine1String[21];
BYTE SavedDisplayLine2String[21];
BYTE SavedDisplayLine3String[21];
BYTE SavedDisplayLine4String[21];

int lastSecond;

unsigned int8 BatteryVoltage;  // stores ADC response converted to 'voltage * 10' for battery voltage sample
const unsigned int8 MinimumBatteryVoltage = 36;  // represents 3.6V
enum pwrstat PowerStateCode;
BOOLEAN BatteryVoltageOK;  // need to suspend some operations if battery voltage gets below 3.6

// variables used in configuration process control
BOOLEAN GlobalCfgNvmChanged;
BOOLEAN GlobalFltrNvmchanged;
BOOLEAN GlobalResetOnExit;

// variables used to hold date & time strings instead of using allocated storage
//!BYTE dateTimeArray[24];
//!BYTE uptimeArray[48];

// next section defines items used to schedule health report
// matching hour vector values are monotonic increasing

// replaced by a define: const int healthReportMinuteMatch = 4;  // health report at XX:04 for any hour in which it is sent

// going forward using stopper bytes in arrays.
// matching hour or minute vector values are monotonic increasing.
// negative number signals end of list, since 'zero' could be potentially a valid choice.

const signed int8 HealthReportHourMatchtimes[3][25] = { 
   {12,-1} ,  // once at 12:04:00pm
   {0,12,-1} ,  // twice at 12:04:00am and 12:04:00pm
   // and just for me while developing
   {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,-1}   // every hour at XX:04:00
};

// next section defines items used to schedule detection report.
// matching minute vector values are monotonic increasing.
// we maintain the entries in order of most-frequent to least-frequent.
 
// highest frequency is last item
const signed int8 DetectionReportMinuteMatchtimes[5][31] = {
/* 0 */   {2,7,12,17,22,27,32,37,42,47,52,57,-1} , // twelve times per hour XX:02, XX:07, XX:12, ... XX:47, XX:52, XX:57
/* 1 */   {2,12,22,32,42,52,-1} ,   // six times per hour, at XX:02, XX:12, XX:22, XX:32, XX:42, and XX:52
/* 2 */   {2,32,-1} ,   // twice per hour, at XX:02 and XX:32
/* 3 */   {2,-1} ,  // once per hour, at XX:02
/* 4 */   {0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,
    30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,-1}   // thirty times per hour XX:00, XX:02, XX:04, ... XX:54, XX:56, XX:58
};


// a pool of buffers.
// two sizes will be supported, 'small' and 'large'
// only for PIC24/XBEE modem, also support 'packet' buffers via large buffers

//
// small data buffers
//
BYTE *sbptrs[SMALL_BUFFER_POOL_SIZE+1];  // small data buffer pointers, used as stack of free buffers
//BYTE *smallPool;  // pointer to small buffer storage pool, allocated at run time. pool-size count of buffer-size buffers
BYTE smallPool[SMALL_BUFFER_POOL_SIZE][SMALL_BUFFER_SIZE];  // small buffer storage pool. pool-size count of buffer-size buffers
int sbptrDepth;  // depth of free buffer pointer stack, empty if zero. depth is a small number, probably less than 16
int sbptrLeastDepth;  // tracked value indicating lowest free buffer count

//
// large data buffers
//
BYTE *lbptrs[LARGE_BUFFER_POOL_SIZE+1];  // large data buffer pointers, used as stack of free buffers
//BYTE *largePool;  // pointer to large buffer storage pool, allocated at run time. pool-size count of buffer-size buffers
BYTE largePool[LARGE_BUFFER_POOL_SIZE][LARGE_BUFFER_SIZE];  // large buffer storage pool. pool-size count of buffer-size buffers
int lbptrDepth;  // depth of free buffer pointer stack, empty if zero. depth is a small number, probably less than 16
int lbptrLeastDepth;  // tracked value indicating lowest free buffer count

BYTE OKrsp[] = "OK";


//
// special for Timer2 Interrupt Service and Tick Task
// formerly local static declarations, about which we are now skeptical
//

BYTE pbPrescale = PB_CHECK_MSEC;
BYTE rtcLcdPrescale = RTC_LCD_CHECK_MSEC;
BYTE xorByte = 0;  // this toggles between 0 and 1 to steer the 250-mS actions
unsigned int16 divideBy1000 = 0;
unsigned int16 divideBy500 = 0;

// miscellaneous storage


BYTE DetectReportSeqNumber = 0;  // init to zero because pre-incremented. display ranges from 1..99, permits identification of missed SMS messages

BYTE* DetectReportFields[8];  // hold pointers to tokenized strings from SUR detection record parsing

// flags to control active host port usage.
// don't set both at once
BOOLEAN UsingHostSUR = FALSE;  // host interaction occurs on receiver serial port if true
BOOLEAN UsingHostBTLE = FALSE;  // host interaction occurs on btle serial port if true
BOOLEAN SwallowedCSI = FALSE;  // consumed a CSI attempting to decide which host port to use
BYTE BTLEactiveState = __BTLEQ_LOOK_CSI_OR_PCT;
BYTE percentStringBuffer[80] = { '\0' };  // temp storage for assembling %YADAYADA% type strings
BYTE percentStringBufferSub = 0;  // used to assemble string in that buffer
//!BYTE BTLE_SpyOrientation = __BTLE_SPY_UNDEFINED;
//!BYTE BTLE_SpyBufCount = 0;  // count of characters on line of spy display
//!BYTE BTLE_SpyBufSub = 0;
//!BYTE BTLE_SpyBuf[128];


// scratch static storage for phone number string assembly
BYTE phoneNumberFormationBuffer[32];


//
// we simulate errors sometimes during development
//

#if __PHONY_NO_SMS_STATUS
BYTE __phonyNoSmsStatus = 0;
#endif

#if __PHONY_BAD_SMS_STATUS
BYTE __phonyBadSmsStatus = 0;
#endif


#endif  // _GLOBALS_C

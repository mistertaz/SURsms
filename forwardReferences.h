#ifndef _FORWARD_REFS_H
#define _FORWARD_REFS_H

//##############################################################################################################################################################

//
// forward references common to all PIC code
//

BYTE *prefixDetectionReportResponse();
BYTE *formCompletePrimaryNumber();
BYTE *formCompleteSecondaryNumber();
BYTE getModemIndicator();
BYTE gethex();
BYTE gethex1();
BYTE read_fltr_nvm(long int);
BYTE xbeeApiSendAtCommand(BYTE *, U32 x=2000L);
//BYTE phoneNumberFormationBuffer[];  // scratch static storage for phone number string assembly
BFATCR *xbeeSendAtCommand(BYTE *, int16 x = 500);
UW wordqDequeue(WORDQ *);
BOOLEAN SURCommandParsedReply(BYTE *, SPARSE &x);
BOOLEAN SURCommandRawReply(BYTE *, BYTE *, int);
BOOLEAN dbpEnabled(int);  // test for debug print enabled and at what level
BOOLEAN ignoreThisString(char *);
BOOLEAN IsBtleConnected();
BOOLEAN isShadowCrcCorrect();
BOOLEAN isXbeeAsleep();
BOOLEAN isXbeeAwake();
BOOLEAN isXbeeOkToUse();
BOOLEAN xbeeCmdOkReply(BYTE *cstr, int16 x = 500);
BOOLEAN xbeeCommandMode();
BOOLEAN xbeeExitCommandMode();
BOOLEAN xbeeRuntimeConfiguration();
BOOLEAN xbeeWaitAndBlink(BYTE x = 45);
char *stringModemPower();
int bcd2hex(int x);
int hex2bcd(int x);
int wordqCount(WORDQ *);
int xbcheck(void *);
signed int16 convertForConfig(int8, BYTE*);
signed int16 convertHexForConfig(int8, BYTE*);
signed int16 didStringMatch(BYTE *teststr, int x=0);
unsigned int1 write_fltr_nvm(long int, BYTE);
unsigned int16  storeShadowCFG_NVM();
unsigned int16 gethex3();
unsigned int16 round_up(unsigned int16, unsigned int16);  // round an integer up to the next even multiple of another integer
U32 uptimeMilliseconds();
U32 uptimeSeconds();
UW modemWaitTime(UW x = 5000L);
void *lballoc();
void *sballoc(); 
void *stringModemStatus(BYTE);
void *stringTheDateAndTime();
void *stringTheDateTimeUptime();
void *stringTheUptime(U32 x=0xFFFFFFFF);
void *stringTransmitStatus(BYTE);
void appStartUpModem();
void BigLoopMaintenance(BOOLEAN immediate=FALSE);
void DebouncePb();
void WaitWithHousekeeping(int);
void appShutDownModem();
void appShutDownModem_Inner();
void btGetCmdResponse();
void clearRunning();
void commandProc(BOOLEAN calledFromConfig=FALSE);
void configuredValueInit();
void defaultShadowCFG_NVM();
void defaultShadowFLTR_NVM();  // normal product form with no preloaded datavoid drainBtBufAndProcess();
void drainSmsBufAndProcess(int16 durationMs=150);  // nominally process for 150ms
void drainSmsBufAndParse();
void drainSmsBufAndTrash();
void dumpToHost(BYTE c[], unsigned long);  // formatted hex and character display, using offset addressing
void dumpToHostNoAscii(BYTE c[], unsigned long);  // formatted hex and character display, using offset addressing
void dumpToHostWithAddress(BYTE c[], unsigned long);  // formatted hex and character display, using PIC memory addressing
void dumpToHostWithBoth(BYTE c[], unsigned long);  // formatted hex and character display, using PIC memory addressing
void esad(char *x=0);
void fetchShadowCFG_NVM();
void fetchShadowFLTR_NVM();  // PIC24 stores filter data in expanded external EEPROM
void fillBytes(BYTE *, unsigned int16, BYTE, BOOLEAN termAsString=FALSE);
void fillWords(unsigned int16 *, unsigned int16, unsigned int16);
void fiveSecCountDown();
void initUptimerMilliseconds();
void innerDumpToHost(BYTE c[], unsigned long, int, BOOLEAN);  // formatted hex and character display
void lbfree(BYTE *);
void lbload(BYTE *);
void loopingPacketsProcess(int16 durationMs=150);  // nominally process for 150ms (150 character times)
void maintainLCD(BOOLEAN x=TRUE);
void MinuteWork();  // items which need to be performed each minute, or multiple thereof
void needResetMessage();
void packetInfoDisplay(APBU *, BOOLEAN x=FALSE);
void packetsMatch();
void packetsParse(char mch);
void packetsProcess();
void packetsStalenessCheck();
void postConfigInit();
void preConfigInit();
void prepareShadowCFGForMods();
void prepareShadowFLTRForMods();
void resetPacketsParse();
void resetTickTask();
void sbfree(BYTE *);
void sbload(BYTE *);
void setModemIndicator(BYTE);
void setRunning();
void set_time(BYTE *);
void shadowCFG_FutureToCurrent();
void shadowFLTR_FutureToCurrent();
void shadowedEEPROMSetup();
void shortConfigQuery();
void skip_btle_until_pct();
void skip_until_pct();
void snapBufferStats();
void snapQueueCounts();
void snapReport();
unsigned int16 storeShadowFLTR_NVM();
void tell_time();
void tickTask();  // forward reference for timer 2 quick task routines
void waitXbeeDataStart(int16 x = 750);
void waitXbeeDataFinish(int16 x = 250);
void wordqEnqueue(WORDQ *, UW);
void xbeeAirplaneModeControl(int);
void xbeeApiAirplaneModeControl(int);
void xbeeApiGetRssi();
void xbeeApiPacketSend(APBU *);
void xbeeApiSendSMSmessage(BYTE *, BYTE *, int x=0);
void xbeeGetRssi();
void xbeePutToSleep();
void xbeeSetPowerControlFlags();
void xbeeShutDownModem();
void xbeeWakeUp();
void xbfree(void *);
void xx_RTC_Read();
void xx_RTC_Write();
void xx_RTC_init();


//##############################################################################################################################################################

#endif  // _FORWARD_REFS_H

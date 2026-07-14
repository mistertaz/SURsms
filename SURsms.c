/* 
   SURsms.c
   SMS text message gateway for SUR immersible acoustic receiver
   Using PIC24FJ128GA204 processor
*/

#ifndef __PCD__
#error This application must compile only for a PIC24 device
#endif

// specialized feature control

#define __IMPLEMENT_PIN_SLEEP 0

// specialized debug print control


// control inclusion of debug printing code, area by area

// nothing enabled
//!#define __DEBUG_INIT 0
//!#define __DEBUG_INIT_CORE 0
//!#define __DEBUG_MISC 0
//!#define __DEBUG_LCD 0
//!#define __DEBUG_BTLE 0
//!#define __DEBUG_XBEE 0
//!#define __DEBUG_XBEE_API 0
//!#define __DEBUG_XBEE_API_PKT 0
//!#define __DEBUG_XBEE_API_SMS 0
//!#define __DEBUG_QMGR 0
//!#define __DEBUG_BMGR 0
//!#define __DEBUG_DETREP 0
//!#define __DEBUG_HLTHREP 0
//!#define __DEBUG_SURCOM 0
//!#define __DEBUG_CRC 0
//!#define __DEBUG_NVM 0


// small amount of debugging
#define __DEBUG_INIT 0
#define __DEBUG_INIT_CORE 1 //0
#define __DEBUG_MISC 1 //0
#define __DEBUG_LCD 0
#define __DEBUG_BTLE 0
#define __DEBUG_XBEE 1 //0
#define __DEBUG_XBEE_API 0
#define __DEBUG_XBEE_API_PKT 1 //0
#define __DEBUG_XBEE_API_SMS 0
#define __DEBUG_QMGR 0
#define __DEBUG_BMGR 0
#define __DEBUG_DETREP 1 //0
#define __DEBUG_HLTHREP 0
#define __DEBUG_SURCOM 0
#define __DEBUG_CRC 0
#define __DEBUG_NVM 0


// '__TAZ_DEBUG' adds some configuration defaults used in development if non-zero
#define __TAZ_DEBUG 0
#warning/information TAZ DEBUG INITS __TAZ_DEBUG

// ***DANGEROUS***
#define __SUPPRESS_NVM_DEFAULTS 0
#warning/information SUPPRESS NVM DEFAULTS __SUPPRESS_NVM_DEFAULTS

#define __COMPILE_FOR_DEVELOPMENT 0
#warning/information COMPILE FOR DEVELOPMENT __COMPILE_FOR_DEVELOPMENT

#if __COMPILE_FOR_DEVELOPMENT

#define __USING_ICD 1  // nonzero for ICD
#warning/information Using ICD __USING_ICD

#define __USING_LDR 0  // nonzero for resident loader usage
#warning/information Using LDR __USING_LDR

#else  // product

#define __USING_ICD 0  // nonzero for ICD
#warning/information Using ICD __USING_ICD

#define __USING_LDR 1  // nonzero for resident loader usage
#warning/information Using LDR __USING_LDR

#endif

#define __USING_SMSISR 0  // nonzero for interrupt-driven SMS serial data receive
#warning/information Using SMS ISR __USING_SMSISR

#define __USING_BTLEISR 0  // nonzero for interrupt-driven BTLE serial data receive
#warning/information Using BTLE ISR __USING_BTLEISR

// while debugging, may force seldom-seen error conditions
#define __PHONY_NO_SMS_STATUS 0
#define __PHONY_BAD_SMS_STATUS 0

#define BALLOC_FAIL_FATAL 1  // make buffer allocation failure a fatal error for product. not while debugging.

#define LONG_PRESS_RESET 1  // 0 means long press engages bluetooth. 1 means long press resets processor

#define __USE_IFD 0  // nonzero to introduce delay between packet processing

#include "SURsmsHw.h"  // has to precede library includes because it contains the '#device'

//#HEXCOMMENT\ Hey You Guys!!! comment for the end of hex file

#case  // enable case sensitivity

//!#use profile(ICD)
//!#profile functions,profileout

#include <stdlib.h>
//#include <stdlibm.h>  // lets us use malloc()/calloc()
#include <ctype.h>
#include <string.h>
#include "stringb.h"  // modified string handling, for type 'BYTE'
#include "stringsParse.h"  // common string-parsing functions
#include <24128.c>  // vendor-supplied driver for 16Kb EEPROM
#include "SURsmsNvm.h"
#include "SURsms.h"
#include "forwardReferences.h"  // organize forward reference definition prototypes here
#include "globals.c"  // common global definitions

#if __USING_LDR != 0
#include <pcd_bootloader.h>  // need this if using the resident bootloader
#endif


//int const VERSION = 0x11;  // 29-Sep_2019 by TAZ
//int const VERSION = 0x12;  // 15-Dec_2019 by TAZ  version 1.2 for production PIC24 board
//const BYTE VERSION = 0x13;  // 12-Apr-2020 by TAZ  version 1.3 for production PIC24 board, adds common sources and tag filtering
//const BYTE VERSION = 0x23;  // 05-Aug-2020 by TAZ  version 2.3 for production PIC24 board, major version bumped up by one
//const BYTE VERSION = 0x24;  // 03-Oct-2022 by TAZ  version 2.4 for revised production PIC24 board, has new RTC, minor version bumped up by one
//const BYTE VERSION = 0x25;  // 07-Jan-2023 by TAZ  version 2.5 for refinement of product after shipping release 2.4
//const BYTE VERSION = 0x30;  // 23-Apr-2023 by TAZ  version 3.0 for PIC24 board development going forward, using BTLE for the host interaction
//const BYTE VERSION = 0x31;  // 13-Jun-2023 by TAZ  version 3.1 for PIC24 board development going forward, using BTLE for the host interaction
//const BYTE VERSION = 0x32;  // 15-Jul-2023 by TAZ  version 3.2 for PIC24 board development going forward, using BTLE for the host interaction
//BYTE const VERSION = 0x40;  // 06-Apr-2024 by TAZ  version 4.0 for revised PIC24 board development going forward, larger display, revert to old RTC
//BYTE const VERSION = 0x4F;  // 29-May-2024 by TAZ  version 4.F using Sonotronics versioning convention for development versions
//BYTE const VERSION = 0x41;  // 07-Jun-2024 by TAZ  version 4.1 for product release of revised PIC24 board. larger display, revert to old RTC
BYTE const VERSION = 0x4E;  // 22-Jun-2026 by TAZ  version 4.E using Sonotronics versioning convention for development versions

//const BYTE vmsd = VERSION / 16;
//const BYTE vlsd = VERSION % 16;
BYTE const vmsd = (VERSION / 16);
BYTE const vlsd = (VERSION % 16);

//const char DialNum[] = "5202417354";  // Marlin phone
//const char DialNum[] = "5204047475";  // TAZ phone


//
// much of the work of this application is performed by functions
// residing in these included files.
//

#include "streamRevector.c"
#include "stringsParse.c"  // common string-parsing functions
//#include "lcd_SMS.c"
#include "Flex_LCD420_fast.c"
#include "touch_time.c"  // re-introduced for v4.X
/////#include "RTC_RV3129_120522.c"  // newer driver code for Micro Crystal RV-3129-C3 RTC Clock/Calendar
#include "RTC.C"
#include "crcSMS.c"
#include "configSMS.c"
#include "uptimeTimer2.c"  // put this here 
#include "utilityFunctions.c"  // common functions code
#include "NvmUtilityFunctions.c"
#include "SURCommFunctions.c"
#include "detectionReport.c"
#include "healthReport.c"
#include "bufferManager.c"
#include "queueManager.c"
#include "initializations.c"
#include "xbeeModemControlFunctions.c"
#include "xbeeModemUtilityFunctions.c"
#include "xbeeApiProtocolUtilityFunctions.c"
#include "xbeeApiProtocolFunctions.c"
#include "xbeeApiReceivedMessageHandler.c"
#include "mainProcess.c"

#TODO See just how "TODO" manifests itself

#if __USING_SMSISR  // not using this ISR anymore
//
// SMS serial port received data interrupt service
//
// read character, send to packet parser if API is running,
// otherwise send to strings parser to handle responses to
// AT-mode commands.
//
#int_RDA2
void rda2_isr()
{
   unsigned char rch;

   rch = fgetc(SMSport);  // ...read 'em and save 'em
   if (XBeeApiIsActive)
   {
      packetsParse(rch);  // deliver character to packet parsing function
   }
   else
   {
      
      // ??? stringsParse(rch);  // deliver character to string parsing function
      // must be intended as below:
      stringsParse(rch, XBEEstrings);  // deliver character to parsing function
   }
}
#endif



#if __USING_BTLEISR
//
// BTLE serial port received data interrupt service
//
// read character, send to ???
//
#int_RDA3
void rda3_isr()
{
   unsigned char rch;

   rch = fgetc(BTport);  // ...read 'em and save 'em
   // now what?
}
#endif


//
// main program entry point
//
void main()
{
   // capture the processor's value indicating the cause of this startup
   resetCause = restart_cause();  // grab the restart reason now
   PowerLost = resetCause == RESTART_POWER_UP;
   // examine CFG_NVM shadow now to see whether or not this is a warm start
   InferPowerMaintained = isShadowCrcCorrect();
   InferPowerLost = !InferPowerMaintained;

   preConfigInit();  // initialize some items

   // %%%%%%%%%%%%%%%%%%%%%%%%
   // %%%%%%%%%%%%%%%%%%%%%%%%
   // ---> we're not doing this anymore: configuration();
   // %%%%%%%%%%%%%%%%%%%%%%%%
   // %%%%%%%%%%%%%%%%%%%%%%%%
   
   postConfigInit();  // initialize some other items
   
   // we're done with initialization now. say so, and start updating time-of-day
   //                         12345678901234567890
   strcpy(DisplayLine2String,"INITIALIZED         ");
   //strcpy(DisplayLine2String,"INITIZED            ");
   
#if __DEBUG_INIT || __DEBUG_INIT_CORE || __DEBUG_MISC
   if (dbpEnabled(LEV2))
   {
      printf(dpo, "##### INITIALIZATION COMPLETED, ELAPSED TIME %s\r\n", stringTheUptime());
   }
#endif
   
   
   ButtonWaitMatch = uptimeMilliseconds() + 60000L;  // button disabled for another minute
   maintainLCD();

   // initialize some flags here
   clearRunning();  // indicator in the LCD
   stringsParseSetInitialConditions(XBEEstrings);  // setup the string parsing process
   resetPacketsParse();  // setup the protocol parsing state machine
   buttonWasPushed = swpNONE;  // ignore any pushes during initialization
   
   // run the Big Loop. function should never return.
   process();
   
   // if it does return, reset the system
   printf(dpo, "##### inappropriate main process end, resetting system!!!\r\n");
   reset_cpu();   // reset program
}



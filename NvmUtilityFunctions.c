#ifndef __NVM_UTILITY_FUNCTIONS_C_
#define __NVM_UTILITY_FUNCTIONS_C_
//
// short functions to perform odd, handy tasks
//


//BYTE read_ext_eeprom(long int address) {void write_ext_eeprom(long int address, BYTE data)
// wrapper functions for filter data in external EEPROM
// addressing in the actual external memory is in reference to the
// same zero base as the configuration memory, an offset needs to be applied
// so caller can use zero-based addresses.
// the offset used is actually a defined constant FLTR_NVM_BASE

BYTE read_fltr_nvm(long int addy) 
{
   return read_ext_eeprom(addy + FLTR_NVM_BASE);
}


unsigned int1 write_fltr_nvm(long int addy, BYTE val) 
{
   write_ext_eeprom(addy + FLTR_NVM_BASE, val);
   return TRUE;  // these always succeed
}





// 
// some subsidiary functions for non-volatile configuration storage management
//

//
// utility functions to completely copy FLTR_NVM contents
// to or from a fixed RAM array.
// FLTR_NVM contents reside in the external EEPROM, just 'above'
// the configuration (CFG_NVM) contents.
//

//
// copy content from NVRAM to both of the shadow nvm images
//

void fetchShadowFLTR_NVM()
{
   long int j;
   BYTE dbyte;

   for (j = 0 ; j < FLTR_NVM_SIZE ; j++)
   {
      dbyte = read_fltr_nvm(j);
      FLTR_NVMshadow[0].Bv[j] = dbyte;
      FLTR_NVMshadow[1].Bv[j] = dbyte;
   }
}



//
// form CRC over entire shadow RAM data field, store in the last two bytes
// copy shadow RAM data including CRC to the FLTR_NVM storage
//

//
// new thing: return elapsed time in milliseconds
//
//void storeShadowFLTR_NVM()
unsigned int16 storeShadowFLTR_NVM()
{
   unsigned int16 CrcNow;
   unsigned int16 *pCrc = (unsigned int16 *)&FLTR_NVMshadow[flt_cur].Bv[FLTR_NVM_CRC_LSB];  // little-endian 2-byte integer
   long int i;
   U32 thisCycleStartTime;
   U32 thisCycleEndTime;
   U32 thisCycleTimeSpan;
   
   thisCycleStartTime = uptimeMilliseconds();
   
   // form new CRC over shadow copy and store in array
   //CrcNow = crcFast(FLTR_NVMshadow[flt_cur].Bv, (FLTR_NVM_SIZE - 2));
   CrcNow = crcFast(FLTR_NVMshadow[flt_cur].Bv, (FLTR_NVM_SIZE - 2), TRUE);  // include some debug print
   *pCrc = CrcNow;

   // rewrite shadow copy data to FLTR_NVM
   for (i = 0 ; i < FLTR_NVM_SIZE ; i++)
   {
      //write_ext_eeprom(i + CFG_NVM_SIZE, FLTR_NVMshadow[flt_cur].Bv[i]);
      write_fltr_nvm(i, FLTR_NVMshadow[flt_cur].Bv[i]);
   }
   
   thisCycleEndTime = uptimeMilliseconds();
   thisCycleTimeSpan = thisCycleEndTime - thisCycleStartTime;
   return thisCycleTimeSpan & 0xFFFF;
}



//
// exchange the FLTR_NVM subscripts, exchanging the
// roles of 'current' and 'future'
//

void swapShadowFLTR_NVMRoles()
{
   BYTE x = flt_cur;
//!   cfg_cur = flt_fut;
//!   cfg_fut = x;
   flt_cur = flt_fut;
   flt_fut = x;
}


//
// copy contents of 'current' shadow NVM image to 'future',
// in preparation for modification.
//

void copyShadowFLTR_currentToFuture()
{
//!   int i;
//!   
//!   for (i = 0; i < FLTR_NVM_SIZE; i++)
//!   {
//!      FLTR_NVMshadow[cfg_fut][i] = FLTR_NVMshadow[cfg_cur][i];
//!   }

//!   memcpy(FLTR_NVMshadow[cfg_fut], FLTR_NVMshadow[cfg_cur], FLTR_NVM_SIZE);
//!   memcpy(FLTR_NVMshadow[flt_fut], FLTR_NVMshadow[flt_cur], FLTR_NVM_SIZE);
   memcpy(FLTR_NVMshadow[flt_fut].Bv, FLTR_NVMshadow[flt_cur].Bv, FLTR_NVM_SIZE);
}


//
// prepare for configuration NVRAM changes by
// copying 'current' to 'future'
//
// initially trivial, make this its own function in case
// other steps are required later on.
//

void prepareShadowFLTRForMods()
{

   copyShadowFLTR_currentToFuture();
}


//
// implement configuration NVRAM changes by
// comparing 'current' to 'future' contents,
// writing differences from 'future' to
// actual NVRAM, exchanging roles.
//

void shadowFLTR_FutureToCurrent()
{
   unsigned int16 ii;
   int i,j ;
   BYTE x,y;
   BOOLEAN somethingChanged = FALSE;  // we should always change something...
         
   // form new CRC over shadow copy and store in array
   ii = crcFast((BYTE *)FLTR_NVMshadow[flt_fut].Bv, (FLTR_NVM_SIZE - 2), TRUE);  // include some debug print
   i = (ii >> 8 ) & 0xff ; // msb
   j = ii & 0xff ; // lsb
   FLTR_NVMshadow[flt_fut].Bv[FLTR_NVM_CRC_MSB] = i;   // update shadow copy then rewrite those bytes in CFG_NVM
   FLTR_NVMshadow[flt_fut].Bv[FLTR_NVM_CRC_LSB] = (BYTE)j;

   // rewrite shadow copy data to FLTR_NVM
   // FLTR_NVM_BASE must be added to the loop index
   // to find the actual address in NVRAM
   for (j = 0 ; j < FLTR_NVM_SIZE ; j++)
   {
      x = FLTR_NVMshadow[flt_fut].Bv[j];  // putative new value for this cell
      y = FLTR_NVMshadow[flt_cur].Bv[j];  // current value for this cell
      if (x == y)  // if new value same as old, do nothing
         continue;
      write_ext_eeprom(j + FLTR_NVM_BASE, x);  // otherwise write the new value to NVRAM
      //printf(hpo,"*** NVRAM[0x%02X] 0x%02X->0x%02X\r\n", j, y, x);
      somethingChanged = TRUE;
   }
   // exchange NVM buffer roles
   swapShadowFLTR_NVMRoles();
#if __DEBUG_NVM
   if (!somethingChanged)
   {
      printf(hpo,"*** FILTER NVRAM no change (suspicious)\r\n");
   }
#endif
}



//
// fill the shadow FLTR_NVM with 0xFF, then set current firmware version number
// and default content for all configuration items.
// write to the real FLTR_NVM.
//


void defaultShadowFLTR_NVM()  // normal product form with no preloaded data
{

#if __SUPPRESS_NVM_DEFAULTS
   return;  // short-circuit this process
#endif

  
   for (int16 jjj = 0 ; jjj < FLTR_NVM_SIZE ; jjj++)
   {
      FLTR_NVMshadow[flt_cur].Bv[jjj] = 0xFF;  // set the shadow RAM contents to be 0xFF
   }
#if __DEBUG_NVM
   printf(hpo,"*** defaultShadowFLTR_NVM() -- FLTR_NVM shadow background initialization done.\r\n");
#endif

   // initialize these two byte values
   FLTR_NVMshadow[flt_cur].Bv[FLTR_NVM_VERSION] = VERSION;   // current version
   FLTR_NVMshadow[flt_cur].Bv[FLTR_NVM_LISTSIZE] = MATCH_LIST_SIZE;  // set field to integer size of match list
   
#if __DEBUG_NVM
   printf(hpo,"*** defaultShadowFLTR_NVM() -- FLTR_NVM shadow version, match list size initialization done.\r\n");
#endif

   // set all the type-related counts to zero for all lists empty
   for (int16 jjj = 0 ; jjj < 16 ; jjj++)
   {
      FLTR_NVMshadow[flt_cur].Bv[jjj * 2 + 2] = 0;
   }
   
#if __DEBUG_NVM
   printf(hpo,"*** defaultShadowFLTR_NVM() -- FLTR_NVM shadow list item counts initialization done.\r\n");
#endif

   // debug output
#if __DEBUG_INIT || __DEBUG_NVM
   printf(dpo,"FLTR_NVM after general initialization:\r\n");
   dumpToHostWithBoth(FLTR_NVMshadow[flt_cur].Bv, FLTR_NVM_SIZE);
#endif
   
   storeShadowFLTR_NVM();  // compute new CRC, store entire content in the FLTR_NVM
}


// 
// some subsidiary functions for non-volatile configuration storage management
//
// utility functions to completely copy CFG_NVM contents
// to or from a fixed RAM array

//
// copy content from NVRAM to both of the shadow nvm images
//

void fetchShadowCFG_NVM()
{
   int j;
   BYTE dbyte;

   for (j = 0 ; j < CFG_NVM_SIZE ; j++)
   {
      dbyte = read_ext_eeprom(j);
      CFG_NVMshadow[0][j] = dbyte;
      CFG_NVMshadow[1][j] = dbyte;
   }
}


//
// form new CRC over shadow CFG_NVM contents,
// store that and current values from shadow
// CFG_NVM in external EEPROM

//
// new design allows more frequent updating of actual NVRAM without
// rewriting the entire thing (consumes over 1.3 seconds)
//

//
// new thing: return elapsed time in milliseconds
//
//void storeShadowCFG_NVM()
unsigned int16 storeShadowCFG_NVM()
{
   unsigned int16 ii;
   int i,j ;
   U32 thisCycleStartTime;
   U32 thisCycleEndTime;
   U32 thisCycleTimeSpan;
   
   thisCycleStartTime = uptimeMilliseconds();
         
   // form new CRC over shadow copy and store in array
   ii = crcFast((BYTE *)CFG_NVMshadow, (CFG_NVM_SIZE - 2));
   i = (ii >> 8 ) & 0xff ; // msb
   j = ii & 0xff ; // lsb
   CFG_NVMshadow[cfg_cur][CRC_MSB] = i;   // update shadow copy then rewrite those bytes in CFG_NVM
   CFG_NVMshadow[cfg_cur][CRC_LSB] = j;

   // rewrite shadow copy data to CFG_NVM
   for (j = 0 ; j < CFG_NVM_SIZE ; j++)
   {
      write_ext_eeprom(j, CFG_NVMshadow[cfg_cur][j]);
   }
   
   thisCycleEndTime = uptimeMilliseconds();
   thisCycleTimeSpan = thisCycleEndTime - thisCycleStartTime;
   return thisCycleTimeSpan & 0xFFFF;
}

//
// exchange the CFG_NVM subscripts, exchanging the
// roles of 'current' and 'future'
//

void swapShadowCFG_NVMRoles()
{
   BYTE x = cfg_cur;
   cfg_cur = cfg_fut;
   cfg_fut = x;
}


//
// copy contents of 'current' shadow NVM image to 'future',
// in preparation for modification.
//

void copyShadowCFG_currentToFuture()
{
//!   int i;
//!   
//!   for (i = 0; i < CFG_NVM_SIZE; i++)
//!   {
//!      CFG_NVMshadow[cfg_fut][i] = CFG_NVMshadow[cfg_cur][i];
//!   }
   memcpy(CFG_NVMshadow[cfg_fut], CFG_NVMshadow[cfg_cur], CFG_NVM_SIZE);
}


//
// prepare for configuration NVRAM changes by
// copying 'current' to 'future'
//
// initially trivial, make this its own function in case
// other steps are required later on.
//

void prepareShadowCFGForMods()
{

   copyShadowCFG_currentToFuture();
}


//
// implement configuration NVRAM changes by
// comparing 'current' to 'future' contents,
// writing differences from 'future' to
// actual NVRAM, exchanging roles.
//

void shadowCFG_FutureToCurrent()
{
   unsigned int16 ii;
   int i,j ;
   BYTE x,y;
   BOOLEAN somethingChanged = FALSE;  // we should always change something...
         
   // form new CRC over shadow copy and store in array
   ii = crcFast((BYTE *)CFG_NVMshadow[cfg_fut], (CFG_NVM_SIZE - 2));
   i = (ii >> 8 ) & 0xff ; // msb
   j = ii & 0xff ; // lsb
   CFG_NVMshadow[cfg_fut][CRC_MSB] = i;   // update shadow copy then rewrite those bytes in CFG_NVM
   CFG_NVMshadow[cfg_fut][CRC_LSB] = j;

   // rewrite shadow copy data to CFG_NVM
   // CFG_NVM_BASE is zero, so value of loop index
   // is the actual address in NVRAM
   for (j = 0 ; j < CFG_NVM_SIZE ; j++)
   {
      x = CFG_NVMshadow[cfg_fut][j];  // putative new value for this cell
      y = CFG_NVMshadow[cfg_cur][j];  // current value for this cell
      if (x == y)  // if new value same as old, do nothing
         continue;
      write_ext_eeprom(j, x);  // otherwise write the new value to NVRAM
#if __DEBUG_NVM
      printf(hpo,"*** NVRAM[0x%02X] 0x%02X->0x%02X\r\n", j, y, x);
#endif
      somethingChanged = TRUE;
   }
   // exchange NVM buffer roles
   swapShadowCFG_NVMRoles();
#if __DEBUG_NVM
   if (somethingChanged)
   {
      printf(hpo,"*** NVRAM current contents after mods\r\n");
      dumpToHostWithBoth(CFG_NVMshadow[cfg_cur], CFG_NVM_SIZE);
   }
   else
   {
      printf(hpo,"*** NVRAM no change (suspicious)\r\n");
   }
#endif
}



//
// fill the shadow CFG_NVM with 0xFF, then set current firmware version number
// and default content for all configuration items.
// write to the real CFG_NVM.
// compute and store new CRC for the new content.
//
// also use this occasion to set initial clock/calendar values to
// some reasonable settings: 00:00:01 01/01/00 (2000)
//

void defaultShadowCFG_NVM()
{

#if __SUPPRESS_NVM_DEFAULTS
   return;  // short-circuit this process
#endif


   fillBytes(CFG_NVMshadow[cfg_cur], CFG_NVM_SIZE, 0xFF);
      
   // "visible" parameter default settings
   CFG_NVMshadow[cfg_cur][VERS] = VERSION;   // current version
   CFG_NVMshadow[cfg_cur][USETMATCH] = 0;   // no filtering of tag data to prioritize reporting
   CFG_NVMshadow[cfg_cur][USEDMATCH] = 1 ;  // enable filtering of detections based on multiple entries "Enable Duplicate Tag Type 9 Match"
   CFG_NVMshadow[cfg_cur][SECONDARY] = 0; // no secondary report
   CFG_NVMshadow[cfg_cur][DETREPINT] = 1; // detection report every ten minutes
   CFG_NVMshadow[cfg_cur][HLTHREPINT] = 0;   // health report once per day
   CFG_NVMshadow[cfg_cur][USERAWLOGS] = 0;   // use spreadsheet-ready data not raw
   CFG_NVMshadow[cfg_cur][ALLTAGS] = 0;   // not in promiscuous detection report mode
   CFG_NVMshadow[cfg_cur][PBLPRTHR] = 5;   // pushbutton long-press threshold 5 seconds
   strcpy(&CFG_NVMshadow[cfg_cur][PRPHNUM], "5205550000");   // bogus phone number for primary report
   CFG_NVMshadow[cfg_cur][PRPHLEN] = strlen(&CFG_NVMshadow[cfg_cur][PRPHNUM]); // length of primary number string
   strcpy(&CFG_NVMshadow[cfg_cur][SEPHNUM], "5205559999");   // bogus phone number for secondary report
   CFG_NVMshadow[cfg_cur][SEPHLEN] = strlen(&CFG_NVMshadow[cfg_cur][SEPHNUM]); // length of secondary number string
   strcpy(&CFG_NVMshadow[cfg_cur][PRPHCCD], "");   // *empty* string value
   CFG_NVMshadow[cfg_cur][PRCCLEN] = strlen(&CFG_NVMshadow[cfg_cur][PRPHCCD]); // length of primary number country code string
   strcpy(&CFG_NVMshadow[cfg_cur][SEPHCCD], "");   // *empty* string value
   CFG_NVMshadow[cfg_cur][SECCLEN] = strlen(&CFG_NVMshadow[cfg_cur][SEPHCCD]); // length of primary number country code string
   CFG_NVMshadow[cfg_cur][MATCH_T0] = IGNORE_ALL;   // no reporting of tag data Type 0
   CFG_NVMshadow[cfg_cur][MATCH_T1] = IGNORE_ALL;   // no reporting of tag data Type 1
   CFG_NVMshadow[cfg_cur][MATCH_T2] = IGNORE_ALL;   // no reporting of tag data Type 2
   CFG_NVMshadow[cfg_cur][MATCH_T3] = IGNORE_ALL;   // no reporting of tag data Type 3
   CFG_NVMshadow[cfg_cur][MATCH_T4] = IGNORE_ALL;   // no reporting of tag data Type 4
   CFG_NVMshadow[cfg_cur][MATCH_T5] = IGNORE_ALL;   // no reporting of tag data Type 5
   CFG_NVMshadow[cfg_cur][MATCH_T6] = IGNORE_ALL;   // no reporting of tag data Type 6
   CFG_NVMshadow[cfg_cur][MATCH_T7] = IGNORE_ALL;   // no reporting of tag data Type 7
   CFG_NVMshadow[cfg_cur][MATCH_T8] = IGNORE_ALL;   // no reporting of tag data Type 8
   CFG_NVMshadow[cfg_cur][MATCH_T9] = IGNORE_ALL;   // no reporting of tag data Type 9
   CFG_NVMshadow[cfg_cur][MATCH_TA] = IGNORE_ALL;   // no reporting of tag data Type 10 (0xA)
   CFG_NVMshadow[cfg_cur][MATCH_TB] = IGNORE_ALL;   // no reporting of tag data Type 11 (0xB)
   CFG_NVMshadow[cfg_cur][MATCH_TC] = IGNORE_ALL;   // no reporting of tag data Type 12 (0xC)
   CFG_NVMshadow[cfg_cur][MATCH_TD] = IGNORE_ALL;   // no reporting of tag data Type 13 (0xD)
   CFG_NVMshadow[cfg_cur][MATCH_TE] = IGNORE_ALL;   // no reporting of tag data Type 14 (0xE)
   CFG_NVMshadow[cfg_cur][MATCH_TF] = IGNORE_ALL;   // no reporting of tag data Type 15 (0xF)

   // "hidden" parameter default settings
   CFG_NVMshadow[cfg_cur][DBG_LEV] = 0;   // no debug print enabled
   CFG_NVMshadow[cfg_cur][MDM_EARLY] = 0;   // not presently used, retained in code for future. // no early modem start up
   CFG_NVMshadow[cfg_cur][XBEE_LOPWR] = mlpAIRPL;  // XBee modem low power mode, default to airplane mode  [old way] = mlpFULL;  // XBee modem low power mode, default to full power
//!   CFG_NVMshadow[cfg_cur][MDM_WT_SECS] = XBEE_WAIT_RESPONSE_SECS_DEF;  // not presently used, retained in code for future. // wait time in seconds for modem response to command, defaults to '45'
//!                                  // be very cautious in setting this too low
//!                                  // if it gives up too soon, and the reply eventually arrives,
//!                                  // it will result in the enqueue forever of the received
//!                                  // packet. that leak will eventually consume all the packets

#if __TAZ_DEBUG > 0  // lets us restore debug-appropriate configuration items without having to run the windows program 
   CFG_NVMshadow[cfg_cur][SECONDARY] = 1; // include secondary report
   CFG_NVMshadow[cfg_cur][DETREPINT] = 0; // detection report every five minutes
   strcpy(&CFG_NVMshadow[cfg_cur][PRPHNUM], "5204057635");   // TAZ android phone number for primary report
   CFG_NVMshadow[cfg_cur][PRPHLEN] = strlen(&CFG_NVMshadow[cfg_cur][PRPHNUM]); // length of primary number string
   strcpy(&CFG_NVMshadow[cfg_cur][SEPHNUM], "5204047475");   // TAZ iPhone number for secondary report
   CFG_NVMshadow[cfg_cur][SEPHLEN] = strlen(&CFG_NVMshadow[cfg_cur][SEPHNUM]); // length of secondary number string
   strcpy(&CFG_NVMshadow[cfg_cur][PRPHCCD], "+1");   // value for US "+1"
   CFG_NVMshadow[cfg_cur][PRCCLEN] = strlen(&CFG_NVMshadow[cfg_cur][PRPHCCD]); // length of primary number country code string
   strcpy(&CFG_NVMshadow[cfg_cur][SEPHCCD], "011");   // value for US, not using plus code
   CFG_NVMshadow[cfg_cur][SECCLEN] = strlen(&CFG_NVMshadow[cfg_cur][SEPHCCD]); // length of primary number country code string
   CFG_NVMshadow[cfg_cur][DBG_LEV] = 3;   // most debug print enabled
   CFG_NVMshadow[cfg_cur][ALLTAGS] = 1;   // in promiscuous detection report mode, pass all tag types
#endif
  

   // form new CRC over shadow copy and update CFG_NVM
   storeShadowCFG_NVM();
   
}

   

//
// check the current CRC for CFG_NVM content
// use the shadow copy for values, should be recent...
//
BOOLEAN isShadowCrcCorrect()
{
   unsigned int16 computedCRC;
   unsigned int16 retrievedCRC;
   BOOLEAN ShadowCrcGood;
   
   computedCRC = crcFast(CFG_NVMshadow, (CFG_NVM_SIZE - 2));
   retrievedCRC = (((unsigned int16)CFG_NVMshadow[cfg_cur][CRC_MSB]) << 8) | CFG_NVMshadow[cfg_cur][CRC_LSB];
   ShadowCrcGood = (computedCRC == retrievedCRC);  // do these values match?
   return ShadowCrcGood;
}




#endif  // __NVM_UTILITY_FUNCTIONS_C_


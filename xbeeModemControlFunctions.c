#ifndef MODEMCTLFXNS_XBEE
#define MODEMCTLFXNS_XBEE
//
// functions to perform odd, handy tasks for the Digi XBee 3G cellular modem (SMS modem)
//

// 
// AT commands for initial modem setup:
// AP1: use API mode for modem interaction
// IP2: protocol sends & receives SMS text messages
// SM1: modem sleep state selected by 'pin sleep'
// AM0: airplane mode is off
// ANwap.cingular: sets access point name
// DO0: device option inhibits remote management
// DO2: device option inhibits remote management and permits 2G fallback
// CN: exit command mode and apply these commands
//
// as we have discovered, not every one of those commands we identified
// need to be used in our circumstance.
//


//
// runtime configuration of XBee modem occurs here
//
// use one AT command to get into API mode, 
//   ATAP1    // API mode enabled
//
// then use API commands to complete the setup
//
// API mode list of XBee setup commands
//   ATIP2    // IP protocol is SMS
//   ATSM0    // sleep mode 0: device never sleeps (airplane mode is not slssp)
//   ATSM1    // sleep mode 1: pin sleep
//   ATDO0    // device option 0: disable remote management (costs us money for no benefit)
//
// informational query commands
//   ATDT1    // get cellular network time
//   ATDB0    // get most recent cached RSSI value
//   ATDB1    // get fresh, uncached RSSI value

// may send multiple commands in one string using comma separation.
//
// we keep a running indication of the success of any commands issues.
// the first failure encountered is fatal to modem usage.
// if that occurs, we set a global flag to indicate it.
//
// return FALSE if every command was successful
// return TRUE at the first failure
//
BOOLEAN xbeeRuntimeConfiguration()
{

   BYTE XBforcereset[] = "FR";  // will reset the XBEE, exit command mode

#if __IMPLEMENT_PIN_SLEEP
   BYTE XBsetupcmds[] = "AP0,IP2,SM1,AM0,DO0";  // 'SM1' selects pin sleep 'AP0' defers entry to API mode  AT auto-inserted
#else
   BYTE XBsetupcmds[] = "AP0,IP2,SM0,AM0,DO0";  // 'SM0' selects normal (no sleep) 'AP0' defers entry to API mode  AT auto-inserted
#endif
   BYTE XBsetupcmds2[] = "AP1";  // 'AP1' enters API mode  AT auto-inserted
   
   //BFATCR *reply;  // response from AT command function
   BOOLEAN scrBool;  // scratch BOOLEAN value
   BYTE *scrlBuf;  // pointer to allocated large buffer for scratch string construction
   BYTE *scrsBuf;  // pointer to allocated small buffer for scratch string construction
   
#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "\r\nabout to ForceReset XBEE  modem, then delay 5 seconds\r\n");
   }
#endif
   
   scrBool = xbeeCmdOkReply(XBforcereset, 5000L);  // send the command, long wait first cmd.
   if (scrBool)  // TRUE reply means error occurred. 
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "No OK resp to reset cmd: \"%s\"\r\n", XBforcereset);
      }
#endif
      return TRUE;  // configuration aborted
   }

   delay_ms(2*100);  // force reset needs 100mS time to act, giver it lots
   
   scrBool = xbeeWaitAndBlink();  // wait for association on network
   if (scrBool)  // TRUE reply means error occurred. 
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "No network association\r\n");
      }
#endif
      return TRUE;  // configuration aborted
   }
   
   scrBool = xbeeCmdOkReply(XBsetupcmds, 500);  // send the command(s), all must respond with "OK"
   if (scrBool)  // TRUE reply means error occurred. 
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "No OK resp to setup cmds: \"%s\"\r\n", XBsetupcmds);
      }
#endif
      return TRUE;  // configuration aborted
   }


   // initial setup has been performed, except for the transition to API mode.
   // while still in transparent mode, send the wakeup SMS message(s) transparently
   // need to set phone numbers explicitly.
   xbeeGetRssi();  // get RSSI at this moment. ignore success or failure
   scrsBuf = sballoc();  // allocate a small buffer in which to construct phone number strings
   scrlBuf = lballoc();  // allocate a large buffer in which to construct the wakeup string
   sprintf(scrlBuf, "SMS v%X.%X\r\nRSSI:-%2u  BV:%3.1w", vmsd, vlsd, savedRSSI, BatteryVoltage);  // wakeup string the same for both phones
   sprintf(scrsBuf, "P#%s", &CFG_NVMshadow[cfg_cur][PRPHD1]);  // primary phone number without <cr>, then AT supplied elsewhere
   scrBool = xbeeCmdOkReply(scrsBuf, 500);  // send the command, must respond with "OK". drops out of command mode
   if (scrBool)  // TRUE reply means error occurred. 
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "No OK resp to primary phone number set: \"%s\"\r\n", scrsBuf);
      }
#endif
      lbfree(scrlBuf);  // free the allocated message composition buffers
      sbfree(scrsBuf);
      return TRUE;  // configuration aborted
   }
   // send the wakeup message, <cr> terminated
   fprintf(SMSport,"%s\r",scrlBuf);  // there is no response to this message

   //   Send secondary message if enabled, and first digit of number is a real digit 1..9
   if ((CFG_NVMshadow[cfg_cur][SECONDARY] != 0) && (isdigit(CFG_NVMshadow[cfg_cur][SEPHD1])))  
   {
      sprintf(scrsBuf, "P#%s", &CFG_NVMshadow[cfg_cur][SEPHD1]);   // secondary phone number from configuration CFG_NVM, without <cr>  AT supplied elsewhere
      scrBool = xbeeCmdOkReply(scrsBuf, 500);  // send the command, must respond with "OK". drops out of command mode
      if (scrBool)  // TRUE reply means error occurred. 
      {
#if __DEBUG_XBEE
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "No OK resp to secondary phone number set: \"%s\"\r\n", scrsBuf);
         }
#endif
         lbfree(scrlBuf);  // free the allocated message composition buffers
         sbfree(scrsBuf);
         return TRUE;  // configuration aborted
      }
      // send the wakeup message, <cr> terminated
      fprintf(SMSport,"%s\r",scrlBuf);  // there is no response to this message
   }
   lbfree(scrlBuf);  // free the allocated message composition buffers
   sbfree(scrsBuf);


   // now complete the initialization by invoking API mode, closing command mode
   scrBool = xbeeCmdOkReply(XBsetupcmds2, 500);  // send the commands
   if (scrBool)  // TRUE reply means error occurred. 
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "No OK resp to 2nd setup cmds: \"%s\"\r\n", XBsetupcmds2);
      }
#endif
      return TRUE;  // configuration aborted
   }

   // if we arrive at this point, all configuration items were successful
   // return FALSE to indicate that

   return FALSE;
   
}

//
// park the wakeup code here
//
BOOLEAN phonyWakeup()
{
   BOOLEAN scrBool;  // scratch BOOLEAN value
   BYTE *scrlBuf;  // pointer to allocated large buffer for scratch string construction
   BYTE *scrsBuf;  // pointer to allocated small buffer for scratch string construction

   // initial setup is performed, except for the transition to API mode.
   // while still in transparent mode, send the wakeup SMS message(s) transparently
   // need to set phone numbers explicitly.
   xbeeGetRssi();  // get RSSI at this moment. ignore success or failure
   scrsBuf = sballoc();  // allocate a small buffer in which to construct phone number strings
   scrlBuf = lballoc();  // allocate a large buffer in which to construct the wakeup string
   sprintf(scrlBuf, "SMS v%X.%X\r\nRSSI:-%2u  BV:%3.1w", vmsd, vlsd, savedRSSI, BatteryVoltage);  // wakeup string the same for both phones
   sprintf(scrsBuf, "P#%s", &CFG_NVMshadow[cfg_cur][PRPHD1]);  // primary phone number without <cr>, then AT supplied elsewhere
   scrBool = xbeeCmdOkReply(scrsBuf, 500);  // send the command, must respond with "OK"
   if (scrBool)  // TRUE reply means error occurred. 
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "No OK resp to primary phone number set: \"%s\"\r\n", scrsBuf);
      }
#endif
      lbfree(scrlBuf);  // free the allocated message composition buffers
      sbfree(scrsBuf);
      return TRUE;  // configuration aborted
   }
   // send the wakeup message, <cr> terminated
   fprintf(SMSport,"%s\r",scrlBuf);  // there is no response to this message

   //   Send secondary message if enabled, and first digit of number is a real digit 1..9
   if ((CFG_NVMshadow[cfg_cur][SECONDARY] != 0) && (isdigit(CFG_NVMshadow[cfg_cur][SEPHD1])))  
   {
      sprintf(scrsBuf, "P#%s", &CFG_NVMshadow[cfg_cur][SEPHD1]);   // secondary phone number from configuration CFG_NVM, without <cr>  AT supplied elsewhere
      scrBool = xbeeCmdOkReply(scrsBuf, 500);  // send the command, must respond with "OK"
      if (scrBool)  // TRUE reply means error occurred. 
      {
#if __DEBUG_XBEE
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "No OK resp to secondary phone number set: \"%s\"\r\n", scrsBuf);
         }
#endif
         lbfree(scrlBuf);  // free the allocated message composition buffers
         sbfree(scrsBuf);
         return TRUE;  // configuration aborted
      }
      // send the wakeup message, <cr> terminated
      fprintf(SMSport,"%s\r",scrlBuf);  // there is no response to this message
   }
   lbfree(scrlBuf);  // free the allocated message composition buffers
   sbfree(scrsBuf);

   
}


//
// send a command to XBee modem, expect an "OK" reply
//
// called with a string holding only the AT command(s).
// the prefix "AT", and the terminating <cr> are supplied here.
// second parameter is the time in milliseconds to pause between the issuance
// of the command and the reply processing. default 500 mS
//
// return "TRUE" if reply is NOT OK, else "FALSE" for good outcome.
// since the XBEE is able to process multiple AT commands in a single input string,
// it will return as many response strings. 
// we don't use commands that generate response values, so "OK" is the
// only response a command should elicit.
// we parse all the returned data into strings. 
// if each one of those strings is "OK", return "FALSE" for good outcome.
//
BOOLEAN xbeeCmdOkReply(BYTE *cstr, int16 msTimeToWait = 500)
{
   BOOLEAN scrBool = FALSE;  // scratch BOOLEAN varialbe
   int i,j;

#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      printf(dpo, ">>>>>xbeeCmdOkReply(\"%s\", %Ld) %s\r\n", cstr, msTimeToWait, stringTheDateTimeUptime());
   }
#endif
   
   // enter XBee modem command mode
   
   // get into command mode. one retry if fails.
   // this is pretty reliable so just go for it.

   scrBool = xbeeCommandMode();  // TRUE reply means error
   if (scrBool)  // TRUE reply means error, one retry is attempted
   {
      delay_ms(500);  // wait some amount of time
      scrBool = xbeeCommandMode();  // try again to get into command mode.
      if (scrBool)  // error again?
      {
         return TRUE;  // too bad
      }
   }
   
   // do these things  before issuance of command string

   stringsParseSetInitialConditions(XBEEstrings);
   fillWords((unsigned int16 *)xbeeStringAssembly, 8, 0x552A);
   
   // send this command
   fprintf(SMSport,"AT%s,CN\r",cstr);  // form the command for transmission by prefixing the input with "AT", suffixing with CN and <cr> and send it
   
   // I think we should wait at this point
   if(msTimeToWait > 0)
   {
      delay_ms(msTimeToWait);
   }
   
   drainSmsBufAndProcess();  // fetch characters and parse into strings
   if(XBEEstrings.rstrPtrCt == 0)  // no response received? bail out
      {
#if __DEBUG_XBEE
         if (dbpEnabled(LEV3))
            {
               printf(dpo, "\r\nXBEE NO REPLY TO \"%s\" CMD %s\r\n", cstr, stringTheDateTimeUptime());
           }
#endif
         return TRUE;   // return failure reply value
      }

#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\nXBEE \"%s\" RESPONSE: %s", cstr, stringTheDateTimeUptime());
         dumpToHost((BYTE *)XBEEstrings.sab, XBEEstrings.sabCt);
//!         printf(dpo, "\r\nXBEE RESPONSE STRINGS[%d]:", XBEEstrings.rstrPtrCt);
//!         if(XBEEstrings.rstrPtrCt)
//!            {
//!               for (int i = 0 ; i < XBEEstrings.rstrPtrCt ; i++)
//!                  {
//!                     printf(dpo, "\r\n|%s|", XBEEstrings.rstrPtr[i]);
//!                  }
//!            }
//!         printf(dpo, "\r\n");
      }
#endif

   // did the modem respond with "OK" in every one of the strings? bail if not
   
   j = 0;  // count mismatches   
   for (i = 0 ; i < XBEEstrings.rstrPtrCt ; i++)
      {
         if (strcmpb(XBEEstrings.rstrPtr[i], OKrsp))  // does not match if return value is non-zero
         {
#if __DEBUG_XBEE
            if (dbpEnabled(LEV3))
            {
               printf(dpo, "NOT OK RESPONSE |%s| IN RSP %d OF %d\r\n", XBEEstrings.rstrPtr[i], i, XBEEstrings.rstrPtrCt);
            }
#endif
            j++;  // count this mismatch
         }
      }
   
   if(j != 0)  // modem did not respond with "OK" in every response, bail
      {
#if __DEBUG_XBEE
         if (dbpEnabled(LEV3))
            {
               printf(dpo, "\r\nXBEE \"%s\" REPLY NOT \"OK\" %s\r\n", cstr, stringTheDateTimeUptime());
            }
#endif
         return TRUE;   
      }

   return FALSE;  // actually, this reports success
}





//
// send an AT command to XBee modem via command mode
//
// XXXcalled with a string holding the command exactly as is to be sent, excluding <cr> which is supplied here
// called with a string holding only the AT command(s).
// the prefix "AT", and the terminating <cr> are supplied here.
// second parameter is the time in milliseconds to pause between the issuance
// of the command and the reply processing. default 500 mS
//
// return the responded string in a pointer to a response structure. 
//
// terminate command mode if the third parameter is TRUE
//
BFATCR *xbeeSendAtCommand(BYTE *cstr, int16 msTimeToWait = 500)
{
   int16 i;
   BFATCR *RetVal = 0;  // return value pointer stored here
   BOOLEAN scrBool = FALSE;  // scratch BOOLEAN varialbe

#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      printf(dpo, ">>>>>xbeeSendAtCommand(\"%s\") %s\r\n", cstr, stringTheDateTimeUptime());
   }
#endif

   // fetch small buffer to use as 'BFATCR' response
   RetVal = (BFATCR *)sballoc();
   RetVal->respbl = 0;  // set length zero for no response

   // enter XBee modem command mode
   
   // get into command mode. one retry if fails.
   // this is pretty reliable so just go for it.

   scrBool = xbeeCommandMode();  // TRUE reply means error
   if (scrBool)  // TRUE reply means error, one retry is attempted
   {
      delay_ms(500);  // wait some amount of time
      scrBool = xbeeCommandMode();  // try again to get into command mode.
      if (scrBool)  // error again?
      {
         //XBeeApiIsActive = saveApiFlag;  // restore original value
         return RetVal;  // too bad
      }
   }
   
   // do these things  before issuance of command string

   stringsParseSetInitialConditions(XBEEstrings);
   fillWords((unsigned int16 *)xbeeStringAssembly, 8, 0x552A);
   
   // send this command
   fprintf(SMSport,"AT%s,CN\r",cstr);  // form the command for transmission by prefixing the input with "AT", suffixing with CN and <cr> and send it
   
   // I think we should wait at this point
   if(msTimeToWait > 0)
   {
      delay_ms(msTimeToWait);
   }
   
   
   drainSmsBufAndProcess();  // fetch characters and parse into strings
   if(XBEEstrings.rstrPtrCt == 0)  // no response received? bail out
      {
#if __DEBUG_XBEE
         if (dbpEnabled(LEV3))
            {
               printf(dpo, "\r\nXBEE NO REPLY TO \"%s\" CMD %s\r\n", cstr, stringTheDateTimeUptime());
           }
#endif
         return RetVal;   
      }
#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\nXBEE \"%s\" RESPONSE: %s", cstr, stringTheDateTimeUptime());
         dumpToHost((BYTE *)XBEEstrings.sab, XBEEstrings.sabCt);
//!         printf(dpo, "\r\nXBEE RESPONSE STRINGS[%d]:", XBEEstrings.rstrPtrCt);
//!         if(XBEEstrings.rstrPtrCt)
//!            {
//!               for (int i = 0 ; i < XBEEstrings.rstrPtrCt ; i++)
//!                  {
//!                     printf(dpo, "\r\n|%s|", XBEEstrings.rstrPtr[i]);
//!                  }
//!            }
//!         printf(dpo, "\r\n");
      }
#endif

   // if we make it to here, the command has responded with at least one string
   // allocate a buffer big enough to hold it, and return the string within
   
   i = strlenb(XBEEstrings.rstrPtr[0]);  // how long is the first string?
   if (i > 30)  // too big for small buffer, replace with large buffer
   {
      sbfree((void *)RetVal);
      RetVal = (BFATCR *)lballoc();
   }
   RetVal->respbl = i;  // set length of response string
   strcpyb(RetVal->respb, XBEEstrings.rstrPtr[0]);  // copy response string for return

   return RetVal;
   
}



// enter command mode.
//
// if already in command mode, do nothing and return success
//
// otherwise read and send any pending sms data to the packets parse function.
// mark in command mode (unless it fails)
// (?)set the input diversion flag. maybe don't need this...
//
// return 'FALSE' if successful, 'TRUE' if no response from modem
//
BOOLEAN xbeeCommandMode()
{
   
   // I no longer thing trashing input is correct here.
   // instead, I think any waiting characters are part of an api received message.
   // read them and send to parsing process
   drainSmsBufAndParse();

   // do these things  before issuance of command string
   stringsParseSetInitialConditions(XBEEstrings);
   fillWords((unsigned int16 *)xbeeStringAssembly, 8, 0x552A);
//!#if !__USING_SMSISR  // unnecessary if the interrupt stuff works
   //drainSmsBufAndTrash();  // empty any characters pending from modem  <-- I think this remains appropriate
//!#endif

   delay_ms(1000);  // quiet on serial line for one second
   fprintf(SMSport,"+++");  // get modem into serial command mode
   delay_ms(1000);  // quiet on serial line for another second

//!#if !__USING_SMSISR  // unnecessary if the interrupt stuff works
      drainSmsBufAndProcess(750L);  // fetch characters and parse into strings
//!#else  // using interrupts
//!   //delay_ms(750);  // kill some time and hope characters arrive
//!   loopingPacketsProcess(750L);  // just kill time waiting for response
//!#endif

//!      drainSmsBufAndProcess();  // fetch characters and parse into strings

   if(XBEEstrings.rstrPtrCt == 0)  // no response received? bail out
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\nXBEE NO REPLY TO \"+++\" CMD %s\r\n", stringTheDateTimeUptime());
      }
#endif
      return TRUE;  
   }
   XbeeInCommandMode = TRUE;  // set this mode
//!#if __DEBUG_XBEE
//!   if (dbpEnabled(LEV3))
//!   {
//!      printf(dpo, "\r\n########## XBEE COMMAND MODE SET TO TRUE %s##########\r\n", stringTheDateTimeUptime());
//!   }
//!#endif
   return FALSE;  // means no error, success
}



// exit command mode.
//
// if not in command mode, do nothing and return success
//
// otherwise send "CN" command to modem.
// ignore any reply data, it will get sent to packet parse and ignored
// mark not in command mode
// (?)clear the input diversion flag. maybe don't need this...
//
// return 'FALSE' if successful, 'TRUE' if no response from modem
//
BOOLEAN xbeeExitCommandMode()
{
   // just send the command without any other action, then ignore the result
   fprintf(SMSport, "ATCN\r");
   
   XbeeInCommandMode = FALSE;  // clear this mode
//!#if __DEBUG_XBEE
//!   if (dbpEnabled(LEV3))
//!   {
//!      printf(dpo, "\r\n########## XBEE COMMAND MODE SET TO FALSE (1) %s##########\r\n", stringTheDateTimeUptime());
//!   }
//!#endif
   return FALSE;  // means no error, success
}




#endif

#ifndef HLTHREP
#define HLTHREP
//
// functions to perform odd, handy tasks
// related to producing the health report
//


void processHealthReport()
{
   BYTE *hdbp;  // health report string data buffer
   BYTE CrlfStr[] = { "\r\n" };
   signed int newRecordCount;
   int jj ;
   
   // if the battery voltage is too low,
   // transmission of this report cannot occur.
   // short-circuit the process, do not fetch the tag count from the SUR,
   // just exit now. no allocatable storage obtained thus far.
   if (!BatteryVoltageOK)
   {
#if __DEBUG_MISC
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\n*** processHealthReport() -- early exit battery voltage too low\r\n");
      }
#endif
      return;
   }
   
   // if the XBEE cellular modem has been disabled,
   // transmission of this report cannot occur.
   // short-circuit the process, do not fetch the tag count from the SUR,
   // just exit now. no allocatable storage obtained thus far.
   if (!isXbeeOkToUse())
   {
#if __DEBUG_MISC
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\n*** processHealthReport() -- early exit cellular modem disabled\r\n");
      }
#endif
      return;
   }

   // allocate message buffer, initialize to null string
   hdbp = lballoc();  // allocate health report string data buffer
   hdbp[0] = 0;  // health report string data buffer
   
   // query SUR for record count, version string returned in 'hdbp'
   newRecordCount = getSurVersAndRc(hdbp);
   if (newRecordCount < 0)   // error fetching version and record count
   {
      //                         12345678901234567890
      strcpy(DisplayLine2String,"FAIL HEALTH CHECK   ");
      //sprintf(DisplayLine2String,"FAILHCHK");
      strcpy(hdbp, "Health Check Failed...");
   }
   else
   {
      //                         12345678901234567890
      strcpy(DisplayLine2String,"PASS HEALTH CHECK   ");
      //sprintf(DisplayLine2String,"PASSHCHK");
   }
   strcat((char *)hdbp, (char *)CrlfStr);


   // append version, date, time information to response string   
   jj = strlen((char *)hdbp) ;
   sprintf(hdbp+jj, "SUR-SMS v%X.%X %s\r\n", vmsd, vlsd, stringTheDateAndTime());
//!   prefixHrepResponse(hdbp+jj);  // fetch version, date, time strng
//!   strcat((char *)hdbp, (char *)CrlfStr);


   appStartUpModem();
#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "\r\n*** appStartUpModem() called from processHealthReport()\r\n");
   }
#endif

   xbeeApiSendSMSmessage(&CFG_NVMshadow[cfg_cur][PRPHD1], hdbp);   // primary phone number from configuration CFG_NVM
   if ((CFG_NVMshadow[cfg_cur][SECONDARY] != 0) && (isdigit(CFG_NVMshadow[cfg_cur][SEPHD1])))  //   Send secondary message if enabled, and first digit of number is a real digit 1..9
   {
      xbeeApiSendSMSmessage(&CFG_NVMshadow[cfg_cur][SEPHD1], hdbp);   // secondary phone number from configuration CFG_NVM
   }
   
   lbfree(hdbp);  // return the buffer to the pool.  this can only be a large buffer
   appShutDownModem();  // finished with modem
   BigLoopMaintenance();  // maintain display and other things
}


#endif

#ifndef __UTILITY_FUNCTIONS_C_
#define __UTILITY_FUNCTIONS_C_
//
// short functions to perform odd, handy tasks
//


//
// notify of fatal error, reset processor
//
void esad(char *messageToDisplay=0)
{

   if (messageToDisplay)
   {
      printf(dpo, "\r\nFATAL ERROR: %s\r\n", messageToDisplay);
   }
   
   printf(dpo, "%s\r\n", fortyBucks);
   printf(dpo, "%s\r\n", fortyBucksBoom);
   printf(dpo, "$$$$$                              $$$$$\r\n");
   printf(dpo, "$$$$$  $$$$    $$$    $$$   $   $  $$$$$\r\n");
   printf(dpo, "$$$$$  $   $  $   $  $   $  $$ $$  $$$$$\r\n");
   printf(dpo, "$$$$$  $   $  $   $  $   $  $ $ $  $$$$$\r\n");
   printf(dpo, "$$$$$  $$$$   $   $  $   $  $   $  $$$$$\r\n");
   printf(dpo, "$$$$$  $   $  $   $  $   $  $   $  $$$$$\r\n");
   printf(dpo, "$$$$$  $   $  $   $  $   $  $   $  $$$$$\r\n");
   printf(dpo, "$$$$$  $$$$    $$$    $$$   $   $  $$$$$\r\n");
   printf(dpo, "$$$$$                              $$$$$\r\n");
   printf(dpo, "%s\r\n", fortyBucks);
   printf(dpo, "%s\r\n", fortyBucks);

   reset_cpu();   // reset program rather than simply returning

}




#INT_ADDRERR
void ADDRERR_isr(void)
{
   unsigned long trapaddr;

   #asm
   mov w15, w0
   sub #38, w0
   mov [w0++], w1
   mov w1, trapaddr
   mov [w0], w1
   and #0x7f, w1
   mov w1, trapaddr+2
   #endasm

   printf(dpo, "\r\nADDRERR TRAP ADDRESS: 0x%08X\r\n", trapaddr );
   //fprintf(SURport, "\r\nADDRERR TRAP ADDRESS: 0x%08X\r\n", trapaddr );
   esad();  // squawk and die
} 

//
// display character or hex value on debug port
//
void dpoecho(BYTE xch)
{
   if ((xch > 0x19) && (xch < 0x7F))  // character is printable ASCII
   {
      printf(dpo, "\\%d:%c", BTLEactiveState, xch);
   }
   else  // character is not printable ASCII
   {
      printf(dpo, "\\%d:x%x", BTLEactiveState, xch);
   }
}


//
// print the information that was formerly output at startup
//
void itsAlive()
{

      printf(dpo, "\r\n\r\n########################################################################\r\n");
#if __USING_SMSISR  // different definitions used for ISR and runtime buffering
      printf(dpo,"\r\nSURsms Starting using interrupt-driven XBEE data receive\r\n\r\n");
#else
      printf(dpo,"\r\nSURsms Starting using polled (runtime buffered) XBEE data receive\r\n\r\n");
#endif
      printf(dpo, "COMPILED BY: PICC VERSION %s ON: ", getenv("VERSION_STRING"));
      printf(dpo, __DATE__);
      printf(dpo, " AT: ");
      printf(dpo, __TIME__);
      printf(dpo, "\r\n");
      if (InferPowerMaintained)
      {
         printf(dpo, "\r\nWARM START\r\n");
      }
      else
      {
         printf(dpo, "\r\nCOLD START\r\n");
      }
      
      // interpret the saved restart reason
      switch(resetCause)
      {
         case RESTART_POWER_UP:
            printf(dpo, "RESTART_POWER_UP %Lu[0x%4LX]\r\n", resetCause, resetCause);
            break;
            
         case RESTART_BROWNOUT:
            printf(dpo, "RESTART_BROWNOUT %Lu[0x%4LX]\r\n", resetCause, resetCause);
            break;
            
         case RESTART_WATCHDOG:
            printf(dpo, "RESTART_WATCHDOG %Lu[0x%4LX]\r\n", resetCause, resetCause);
            break;
            
         case RESTART_SOFTWARE:
            printf(dpo, "RESTART_SOFTWARE %Lu[0x%4LX]\r\n", resetCause, resetCause);
            break;
            
         case RESTART_MCLR:
            printf(dpo, "RESTART_MCLR %Lu[0x%4LX]\r\n", resetCause, resetCause);
            break;
            
         case RESTART_ILLEGAL_OP:
            printf(dpo, "RESTART_ILLEGAL_OP %Lu[0x%4LX]\r\n", resetCause, resetCause);
            break;
            
         case RESTART_TRAP_CONFLICT:
            printf(dpo, "RESTART_TRAP_CONFLICT %Lu[0x%4LX]\r\n", resetCause, resetCause);
            break;
            
         default:
            printf(dpo, "UNKNOWN RESTART %Lu[0x%4LX]\r\n", resetCause, resetCause);
            break;
      }
      
      printf(dpo, "STACK SIZE:%Lu\r\n", getenv("STACK"));
//!      printf(dpo, "API PACKET BUFFER SIZE:%u\r\n", PACKET_BUFFER_SIZE);
//!      printf(dpo, "API PACKET BUFFER SIZE NO SMS:%u\r\n", PACKET_BUFFER_SIZE_NOSMS);
//!      printf(dpo, "LARGE BUFFER SIZE:%u\r\n", LARGE_BUFFER_SIZE);
//!      printf(dpo, "SMALL BUFFER SIZE:%u\r\n", SMALL_BUFFER_SIZE);
      printf(dpo, "\r\n########################################################################\r\n\r\n");

}



//
// fill a byte array @ 'ap' with numeric value 'fv', 'ct' times
// if 'termAsString' is true, supply a string-terminating NULL after the filled range
//
void fillBytes(BYTE *ap, unsigned int16 ct, BYTE fv, BOOLEAN termAsString)
{
   for (int i = 0 ; i < ct ; i++)  // loop won't run if 'ct' is zero
   {
      ap[i] = fv;
   }
   if (termAsString)
   {
      ap[ct] = 0;
   }
}


//
// fill a word array @ 'ap' with numeric value 'fv', 'ct' times
//
void fillWords(unsigned int16 *ap, unsigned int16 ct, unsigned int16 fv)
{
   for (int i = 0 ; i < ct ; i++)  // loop won't run if 'ct' is zero
   {
      ap[i] = fv;
   }
}



// function to round an integer up to the next even multiple of 'factor'
unsigned int16 round_up(unsigned int16 num, unsigned int16 factor)
{
   return num + factor - 1 - (num - 1) % factor;
}


int bcd2hex(int x)
{
   int y; 
   y = 10*(x>>4); 
   y = y + (x & 0x0f); 
   return (y);
}


int hex2bcd(int x)
{
   int y; 
   y = (x / 10) << 4; 
   y = y | (x % 10); 
   return (y);
}


//
// function to provide a small integer which increments from 1 to 99,
// then back to 1. used for detection report sequence number.
//
BYTE nextDetRepSeq()
{
   ++DetectReportSeqNumber;
   if (DetectReportSeqNumber > 99)
      DetectReportSeqNumber = 1;
   return DetectReportSeqNumber;
}


//
// function used by configuration process to accept arbitrary ascii strings of
// decimal digits and convert to integer values.
//
// also may accept '?' for use in current value display.
//
// function return:
//    integer value in range 0..32767 means successful conversion of one to five digits
//    -1 integer conversion fails
//    (since we use signed numbers, failure can occur if value exceeds 32767)
//    -2 input was a question mark ('?')
//
signed int16 convertForConfig(int8 fieldSize, BYTE *argPointer)
{
   BYTE kraker[16];  // string in which to collect ASCII for integer conversion
   signed int16 RetVal;  // value for return
   int i;
   int16 ch;
   BOOLEAN stringErr = FALSE;

   if (fieldSize > 5)  // can't handle this
   {
      return -1;  // conversion error
   }

   //if (strlen(argPointer) < fieldSize)  // argument string is too short
   if (strlenb(argPointer) < fieldSize)  // argument string is too short  // custom function
   {
      return -1;  // conversion error
   }
   
   for (i = 0 ; i < fieldSize ; i++)
   {
      ch = *argPointer++;  // get next field character
      if ((i == 0) && (ch == '?'))  // special case, first char is question mark
      {
      return -2;  // return signals question mark
      }
      
      if (isdigit(ch))  // looking for ASCII decimal digit
      {
         kraker[i] = ch;  // save the character
      }
      else
      {
         stringErr = TRUE;  // flag error, keep looping for entire field width 
      }
   }
   
   if (stringErr)
   {
      return -1;  // conversion error
   }
   
   kraker[fieldSize] = '\0';  // terminate the string
   RetVal = atol((char *)kraker);  // crack the value
   //if ((RetVal < 0) || (RetVal > 32767))
   if (RetVal < 0)  // any negative result is an error
   {
      return -1;  // conversion error
   }
   else 
   {
      return RetVal;  // success
   }
}


//
// function used by configuration process to accept arbitrary ascii strings of
// hexadecimal digits and convert to integer values.
//
// also may accept '?' for use in current value display.
//
// function return:
//    integer value in range 0..32767 means successful conversion of one to four digits
//    -1 integer conversion fails
//    (since we use signed numbers, failure can occur if value exceeds 32767)
//    -2 input was a question mark ('?')
//
signed int16 convertHexForConfig(int8 fieldSize, BYTE *argPointer)
{
   BYTE kraker[16];  // string in which to collect ASCII for integer conversion
   signed int16 RetVal;  // value for return
   int i;
   int16 ch;
   BOOLEAN stringErr = FALSE;

   if (fieldSize > 4)  // can't handle this
   {
      return -1;  // conversion error
   }

   //if (strlen(argPointer) < fieldSize)  // argument string is too short
   if (strlenb(argPointer) < fieldSize)  // argument string is too short  // custom function
   {
      return -1;  // conversion error
   }
   
   // prefix the copied string with '0x' for hex conversion
   kraker[0] = '0';
   kraker[1] = 'x';
   
   for (i = 0 ; i < fieldSize ; i++)
   {
      ch = *argPointer++;  // get next field character
      if ((i == 0) && (ch == '?'))  // special case, first char is question mark
      {
      return -2;  // return signals question mark
      }
      
      if (isalnum(ch))  // looking for ASCII hex digit
      {
         kraker[i+2] = ch;  // save the character
      }
      else
      {
         stringErr = TRUE;  // flag error, keep looping for entire field width 
      }
   }
   
   if (stringErr)
   {
      return -1;  // conversion error
   }
   
   kraker[fieldSize+2] = '\0';  // terminate the string
   RetVal = atol((char *)kraker);  // crack the value
   //if ((RetVal < 0) || (RetVal > 32767))
   if (RetVal < 0)  // any negative is an error
   {
      return -1;  // conversion error
   }
   else 
   {
      return RetVal;  // success
   }
}



// utility function to check for desired level of debug print
// returns TRUE if print OK. (set level is >= match level)
// ascending matchlevel is increasingly restrictive.

// KLUDGE: always true if debugging second prototype board

BOOLEAN dbpEnabled(int matchLevel)
{
   if ((CFG_NVMshadow[cfg_cur][DBG_LEV] == 0xff) // uninitialized in earlier versions
      || (CFG_NVMshadow[cfg_cur][DBG_LEV] == 0)  // set zero means no debug printing
      || (matchLevel < 1))  // not really a valid input
      {
         return FALSE;
      }
   else
      {
         return CFG_NVMshadow[cfg_cur][DBG_LEV] >= matchLevel;
      }
}


//
// function to temporarily change the debug level
//
// return the current value, set the value passed as parameter
dbpr_code exchangeDebugPrintLevel(dbpr_code newDbprCode)
{
   dbpr_code currentCode = (dbpr_code)CFG_NVMshadow[cfg_cur][DBG_LEV];
   CFG_NVMshadow[cfg_cur][DBG_LEV] = newDbprCode;
   return currentCode;
}


//
// functions to form phone number strings combining the country code prefix and phone number
//


BYTE *formCompletePrimaryNumber()
{
   phoneNumberFormationBuffer[0] = '\0';  // null string for destination, so we can use concatenation everywhere
   
   if (CFG_NVMshadow[cfg_cur][PRCCLEN] > 0)  // country code string is not empty and not disabled
   {
      // a counry code has been supplied, combine with the number string in scratch buffer
      strcatb(phoneNumberFormationBuffer, &CFG_NVMshadow[cfg_cur][PRPHCCD]);  // add the country code string
   }

   strcatb(phoneNumberFormationBuffer, &CFG_NVMshadow[cfg_cur][PRPHNUM]);  // add the phone number string
   return phoneNumberFormationBuffer;
}

// 0xff

BYTE *formCompleteSecondaryNumber()
{
   phoneNumberFormationBuffer[0] = '\0';  // null string for destination, so we can use concatenation everywhere
   
   if (CFG_NVMshadow[cfg_cur][SECCLEN] > 0)  // country code string is not empty and not disabled
   {
      // a counry code has been supplied, combine with the number string in scratch buffer
     strcatb(phoneNumberFormationBuffer, &CFG_NVMshadow[cfg_cur][SEPHCCD]);  // add the country code string
   }

   strcatb(phoneNumberFormationBuffer, &CFG_NVMshadow[cfg_cur][SEPHNUM]);  // add the phone number string
   return phoneNumberFormationBuffer;
}




#if __DEBUG_XBEE
//
// small function to return string corresponding to selected power mode for XBEE modem.
//
// valid configuration values are 0, 1, 2. others return 'unknown'.
//
char *stringModemPower()
{
   static char powerLabel[16];
   
   switch(CFG_NVMshadow[cfg_cur][XBEE_LOPWR])
   {
      case 0:
         strcpy(powerLabel, "FULL POWER");
         break;
      
      case 1:
         strcpy(powerLabel, "AIRPLANE");
         break;
      
#if __IMPLEMENT_PIN_SLEEP
      case 2:
         strcpy(powerLabel, "PIN-SLEEP");
         break;
#endif

      default:
         strcpy(powerLabel, "<UNKNOWN>");
         break;
   }

   return powerLabel;
}
#endif



// format the uptime portion of the date & time information.
// no leading space, no CRLF
void *stringTheUptime(U32 uptimeNow=0xFFFFFFFF)  // default input means read the time now, else use parameter for conversion
{
   static BYTE hiddenUptimeArray[16];

   U32 uhours;
   U32 umins;
   U32 usecs;
   U32 umsecs;
   U32 uscratch;

   if (uptimeNow == 0xFFFFFFFF)
   {
      uptimeNow = uptimeMilliseconds();
   }

   umsecs = uptimeNow % 1000UL;
   uscratch = uptimeNow / 1000UL;
   usecs = uscratch % 60UL;
   uscratch /= 60UL;
   umins = uscratch % 60UL;
   uhours = uscratch / 60UL;

   sprintf(hiddenUptimeArray, "%Lu:%02Lu:%02Lu.%03Lu", uhours, umins, usecs, umsecs);

   return hiddenUptimeArray;
}


// small functions to form date/time or date/time/uptime strings from the RTCC.
// no leading spaces, no CRLF
//
BYTE *stringTheDateAndTime()
{
   static BYTE hiddenDateTimeArray[24];

   sprintf (hiddenDateTimeArray, "%02x/%02x/%02x %02x:%02x:%02x", MonthBCD, DayBCD, YearBCD, HourBCD, MinuteBCD, SecondBCD);
   return hiddenDateTimeArray;
}


// let this one have a leading space, backwards compatible with the old way
void *stringTheDateTimeUptime()
{
   static BYTE hiddenDateTimeUptimeArray[48];

   sprintf(hiddenDateTimeUptimeArray, " %s %s", stringTheDateAndTime(), stringTheUptime());
   return hiddenDateTimeUptimeArray;
}


//
// during interactive configuration,
// store new date and time values into the PIC real time clock
// and the CM RTC device.
//
// values are obtained in real-time interaction over the host port,
// buffered, and passed to us as a 12-character string of ascii decimal digits.
//
// the digit pairs are interpreted as BCD values, and saved in the conventional
// storage locations.
//
// since this user input can be from direct keyboard entry, and there is no
// edit possible for invalid characters or values, do some basic checks
// and don't perform the update if anything is wrong.
//
void set_time(BYTE *setString)
{
   HourBCD = ((setString[0] & 0x0F) << 4) | (setString[1] & 0x0F);
   MinuteBCD = ((setString[2] & 0x0F) << 4) | (setString[3] & 0x0F);
   SecondBCD = ((setString[4] & 0x0F) << 4) | (setString[5] & 0x0F);
   MonthBCD = ((setString[6] & 0x0F) << 4) | (setString[7] & 0x0F);
   DayBCD = ((setString[8] & 0x0F) << 4) | (setString[9] & 0x0F);
   YearBCD = ((setString[10] & 0x0F) << 4) | (setString[11] & 0x0F);
   xx_RTC_Write();  // set the updated time into the PIC24 RTC
      printf(hpo,"%02x:%02x:%02x %02x/%02x/%02x\r\n",HourBCD,MinuteBCD,SecondBCD,MonthBCD,DayBCD,YearBCD);
}

//
// print current time and date as found in the BCD-formatted cells
//
void tell_time()
{
   printf(dpo,"%02x:%02x:%02x %02x/%02x/%02x\r\n",HourBCD,MinuteBCD,SecondBCD,MonthBCD,DayBCD,YearBCD);
}




//
// display updates which may be called from anywhere
//

void maintainLCD(BOOLEAN FormTodTopLine=TRUE)
{
   
   // recompute time of day, indicators in LCD image top line buffer
   // always update the top line.
   // battery voltage is an analog input.
   // the expression used to convert the 12-bit ADC reading to a voltage value is:
   // 2 * <adc> / 4096 * 3.3
   // we can do this using floating point computations, but that adds a lot of code
   // which has no other use in this app.
   // what we do instead is keep everything as integers and multiply it all by 100.
   // that gives us a number which we can format as integer and then use character-by-character.
   // we can streamline the computation by rearranging the terms.
   // 2 * 3.3 * 100 * <adc> / 4096 --> 660 * <adc> /4096
   // then we can eliminate the division by right-shifting the result by 12 bits.
   // the computation may be further simplified if we precompute 66/4096 and invert it.
   // that yields 62, and the entire voltage computation becomes <adc>/62. 
   // 16-bit integers are the largest fields we need to use.
   // that result is voltage*10, so we need to accomodate that for display.
   // the results of that computation will always be less than 700, and will be
   // expressible in three decimal digits.
   
   if (FormTodTopLine)
   {
      //unsigned int8 BatteryVoltage;
      
      fillBytes(DisplayLine1String, 20, ' ', TRUE);
      DisplayLine1String[0] = (RunningNow) ? LcdActivityRun : LcdActivityIdle;  // get running indicator
      DisplayLine1String[1] = (XbeeModemIsActive) ? LcdModemActive : LcdModemIdle;  // get modem active indicator
      
      DisplayLine1String[2] = ((HourBCD / 16) & 0xF) + '0';
      DisplayLine1String[3] = (HourBCD % 16) + '0'; 
      DisplayLine1String[4] = ':';
      DisplayLine1String[5] = ((MinuteBCD / 16) & 0xF) + '0';
      DisplayLine1String[6] = (MinuteBCD % 16) + '0'; 
      DisplayLine1String[7] = ':';
      DisplayLine1String[8] = ((SecondBCD / 16) & 0xF) + '0';
      DisplayLine1String[9] = (SecondBCD % 16) + '0';
      
      if (!BatteryVoltageOK && FlagHalfHz)  // flash at 0.5 Hz rate
      {
         DisplayLine1String[11] = '*';
         DisplayLine1String[12] = 'L';
         DisplayLine1String[13] = 'O';
         DisplayLine1String[14] = 'W';
         DisplayLine1String[15] = '*';
      }
     
      DisplayLine1String[16] = (BatteryVoltage / 10) + '0';
      DisplayLine1String[17] = '.';
      DisplayLine1String[18] = (BatteryVoltage % 10) + '0';
      DisplayLine1String[19] = 'V';
   }

   // update display lines which have changed
   
   // line 1
   if (strcmpb(SavedDisplayLine1String, DisplayLine1String) != 0)
   {
      lcd_gotoxy(1,1);  // column 1, line 1
      for (int i = 0; i < 20; i++)
      {
         lcd_putc(DisplayLine1String[i]);  // write to LCD
         SavedDisplayLine1String[i] = DisplayLine1String[i];  // save modified line contents for future comparisons
      }
   }
   
   // line 2
   if (strcmpb(SavedDisplayLine2String, DisplayLine2String) != 0)
   {
      lcd_gotoxy(1,2);  // column 1, line 2
      for (int i = 0; i < 20; i++)
      {
         lcd_putc(DisplayLine2String[i]);  // write to LCD
         SavedDisplayLine2String[i] = DisplayLine2String[i];  // save modified line contents for future comparisons
      }
   }
   
   // line 3
   if (strcmpb(SavedDisplayLine3String, DisplayLine3String) != 0)
   {
      lcd_gotoxy(1,3);  // column 1, line 3
      for (int i = 0; i < 20; i++)
      {
         lcd_putc(DisplayLine3String[i]);  // write to LCD
         SavedDisplayLine3String[i] = DisplayLine3String[i];  // save modified line contents for future comparisons
      }
   }
   
   // line 4
   if (strcmpb(SavedDisplayLine4String, DisplayLine4String) != 0)
   {
      lcd_gotoxy(1,4);  // column 1, line 1
      for (int i = 0; i < 20; i++)
      {
         lcd_putc(DisplayLine4String[i]);  // write to LCD
         SavedDisplayLine4String[i] = DisplayLine4String[i];  // save modified line contents for future comparisons
      }
   }
   
}


void maintainDateTimeValues()
{
   // fetch the current date/time values from PIC RTC into the common structure 'datetime'.
   // compute and set all the individual BCD values.
   rtc_read(&datetime);  // set the running date & time structure
   //dateTimeNeedsUpdate = FALSE;  // show we did this <-- done by the caller
   SecondBin = datetime.tm_sec;
   SecondBCD = hex2bcd(SecondBin);  // compute the separate values based on structure just obtained
   MinuteBin = datetime.tm_min;
   MinuteBCD = hex2bcd(MinuteBin);
   HourBin = datetime.tm_hour;
   HourBCD = hex2bcd(HourBin);
   DayBCD = hex2bcd(datetime.tm_mday);
   MonthBCD = hex2bcd(datetime.tm_mon);
   YearBCD = hex2bcd(datetime.tm_year);
   // check for hour and minute and second zero crossings
   if (LastHourBin != HourBin )  // hour value changed this time
   {
      LastHourBin = HourBin;  // save current value
      if (HourBin == 0)  // changed to zero
      {
         HourZeroCrossing = TRUE;  // set flag to reflect the condition: new day
      }
   }
   
   if (LastMinuteBin != MinuteBin )  // minute value changed this time
   {
      LastMinuteBin = MinuteBin;  // save current value
      if (MinuteBin == 0)  // changed to zero
      {
         MinuteZeroCrossing = TRUE;  // set flag to reflect the condition: new hour
      }
   }
   
   if (LastSecondBin != SecondBin )  // second value changed this time
   {
      LastSecondBin = SecondBin;  // save current value
      if (SecondBin == 0)  // changed to zero
      {
         SecondZeroCrossing = TRUE;  // set flag to reflect the condition: new minute
         MinuteWork();  // what needs doing every minute?
      }
   }

}


// items which need to be performed each minute, or multiple thereof
void MinuteWork()
{
#if __DEBUG_MISC
   char mreg, chd, chh, chhc;
#endif

   // once per minute, read and save analog battery voltage reading
   BatteryVoltage = read_adc() / 62;  // read ADC and convert reading to voltage * 10 (computation explained elsewhere)
   
   // set power state code based upon present state and that new value
   switch(PowerStateCode)
   {
      case POWER_GOOD:  // good before this
         if (BatteryVoltage < MinimumBatteryVoltage)  // has dropped too low
         {
            PowerStateCode = POWER_LOW;  // reflect this condition in the state code
            BatteryVoltageOK = FALSE;  // simplistic variable setting
         }
         break;
         
      case POWER_LOW:  // too low, check for recharge with hysteresis
         if (BatteryVoltage > MinimumBatteryVoltage)  // has gotten higher, state is 'RECOVERING' until 0.1V above threshold
         {
            PowerStateCode = POWER_RECOVERING;  // reflect this condition in the state code
         }
         break;
         
      case POWER_RECOVERING:  // check for above the low voltage threshold
         if (BatteryVoltage > (MinimumBatteryVoltage + 1))  // has gotten higher, state returns to 'GOOD'
         {
            PowerStateCode = POWER_GOOD;  // reflect this condition in the state code
            BatteryVoltageOK = TRUE;  // recovery achieved
         }
         break;
         
      default:  // ???
         break;
   }
     
      // produce the "MinuteReport"
#if __DEBUG_MISC
   if (dbpEnabled(LEV2))
   {
      printf(dpo, "*** MinuteBin:%u  BV:%3.1w  BTS:%d  HP:", MinuteBin, BatteryVoltage, BTLEactiveState);
      if(UsingHostBTLE)
      {
         printf(dpo, "BTLE");
      }
      else if(UsingHostSUR)
      {
         printf(dpo, "SUR");
      }
      else  // host unassigned
      {
         printf(dpo, "<none>");
      }
      printf(dpo, "  %s\r\n", stringTheDateTimeUptime());
      // show whether xbee modem is registered with cell network
      mreg = (RegisteredToCellNetwork) ? 'Y' : 'N';
      chd = (NeedTheDetectionReport) ? 'T' : 'F';
      chh = (NeedTheHealthReport) ? 'T' : 'F';
      chhc = (ConditionalNeedTheHealthReport) ? 'T' : 'F';
      printf(dpo, "    MREG:%c DR:%c HR:%c HRC:%c\r\n", 
         mreg, chd, chh, chhc);
      
      // display item counts for all allocatable buffers
      printf(dpo, "    FSB:%d LSB:%d FLB:%d LLB:%d\r\n", 
         sbptrDepth, sbptrLeastDepth, lbptrDepth, lbptrLeastDepth);

      // display item counts for all queues
      printf(dpo, "    apiRcvQueue:%d  apiXmitQueue:%d  smsPktDispQueue:%d  smsStatMatchQueue:%d\r\n",
         wordqCount(apiRcvQueue),
         wordqCount(apiXmitQueue),
         wordqCount(smsPktDispQueue),
         wordqCount(smsStatMatchQueue));
   }
#endif

   // if the XBEE modem has failed, display message requiring power cycle in display lines 3 and 4
   if (!isXbeeOkToUse())
   {
      needResetMessage();
   }

}


//
// utility function to echo single character, formatted in hex and ASCII
// leading blank is also displayed
// optionally append newline
//
void charToHost(char rch, BOOLEAN nl=FALSE)  // formatted hex OR character display
{
   hpo(' ');
   if ((rch < 0x20) || (rch > 0x7E))  // nonprinting character
   {
      printf(hpo, "x%02X", rch);
   }
   else  // printing character
   {
      hpo(rch);
   }
   if (nl)
   {
      printf(hpo, "\r\n");
   }
}


//
// utility function to echo single character, formatted in hex and ASCII
// leading blank is also displayed
// optionally append newline
//
void charToDebug(char rch, BOOLEAN nl=FALSE)  // formatted hex OR character display
{
   dpo(' ');
   if ((rch < 0x20) || (rch > 0x7E))  // nonprinting character
   {
      printf(dpo, "x%02X", rch);
   }
   else  // printing character
   {
      dpo(rch);
   }
   if (nl)
   {
      printf(dpo, "\r\n");
   }
}



//
// utility function to display numeric value of single character, formatted in decimal and hex.
// optionally append newline
//
void valueToHost(char rch, BOOLEAN nl=FALSE)  // formatted hex OR character display
{
   printf(hpo, " %u x%2X", rch, rch);
   if (nl)
   {
      printf(hpo, "\r\n");
   }
}



//
// utility function to display numeric value of single character, formatted in decimal and hex.
// optionally append newline
//
void valueToDebug(char rch, BOOLEAN nl=FALSE)  // formatted hex OR character display
{
   printf(dpo, " %u x%2X", rch, rch);
   if (nl)
   {
      printf(dpo, "\r\n");
   }
}


// decode a two-character string as a hex number
// return value of that number, or -1 if decode error
int16 decodeTwoHex(char *hstr)
{
   int16 retval;
   char msd = hstr[0];
   char lsd = hstr[1];

   if (strlen((char *)hstr) != 2)
      return -1;  // strict enforcement, two-character string

   if(!isxdigit(msd)) // msd not valid hex digit
      return -1;  // error

   if(!isxdigit(lsd)) // lsd not valid hex digit
      return -1;  // error

   retval = strtoul((char *)hstr, 0, 16);  // convert string as a hexadecimal integer
   return retval;
}


//
// display message regarding power-cycling on lines 3 & 4 of LCD
//
void needResetMessage()
{
   //                         12345678901234567890
   static BYTE* ErrorLine3 = "CMODEM UNRESPONSIVE ";
   static BYTE* ErrorLine4 = "POWER-CYCLE REQUIRED";
   strcpyb(DisplayLine3String, ErrorLine3);
   strcpyb(DisplayLine4String, ErrorLine4);

}


//
// utility function to dump memory contents formatted in hex and ASCII
//
// always outputs to the designated debug output port
//
void innerDumpToDbg(BYTE mem[], unsigned long count, enum dmpadr x, BOOLEAN addAscii)  // formatted hex and character display
{
   unsigned long i,j;
   unsigned long roundedCount;  // round up to even multiple of 16
   unsigned long lineStart;
   int lc;  // how many bytes on current line
   char ch;
   
   if (count == 0)
      return;  // don't call the function this way
   
   roundedCount = round_up(count, 16);  // yields even 16-byte lines

   for(i = 0 ; i < roundedCount ; i++)
      {
         if (!(i % 16))
            {
               lc = 0;  // reset bytes-displayed count for each new line of display
               lineStart = i;  // save buffer subscript at beginning of line
               if (x == DMP_BOTH)  // both real address and offset
               {
                  U32 addressOfLine;
                  addressOfLine = mem + i;
                  // adjust width of offset based upon count, 2..4
                  if (roundedCount > 0xfff)  // display four places
                  {
                     printf(dpo,"\r\n%04LX/%04LX:", addressOfLine,i);  // newline and address print
                  }
                  else if (roundedCount > 0xff)  // display three places
                  {
                     printf(dpo,"\r\n%04LX/%03LX:", addressOfLine,i);  // newline and address print
                  }
                  else  // display two places
                  {
                     printf(dpo,"\r\n%04LX/%02X:", addressOfLine,i);  // newline and address print
                  }
               }
               else if (x == DMP_OFST)
               {
                  // adjust width of offset based upon count, 2..4
                  if (roundedCount > 0xfff)  // display four places
                  {
                     printf(dpo,"\r\n%04LX:",i);  // newline and address print
                  }
                  else if (roundedCount > 0xff)  // display three places
                  {
                     printf(dpo,"\r\n%03LX:",i);  // newline and address print
                  }
                  else  // display two places
                  {
                     printf(dpo,"\r\n%02X:",i);  // newline and address print
                  }
               }
               else if (x == DMP_ADDR)
               {
                  U32 addressOfLine;
                  addressOfLine = mem + i;
                  printf(dpo,"\r\n%04LX:", addressOfLine);  // newline and address print
               }
               else  // something else, don't put out address value
               {
                  printf(dpo,"\r\n");  // newline and address print
               }
            }
         if (!(i % 8))
            printf(dpo," ");  // extra space here and there
         if (i < count)  // displaying real data
         {
            ++lc;  // count real displayed character
            printf(dpo," %02X", mem[i]);
         }
         else  // spacing out the end of a short final line
         {
            printf(dpo,"   ");
         }
         if (((i % 16) == 15) && addAscii)  // last number on this line, add ASCII data if requested
         {
            printf(dpo," ");  // space over for ascii
            for(j = lineStart ; j < (lineStart + lc) ; j++)
               {
                  if (!(j % 8))
                     printf(dpo," ");  // extra space here and there
                  ch = mem[j];
                  printf(dpo,"%c", ((ch < 0x80) && (isprint(ch))) ? ch : '.');
               }   
         }
       }
   printf(dpo,"\r\n");   // final CRLF at the end
}



//
// utility function to dump memory contents formatted in hex and ASCII
//
// always outputs to the designated host output port
//
void innerDumpToHost(BYTE mem[], unsigned long count, enum dmpadr x, BOOLEAN addAscii)  // formatted hex and character display
{
#if 0  // this is the real code, sidelined for now
   unsigned long i,j;
   unsigned long roundedCount;  // round up to even multiple of 16
   unsigned long lineStart;
   int lc;  // how many bytes on current line
   char ch;
   
   if (count == 0)
      return;  // don't call the function this way
   
   roundedCount = round_up(count, 16);  // yields even 16-byte lines

   for(i = 0 ; i < roundedCount ; i++)
      {
         if (!(i % 16))
            {
               lc = 0;  // reset bytes-displayed count for each new line of display
               lineStart = i;  // save buffer subscript at beginning of line
               if (x == DMP_BOTH)  // both real address and offset
               {
                  U32 addressOfLine;
                  addressOfLine = mem + i;
                  // adjust width of offset based upon count, 2..4
                  if (roundedCount > 0xfff)  // display four places
                  {
                     printf(hpo,"\r\n%04LX/%04LX:", addressOfLine,i);  // newline and address print
                  }
                  else if (roundedCount > 0xff)  // display three places
                  {
                     printf(hpo,"\r\n%04LX/%03LX:", addressOfLine,i);  // newline and address print
                  }
                  else  // display two places
                  {
                     printf(hpo,"\r\n%04LX/%02X:", addressOfLine,i);  // newline and address print
                  }
               }
               else if (x == DMP_OFST)
               {
                  // adjust width of offset based upon count, 2..4
                  if (roundedCount > 0xfff)  // display four places
                  {
                     printf(hpo,"\r\n%04LX:",i);  // newline and address print
                  }
                  else if (roundedCount > 0xff)  // display three places
                  {
                     printf(hpo,"\r\n%03LX:",i);  // newline and address print
                  }
                  else  // display two places
                  {
                     printf(hpo,"\r\n%02X:",i);  // newline and address print
                  }
               }
               else if (x == DMP_ADDR)
               {
                  U32 addressOfLine;
                  addressOfLine = mem + i;
                  printf(hpo,"\r\n%04LX:", addressOfLine);  // newline and address print
               }
               else  // something else, don't put out address value
               {
                  printf(hpo,"\r\n");  // newline and address print
               }
            }
         if (!(i % 8))
            printf(hpo," ");  // extra space here and there
         if (i < count)  // displaying real data
         {
            ++lc;  // count real displayed character
            printf(hpo," %02X", mem[i]);
         }
         else  // spacing out the end of a short final line
         {
            printf(hpo,"   ");
         }
         if (((i % 16) == 15) && addAscii)  // last number on this line, add ASCII data if requested
         {
            printf(hpo," ");  // space over for ascii
            for(j = lineStart ; j < (lineStart + lc) ; j++)
               {
                  if (!(j % 8))
                     printf(hpo," ");  // extra space here and there
                  ch = mem[j];
                  printf(hpo,"%c", ((ch < 0x80) && (isprint(ch))) ? ch : '.');
               }   
         }
       }
   printf(hpo,"\r\n");   // final CRLF at the end
#else  // hotwire in the "dpo" dump
   unsigned long i,j;
   unsigned long roundedCount;  // round up to even multiple of 16
   unsigned long lineStart;
   int lc;  // how many bytes on current line
   char ch;
   
   if (count == 0)
      return;  // don't call the function this way
   
   roundedCount = round_up(count, 16);  // yields even 16-byte lines

   for(i = 0 ; i < roundedCount ; i++)
      {
         if (!(i % 16))
            {
               lc = 0;  // reset bytes-displayed count for each new line of display
               lineStart = i;  // save buffer subscript at beginning of line
               if (x == DMP_BOTH)  // both real address and offset
               {
                  U32 addressOfLine;
                  addressOfLine = mem + i;
                  // adjust width of offset based upon count, 2..4
                  if (roundedCount > 0xfff)  // display four places
                  {
                     printf(dpo,"\r\n%04LX/%04LX:", addressOfLine,i);  // newline and address print
                  }
                  else if (roundedCount > 0xff)  // display three places
                  {
                     printf(dpo,"\r\n%04LX/%03LX:", addressOfLine,i);  // newline and address print
                  }
                  else  // display two places
                  {
                     printf(dpo,"\r\n%04LX/%02X:", addressOfLine,i);  // newline and address print
                  }
               }
               else if (x == DMP_OFST)
               {
                  // adjust width of offset based upon count, 2..4
                  if (roundedCount > 0xfff)  // display four places
                  {
                     printf(dpo,"\r\n%04LX:",i);  // newline and address print
                  }
                  else if (roundedCount > 0xff)  // display three places
                  {
                     printf(dpo,"\r\n%03LX:",i);  // newline and address print
                  }
                  else  // display two places
                  {
                     printf(dpo,"\r\n%02X:",i);  // newline and address print
                  }
               }
               else if (x == DMP_ADDR)
               {
                  U32 addressOfLine;
                  addressOfLine = mem + i;
                  printf(dpo,"\r\n%04LX:", addressOfLine);  // newline and address print
               }
               else  // something else, don't put out address value
               {
                  printf(dpo,"\r\n");  // newline and address print
               }
            }
         if (!(i % 8))
            printf(dpo," ");  // extra space here and there
         if (i < count)  // displaying real data
         {
            ++lc;  // count real displayed character
            printf(dpo," %02X", mem[i]);
         }
         else  // spacing out the end of a short final line
         {
            printf(dpo,"   ");
         }
         if (((i % 16) == 15) && addAscii)  // last number on this line, add ASCII data if requested
         {
            printf(dpo," ");  // space over for ascii
            for(j = lineStart ; j < (lineStart + lc) ; j++)
               {
                  if (!(j % 8))
                     printf(dpo," ");  // extra space here and there
                  ch = mem[j];
                  printf(dpo,"%c", ((ch < 0x80) && (isprint(ch))) ? ch : '.');
               }   
         }
       }
   printf(dpo,"\r\n");   // final CRLF at the end
#endif
}


//
// utility functions to dump memory contents in a variety of ways
//
void dumpToHost(BYTE mem[], unsigned long count)  // formatted hex and character display, using offset addressing
{
   innerDumpToHost(mem, count, DMP_OFST, TRUE);  // formatted hex and character display
}

void dumpToHostNoAscii(BYTE mem[], unsigned long count)  // formatted hex and character display, using offset addressing
{
   innerDumpToHost(mem, count, DMP_OFST, FALSE);  // formatted hex display, no ASCII display
}

void dumpToHostWithAddress(BYTE mem[], unsigned long count)  // formatted hex and character display, using PIC memory addressing
{
   innerDumpToHost(mem, count, DMP_ADDR, TRUE);  // formatted hex and character display
}

void dumpToHostWithBoth(BYTE mem[], unsigned long count)  // formatted hex and character display, using PIC memory addressing
{
   innerDumpToHost(mem, count, DMP_BOTH, TRUE);  // formatted hex and character display, using both PIC memory and offset addressing
}


// set or clear a "running" indicator in display top line
void setRunning()
{
#if __DEBUG_SURCOM
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "\r\n%s\r\n", fortyBucks);
      printf(dpo, "*** RUNNING INDICATOR SET  %s\r\n", stringTheDateTimeUptime());
      printf(dpo, "%s\r\n", fortyBucks);
   }
#endif

   RunningNow = TRUE ;
   maintainLCD();  // update display to show running now
}

void clearRunning()
{
#if __DEBUG_SURCOM
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "\r\n%s\r\n", fortyBucks);
      printf(dpo, "*** RUNNING INDICATOR CLEAR  %s\r\n", stringTheDateTimeUptime());
      printf(dpo, "%s\r\n", fortyBucks);
   }
#endif

   RunningNow = FALSE ;
   maintainLCD();  // update display to show not running now
}


// set or clear a "modem active" indicator in display top line
void setModemActive()
{
//!#if __DEBUG_XBEE
//!   if (dbpEnabled(LEV3))
//!   {
//!      printf(dpo, "\r\n%s\r\n", fortyBucks);
//!      printf(dpo, "*** MODEM ACTIVE INDICATOR SET  %s\r\n", stringTheDateTimeUptime());
//!      printf(dpo, "%s\r\n", fortyBucks);
//!   }
//!#endif

   XbeeModemIsActive = TRUE ;
   maintainLCD();  // update display to show running now
}

void clearModemActive()
{
//!#if __DEBUG_XBEE
//!   if (dbpEnabled(LEV3))
//!   {
//!      printf(dpo, "\r\n%s\r\n", fortyBucks);
//!      printf(dpo, "*** MODEM ACTIVE INDICATOR CLEAR  %s\r\n", stringTheDateTimeUptime());
//!      printf(dpo, "%s\r\n", fortyBucks);
//!   }
//!#endif

   XbeeModemIsActive = FALSE ;
   maintainLCD();  // update display to show not running now
}


// check whether time for an action is matched


// functions used to indicate need for reports at points in the BigLoop

// in each hour, indicate the minute value for which a detection report should be produced.
// configuration parameter selects one of six possible sets of reporting times.
// greatest number of times in any set is 10. a "stopper" value is used for sets with fewer entries.
// this means a 6 X 11 matrix of minute values is used.
// return TRUE if the minute value submitted as the parameter is found in the set corresponding
// to the selected configuration value.
BOOLEAN CheckNeedDetectionReportThisMinute(int thisMinute)  
{
   // first subscript is the configured parameter.
   // second subscript is stopper-terminated list of minutes in which the report is produced
   int mm = 0;
   while (DetectionReportMinuteMatchtimes[CFG_NVMshadow[cfg_cur][DETREPINT]][mm] >= 0)  // not the stopper
   {
      if(DetectionReportMinuteMatchtimes[CFG_NVMshadow[cfg_cur][DETREPINT]][mm] == thisMinute)  // got a match
      {
         return TRUE;  // return success
      }
      ++mm;
   }
   return FALSE;  // never matched the input minute value
}


// in each hour, indicate the hour for which a health report should be produced.
// configuration parameter selects one of three possible sets of reporting times.
// greatest number of times in any set is 3. a "stopper" value is used for sets with fewer entries.
// this means a 3 X 3 matrix of minute values is used.
// return TRUE if the minute value submitted as the parameter is found in the set corresponding
// to the selected configuration value.
BOOLEAN CheckNeedHealthReportThisHour(int thisHour)  // flag is set for each hour in which to produce a health report
{
   // first subscript is the configured parameter.
   // second subscript is stopper-terminated list of hours in which the report is produced
   int mm = 0;
   while (HealthReportHourMatchtimes[CFG_NVMshadow[cfg_cur][HLTHREPINT]][mm] >= 0)  // not the stopper
   {
      if(HealthReportHourMatchtimes[CFG_NVMshadow[cfg_cur][HLTHREPINT]][mm] == thisHour)  // got a match
      {
         return TRUE;  // return success
      }
      ++mm;
   }
   return FALSE;  // never matched the input minute value
}


//
// items below for pushbutton switch debouncing
//

// This holds the debounced state of the key.
// set by interrupt service, read by task code
volatile BOOLEAN DebouncedKeyPress = FALSE;

// These cells hold the uptime time stamp at press recognition, release recognition
// set by interrupt service, read by task code
volatile U32 PbPressedTime = 0;
volatile U32 PbReleasedTime = 0;


BOOLEAN Key_changed;
BOOLEAN Key_pressed;

//
// read the pushbutton hardware bit
// return 'TRUE' if pressed, FALSE if released
// actual switch operation is the reverse so invert the bit which is read
//
// this function is used in interrupt service, and should not
// be called from the ordinary application code
//
BOOLEAN RawKeyPressed()
{
   //return ~input(sw1);
   return ~input(PUSH_BUTTON);
}


// Service routine called every PB_CHECK_MSEC to
// debounce both edges
//
// this function is used in interrupt service, and should not
// be called from the ordinary application code
//
void DebouncePb()
{
    static char Count = PB_RELEASE_MSEC / PB_CHECK_MSEC;
    BOOLEAN RawState;
    Key_changed = FALSE;
    Key_pressed = DebouncedKeyPress;
    RawState = RawKeyPressed();
    if (RawState == DebouncedKeyPress) 
    {
        // Set the timer which will allow a change from the current state.
        if (DebouncedKeyPress) 
           Count = PB_RELEASE_MSEC / PB_CHECK_MSEC;
        else                 
           Count = PB_PRESS_MSEC / PB_CHECK_MSEC;
    } 
    else 
    {
        // Key has changed - wait for new state to become stable.
        if (--Count == 0) 
        {
            // Timer expired - accept the change.
            DebouncedKeyPress = RawState;
            Key_changed=TRUE;
            Key_pressed=DebouncedKeyPress;
            // And reset the timer.
            if (DebouncedKeyPress)
            {
               Count = PB_RELEASE_MSEC / PB_CHECK_MSEC;
               PbPressedTime = uptimeMilliseconds();  // note time press was recognized
               //PbPressedTime = uptime_milliseconds;  // note time press was recognized. read cell directly, don't use function
               //PbReleasedTime = 0;  // invalidate released time  ??? do we need to do this?
            }
            else                 
            {
               Count = PB_PRESS_MSEC / PB_CHECK_MSEC;
               PbReleasedTime = uptimeMilliseconds(); // note time release was recognized
               //PbReleasedTime = uptime_milliseconds; // note time release was recognized. read cell directly, don't use function
            }
        }
    }
} 


//
// enhanced to provide short-press, long-press discrimination.
// now the return  will be:
// swpNONE: no key press
// swpSHORT: short key press
// swpLONG: long key press
//
swp_code buttonCycled(BOOLEAN initializeNow=FALSE)
{
   static U32 pressedTimeAtReport = 0;
   static U32 releasedTimeAtReport = 0;
   
   U32 localPbPressedTime = PbPressedTime;  // freeze local copies of these two items
   U32 localPbReleasedTime = PbReleasedTime;
   
   U32 uptimeNow = uptimeMilliseconds();  // hey kids, what time is it?
   int32 pbDuration;  // note this is SIGNED
   int32 sinceLastRelease;  // note this is SIGNED
   swp_code retv;  // return value
   
   // maybe initialize some things and exit
   if (initializeNow)  // kludge to get initialized
   {
      pressedTimeAtReport = releasedTimeAtReport = uptimeMilliseconds();  // set some non-zero but valid values
      return swpNONE;  // no key press
   }
   
   
   pbDuration = localPbReleasedTime - localPbPressedTime;  // compute the delta using saved uptime values
   sinceLastRelease = uptimeNow - localPbReleasedTime;
                        
   // note whether a release time is recorded 
   if (localPbReleasedTime == 0)  // key has not yet cycled, cannot continue
   {
      retv = swpNONE;  // no key press
   }
   // if the most recent times associated with our report
   // match the current press and release times, then there
   // has been no new activity since the last key we reported.
   else if ((localPbReleasedTime == releasedTimeAtReport) && (localPbPressedTime == pressedTimeAtReport))
   {
      retv = swpNONE;  // no key press
   }
   // if the most recent released time is older than ten seconds,
   // call the information stale and ignore it.
   else if (sinceLastRelease > 5000UL)
   {
      retv = swpNONE;  // no key press
   }
   else  // press and release times appear to be recent
   {
      if (pbDuration < 30)  // negative or very small implies bad data points
      {
         retv = swpNONE;  // no key press
      }
      else if (pbDuration < longPressThresholdMsecs)  // it's a short press
      {
#if __DEBUG_MISC
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "** Short Press Reported %s\r\n", stringTheDateTimeUptime());
         }
#endif
         retv = swpSHORT;  // short key press
      }
      else  // it's a long press
      {
#if __DEBUG_MISC
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "** Long Press Reported %s\r\n", stringTheDateTimeUptime());
         }
#endif
         retv = swpLONG;  // long key press
      }
   }
   
   if(retv > swpNONE)  // save times associated with push report
   {
      pressedTimeAtReport = localPbPressedTime;  // freeze local copies of these two items
      releasedTimeAtReport = localPbReleasedTime;
   }
   
   return retv;
}




//
// specialized function to ensure that a memory pointer value is even.
// pointer value may increase or stay the same, never decrease.
// got rid of debug print because this has been present and working for years.
//
void *advanceToEven(void *aptr)
{
   void *retVal = (aptr + 1) & ~1;
   return retVal;
}


//
// specialized function to convert ascii string to decimal number.
// returns signed value.
// successful conversion will be 0..32767.
// negative number indicates some failure. (non-decimal digit, string too long, etc.)
//
signed int16 zasciiConvert(BYTE *astring)
{
   int8 i;
   signed int16 retval;
   int8 slen = strlen((char *)astring);
   if ((slen == 0) || (slen > 5))
      return -1;  // too short or too long
   for (i = 0 ; i < slen ; i++)
   {
      if (!isdigit(astring[i]))  // not a decimal digit
         return -1;
   }
   retval = strtol((char *)astring, 0, 10);  // convert string as a signed decimal integer
   if (retval < 0) // could mean that retval > 32767L
      return -1;
   return retval;  // success
}




//
// begin a command response SMS message with standardized contents
//
// response string built in buffer passed as first parameter
//
//
//!void prefixHrepResponse(BYTE *mbuf)
//!{
//!
//!
//!
//!   BYTE vmsb,vlsb;
//!   
//!   vmsb = CFG_NVMshadow[cfg_cur][VERS] / 16;
//!   vlsb = CFG_NVMshadow[cfg_cur][VERS] % 16;
//!   sprintf(mbuf, "SUR-SMS v%x.%x %s\r\n", vmsb, vlsb, stringTheDateAndTime());
//!}



//
// special-purpose string copy function
//
// copy characters from source to destination until
// encountering a null character, or a cr-lf pair.
// copy the cr-lf to destination, terminate with null, and return.
//
void  copy_upto_crlf(BYTE *destPtr, BYTE *srcPtr)
{
   BOOLEAN keepGoing = TRUE;
   while (keepGoing)
   {
      if (*srcPtr == 0)  // no cr-lf in this string
      {
         *destPtr = 0;
         break;
      }
      else if ((srcPtr[0] == '\r') && (srcPtr[1] == '\n'))  // cr-lf in source string, copy is over
      {
         *destPtr++ = '\r';
         *destPtr++ = '\n';
         *destPtr = 0;
         break;
      }
      else  // just copy the present character
      {
         *destPtr++ = *srcPtr++;
      }
   }
}



// special purpose function to check whether BTLE port has been connected
BOOLEAN IsBtleConnected()
{
   return BTLEactiveState == __BTLEQ_DONE;
}



//
// special purpose function to read and discard host port input
// characters until a percent sign is found. 
//
// used to skip over spontaneous status message strings
// from the BTLE controller.
//
// echo the characters on the debug port
//
void skip_until_pct()
{
   BYTE xch;
   
   percentStringBufferSub = 0;
   percentStringBuffer[percentStringBufferSub++] = '%';
   percentStringBuffer[percentStringBufferSub] = 0;  // always a proper C string, point to the null
   do
   {
      xch = hpi_nblk();  // big risk here. expect that '%' will eventually arrive
      //strcatchb(percentStringBuffer, xch);
      percentStringBuffer[percentStringBufferSub++] = xch;
      percentStringBuffer[percentStringBufferSub] = 0;  // always a proper C string, point to the null
   } while (xch != '%');
   printf(dpo, ">>> skip_until_pct() %s  %s\r\n", percentStringBuffer, stringTheUptime());
}



//
// note the connection of the BTLE controller by an external device.
// use a state machine to distribute this determination across multiple
// invocations of this function.
//
//!void trackBtleConnection()
//!{
//!   U32 respondByTime;
//!   BYTE xch;
//!   
//!   switch(BTLEactiveState)
//!   {
//!   case __BTLEQ_IDLE:
//!      if (kbhit(BTport))
//!      {
//!         BTLEactiveState = __BTLEQ_LOOK_1STPCT;
//!      }
//!      break;
//!      
//!   case __BTLEQ_LOOK_1STPCT:
//!      if (kbhit(BTport))
//!      {
//!         xch = fgetc(BTport);
//!         BTLE_Spy(xch, __BTLE_SPY_READING);  // snoopy
//!         if (xch == '%')
//!         {
//!            percentStringBufferSub = 0;
//!            percentStringBuffer[percentStringBufferSub++] = '%';
//!            percentStringBuffer[percentStringBufferSub] = 0;  // always a proper C string, point to the null
//!            BTLEactiveState = __BTLEQ_BUFFER_MSG;
//!            respondByTime = uptimeMilliseconds() + 750L;  // next character must arrive within 3 character times
//!         }
//!      }
//!      break;
//!      
//!   case __BTLEQ_BUFFER_MSG:
//!      if (kbhit(BTport))
//!      {
//!         xch = fgetc(BTport);
//!         BTLE_Spy(xch, __BTLE_SPY_READING);  // snoopy
//!         percentStringBuffer[percentStringBufferSub++] = xch;
//!         percentStringBuffer[percentStringBufferSub] = 0;  // always a proper C string, point to the null
//!         if (xch == '%')  // string is complete
//!         {
//!            BTLEactiveState = __BTLEQ_CHECK_CONN;  // check the result for %CONNECT...
//!         }
//!         else  // keep buffering, update the timeout value
//!         {
//!            respondByTime = uptimeMilliseconds() + 750L;  // next character must arrive within 3 character times
//!         }
//!      }
//!      else  // has too much time elapsed?
//!      {
//!         if (uptimeMilliseconds() > respondByTime)  // taking too long, start over
//!         {
//!            BTLEactiveState = __BTLEQ_LOOK_1STPCT;  // start over
//!         }
//!      }
//!      break;
//!      
//!   // complete percent string received, look for %CONNECT...
//!   // several strings have been observed, desired match is unique in "%CONNE"
//!   case __BTLEQ_CHECK_CONN:  
//!      printf(dpo, "***** trackBtleConnection() %s  %s\r\n", percentStringBuffer, stringTheUptime());
//!      if (memcmpb(percentStringBuffer, "%CONNE", 5) == 0)  // we have a winner
//!      {
//!         BTLEactiveState = __BTLEQ_DONE;  // stop looking for percent strings at this point in the program
//!      }
//!      else
//!      {
//!         BTLEactiveState = __BTLEQ_LOOK_1STPCT;  // start the process over
//!      }
//!      break;
//!      
//!   default:
//!      printf(dpo, "\r\n***** trackBtleConnection() bad state: %d\r\n", BTLEactiveState);
//!      BTLEactiveState = __BTLEQ_LOOK_1STPCT;  // start the process over
//!   case __BTLEQ_DONE:
//!      break;
//!      
//!   }
//!
//!}



//
// make a determination of host port in use (serial or btle) by checking
// for received characters and choosing the port that first receives
// a 'CSI' character as the winner.
//
// note the connection messages from the BTLE controller. many different
// status messages occur from that source, all are begun and ended by
// '%' characters. we track these strings and ignore.
// use a state machine to distribute this determination across multiple
// invocations of this function.
//
// caller of this function should check beforehand that the host port
// is not yet assigned.
//
BOOLEAN checkHostConnection()
{
   BYTE xch;
   
   // early out if the host port assignment has been made already
   if ((UsingHostSUR) || (UsingHostBTLE))
   {
      return FALSE;  // no swallowing of CSI has occurred
   }
   
   // check SUR serial port first
   if (kbhit(SURport))  // is a character waiting?
   {
      xch = fgetc(SURport);  // read the character
      if ((xch == AsciiESC) || (xch == '+'))  // if this is CSI, we note the state and exit
      {
         UsingHostSUR = TRUE;  // use SUR port for host
         //printf(dpo, "\r\n*****\r\n***** selected host port SUR\r\n*****\r\n");
         return TRUE;  // this read has consumed the CSI, note that for caller
      }
      // otherwise got a character but it's not a CSI, exit this function at the bottom
   }
   else if (kbhit(BTport))  // is a character waiting?
   {
      xch = fgetc(BTport);
//!      BTLE_Spy(xch, __BTLE_SPY_READING);  // snoopy
   
      switch(BTLEactiveState)  // process character according to state machine
      {
      case __BTLEQ_LOOK_CSI_OR_PCT:
         // check here for CSI
         if ((xch == AsciiESC) || (xch == '+'))  // if this is CSI, we note the state and exit
         {
            UsingHostBTLE = TRUE;  // use BTLE port for host
            //printf(dpo, "\r\n*****\r\n***** selected host port BLE\r\n*****\r\n");
            BTLEactiveState = __BTLEQ_DONE;  // stop looking for host port assignment at this point in the program
            return TRUE;  // this read has consumed the CSI, note that for caller
         }
         else if (xch == '%')  // leading percent of a pair
         {
            BTLEactiveState = __BTLEQ_LOOK_2NDPCT;  // now looking for closing percent character
         }
         // otherwise, ignore this character and exit function at the bottom
         break;
         
      case __BTLEQ_LOOK_2NDPCT:
         if (xch == '%')  // trailing percent of the pair
         {
            BTLEactiveState = __BTLEQ_LOOK_CSI_OR_PCT;  // now looking for CSI or opening percent character
         }
         // otherwise, ignore this character and exit function at the bottom
         break;
         
      case __BTLEQ_DONE:
         // ignore this character and exit function at the bottom
         break;
         
      default:
         printf(dpo, "\r\n***** trackBtleConnection() bad state: %d\r\n", BTLEactiveState);
         BTLEactiveState = __BTLEQ_LOOK_CSI_OR_PCT;  // start the process over
         break;
         
      }
   }  // end if (kbhit(BTSURport))
      
      return FALSE;  // have not received a CSI

}



//
// Big Loop Maintenance Items
//
// work performed each pass through the Big Loop
//
// also can be called from the midst of other tasks if
// they take a long time, to keep things current.
//
// 'immediate' BOOLEAN parameter may be set to handle cases
// where we don't want to wait for the cyclic flags.
//
void BigLoopMaintenance(BOOLEAN immediate=FALSE)
{
   // check if updating of the common date/time values is required now
   if ((immediate) || (dateTimeNeedsUpdate))
   {
      dateTimeNeedsUpdate = FALSE;  // clear this flag
      maintainDateTimeValues();  // actually do it
   }      
   
   // check if updating of the LCD is required now
   if ((immediate) || (updateLcdNow))
   {
      updateLcdNow = FALSE;  // clear this flag
      maintainLCD();  // actually do it
   }      

}

//
// WaitWithHousekeeping()
//
// delay for some amount of seconds, while periodically
// invoking 'BigLoopMaintenance' (every 100 mS)
//
void WaitWithHousekeeping(int nSecs)
{
   U32 timeLimit;

   if(nSecs < 1)  // zero or negative, exit right away
   {
      return;
   }
   
   timeLimit = uptimeMilliseconds() + (nSecs * 1000L);  // this will be the uptime 'nSecs'seconds from now
   while (uptimeMilliseconds() < timeLimit)
   {
      BigLoopMaintenance(TRUE);  // drive the background updates every quarter-second
      delay_ms(250);
   }
}



//
// Big Loop Instrumentation Items
//
// work performed each pass through the Big Loop
//
// in cycles of 16384 loops, note the time spanned.
// big loop time for PIC24 turns out to be less than a millisecond.
//
void BigLoopInstrumentation()
{
   
   if (cycleCounter < 4096)  // don't do this forever
   {
      ++loopCycleCounter;
      if (loopCycleCounter == 1)  // it's the first pass of a cycle
      {
         thisCycleStartTime = uptimeMilliseconds();
      }
      else if (loopCycleCounter == 16384)  // it's the last of a cycle
      {
         ++cycleCounter;  // count cycles
         thisCycleEndTime = uptimeMilliseconds();
         thisCycleTimeSpan = thisCycleEndTime - thisCycleStartTime;
         printf(hpo, "*** Loop Data  cycle:%Lu  span:%Lu   %s\r\n", cycleCounter, thisCycleTimeSpan, stringTheDateTimeUptime());
         loopCycleCounter = 0;  // reset count for next cycle
      }
   }

}


  
//!void BigLoopInstrumentation()
//!{
//!   
//!   if (cycleCounter < 250)  // don't do this forever
//!   {
//!      ++loopCycleCounter;
//!      if (loopCycleCounter == 1)  // it's the first pass of a cycle
//!      {
//!         thisCycleStartTime = uptimeMilliseconds();
//!      }
//!      else if (loopCycleCounter == 10240)  // it's the last of a cycle
//!      {
//!         ++cycleCounter;  // count cycles
//!         thisCycleEndTime = uptimeMilliseconds();
//!         thisCycleTimeSpan = thisCycleEndTime - thisCycleStartTime;
//!         printf(hpo, "*** Loop Data  cycle:%Lu  span:%Lu   %s\r\n", cycleCounter, thisCycleTimeSpan, stringTheDateTimeUptime());
//!         loopCycleCounter = 0;  // reset count for next cycle
//!      }
//!   }
//!
//!}


  
  
//
// five-second countdown
//
// delay five seconds while displaying a pattern on the SUR port
//
void fiveSecCountDown()
{
   printf(dpo, ".....\r\n");
   delay_ms(1000);
   printf(dpo, "....\r\n");
   delay_ms(1000);
   printf(dpo, "...\r\n");
   delay_ms(1000);
   printf(dpo, "..\r\n");
   delay_ms(1000);
   printf(dpo, ".\r\n");
   delay_ms(1000);
}


//
// Bluetooth Low Energy functions
//

//
// Bluetooth support functions
// functions to perform odd, handy tasks for the RN487X Bluetooth Interface
//

// perform hardware reset on BTLE controller.
// we do this after power-up to ensure controller is correctly initialized.
// we do this when using the controller to bring it out of power-saving mode.

void resetBT()
{

   // fabricate reset pulse for RN4871 Bluetooth controller
   output_low(bRS);
   delay_ms(1);
   output_high(bRS);       // reset BLE
}




//
// allocate a large buffer for the detection report response.
//
// format the sequence number and RSSI value
// at the start of that detection report
//
// begin the output buffer with an incrementing value which
// increments each time a detection report is created and sent.
// that permits the recipient to detect whether any reports were
// missed at his phone. we'll count up every message, and use the
// lower N bits for our number. that number will repeat every so 
// often, such as 00..31 for 5 bits.
//
// newer addition is the modem RSSI number, following the sequence number.
//
BYTE *prefixDetectionReportResponse()
{
   BYTE *mbuf = lballoc();  // allocate large string data buffer in which to construct data report
   BYTE rsnValue;
   BYTE rssiValue;
   
   //
   // insert the sequence number and RSSI at the start of the message
   // recent modification, add the current battery voltage 
   //
   rsnValue = nextDetRepSeq();
   rssiValue = savedRSSI;
   if (rssiValue == 0x99)  // actual value is unobtainable now
   {
      sprintf(mbuf, "RSN%02d|-XX|%3.1w\r\n", rsnValue, BatteryVoltage);  // this string means can't get RSSI now
   }
   else
   {
      sprintf(mbuf, "RSN%02d|-%d|%3.1w\r\n", rsnValue, rssiValue, BatteryVoltage);  // supply leading minus sign
   }
   return mbuf;
}


#endif  // __UTILITY_FUNCTIONS_C_


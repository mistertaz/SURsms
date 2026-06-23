#ifndef S_C_FXNS
#define S_C_FXNS
//
// short functions to perform odd, handy tasks to communicate with
// the SUR. (Submersible Ultrasonic Receiver)
//



//
// long delay used when communicating with emulated SUR.
// waits up to ten seconds for a character to arrive on the
// serial interface, but returns as soon as one does arrive.
//
// interaction with the SUR (real or emulated) may use the SURport UART directly.
//
void relaxedResponseFromSUR(unsigned int16 milliTimeInterval)
{
   U32 timeAtEntry = uptimeMilliseconds();  // all timing measured from this value
   unsigned int16 deltaResponse;
   
   if (milliTimeInterval > 10000UL)  // ten second maximum
   {
      milliTimeInterval = 10000UL;  // enforce this
   }
   
   if (milliTimeInterval < 550)  // don't bother
   {
      return;
   }
   
   while (TRUE)
   {
      deltaResponse = uptimeMilliseconds() - timeAtEntry;  // how much time has passed
      if (kbhit(SURport))  // got a character.  always use port directly
      {
         break;
      }
      else if (deltaResponse > milliTimeInterval)  // time's up
      {
         break;
      }
      else
      {
         delay_ms(130);  // wait about half a charater time
      }
   }
}


// read characters from the SUR.
// at the required baud rate of 38400, one character time is about 260uS.
// we will use that span of time to express timeout wait times.
// each loop checks for the presence of data, so no time is wasted delaying.
// if no character is received after n-many nominal character times, conclude
// that the response string is complete and return 0
//
// this function accesses the real SUR always, so it does not use the stream revector
//
int surGetch_w_timeout(unsigned int16 toInterval)   // parameter is the number of nominal character times to wait
{
   int x;
   unsigned int16 toLoop = toInterval;  // unsigned long toLoop = toInterval;
   
   while (toLoop--)
   {
   if (kbhit(SURport))  // got a character.  always use port directly
      {
         x = fgetc(SURport);  // read character and reply.  always use port directly
         return x ;
      }
   else
      delay_us(260);  // wait one character time
   }
   
   return 0 ;  // never got a character
}



//
// send the command character 'esc' and wait for the responding character ':'
//
// time delay spent waiting varies, as the SUR responds quicker than simulation
// under Windows. Check the hidden parameter value to determine which delay to use.
// normal delay for the SUR is two seconds. extended delay is ten seconds.
// units for delaying are character times at 38400 baud, or about 260uS.
// two seconds in those units is 7680.
// ten seconds in those units is 38400.
//
// this function accesses the real SUR always, so it does not use the stream revector
//
int surCSI()
{
   unsigned long x = (relaxedSurTiming) ? 38400UL : 7680UL ;
#if __DEBUG_SURCOM
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** sending <CSI>  %s\r\n", stringTheDateTimeUptime());
   }
#endif

   // line below always outputs to SUR, every time...
   fputc('\x1b',SURport);        // send <esc> to introduce command sequence

   // wait some amount of time for the ':' response from SUR
   delay_ms(250) ;      // SUR may need this long to act on the command
   if (relaxedSurTiming)  // if timing is relaxed, just wait here for ten seconds
   {
      relaxedResponseFromSUR(10000UL);  // this interval is milliseconds, not character times
   }

   return surGetch_w_timeout(x);  // about two seconds delay
}




//
// function to issue command to SUR and provide response data parsed into strings
//
// function returns 'TRUE' if successful, FALSE otherwise. (assumes no communication occurring)
// 'success' means that command sequence completed without timeout, and that some data was received. (at least one complete line)
//
// 1st parameter is pointer to null-terminated command string.
// 2nd parameter is pointer to buffer to receive response string.
// 3rd parameter is size of response buffer.
//
// this function accesses the real SUR always, so it does not use the stream revector
//
BOOLEAN SURCommandParsedReply(BYTE *cmdp, SPARSE &sp)
{
   BYTE xch;
   int16 j;

#if __DEBUG_SURCOM
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** SURCommandParsedReply() First CSI ");
   }
#endif
   xch = surCSI(); // try once
   if (!(xch == ':'))  // didn't get the reply we want, try again
   {
      delay_ms(200);  // wait a SWAG amount of time
#if __DEBUG_SURCOM
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** SURCommandParsedReply() Second CSI ");
      }      
#endif
      xch = surCSI(); // try again
      if (!(xch == ':'))  // didn't get the reply we want
      {
      // ***need recovery action, like SYNC here
      // for now just bail
      return FALSE;
      }
   }

   // line below outputs to SUR all the time, every time...
   fprintf(SURport, "%s", cmdp);     // issue command string, always use fprintf() directly, not revector

   delay_ms(250) ;      // SUR may need this long to act on the command

   // interacting with the simulated SUR presents its own problems in that the first responded
   // character can take a very long time to arrive. Time out the first character
   // here for a very long time, then use normal timeouts for other reads.

   if (relaxedSurTiming)  // if timing is relaxed, just wait here for ten seconds
   {
      relaxedResponseFromSUR(10000UL);
   }
      
   // loop reading characters until you get a timeout. This is possible
   // because of the interrupt-driven receiver.
   // timeout is indicated by return of a zero value from the character reading function.
   // store the zero as string terminator for the message.
   j = 0 ;
   xch = surGetch_w_timeout(7680UL);
   if (xch == 0)  // no character received
   {
      return FALSE;
   }
   
   // first character read is non-null, buffer it up in the string structure
   stringsParseSetInitialConditions(sp);  // first set up the string parse structure values
   stringsParse(xch, sp);  // buffer up first character
      
   //while(1)    // read until timeout
   for(;;)    // read until timeout
   {
      xch = surGetch_w_timeout(8UL);
      stringsParse(xch, sp);  // this will store the null when it happens
      if (xch == 0)  // returned null means read timed out
      {
         break;
      }
   }
   return TRUE;
}



//
// function to issue command to SUR and provide response data
//
// function returns 'TRUE' if successful, FALSE otherwise. (assumes no communication occurring)
//
// 1st parameter is pointer to null-terminated command string.
// 2nd parameter is pointer to buffer to receive response string.
// 3rd parameter is size of response buffer.
//
// interaction with the SUR may use the SURport UART directly.
//
// received data is stored as a string, not interpreted in any way.
//
BOOLEAN SURCommandRawReply(BYTE *cmdp, BYTE *bufp, int bufs)
{
   BYTE xch;
   int16 j;
   int16 storeSub = 0;
   
   bufp[0] = '\0';   // ensure a null termination to buffer no matter what the outcome

#if __DEBUG_SURCOM
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** SURCommandRawReply() First CSI ");
   }
#endif
   xch = surCSI(); // try once
   if (!(xch == ':'))  // didn't get the reply we want, try again
      {
         delay_ms(200);  // wait a SWAG amount of time
#if __DEBUG_SURCOM
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "*** SURCommandRawReply() Second CSI ");
         }
#endif
         xch = surCSI(); // try again
         if (!(xch == ':'))  // didn't get the reply we want
            {
            // ***need recovery action, like SYNC here
            // for now just bail
            return FALSE;
            }
      }

   // line below outputs to SUR all the time, every time...
   fprintf(SURport, "%s", cmdp);     // issue command string, always use fprintf()
   delay_ms(200) ;      // SUR may need this long to act on the command
   if (relaxedSurTiming)  // if timing is relaxed, just wait here for ten seconds
   {
      relaxedResponseFromSUR(10000UL);
   }
      
   // loop reading characters until you get a timeout. This is possible
   // because of the interrupt-driven receiver.
   // timeout is indicated by return of a zero value from the character reading function.
   // store the zero as string terminator for the message.
   // interacting with the simulated SUR presents its own problems in that the first responded
   // character can take a very long time to arrive. Time out the first character
   // for 7680 character times (around two seconds) then use 8 character times for the balance.
   j = 0 ;
   xch = surGetch_w_timeout(7680UL);
   bufp[storeSub++] = xch;
   if (xch == 0)  // no character received
      {
         return FALSE;
      }
      
   for (j = 1 ; j < bufs ; j++)    // read until timeout or buffer full
      {
      xch = surGetch_w_timeout(8UL);
      bufp[storeSub++] = xch;
      if (xch == 0)
         break;
      }
   bufp[storeSub] = '\0';   // ensure a null termination to buffer
   //?? len = strlen(bufp);
   return TRUE;
}



// query the SUR for the version response, which also contains the number
// of detections currently stored.
// if all succeeds, return that detection count. zero is a valid response.
// if some error, return negative error code.
signed int8 getSurVersAndRc(BYTE *Vresponse=0)  // copy the response to the V/v command here if nonzero
{
   static signed long prev_vMsg_rcount = 0;    // saved SUR record count, used to discover new detections
   signed long vMsg_recCount = 0;
   signed long vMsg_lrecCount = 0;
   signed long vMsg_drecCount = 0;
   signed long new_vMsg_rcount = 0;

   signed int8 retVal;
   
   BYTE LogRecPref[] = {"L="};  // need storage rather than quoted strings for the library string functions
   BYTE DecRecPref[] = {"D="};
   
   BYTE Vcommand[] = {"?"};  // must be set at execution time

   BOOLEAN VersionBailOut = FALSE ;
   
   BYTE *vdbp;
   BYTE *mpx, *mpy;

   // some SUR commands vary based on USERAWLOGS setting in CFG_NVM
   if (CFG_NVMshadow[cfg_cur][USERAWLOGS] != 0)
   {
      Vcommand[0] = 'v';   // use the old way
   }
   else 
   {
      Vcommand[0] = 'V';   // use the new way
   }

   // query SUR for version string.
   // data returns in SURstrings.rstrPtr[0].
   //
   if (SURCommandParsedReply(Vcommand, SURstrings)) // use new-style version request, capture response string
   {
      vdbp = SURstrings.rstrPtr[0];  // copy this value to simple pointer
      //                         12345678901234567890
      strcpy(DisplayLine2String,"VERSION QUERY PASSED");
      //sprintf(DisplayLine2String,"PASSVERS");
#if __DEBUG_SURCOM
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** getSurVersAndRc() read string: -->|%s|<--\r\n", vdbp);
      }
#endif
   }
   else
   {
      //                         12345678901234567890
      strcpy(DisplayLine2String,"VERSION QUERY FAILED");
      //sprintf(DisplayLine2String,"FAILVERS");
      VersionBailOut = TRUE;   // too bad
      retVal = -1;  // indicate error occurred
   } 
   
   BigLoopMaintenance();
   
   if(VersionBailOut)
      return retVal;
   
   //
   // we have a good version string.
   // if caller has supplied a pointer, copy
   // that version string there.
   //
   if (Vresponse)
   {
      strcpy((char *)Vresponse, (char *)vdbp);
   }

   //
   // sampling with a serial monitor leads us to expect these results
   // from the SUR in response to a version request:
   //    Original query: ('v')
   //    10686 v6.E: 7
   //
   //    Enhanced query: ('V')
   //    10686 v6.E: L=2,D=2
   //
   // In the case of the original query, the characters following the colon to the end
   // of the message (CR) representthe count of stored records. This can be a
   // breathtaking six-digit decimal integer.
   //
   // until I find a reason to do something different, I will interpret that field, between
   // the colon and the <CR>, as the record count and crack it.
   //
   // In the case of the enhanced query two counts are returned. One reflects the total count
   // of stored records the same as before. The other count represents the count of stored records
   // which fulfill the more strict validity checking which the SUR can now perform. We will crack
   // both numbers and use that which represents the selected strictness setting in SUR-SMS.
   // (the 'D=' value)
   //
   if (CFG_NVMshadow[cfg_cur][USERAWLOGS] == 0)  // use the 'decoded' detection count
   {
      mpx = strchr((char *)vdbp,':') ; // look for the colon
      if (*mpx == 0)    // points to terminating null, no colon
      {
         retVal = -2 ;
         VersionBailOut = TRUE;   // too bad
      }
      else
      {
         ++mpx ; // increment off of the colon
         //mpy = strstr((char *)mpx,LogRecPref) ;   // find the L=
         mpy = strstr((char *)mpx, (char *)LogRecPref) ;   // find the L=
   
         if (!mpy)    // null pointer means no string match
         {
            retVal = -3 ;
            VersionBailOut = TRUE;   // too bad
         }
         else
         {
            mpx = mpy + 2;  // skip the matched string
            vMsg_lrecCount = strtol((char *)mpx,0,10) ;    // crack the integer
         }

         //mpy = strstr((char *)mpx,DecRecPref) ;   // find the D=
         mpy = strstr((char *)mpx, (char *)DecRecPref) ;   // find the D=
   
         if (!mpy)    // null pointer means no string match
         {
            retVal = -4 ;
            VersionBailOut = TRUE;   // too bad
         }
         else
         {
            mpx = mpy + 2;  // skip the matched string
            vMsg_drecCount = strtol((char *)mpx,0,10) ;    // crack the integer
         }
      }
      
      if(VersionBailOut)  // some error happened
      {
#if __DEBUG_SURCOM
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "*** getSurVersAndRc() couldn\'t crack new-style record count: %d\r\n", retVal);
         }
#endif
         //                         12345678901234567890
         strcpy(DisplayLine2String,"BAD VERSION FORMAT  ");
         //sprintf(DisplayLine2String,"BADVFMT ");
         BigLoopMaintenance();
         return retVal;
      }
      else
      {
         vMsg_recCount = vMsg_drecCount;  // return the appropriate count
         //                          12345678901234567890
         sprintf(DisplayLine2String,"REC COUNT     %6Ld",vMsg_recCount);
         //sprintf(DisplayLine2String,"RC%6Ld",vMsg_recCount);
         BigLoopMaintenance();
      }
   }
   else  // use original message format ("raw logs")
   {
      mpx = strchr((char *)vdbp,':') ; // look for the colon
      if (*mpx == 0)    // points to terminating null, no colon
      {
         retVal = -2 ;
         VersionBailOut = TRUE;   // too bad
      }
      else
      {
         ++mpx ; // increment off of the colon
         mpy = strchr((char *)mpx,'\r') ;   // find the <cr>
   
         if (*mpy == 0)    // points to terminating null, no <cr>
         {
            retVal = -3 ;
            VersionBailOut = TRUE;   // too bad
         }
         else
         {
            *mpy = '\0';   // terminate string by writing nul over the <cr>
            vMsg_recCount = strtol((char *)mpx,0,10) ;    // crack the integer
         }
      }
   
      if(VersionBailOut)  // some error happened
      {
#if __DEBUG_SURCOM
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "*** getSurVersAndRc() couldn\'t crack old-style record count: %d\r\n", retVal);
         }
#endif
         //                         12345678901234567890
         strcpy(DisplayLine2String,"BAD VERSION FORMAT  ");
         //sprintf(DisplayLine2String,"BADVFMT ");
         BigLoopMaintenance();
         return retVal;
      }
      else
      {
         //                          12345678901234567890
         sprintf(DisplayLine2String,"REC COUNT     %6Ld",vMsg_recCount);
         //sprintf(DisplayLine2String,"RC%6Ld",vMsg_recCount);
         BigLoopMaintenance();
      }
   }
   
   //
   // if we make it this far, (have not returned with an error code)
   // then 'vMsg_recCount' holds the number of detection records present
   // in SUR at the current time. it is the cumulative total
   // of all the requests that the SUR holds.
   //
   // compute number of records which are new this run
   // using the global cells.
   //
   new_vMsg_rcount = vMsg_recCount - prev_vMsg_rcount ;
#if __DEBUG_SURCOM
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** getSurVersAndRc() new_vMsg_recCount:%Ld  vMsg_rcount:%Ld  prev_vMsg_rcount:%Ld\r\n", new_vMsg_rcount, vMsg_recCount, prev_vMsg_rcount);
   }
#endif
   prev_vMsg_rcount = vMsg_recCount ;   // update saved count to reflect current total for next time
   return (signed int8)new_vMsg_rcount;
}



//
// read byte from SUR EEPROM
//
// parameter is offset into EEPROM, 0..255
//
// function reply >= 0 is the byte value returned from SUR
//
// negative number indicates failure from any source
// (timeout, bad address, ???)
//
int16 getSurEepromByte(int16 addr)
{
   BYTE *cmdp;  // allocated small string data buffer in which to construct SUR command
   BYTE *resp;  // allocated small string data buffer in which to receive SUR reply
   BYTE *xptr;
   int16 retval;
   char msd, lsd;

   if (addr > 255)
      return -1;  // bad address specified

     cmdp = sballoc();  // allocate small string data buffer in which to construct SUR command
     resp = sballoc();  // allocate small string data buffer in which to receive SUR reply
   
   sprintf(cmdp, "w<%02X", (int)addr);  // construct the byte read command for the SUR
   
   if (SURCommandRawReply(cmdp, resp, SMALL_BUFFER_SIZE)) // store response in time data buffer
      {
         // data reply is AA:DD (or maybe <cr><lf>AA:DD
         // find the colon and decode the next two characters as hex
         xptr = strchr((char *)resp, ':');  // returns pointer to the colon or null
         if (xptr)
            {
               ++xptr;  // point to the data value
               msd = xptr[0];
               lsd = xptr[1];
               if ( (strlen((char *)xptr) >= 2)  // remaining string is long enough
                  && (isxdigit(msd))  // first char is hex digit
                  && (isxdigit(lsd)) )  // second char is hex digit
                  {
                     retval = decodeTwoHex((char *)xptr);  // crack the value
                     sbfree(cmdp);  // free the allocated buffers
                     sbfree(resp);
                     return retval;
                  }
               else
               {
                  sbfree(cmdp);  // free the allocated buffers
                  sbfree(resp);
                  return -1;  // bad format
               }
            }
         else
         {
            sbfree(cmdp);  // free the allocated buffers
            sbfree(resp);
            return -1;  // bad format
         }
      }
   else // some error occurred
   {
      sbfree(cmdp);  // free the allocated buffers
      sbfree(resp);
      return -1;  // bad format
   }
}





BOOLEAN checkCommAndRecover(BYTE *tdbp, int tdbpSize)  // return the string in this buffer
{
   //BYTE *CRcommand = "\r";
   BYTE CRcommand[] = {"\r"};
   BOOLEAN retval = FALSE;
   int i;

   if (SURCommandRawReply(CRcommand,tdbp,tdbpSize)) // just check, don't return response string
   {
      retval = TRUE ;  // passed first time
   }
   else // perform the re-synchronize and try checking again
   {
      for (i =0 ; i < 8 ; i++)
      {
         fputc('\r',SURport);  // always use port directly
      }
      delay_ms(2000);
      retval = SURCommandRawReply(CRcommand,tdbp,tdbpSize);   // success or failure indicated by this function return
   }
   return retval;  // return success or failure
}


//
// write byte to SUR EEPROM
//
// parameters are offset into EEPROM, 0..255; data for writing, 0..255
//
// function reply >= 0 is the byte value returned from SUR
//
// negative number indicates failure from any source
// (timeout, bad address, ???)
//
// SUR memory write command does not reply at all,
// so we just wait for timeout and give a good return.
//
BOOLEAN setSurEepromByte(int16 addr, int16 data)
{
   BYTE *cmdp;  // allocated small string data buffer in which to construct SUR command
   BYTE *resp;  // allocated small string data buffer in which to receive SUR reply

   if (addr > 255)
      return FALSE;  // bad address specified

   if (data > 255)
      return FALSE;  // bad data value specified

     cmdp = sballoc();  // allocate small string data buffer in which to construct SUR command
     resp = sballoc();  // allocate small string data buffer in which to receive SUR reply

   sprintf(cmdp, "w>%02X%02X", (int)addr, (int)data);  // construct the byte write command for the SUR
   
   SURCommandRawReply(cmdp, resp,SMALL_BUFFER_SIZE); // not really expecting a reply
   sbfree(cmdp);  // free the allocated buffers
   sbfree(resp);
   return TRUE;  // it's all good
}


#endif


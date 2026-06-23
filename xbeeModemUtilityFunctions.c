#ifndef MODEMFXNS_XBEE
#define MODEMFXNS_XBEE
//
// functions to perform odd, handy tasks for the Digi XBee 4G LTE cellular modem (SMS modem)
//
//
// Below is information from the XBEE 3G Global user manual describing the steps
// that occur as the device is connecting to cellular and data networks.
// Still correct for Digi XBEE 4G LTE cellular modem.
//
// **********************************************************************************************
// Connecting
// In normal operations, the XBee Cellular Modem automatically attempts both a cellular network
// connection and a data network connection on power-up. The sequence of these connections is as
// follows:
//
// Cellular network
// 1. The device powers on.
// 2. It looks for cellular towers.
// 3. It chooses a candidate tower with the strongest signal.
// 4. It negotiates a connection.
// 5. It completes cellular registration; the phone number and SMS are available.
//
// Data network connection
// 1. The network enables the evolved packet system (EPS) bearer with an access point name
// (APN). See AN (Access Point Name) if you have APN issues.
// 2. The device negotiates a data connection with the access point.
// 3. The device receives its IP configuration and address.
// 4. The AI (Association Indication) command now returns a 0 and the sockets become available.
// **********************************************************************************************
//
// This is relevant to our cell network usage in the following way:
// after step (5), the Association Indication reads as 0x23, indicating that the connection
// to the data network is still in process. However, the cell connection is OK at that point
// and we can proceed. This replaces the initial understanding in which we wait a long time
// for the Association Indication to become 0x00. We seldom wait long enough for that to 
// occur.
//
//

//
//
// silly function to wait for appropriate Association Status
// after initialization, and to strobe the LED while doing it
//
// we want a 10Hz blink with a strobe effect, so 40mS on/60mS off
BYTE const blinkTimeOn = 40;  // units are mS
BYTE const blinkTimeOff = 60;  // units are mS
//
// wait using a loop, checking Association Status
//
// we believe that once the association status becomes 0x00,
// the modem is ready for use. bail out of the status loop if that happens.
// also bail out if the time limit (default 45 seconds) is reached without 0x00 status.
//
// we want a 10Hz blink, so 40mS on/60mS off for 'strobe" effect
//
// keep reading the Association Indication status until time
// expires or the terminating status code is returned.
// the default terminating code of zero is what we want.
// so we will always use a zero AI status to exit the loop.
//
// we will use the LED to track the asssociated status.
// when the loop exits, if the status is 0x00, set the LED
// otherwise, clear the LED. ignore previous status from entry.
//
// returns FALSE if association was successful
// returns TRUE if association never attained
//
BOOLEAN xbeeWaitAndBlink(BYTE waitSecs = 45)
{
   static unsigned int32 tripCount = 0;  // counts calls to this function
   //static BYTE *aiStatusCommandAt = { "ATAI" };  // get association status
   static BYTE *aiStatusCommandAt = { "AI" };  // get association status
   BYTE assocStat = 0x99;  // bogus value allows tracking of changes
   BYTE prevAssocStat = 0x99;  // bogus value allows tracking of changes
   unsigned int32 limitTime;  // done when uptime reaches this value
   //int8 baseLed = input(LED);  // LED value at start, restore at end
   int32 monoTonic = 0;  // count up in this variable
   unsigned int32 readingCount = 0;  // association indication reading count up in this variable
   signed int16 x;  // conversionResult
   int32 inOneSecond = 0;
   int8 ledBit;
   BOOLEAN keepLooping = TRUE;
   BOOLEAN statusMatched = FALSE;
   BFATCR *reply;  // response from AT command function
   
   ++tripCount;  // note our passage
   
   if (waitSecs < 1)
      return TRUE;  // don't wait at all if parameter is zero, indicate association failed
   
//   if (waitSecs < 128)  // kludge for experiment: double the wait time
//      waitSecs *= 2;

#if __DEBUG_XBEE_API
   if (dbpEnabled(LEV3))
   {
      printf(hpo, "*** xbeeWaitAndBlink(%u) tc:%Lu  %s ***\r\n", waitSecs, tripCount, stringTheUptime());
   }
#endif

   
   limitTime = uptimeMilliseconds() + (waitSecs * 1000L);  // done when uptime reaches this value

   inOneSecond = uptimeMilliseconds() + 1000L;  // about a second from now

   do
   {
      ++monoTonic;
      ledBit = monoTonic & 1;  // use ls bit to control the LED
      output_bit(LED, ledBit);
      if(ledBit)  // 'ON' time interval
      {
         delay_ms(blinkTimeOn);  // kill time
      }
      else  // 'OFF' time interval
      {
         delay_ms(blinkTimeOff);  // kill time
      }
      
      // if status interval has elapsed, request the association indication status
      // also drive the LCD update.
      if (uptimeMilliseconds() > inOneSecond)
      {
         ++readingCount;

         // AT command function to get association status
         reply = xbeeSendAtCommand(aiStatusCommandAt);
         if (reply)  // non-null
         {
            if (reply->respbl)  // any strings in the response?
            {
               //x = convertHexForConfig(2, reply->respb);  // first string should have the value 
               x = convertHexForConfig(strlenb(reply->respb), reply->respb);  // first string should have the value 
               if ((x >= 0) && (x <= 255))  // number with appropriate value
               {
                  assocStat = x;  // snag this number
               }
               else  // bad format for some reason
               {
                  assocStat = 0x99;  // use this bogus number
               }
#if __DEBUG_XBEE_API
               if (dbpEnabled(LEV3))
               {
                  printf(dpo, "##### AT Association Indication [%d]:%s  0x%02X\r\n", reply->respbl, reply->respb, assocStat);
               }
#endif
               // if the AI value is zero, we can stop looping
               statusMatched = (assocStat == 0);  // bail out of loop if status matches terminating zero status code
            }
            else  // no strings (can this really happen?)
            {
               statusMatched = FALSE;
            }
            xbfree(reply);  // return allocated buffer to pool
         }
         else  // no result from the command
         {
            statusMatched = FALSE;
         }
         
         if (prevAssocStat != assocStat)  // did the value change?
         {
            prevAssocStat = assocStat;  // save changed value and display information about it
#if __DEBUG_XBEE_API
            if (dbpEnabled(LEV3))
            {
               printf(hpo, "*** xbeeWaitAndBlink() tc:%Lu loop:%u  assoc:0x%02X  %s***\r\n", tripCount, readingCount, assocStat, stringTheUptime());
            }
#endif
         }
         inOneSecond = uptimeMilliseconds() + 1000L;  // about a second from now
         BigLoopMaintenance();  // maintain clocks and LCD
      }
      keepLooping = uptimeMilliseconds() < limitTime;
   } while (!statusMatched && keepLooping);  // run loop while these are true
#if __DEBUG_XBEE_API
   if (dbpEnabled(LEV3))
   {
      if(statusMatched)
      {
         printf(hpo, "*** xbeeWaitAndBlink() tc:%Lu terminated, condition matched %s ***\r\n", tripCount, stringTheUptime());
      }
      else
      {
         printf(hpo, "*** xbeeWaitAndBlink() tc:%Lu terminated, loop limit reached %s ***\r\n", tripCount, stringTheUptime());
      }
   }
#endif

   // show via the onboard LED whether xbee modem is associated with cell network
//!   if (statusMatched)  // associated with cell network
//!   {
//!      output_high(LED);
//!      RegisteredToCellNetwork = TRUE;  // possibly useless 
//!   }
//!   else
//!   {
//!      output_low(LED);
//!      RegisteredToCellNetwork = FALSE;
//!   }
   
   return !statusMatched;  // return value is inverse of this setting, FALSE means success
}




//
// this is some kludge to assert pin-sleep mode at a different place in the code.
//

void xbeePutToSleep()
{
#if __IMPLEMENT_PIN_SLEEP
   if (isXbeeAwake())  // if not alread sleeping
   {
      xbeeApiSetPinSleepModePhonyCommand();
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** xbeePutToSleep() request  %s\r\n", stringTheDateTimeUptime());
      }
#endif
   }
#if __DEBUG_XBEE
   else  // modem is already asleep
   {
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** xbeePutToSleep() -- modem already sleeping  %s\r\n", stringTheDateTimeUptime());
      }
   }
#endif
#endif  // end of 'if __IMPLEMENT_PIN_SLEEP'
}



void xbeeWakeUp()
{
#if __IMPLEMENT_PIN_SLEEP
   if (isXbeeAsleep())  // if not already awake
   {
      output_low(PwKy);  // remove pin-sleep assertion
      // spin here until the awake signal is read
      do
      {
         delay_us(125);
      } while (isXbeeAsleep());
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** xbeeWakeUp() NOW  %s\r\n", stringTheDateTimeUptime());
      }
#endif
   }
#if __DEBUG_XBEE
   else  // modem is already awake
   {
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** xbeeWakeUp() -- modem already awake  %s\r\n", stringTheDateTimeUptime());
      }
   }
#endif
#endif  // end of 'if __IMPLEMENT_PIN_SLEEP'
}


//
// generate relatively unique 8-bit number for use as a frame ID.
// zero is not allowed, and we also choose to not use 255.
// so we cycle a counter through the range 1..254 repeatedly.
//
// while revamping the message handling, restrict the usable
// range of frame numbers to 1..31. (x1..x1F)
//
// if packet retry happens, the packet frame number gets 32 (x20) added to it.
// first retry is original frame + 32. (range x21..3F)
// second retry is 1st retry +32.  (range x41..5F)
//
BYTE nextFrameId()
{
   BYTE retval;
   static BYTE nxFrameId = 1;  // rolling code, 1..254, enables status reply. (0 disables)
   
   retval = nxFrameId++;  // arbitrary frame ID, increment for next time
   //if (nxFrameId > 254)
   if (nxFrameId > 31)
   {
      nxFrameId = 1;  // enforce range by rolling to 1
   }

   return retval;
}

//
// measure time from now until a character arrives from XBEE
// use the C function kbhit()
//
// return a count of milliseconds
//
// optionally,supply a maximum waiting time value
//
UW modemWaitTime(UW maxWaitTimeMs = 5000L)  // default wait is 5 seconds, maximum possible is 65535, make it 60000 for a minute
{
   U32 startTime = uptimeMilliseconds();
   U32 stopTime;
   
   if (maxWaitTimeMs < 10)
   {
      maxWaitTimeMs = 10;  // get real
   }
   else if (maxWaitTimeMs > 60000L)
   {
      maxWaitTimeMs = 60000L;  // don't wait all day
   }
   
   stopTime = startTime + maxWaitTimeMs;  // loop until a character arrives or this time is attained
   
   do
   {
      if (kbhit(SMSport))  // some data has arrived
      {
         break;  // end the loop
      }
      delay_ms(5);  // swag delay value
      BigLoopMaintenance();  // ensure clocks and LCD get updated
   } while (stopTime > uptimeMilliseconds());
   
   return uptimeMilliseconds() - startTime;  // this time span is what we want
}



//
// this function modified so it is used only for AT commands
// when the XBEE is not operating in API mode.
//
// change the interpretation of the duration parameter.
// instead of using it as an envelope limit for receiving characters,
// just wait at the start for that amount of time to allow in-process
// data to be received.
//
void drainSmsBufAndProcess(int16 durationMs=150)  // nominally process for 150ms
{
   char rch;
   int16 xc = 0;  // count the number of bytes we receive
   int16 xcpend;
   
   // wait until response reception begins
   waitXbeeDataStart();  // use default time
   
   xcpend = rcv_buffer_bytes(SMSport);  // number of bytes waiting
   
   if (xcpend == 0)  // no characters received
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         if (xc)
         {
            printf(dpo, "sms drain process NO characters\r\n");
         }
      }
#endif
      return;  // exit doing nothing
   }
   
   // wait until response reception concludes
   waitXbeeDataFinish();  // use default time
   
   while (kbhit(SMSport))  // for any buffered-up SMS bytes...
   {
      rch = fgetc(SMSport);  // ...read 'em and save 'em
      // if the character is the XBEE frame delimiter, set the flag and break out of this loop.
      // leave any remaining characters where they are.
      if (rch == XBEE_API_FRAME_DELIMITER)
      {
         XbeeSwallowedFrameDelimiter = TRUE;
#if __DEBUG_XBEE
         if (dbpEnabled(LEV3))
         {
            if (xc)
            {
               printf(dpo, "sms drain process set \"XbeeSwallowedFrameDelimiter\" pend:%d  %s\r\n", rcv_buffer_bytes(SMSport), stringTheDateTimeUptime());
            }
         }
#endif
         break;
      }
      stringsParse(rch, XBEEstrings);  // deliver character to parsing function
      ++xc;  // and count it
   }

#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      if (xc)
      {
         printf(dpo, "sms drain process %ld characters out of %ld\r\n", xc, xcpend);
      }
   }
#endif
}


//
// read any characters in SMS serial buffer, send to packets parse.
// attempt to wait until reception of message ends.
//
//!#if !__USING_SMSISR  // unnecessary if the interrupt stuff works
void drainSmsBufAndParse()
{
   char rch;
   int16 xc = 0;  // count the number of bytes we receive
   int16 xcpend = rcv_buffer_bytes(SMSport);  // number of bytes waiting
   int16 xcmax;  // max number we will save & print
   BYTE *xcbuf;  // pointer to allocated character buffer
   
   // wait until response reception begins unless there is a pushback character
   if (XbeeSwallowedFrameDelimiter)  // assumption is that a message has been in progress already
   {
      waitXbeeDataFinish();
   }
   else  // wait for data start, then for finish
   {
      waitXbeeDataStart();  // use default time
      if (xcpend == 0)  // nothing arrived
      {
         return;  // early exit if no waiting characters
      }
      waitXbeeDataFinish();
   }
   
   xcpend = rcv_buffer_bytes(SMSport);  // number of bytes waiting now
   
   // get buffer, size depends on present received count
   if (xcpend <= SMALL_BUFFER_SIZE)
   {
      xcmax = SMALL_BUFFER_SIZE;
      xcbuf = sballoc();
   }
   else  // small buffer is not big enough
   {
      xcmax = LARGE_BUFFER_SIZE;
      xcbuf = lballoc();
   }
   
   // process the message if there is any data
   if (XbeeSwallowedFrameDelimiter)  // note previously received frame delimiter
   {
      xcbuf[xc++] = XBEE_API_FRAME_DELIMITER;
      packetsParse(XBEE_API_FRAME_DELIMITER);  // supply the swallowed value
      XbeeSwallowedFrameDelimiter = FALSE;  // clear the flag
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         if (xc)
         {
            printf(dpo, "drainSmsBufAndParse() cleared \"XbeeSwallowedFrameDelimiter\" pend:%d  %s\r\n", rcv_buffer_bytes(SMSport), stringTheDateTimeUptime());
         }
      }
#endif   }

   while (kbhit(SMSport))  // for any buffered-up SMS bytes...
   {
      rch = fgetc(SMSport);  // ...read 'em, buffer 'em, and then parse 'em
      packetsParse(rch);
      if (xc < xcmax)
      {
         xcbuf[xc] = rch;
      }
      ++xc;  // also count 'em
   }

#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      if (xc)
      {
         printf(dpo, "sms drain parse %ld characters  %s\r\n", xc, stringTheDateTimeUptime());
         dumpToHost(xcbuf, xc);
      }
   }
#endif
   xbfree(xcbuf);
}
//!#endif


// read any characters in SMS serial buffer, don't save
//!#if !__USING_SMSISR  // unnecessary if the interrupt stuff works
void drainSmsBufAndTrash()
{
   char rch;
   int16 xc = 0;  // count the number of bytes we receive
   int16 xcpend = rcv_buffer_bytes(SMSport);  // number of bytes waiting
   int16 xcmax;  // max number we will save & print
   BYTE *xcbuf;  // pointer to allocated character buffer
   
   if (xcpend <= SMALL_BUFFER_SIZE)
   {
      xcmax = SMALL_BUFFER_SIZE;  // leave room for the null      xcbuf = sballoc();
      xcbuf = sballoc();
   }
   else  // small buffer is not big enough
   {
      xcmax = LARGE_BUFFER_SIZE;
      xcbuf = lballoc();
   }

   while (kbhit(SMSport))  // for any buffered-up SMS bytes...
   {
      rch = fgetc(SMSport);  // ...read 'em, buffer 'em, and then forget 'em
      if (xc < xcmax)
      {
         xcbuf[xc] = rch;
      }
      ++xc;  // also count 'em
   }

#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      if (xc)
      {
         printf(dpo, "sms drain trash %ld characters\r\n", xc);
         dumpToHost(xcbuf, xc);
      }
   }
#endif
   xbfree(xcbuf);
}
//!#endif



//
// function to wait out the reception of serial data from the XBEE modem.
//
// called when the received buffer byte count is zero.
// monitors that count over a time duration and notes whether it is
// any data is received. once data starts being received, it returns.
//
// assumption is that once a message is in process, characters will arrive
// at essentially baud rate frequency until the transmission is complete.
// so if our sampling interval is greater than that, once the count stops
// changing then the message is over.
//
// default time duration is 750mS. may be overridden.
//
void waitXbeeDataStart(int16 durationMs = 750)
{
   U32 timeThen;
   
   if (durationMs < 100)
   {
      durationMs = 100;  // get real
   }
   else if (durationMs > 2500L)
   {
      durationMs = 2500L;  // don't wait all day
   }
   
   timeThen = uptimeMilliseconds() + durationMs;  // clock reading when the interval is over
   
   // character rate is approximately one per millisecondsecond.
   while (uptimeMilliseconds() < timeThen)
   {
      delay_ms(5);  // wait 5 character times
      BigLoopMaintenance();  // ensure clocks and LCD get updated
      if (rcv_buffer_bytes(SMSport))  // number of bytes waiting now is non-zero
      {
         break;  // exit loop
      }
   }
   
}


//
// function to wait out the reception of serial data from the XBEE modem.
//
// called when the received buffer byte count is non-zero.
// monitors that count over a time duration and notes whether it is
// increasing or not. once data stops being received, it returns.
//
// assumption is that once a message is in process, characters will arrive
// at essentially baud rate frequency until the transmission is complete.
// so if our sampling interval is greater than that, once the count stops
// changing then the message is over.
//
// default time duration is 250mS. may be overridden.
//
// if function is called with no data pending, it will return in 5 mS.
//
void waitXbeeDataFinish(int16 durationMs = 250)
{
   int16 xcpend = rcv_buffer_bytes(SMSport);  // number of bytes waiting, should not be zero
   int16 xcpendLast = xcpend;
   U32 timeThen;
   
   if (durationMs < 10)
   {
      durationMs = 10;  // get real
   }
   else if (durationMs > 2500L)
   {
      durationMs = 2500L;  // don't wait all day
   }
   
   timeThen = uptimeMilliseconds() + durationMs;  // clock reading when the interval is over
   
   // character rate is approximately one per millisecondsecond.
   while (uptimeMilliseconds() < timeThen)
   {
      delay_ms(5);  // wait 5 character times
      BigLoopMaintenance();  // ensure clocks and LCD get updated
      xcpend = rcv_buffer_bytes(SMSport);  // number of bytes waiting now
      if (xcpend == xcpendLast)  // count unchanged
      {
         break;  // exit loop
      }
   }
   
}


//
// utility function to compute a checksum value for
// XBee API frames. sum is computed over a span of bytes, 
// the lower 8 bits are isolated and that 8-bit value subtracted
// from 0xFF to produce the return
//
BYTE xbeeChecksum(BYTE *values, BYTE count)
{
   unsigned int16 accum = 0;  // compute sum here
   
   for (int i = 0 ; i < count ; i++)
   {
      accum += values[i];
   }
      
   return (0xFF - (accum & 0xFF));
}


//
// management of the data stream read from the SMS modem in AT command mode.
//
// data is read character-by-character and parsed into strings according
// to an arbitrary set of rules.
//
// modem response may be one or several strings, delimited by cr-lf pairs.
// we assemble strings of printable ascii only, terminated by zero bytes.
// strings which are "expected" such as those in response to issued commands
// are storedin the strings buffer "smsModemBuf". don't worry about sizes right now.
//
// count the number of strings decoded, and the total number of characters stored.
//



//
// search array of strings for match to input string
//
// return index of string where first matched,
// otherwise negative number to indicate never matched.
//
// I want to use this to search for multiple hits in the same vector,
// so the optional last parameter provides a starting subscript into the array.
//
// the previous passed parameters "stvec" and "numstvec" are removed, and the
// static-allocated variables "rstrPtr" and "rstrPtrCt" are used instead.
//
signed int16 didStringMatch(BYTE *teststr, int startHere=0)
{
   if (XBEEstrings.rstrPtrCt == 0)
      return -1;
   if (startHere >= XBEEstrings.rstrPtrCt)
      return -1;
      
   for (int i = startHere ; i < XBEEstrings.rstrPtrCt ; i++)
      {
#if __DEBUG_XBEE
         if (dbpEnabled(LEV4))
            {
               printf(dpo, "LOOKING FOR |%s| IN |%s| %d\r\n", teststr, XBEEstrings.rstrPtr[i], i);
            }
#endif
         if (strstr(XBEEstrings.rstrPtr[i], (char *)teststr))
            return i;
      }

   return -1;
}





//
// these functions read the sleep status signal from the
// XBEE modem. this is an input signal so it may be read
// using the input() function.
// the signal sense is SLEEP/ so it is low when sleeping
//
BOOLEAN isXbeeAwake()
{
   return input(PS);  // return direct reading of the sleep status pin on XBee
}

BOOLEAN isXbeeAsleep()
{
   return ~input(PS);  // return inverted reading of the sleep status pin on XBee
}


//
// function to determine whether XBEE modem hardware is usable
//
// trivial now, allows for future changes
//
BOOLEAN isXbeeOkToUse()
{
   return !(xbeeFailedReset || xbeeFailedConfiguration);
}


//
// power consumption of the XBee modem is lowered using "pin sleep" mode:
//

//
// set flags to control modem low-power operation
// based upon the configured value.
//
// need to keep this a separate function, called from different places
//
void xbeeSetPowerControlFlags()
{
   //mlp_code lowPowerCode = (mlp_code)CFG_NVMshadow[cfg_cur][XBEE_LOPWR];
   mlp_code lowPowerCode = mlpAIRPL;  // IGNORE CONFIGURED VALUE, USE AIRPLANE MODE ALWAYS

   
   switch (lowPowerCode)  // action depends on selected low power mode value
   {
      case mlpSLEEP:
#if __IMPLEMENT_PIN_SLEEP
         // for sleep mode, select both pin sleep and airplane mode
         xbeeUsingPinSleep = TRUE;
         xbeeUsingAirplaneMode = TRUE;
         break;

      // else promote to airplane mode by falling into case below
#endif
      case mlpAIRPL:
         // for airplane mode, select that, deselect pin sleep
         xbeeUsingPinSleep = FALSE;
         xbeeUsingAirplaneMode = TRUE;
         break;
         
      default:
#if __DEBUG_XBEE
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "Invalid Modem Low Power configured value: %d, Full Power used.\r\n", lowPowerCode);
         }
#endif
         // fall into full power case
      case mlpFULL:
         // for full power, deselect both flags
         xbeeUsingPinSleep = FALSE;
         xbeeUsingAirplaneMode = FALSE;
         break;
   }

}


#if __IMPLEMENT_PIN_SLEEP
//
// send an AT-style phony command to command queue using phony API packet 0xDD
//
// use the query form of the frame where the length is 4 and there is no argument.
//
// "dead time" is an enforced period of no transmission to XBEE modem,
// to permit responses time to arrive. default is 2000ms (2 seconds), may be overridden.
//
void xbeeApiSetPinSleepModePhonyCommand(U32 setDeadTime=XBEE_DEAD_TIME_DEFAULT)
{
   int ilen = 2;  // kludge
   int framelen = ilen + 2;  // easy way to get this
   APBU *localPktPtr = 0;

   if (!XBeeApiIsActive)
   {
#if __DEBUG_XBEE_API
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "PIN SLEEP COMMAND FAIL, API NOT ACTIVE\r\n");
      }
#endif
      return;  // early bail out
   }

   // fetch a buffer in which to construct command packet
   // any of these may be contained in a small buffer, so use one of those
   localPktPtr = (APBU *)sballoc();
   if (!localPktPtr)  // no buffer available
   {
#if __DEBUG_XBEE_API
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "PIN SLEEP COMMAND FAIL, NO PACKET BUFFER\r\n");
      }
#endif
      return;  // no recovery from this condition
   }
   
   // construct packet.
   // we can always use the structure for a command with a parameter. 'FMATCP'
   
   localPktPtr->fmatcp.delim = XBEE_API_FRAME_DELIMITER;  // frame begins
   localPktPtr->fmatcp.frametype = ftPH_SLEEP;  // send phony command frame type for sleep assertion
   localPktPtr->fmatcp.frameid = 0;  // don't need one
   localPktPtr->fmatcp.flength_msb = 0;  // set length in big-endian format
   localPktPtr->fmatcp.flength_lsb = framelen;  // vector length plus frametype plus frame ID
   
   // set the enforced dead time following the transmission
   localPktPtr->fmatcp.deadTime = setDeadTime;
      
   wordqEnqueue(apiXmitQueue, localPktPtr);  // and queue up the packet for transmission.

   return;
}
#endif

  
//
// send AP command to read instantaneous RSSI value
//
// cannot request this if XBEE modem is not active.
// return silently if that error occurs.
// value error or out of range causes value 99 to be set
//
// until shown that we need to do something else,
// this function will not return a value related to success or failure.
//
// cached RSSI works real well.
// select the instantaneous value and see how that does
//
void xbeeGetRssi()
{
   //static BYTE *dbNowStatusCommandAt = { "ATDB1" };  // instantaneous Cellular Signal Stringth
   static BYTE *dbNowStatusCommandAt = { "DB1" };  // instantaneous Cellular Signal Stringth  AT auto-inserted
   BFATCR *reply;  // response from AT RSSI function
   signed int16 x;  // conversionResult

   // AT command function
   reply = xbeeSendAtCommand(dbNowStatusCommandAt);
   if (reply)
   {
#if __DEBUG_XBEE_API
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "##### AT RSSI [%d]:%s\r\n", reply->respbl, reply->respb);
      }
#endif
      x = convertHexForConfig(2, reply->respb);
      // valid RSSI absolute values are 51..113 (x33..x71)
      if ((x < 0x33) || (x > 0x71))  // bad number
      {
#if __DEBUG_XBEE_API
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "*** RSSI fail, bad value 0x%02X\r\n", x);
         }
#endif
         x = 0x99;  // special fail code
      }
#if __DEBUG_XBEE_API
      else  // good response
      {
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "*** RSSI good response %d[0x%02X]\r\n", x, x);
         }
      }
#endif
      // save 'x' as current RSSI in global cell. also save uptime value.
      savedRSSI = x;
      savedRSSI_At = uptimeMilliseconds();
      xbfree(reply);
   }
#if __DEBUG_XBEE_API
   else
   {
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "##### AT RSSI NULL REPLY\r\n");
      }
   }
#endif

}
  
  
   
//
// function to command Airplane Mode on or off using AP command
//
// in our usage of the modem, airplane mode is selected prior to commanding
// modem sleep with a pin signal. airplane mode will elegantly disconnect from
// the cellular system, which Digi advises be done before sleeping. they
// recommend giving the XBEE 30 seconds once airplane mode is entered in order
// for it to finish its disengagement before putting the modem to sleep.
//
void xbeeAirplaneModeControl(int val)  // zero means normal mode, <>0 means airplane mode
{
   static BYTE *airplaneModeCommandAt[2] = { "AM0", "AM1" };  // AT auto-inserted
   BFATCR *reply;  // response from AT AM function
   
#if __DEBUG_XBEE_API
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** xbeeAirplaneModeControl(%d) %s ***\r\n", val, stringTheDateTimeUptime());
   }
#endif

   // AT command function
   if (val != 0)  // 0 and 1 are the only valid values
   {
      val = 1;
   }
   reply = xbeeSendAtCommand(airplaneModeCommandAt[val]);  // reply value is an allocated buffer
   if (reply)
   {
#if __DEBUG_XBEE_API
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "##### %s [%d]:%s\r\n", airplaneModeCommandAt[val], reply->respbl, reply->respb);
      }
#endif
      xbfree(reply);  // return the allocated buffer
   }
   else
   {
      printf(dpo, "##### %s NULL REPLY\r\n", airplaneModeCommandAt[val]);
   }
   
   // if we have transitioned from airplane mode to normal mode,
   // kill 30 seconds. 
   if (val == 0)  // airplane mode to normal mode
   {
      WaitWithHousekeeping(30);
   }

}

  
#endif


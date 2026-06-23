#ifndef APIFXNS_XBEE
#define APIFXNS_XBEE
//
// functions to perform odd, handy tasks for the Digi XBee 4G LTE cellular modem (SMS modem)
//



// DialNum is ascii null-terminated string holding the phone number to which the message is sent
// MessageText is null-terminated text of the message
// if message is longer than 155 characters, truncate
// length maximum had been 160 bytes, but we need 5 to encode RSSI
//
// "dead time" is an enforced period of no transmission to XBEE modem,
// to permit responses time to arrive. default is 2000ms (2 seconds), may be overridden.
//
void xbeeApiSendSMSmessage(BYTE *DialNum, BYTE *MessageText, int dline = 0)  // if 'dline' is 3 or 4, generate display message with RSSI and phone number
{
   // construct API SMS message sending packet and transmit it
   APBU *localPktPtr = 0;
   BYTE myFrameId = nextFrameId();  // for matching the response
   BYTE nlen = strlenb(DialNum);
   BYTE mlen = strlenb(MessageText);
   BYTE *dlptr;  // display line image pointer
   BYTE *lineimage;  // pointer to small buffer in which to construct the display message
   
   if (dline == 3)
   {
      dlptr = DisplayLine3String;
   }
   else if (dline == 4)
   {
      dlptr = DisplayLine4String;
   }
   else
   {
      dlptr = 0;  // for sure
   }
   
   // modem must be awake now

   // phone number string cannot be longer than 20 bytes, just truncate length if it is longer
   // message text string cannot be longer than 160 bytes, just truncate length if it is longer
   if (nlen > 20)
   {
      nlen = 20;
      DialNum[nlen] = 0;  // add the null terminator to the source string!!!
   }
   if (mlen > 155)  // leave 5 characters in which to encode RSSI
   {
      mlen = 155;
      MessageText[mlen] = 0;  // add the null terminator to the source string!!!
   }
  
   // fetch a buffer in which to construct SMS text message packet
   localPktPtr = (APBU *)lballoc();
   if (!localPktPtr)  // no buffer available
   {
#if __DEBUG_XBEE_API
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "new SMS message no buffer\r\n");
      }
#endif
      // this is an unrecoverable condition, ignore the packet
      return;  // RetVal;
   }
   
   // fill in packet fields
   localPktPtr->fmtxsms.delim = XBEE_API_FRAME_DELIMITER;  // frame begins
   localPktPtr->fmtxsms.frametype = ftFMTXSMS;  // send SMS text message
   localPktPtr->fmtxsms.frameid = myFrameId;  // use it here and to match response
   localPktPtr->fmtxsms.options = 0;  // reserved for future use, set to zero here
   // fill the phone number field with zeroes, then copy input number there
   memset(localPktPtr->fmtxsms.phnum, 0, sizeof(localPktPtr->fmtxsms.phnum));  // bah, it's 20
   memcpy(localPktPtr->fmtxsms.phnum, DialNum, nlen);  // explicit copy of message text into packet field 
   // fill the phone number field with zeroes, then copy input number there
   memset(localPktPtr->fmtxsms.mtxt, 0, sizeof(localPktPtr->fmtxsms.mtxt));
   memcpy(localPktPtr->fmtxsms.mtxt, MessageText, mlen);  // explicit copy of message text into packet field 

   // now need to compute total packet message length and set it, also compute checksum and set that.
   // variable portion of packet length begins at frametype and ends before the checksum byte.
   // so that length is:
   //   1 frametype
   //   1 frame id
   //   1 options
   //  20 phone number
   //     plus the length of the message text
   //
   //  23 + mlen
   //
   // frame cannot be longer than around 187 bytes so only the lsb will have significance.
   //

   localPktPtr->fmtxsms.flength_msb = 0;
   localPktPtr->fmtxsms.flength_lsb = mlen + 23;
   
   // set the enforced dead time following the transmission
   localPktPtr->fmtxsms.deadTime = XBEE_DEAD_TIME_DEFAULT;
   
   // if this is a detection report, the optional parameter will be 3 or 4
   // form a string with the RSSI value and destination phone number from the input data
   // and display it on line 3 or 4 of the LCD
   
   if (dlptr)  // is destination string pointer valid?
   {
      lineimage = sballoc();  // get a small buffer in which to construct the display message
      fillBytes(lineimage, 20, ' ', TRUE);  // create a 20-character blank-filled string
      // copy the RSSI from MessageText[] to the first 3 positions of display line message
      lineimage[0] = MessageText[6];
      lineimage[1] = MessageText[7];
      lineimage[2] = MessageText[8];
      lineimage[3] = '>';  // delimiter for phone number
      // add the destination phone number to the display message. max 16 chars
      if (nlen > 16)
      {
         nlen = 16;
      }
      memcpy(lineimage+4, DialNum, nlen);
      strcpy(dlptr, lineimage);  // copy finished display message to appropriate display line buffer
//!#if __DEBUG_MISC
//!      if (dbpEnabled(LEV3))
//!      {
//!         printf(dpo, "\r\n*** xbeeApiSendSMSmessage()\r\n***   lineimage>%s<\r\n***   dlptr>%s<\r\n", lineimage, dlptr);
//!      }
//!#endif
      sbfree(lineimage);  // deallocate small buffer
   }
   
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_SMS || __DEBUG_MISC
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "\r\n*** xbeeApiSendSMSmessage([%u]:\"%s\", [%u]:\"%s\") %s\r\n", nlen, DialNum, mlen, MessageText, stringTheDateTimeUptime());
   }
#endif

   wordqEnqueue(apiXmitQueue, localPktPtr);  // and queue up the packet for transmission.
}




//
// send an AT command to XBee modem using API packet 0x08
//
// there are two different forms of the AT command API packet.
// one is used when a value is queried, the other when a value is set.
//
// experience has shown that the form of this command which we use has
// a two-character command, possibly followed by a single character value.
// selection is made based upon the presence or absence of a third character
// in the input ASCII string.
//
// input string examples: "AI" "DT1"
//
// first two bytes are the command string, chosen from the ASCII 'AT' command set.
// third byte is an ASCII digit representing the parameter value to be set.
// the API allows for longer parameters in a variety of formats but the only
// form we use is the single-byte binary value which we decode from the ASCII digit. (0..9 only)
//
// that makes the length always 4 or 5.
//
// returns the status of the AT command, or '99' if some error occurred
// command string is two or 3 ASCII characters
//
// the response item and its length are passed in some fixed memory locations
//
// "dead time" is an enforced period of no transmission to XBEE modem,
// to permit responses time to arrive. default is 2000ms (2 seconds), may be overridden.
//
BYTE xbeeApiSendAtCommand(BYTE *cmdstr, U32 setDeadTime=XBEE_DEAD_TIME_DEFAULT)
{
   int ilen = strlenb(cmdstr);  // custom function
   int framelen = ilen + 2;  // easy way to get this
   APBU *localPktPtr = 0;
   BYTE retval = 0x99;  // default to special failure code
   BYTE myFrameId = nextFrameId();  // for matching the response
   BYTE commandParameter;

//!   if (!XBeeApiIsActive)
//!   {
//!#if __DEBUG_XBEE_API
//!      if (dbpEnabled(LEV3))
//!      {
//!         printf(dpo, "AT COMMAND FAIL, API NOT ACTIVE\r\n");
//!      }
//!#endif
//!      return retval;  // early bail out, send special fail code
//!   }

   if ((ilen < 2) || (ilen > 3))  // command string suitable length check
   {
#if __DEBUG_XBEE_API
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "AT COMMAND FAIL, BAD FORMAT 1\r\n");
      }
#endif
      return retval;  // early bail out, send special fail code
   }

   if (ilen == 3)  // command string parameter value check
   {
      if (isdigit(cmdstr[2]))  // must be ASCII 0..9
      {
         commandParameter = cmdstr[2] - '0';  // convert ASCII to binary
      }
      else  // bad parameter value
      {
#if __DEBUG_XBEE_API
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "AT COMMAND FAIL, BAD FORMAT 2\r\n");
         }
#endif
         return retval;  // early bail out, send special fail code
      }
   }
   
   // fetch a buffer in which to construct AT command packet
   // any of these may be contained in a small buffer, so use one of those
   //localPktPtr = (APBU *)lballoc();
   localPktPtr = (APBU *)sballoc();
   if (!localPktPtr)  // no buffer available
   {
#if __DEBUG_XBEE_API
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "AT COMMAND FAIL, NO PACKET BUFFER\r\n");
      }
#endif
      return retval;  // no recovery from this condition
   }
   
   // construct packet. 'framelen' will be either 4 or 5, depending on the presence of a parameter value
   // we can always use the structure for a command with a parameter. if we set the frame length correctly,
   // the packet sender will form and store the checksum correctly
   
   localPktPtr->fmatcp.delim = XBEE_API_FRAME_DELIMITER;  // frame begins
   localPktPtr->fmatcp.frametype = 0x08;  // send AT command frame
   localPktPtr->fmatcp.frameid = myFrameId;  // use it here and to match response
   localPktPtr->fmatcp.flength_msb = 0;  // set length in big-endian format
   localPktPtr->fmatcp.flength_lsb = framelen;  // vector length plus frametype plus frame ID
   localPktPtr->fmatcp.command[0] = cmdstr[0];
   localPktPtr->fmatcp.command[1] = cmdstr[1];
   if (framelen == 5)  // include parameter value
   {
      localPktPtr->fmatcp.cmdvalue = commandParameter;  // value for setting
   }
   
   // set the enforced dead time following the transmission
   localPktPtr->fmatcp.deadTime = setDeadTime;
      
   wordqEnqueue(apiXmitQueue, localPktPtr);  // and queue up the packet for transmission.

   return retval;
}



//
// send API packet to read instantaneous RSSI value
//
// cannot request this if API is not active, or if XBEE modem is not active.
// return silently if that error occurs.
//
// cached RSSI works real well.
// select the instantaneous value and see how that does
//
void xbeeApiGetRssi()
{
   static BYTE *dbNowStatusCommand = { "DB1" };  // instantaneous Cellular Signal Stringth

   //if (XBeeApiIsActive)
   //{
      xbeeApiSendAtCommand(dbNowStatusCommand);  // ask for fresh copy of RSSI
   //}
//!#if __DEBUG_XBEE_API
//!   else  // unable to send this request
//!   {
//!      if (dbpEnabled(LEV3))
//!      {
//!         printf(dpo, "*** xbeeApiGetRssi() fail, device (%c) or api (%c) inactive\r\n", (XbeeModemIsActive) ? 'A' : 'I', (XBeeApiIsActive) ? 'A' : 'I');
//!         //printf(dpo, "*** xbeeApiGetRssi() fail, api inactive\r\n");
//!      }
//!   }
//!#endif

}
  
  
  
   
//
// function to command Airplane Mode on or off using API
//
// in our usage of the modem, airplane mode is selected prior to commanding
// modem sleep with a pin signal. airplane mode will elegantly disconnect from
// the cellular system, which Digi advises be done before sleeping. they
// recommend giving the XBEE 30 seconds once airplane mode is entered in order
// for it to finish its disengagement before putting the modem to sleep.
//
void xbeeApiAirplaneModeControl(int val)  // zero means normal mode, <>0 means airplane mode
{
   static BYTE *airplaneModeOffCommand = { "AM0" };
   static BYTE *airplaneModeOnCommand = { "AM1" };
   BOOLEAN InvokingAirplaneMode;
   
#if __DEBUG_XBEE_API
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** xbeeApiAirplaneModeControl(%d) %s ***\r\n", val, stringTheDateTimeUptime());
   }
#endif

   InvokingAirplaneMode = val != 0;
   if (InvokingAirplaneMode)
   {
      xbeeApiSendAtCommand(airplaneModeOnCommand, 30000L);     // send the command string, need 30 seconds dead time
   }
   else
   {
      xbeeApiSendAtCommand(airplaneModeOffCommand, 30000L);     // send the command string, need 30 seconds dead time
   }

}


//
// XBee modem is actually always under power and configured to use API mode.
//
// if the modem has been put to sleep to conserve power, awaken it here.
//
// if the modem has been placed into 'Airplane Mode' to conserve power,
// invoke normal mode here.
// when awoken, a timer is started to put it back to sleep sometime later.
// That is done by a portion of the big loop, so there is no longer a necessity
// for explicit shutdown requestsby the reporting functions, etc.
//
// newer idea, let the reporting functions request the deactivation
// so remove that logic from this function.
//
void appStartUpModem()
{
#if __DEBUG_XBEE
   U32 startingTime = uptimeMilliseconds();  // save time now
   U32 timeAfterReadying;
#endif

#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "\r\n*** appStartUpModem()  %s\r\n", stringTheDateTimeUptime());
   }
#endif
   
   //
   // if modem is sleeping, awaken it with the pin signal.
   //
   xbeeWakeUp();  // wake modem from sleep. function will check whether it's apprpriate
  
      
   //
   // if modem is in airplane mode (inactive) now,
   // issue command to get XBEE modem back to normal mode
   // any required delay is handled in the called function
   //
   // use this point in the code to clear lines 3 & 4 of the LCD.
   // they will be rewritten once new SMS messages go out.
   //
   if (xbeeUsingAirplaneMode)  // instead of reading status, infer it from this boolean
   {
      fillBytes(DisplayLine3String, 20, ' ', TRUE);  // blank display line 3
      fillBytes(DisplayLine4String, 20, ' ', TRUE);  // blank display line 4
      maintainLCD(FALSE);  // update display, leaving top line alone
#if 0  // old way
      // fire off command to end airplane mode
      xbeeApiAirplaneModeControl(0);  // command normal mode
#else  // new way
      // fire off command to end airplane mode
      xbeeAirplaneModeControl(0);  // command normal mode
#endif
      
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** appStartUpModem(), out of airplane mode  %s\r\n", stringTheDateTimeUptime());
      }
#endif
      
      xbeeWaitAndBlink();  // wait for association with network to occur
   }
   
#if 0  // old way
   // fire off command to fetch RSSI, do not wait for response
   xbeeApiGetRssi();
#else  // new way
   // fire off command to fetch RSSI and save in designated cells
   xbeeGetRssi();
#endif

   // set status indicating modem is up
   setModemActive();
   resetPacketsParse();  // unrecoverable packet parse after this
   
#if __DEBUG_XBEE
   timeAfterReadying = uptimeMilliseconds();  // save time now
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** appStartUpModem(), %Lu ms consumed  %s\r\n", 
         (timeAfterReadying - startingTime), stringTheDateTimeUptime());
   }
#endif
}


//
// deactivation function to lower the power consumption of the XBee modem when it is
// not in use for communications.
//
// uses flags to select low power via pin-sleep with Airplane Mode,
// Airplane Mode alone, or no power reduction; in which case full power is consumed at all times.
//
// if in full-power operation, continue to toggle the LED and the simulated activity flag
//
void appShutDownModem_Inner()  // called by big loop to shutdown once activity is finished
{
   //
   // if low power mode includes airplane mode:
   // issue command to get XBEE modem into that mode
   // any required delay is handled in the called function.
   //
   if (xbeeUsingAirplaneMode)
   {
#if 0  // old way
      // fire off command to invoke airplane mode
      xbeeApiAirplaneModeControl(1);  // command airplane mode
#else  // new way
      // fire off command to invoke airplane mode
      xbeeAirplaneModeControl(1);  // command airplane mode
#endif
   }
      

#if __IMPLEMENT_PIN_SLEEP
   //
   // if low power mode includes pin sleep:
   // assert the pin sleep signal.
   // any required delay associated with airplane mode is handled in the called function.
   //
   if (xbeeUsingPinSleep)
   {
      xbeePutToSleep();
   }
#endif
   
   // set status indicating modem is down
   clearModemActive();
#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "XBEE MODEM DEACTIVATED %s\r\n", stringTheDateTimeUptime());
   }
#endif
   resetPacketsParse();  // unrecoverable packet parse after this
   
   
#if __DEBUG_XBEE
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** appShutDownModem() INNER  %s\r\n", stringTheDateTimeUptime());
   }
#endif
}


//
// check whether there is activity ongoing,
// if there is set flag for big loop later on.
// if nothing is happening, do it now.
//
void appShutDownModem()
{
   int i,j,k;
   
   i = wordqCount(apiXmitQueue);
   j = wordqCount(smsPktDispQueue);
   k = wordqCount(smsStatMatchQueue);
   if ((i) || (j) || (k))  // some packets queued
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** appShutDownModem() DEFERRED apiXmitQueue:%d  smsPktDispQueue:%d  smsStatMatchQueue:%d  %s\r\n", i, j, k, stringTheDateTimeUptime());
      }
#endif
      XbeeShutDownModem = TRUE;  // big loop will do this once all the queues are empty
   }
   else  // nothing going on, shut xbee down now
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** appShutDownModem() IMMEDIATE %s\r\n", stringTheDateTimeUptime());
      }
#endif
      appShutDownModem_Inner();
   }
   
   
}




#endif

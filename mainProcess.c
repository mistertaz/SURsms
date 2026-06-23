#ifndef MAINPROCESS_C
#define MAINPROCESS_C

   // %%%%%%%%%%%%%%%%%%%%%%%%
   // %%%%%%%%%%%%%%%%%%%%%%%%
   //
   // the big loop
   //
   // job of the big loop is to assess new animal detections from the SUR
   // and to inform the recipient of that by sending SMS text message
   //
   // details of message occur elsewhere in this program
   //
   // while in development, we will use the pushbutton to instigate
   // a health report.
   //

void process()
{
   BYTE xch;  // scratch character storage

   while (TRUE)  // never-ending loop
   {
      if (needTickTask)
      {
         tickTask();
         needTickTask = FALSE;  // this may be too simplistic but run with it for now
      }
      // every-loop instrumentation
      //BigLoopInstrumentation();  // not much value. quiescent big loop time is ~80uS.
      
      // every-loop housekeeping
      BigLoopMaintenance();
      
      // kludge up flashing the LED at 2Hz to indicate power cycle needed
      if (!isXbeeOkToUse())  // app is crippled because XBEE modem failed to initialize correctly
      {
         if (FlagHalfHz)
         {
            output_high(LED);
         }
         else
         {
            output_low(LED);
         }
      }
      
      // special processing for hour, minute,or second rollover to zero

      // check day rollover first. if that happens, hour and minute will also have rolled over
      if (HourZeroCrossing)  // hours roll to zero, it's a new day
      {
         // nothing to do here right now
         HourZeroCrossing = FALSE;  // clear the flag
      }

      // check hour rollover next. if that happens, minute will also have rolled over
      if (MinuteZeroCrossing)  // minutes roll to zero, it's a new hour
      {
         // nothing to do here right now
         MinuteZeroCrossing = FALSE;  // clear the flag
      }

      // check minute rollover next.
      // for each new minute, set individual
      // flags indicating which actions should occur in this pass of the BigLoop
      //
      // new logic:
      // also detect the minute *before* we take the action.
      // at that time, wake up the modem from pin sleep and let it stabilize in
      // airplane mode for the next minute.
      //
      if (SecondZeroCrossing)  // seconds roll to zero, it's a new minute
      {
         SecondZeroCrossing = FALSE;  // clear the flag
         
         // health report produced at the same fixed minute of any hour in which it is indicated
         NeedTheHealthReport = (MinuteBin == HealthReportMinuteMatch) && CheckNeedHealthReportThisHour(HourBin);
         ConditionalNeedTheHealthReport = FALSE;  // don't need this very often

         // detection report produced in each hour, at varying minute points
         NeedTheDetectionReport = CheckNeedDetectionReportThisMinute(MinuteBin);

         // if it should occur that both the detection report and the health report
         // are requested in the same minute, make the health report conditional
         // on the absence of a detection report.
         if (NeedTheHealthReport && NeedTheDetectionReport)
         {
            NeedTheHealthReport = FALSE;
            ConditionalNeedTheHealthReport = TRUE;
         }
         
         // anticipating modem usage seems to be disused, get rid of 'next minute' items
//!         // compute *next* minute items
//!         NextMinuteBin = (MinuteBin + 1) % 60;
//!         
//!         NeedTheHealthReportNextMinute = (NextMinuteBin == HealthReportMinuteMatch) && CheckNeedHealthReportThisHour(HourBin);
//!         NeedTheDetectionReportNextMinute = CheckNeedDetectionReportThisMinute(NextMinuteBin);
//!
//!         NeedModemNextMinute = NeedTheHealthReportNextMinute || NeedTheDetectionReportNextMinute;
//!
//!         // if modem is possibly used in the following minute, and early startup is configured, ready it now
//!         if (NeedModemNextMinute && CFG_NVMshadow[cfg_cur][MDM_EARLY])
//!         {
//!            NeedModemNextMinute = FALSE;  // only need it once
//!            appStartUpModem();  // if appropriate, wake modem from sleep, exit airplane mode
//!#if __DEBUG_MISC
//!            if (dbpEnabled(LEV3))
//!            {
//!               printf(dpo, "*** Modem Awakened @ MinuteBin:%u  %s\r\n", MinuteBin, stringTheDateTimeUptime());
//!            }
//!#endif
//!         }  // end of 'need modem next minute'
            
      }  // end of 'it's a new minute'
      
      // check button for activity once only per loop. ignore entirely if uptime is less than two minutes.
      if ((ButtonEnabled) || (uptimeMilliseconds() > ButtonWaitMatch))
      {
         ButtonEnabled = TRUE;  // kludge so once enough time has passed, doesn't need to check any more
         buttonWasPushed = buttonCycled();  // note this if it occurred. swpNONE:no press  swpSHORT:short press  swpLONG:long press
      }
      else
      {
         buttonWasPushed = swpNONE;  
      }
      
      if (kbhit(SMSport))  // for any buffered-up SMS bytes...
      {
         //rch = fgetc(SMSport);  // ...read 'em and save 'em
         //packetsParse(rch);  // deliver character to parsing function
         packetsParse(fgetc(SMSport));  //  read character and deliver to parsing function
      }

      // if that resulted in any packet decodes, process them
      if (wordqCount(apiRcvQueue))  // any frames waiting?
      {
         packetsProcess();
      }
      
//!      // are we in the countdown to the modem sleep time?
//!      // if so, note whether uptime value has reached or passed the precomputed
//!      // time at which the interval ends.
//!      // when time expires, deactivate the modem and zero the match time value.
//!      if(modemNeedsToSleep)
//!      {
//!         if(uptimeMilliseconds() > modemSleepTimeout)  // time for modem to sleep
//!         {
//!            output_high(PwKy);  // assert pin-sleep signal
//!            modemNeedsToSleep = FALSE;  // clear the flag
//!#if __DEBUG_XBEE
//!            if (dbpEnabled(LEV3))
//!            {
//!               printf(dpo, "*** XBEE PUT TO SLEEP NOW  %s\r\n", stringTheDateTimeUptime());
//!            }
//!#endif
//!         }   
//!      }      

      // decide whether to perform the health report
      // nominally this will be done when the time of day matches
      // a saved value, and thus once or twice per day.
      // while debugging send a health report once per hour.
      // a structure hm 'hhm' holds the appropriate definitions.
      // also allow manual request via button push. (must be short press)
      // the health report takes around 30 seconds to complete.

      // button short press triggers health report if not already
      // scheduled by the normal time-based scheduling.
      // short press is acted upon if not
      // conditionally scheduled by time-based initiation, if
      // not 'Running', and if not 'ModemActive'.
      // (conditional means both health report and detection report
      // are selected to occur in the same minute.)
      // this logic needs to occur before the ordinary check
      // for health report.
      //
      // SWAG because this doesn't work well very close to power-up:
      // ignore the button if uptime is less than two minutes.
      //
      if (buttonWasPushed == swpSHORT)  // request made via button push
      {
         // need immediate report if none of these other things are asserted
         PBNeedTheHealthReport = !(ConditionalNeedTheHealthReport  || XbeeModemIsActive || RunningNow) ;  
         NeedTheHealthReport |= PBNeedTheHealthReport;  // can set the flag here, cannot clear it
      }

      // check whether time matches for health report.
      // there used to be logic here that enabled the modem if we might
      // need to handle commands incoming via SMS message. that is no longer done.
      if (NeedTheHealthReport)  // is it time to produce health report?
      {
         NeedTheHealthReport = FALSE;  // only one attempt allowed until the next scheduled time
         setRunning();
         processHealthReport();  // make a health report. will wake up modem for itself when appropriate
         clearRunning() ;
      }
   
      //
      // if a long-press button cycle was reported,
      // programmatically perform a system reset.
      //
      if (buttonWasPushed == swpLONG)
      {
         printf(hpo, "\r\nFIVE...");
         delay_ms(1000);
         printf(hpo, "\r\nFOUR...");
         delay_ms(1000);
         printf(hpo, "\r\nTHREE...");
         delay_ms(1000);
         printf(hpo, "\r\nTWO...");
         delay_ms(1000);
         printf(hpo, "\r\nONE...");
         delay_ms(1000);
         printf(hpo, "\r\nKABOOM!!!");
         delay_ms(250);
         reset_cpu();   // reset program rather than simply returning
      }
      
      // decide whether to perform the tag detection report
      // nominally this will be done when reporting is active, and when the time of day matches
      // specified minute values in the hour. (i.e. twice per hour, six times per hour, etc)
      // if we do not actually send a detection report message, check for the conditional
      // request for a health report and send that.

      if (NeedTheDetectionReport)  // check for tag detections
      {
         BOOLEAN sentOne;
         
         NeedTheDetectionReport = FALSE;  // only one attempt until next scheduled interval time
         setRunning();
         sentOne = processDetectionReport();
         clearRunning() ;
         if (ConditionalNeedTheHealthReport && !sentOne)
         {
            setRunning();
            processHealthReport();  // make a health report. will wake up modem for itself when appropriate
            clearRunning() ;
         }
         ConditionalNeedTheHealthReport = FALSE;
      }
      
      // check whether any sms status responses have been queued 
      // for matching against transmitted packets
      if (wordqCount(smsStatMatchQueue))  // any frames waiting?
      {
//!#if __DEBUG_XBEE_API
//!         if (dbpEnabled(LEV3))
//!         {
//!            printf(dpo, "\r\n*** calling packetsMatch() count:%d  %s\r\n", wordqCount(smsStatMatchQueue), stringTheDateTimeUptime());
//!         }
//!#endif
         packetsMatch();
      }
      
      // after the check  has been performed for status response matching,
      // check whether any transmitted packets in the queue have become stale
      if (wordqCount(smsPktDispQueue))  // any frames waiting?
      {
//!#if __DEBUG_XBEE_API
//!         if (dbpEnabled(LEV3))
//!         {
//!            printf(dpo, "\r\n*** calling packetsStalenessCheck() count:%d  %s\r\n", wordqCount(smsPktDispQueue), stringTheDateTimeUptime());
//!         }
//!#endif
         packetsStalenessCheck();
      }
      
      //
      // we need somewhere in this loop to transmit the frames that have been queued for the XBEE
      //
      // let's try it right here.
      //
      // don't know yet whether to do one per loop, or all in the same loop.
      // let's do one per loop to start.
      //
      // later enhancement, may be called upon to invoke modem pin sleep,
      // using a dummy command packet with a phony frame type.
      //
      // the code to react to 'ftPH_SLEEP' remains here.
      // if we have condtionaled out support for PINSLEEP, no
      // packets will be generated with the special code.
      //
      if (wordqCount(apiXmitQueue))  // any frames waiting?
      {
         APBU *x;
         U32 gxx;  // scratch cell for holding uptime value
         
         // see whether we may transmit now
         gxx = uptimeMilliseconds();  // uptime now
         if (gxx > modemDeadTimeEnds)  // enough time has elapsed
         {
            x = wordqDequeue(apiXmitQueue);  // will be at least one
            if(x->fmatcp.frametype == ftPH_SLEEP)  // special sleep assertion command packet
            {
               output_high(PwKy);  // assert pin-sleep signal
#if __DEBUG_XBEE
               if (dbpEnabled(LEV3))
               {
                  printf(dpo, "*** XBEE PUT TO SLEEP NOW  %s\r\n", stringTheDateTimeUptime());
               }
#endif
               xbfree(x);  // free up this buffer
            }
            else  // regular command packet for transmittal
            {
               xbeeApiPacketSend(x);
            }
            // set the next dead time interval completion value if needed
            if (x->fmatcp.deadTime)
            {
               modemDeadTimeEnds = gxx + x->fmatcp.deadTime;  // kick out the jams
            }
#if __DEBUG_XBEE_API
            if (dbpEnabled(LEV3))
            {
               printf(dpo, "\r\n*** calling xbeeApiPacketSend() count:%d  %s\r\n", wordqCount(apiXmitQueue), stringTheDateTimeUptime());
            }
#endif
         }
      }
      
      //
      // as the last packet-related work in the big loop,
      // check whether the XBEE modem needs to transition
      // to lower-power operation after any communication
      // packets have been handled.
      //
      if (XbeeShutDownModem)  // was a shutdown requested?
      {
         int i,j,k;
         
         i = wordqCount(apiXmitQueue);
         j = wordqCount(smsPktDispQueue);
         k = wordqCount(smsStatMatchQueue);
         if ((i == 0) && (j == 0)  && (k== 0))  // nothing going on, shut xbee down now
         {
#if __DEBUG_XBEE
            if (dbpEnabled(LEV3))
            {
               printf(dpo, "*** appShutDownModem() APPLY NOW %s\r\n", stringTheDateTimeUptime());
            }
#endif
            XbeeShutDownModem = FALSE;  // reset this flag
            appShutDownModem_Inner();
         }
      }

      
//!      // third-to-last in the big loop, check for input characters on the
//!      // btle interface. if an external  connection is made into this
//!      // device, some strings bounded by percent signs will arrive.
//!      //
//!      // we look specifically for %CONNECT(...)%. if we find it, the
//!      // interface is deemed active and flagged as such.
//!      //
//!      // the detection is distributed across runs of the big loop, and
//!      // a state machine in the called function. if we find it once, we stop looking.
//!      //
//!      if (!IsBtleConnected())
//!      {
//!         trackBtleConnection();
//!      }
      

      
      // third-to-last in the big loop, check whether host port
      // hass been assigned to one of the possible input ports.
      //
      // simply put, the first port on which input is detected
      // will be chosen. the case of the BTLE port is made more
      // complicated because the controller will send spontaneous
      // status information strings. these are delimited by '%'
      // characters, so we may ignore them.
      //
      // we need to check for command strings presumably sent
      // from the external device which wants to configure us.
      //
      // we look specifically for a 'CSI'. if we find one, the
      // interface is deemed active and flagged as such.
      //
      // the detection is distributed across runs of the big loop, and
      // a state machine in the called function. if we find it once, we stop looking.
      //
      if (!hostPortActive())
      {
         SwallowedCSI = checkHostConnection();
      }
      

      
      // next-to-last in the big loop, check for input characters on the
      // host interface. if we get a 'CSI', go process a command.
      // at this point, no detection or other operations are ongoing
      // so we don't need locks or semaphores. just do it.
      //
      // this has become more complicated since we now support usage
      // of either SUR or BTLE port for host interaction, and dynamic
      // assignment/switching of that assignment.
      //
      // if a host port is currently assigned, check it for CSI.
      //
      // if no host port is yet assigned, do nothing more here.
      //
      if (hostPortActive())  // if a current host port exists
      {
         if (SwallowedCSI)  // CSI received for first time command
         {
            SwallowedCSI = FALSE;
            commandProc();  // invoke immediate command function
         }
         else if (hpsts())  // IFF a character is waiting
         {
            xch = hpi();  // read the character
            if ((xch == AsciiESC) || (xch == '+'))
            {
               //printf(dpo, ">>> GOT A CSI  %s\r\n", stringTheDateTimeUptime());
               commandProc();  // invoke immediate command function
            }
            else if (xch == '%')
             {
               //printf(dpo, ">>> GOT A PERCENT  %s\r\n", stringTheDateTimeUptime());
               skip_until_pct();  // read and trash until a '%' arrives
            }
         }
     
      }  // end 'if (hostPortActive())'

      
  } // end of big loop
  // %%%%%%%%%%%%%%%%%%%%%%%%
  // %%%%%%%%%%%%%%%%%%%%%%%%
}  // end of function 'process()'

#endif

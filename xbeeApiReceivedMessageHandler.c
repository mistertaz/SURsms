#ifndef APIRCV_XBEE
#define APIRCV_XBEE
//
// functions to perform odd, handy tasks for the Digi XBee 4G LTE cellular modem (SMS modem)
//




//
// process received packets from 'smsStatMatchQueue'. (ftFMTXST frame type 0x89)
// free packet buffers when done, maybe in future this will change.
//
// what we do here:
// sms transmit status response packets are queued up.
// one-by-one we fetch them, determine the frame ID, and
// look through a separate queue of transmitted packets
// looking for the matching frame ID.
//
// if we find a match:
//
// if our packet indicates 'success' (meaning no reported error)
// we free both the status packet and the transmitted packet.
//
// if our packet indicates error, we examine the frame ID value.
//
// frame ID will indicate whether packet retry has occurred.
//
// if packet was never retried, queue it for retransmission. free status packet
//
// if packet was already retried, no more chances, packet contents are lost.
// free both packets.
//
// if we do not find a match:
// scratch head, free up status packet, and move on.
//
void packetsMatch()
{
   APBU *sptr;  // status frame pointer
   APBU *pptr;  // sms message packet pointer
   //BYTE ftyp;  // frame type is the first byte of the message body
   BYTE sfid;  // frame ID from responded packet
   BYTE pfid;  // frame ID from message packet
   //BYTE flen;  // frame length from responded packet
   BYTE cmdstatus;  // status reply for response-type messages
   int pendingPacketCount;  // how many queued sms messages
   BOOLEAN foundMatchingFrame;
   U32 tStampPacketSent;
   U32 tStampStatusArrived;
   S32 stampsElapsed;

   while(wordqCount(smsStatMatchQueue))  // fetch all frames and handle. should be at least one
   {
      sptr = wordqDequeue(smsStatMatchQueue);  // this is a type ftFMTXST packet (frame type 0x89)
      tStampStatusArrived = sptr->fmbfr.tStamp;
      sfid = sptr->fmtxst.frameid;
      cmdstatus = sptr->fmtxst.txstat;
      // now see whether a transmitted packet matches the frame ID
      pendingPacketCount = wordqCount(smsPktDispQueue);
      if (pendingPacketCount)  // any packets pending disposition?
      {
         // for each queued packet, dequeue and check frame id
         // if does not match, requeue packet
         // if matches, process that packet
         foundMatchingFrame = FALSE;
         while(pendingPacketCount--)
         {
            pptr = wordqDequeue(smsPktDispQueue);  // this is a type ftFMTXSMS,  SMS transmit message (frame type 0x1F)
            pfid = pptr->fmtxst.frameid;  // frame ID of transmitted packet
            if (pfid != sfid)  // not the droid we're looking for
            {
               wordqEnqueue(smsPktDispQueue, pptr);  // re-queue the packet
               continue;  // loop to next queue entry
            }
            else  // frames match, deal with this message
            {
               foundMatchingFrame = TRUE;
               break;  // stop looping
            }
         }  // end of 'while(pendingPacketCount--)'
         if (foundMatchingFrame)  // we have status frame ID matching the pending transmit frame ID
         {
            tStampPacketSent = pptr->fmbfr.tStamp;
            stampsElapsed = tStampStatusArrived - tStampPacketSent;  // elapsed milliseconds between sms transmission and status response
            
            // is status good? (zero is good, non-zero is error)
            if (cmdstatus)  // error
            {
               // here we should evaluate whether retry is possible
               // leave that for some future day
               // just free both packets and loop again
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
               if (dbpEnabled(LEV3))
               {
                  printf(dpo, "\r\n");
                  printf(dpo, "%s\r\n", fortyBucks);
                  printf(dpo, "%s\r\n", fortyBucks);
                  printf(dpo, "$$$$$ packetsMatch() potential retry here  %s\r\n", stringTheDateTimeUptime());
                  if (stampsElapsed > 0)  // what we expect to occur
                  {
                     printf(dpo, "$$$$$ elapsed time [%ld]:%s\r\n", stampsElapsed, stringTheUptime(stampsElapsed));
                  }
                  else if (stampsElapsed == 0)  // what might possibly occur
                  {
                     printf(dpo, "$$$$$ packets have equal timestamps\r\n");
                  }
                  else  // impossible for status to occur before the packet gets sent
                  {
                     printf(dpo, "$$$$$ packet timestamps have invalid relationship tx [%lu] rx [%lu]\r\n", tStampPacketSent, tStampStatusArrived);
                  }
                  packetInfoDisplay(sptr);  // display information about this status packet
                  printf(dpo, "%s\r\n", fortyBucks);
                  printf(dpo, "%s\r\n", fortyBucks);
               }
#endif
               xbfree(sptr);
               xbfree(pptr);
               continue;
            }
            else  // good status, free both packets and keep looping
            {
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
               if (dbpEnabled(LEV3))
               {
                  printf(dpo, "\r\n*** packetsMatch() matching sms/status packets removed  %s\r\n", stringTheDateTimeUptime());
                  packetInfoDisplay(sptr);  // display information about this status packet
                  packetInfoDisplay(pptr);  // display information about this message packet
                  if (stampsElapsed > 0)  // what we expect to occur
                  {
                     printf(dpo, "*** elapsed time [%ld]:%s\r\n", stampsElapsed, stringTheUptime(stampsElapsed));
                  }
                  else if (stampsElapsed == 0)  // what might possibly occur
                  {
                     printf(dpo, "*** packets have equal timestamps\r\n");
                  }
                  else  // impossible for status to occur before the packet gets sent
                  {
                     printf(dpo, "*** packet timestamps have invalid relationship tx [%lu] rx [%lu]\r\n", tStampPacketSent, tStampStatusArrived);
                  }
               }
#endif
               xbfree(sptr);
               xbfree(pptr);
               continue;
            }
         }
         else  // there is no pending transmit frame matching current status frame
         {
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
            if (dbpEnabled(LEV3))
            {
               printf(dpo, "\r\n*** packetsMatch() no matching sms packets  %s\r\n", stringTheDateTimeUptime());
               packetInfoDisplay(sptr);  // display information about this status packet
            }
#endif
            xbfree(sptr);  // free up the status buffer
            continue;  // loop again consuming status packets
         }
         
         
      }
      else  // no queued packets, ???
      {
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "\r\n*** packetsMatch() no pending sms packets  %s\r\n", stringTheDateTimeUptime());
            packetInfoDisplay(sptr);  // display information about this status packet
         }
#endif
         xbfree(sptr);  // free up the status buffer
         continue;  // loop again consuming status packets
      }
   }
   
}




//
// process transmitted packets from 'smsPktDispQueue'. (ftFMTXSMS frame type 0x1F)
// packets are determined to be 'stale' if no status from their transmission has
// been received after a specified amount of time.
//
// if a packet gets stale, it may be retransmitted one time. after that,
// stale packets are discarded.
//
// what we do here:
// sms transmitted packets have been queued up.
// one-by-one we fetch them, determine whether they are stale based upon
// the recorded time of their transmission, and dispose of them. if not stale,
// they are returned to the disposition queue for future action.
//
// if we find a stale packet:
//
// if our packet indicates one transmission (frame ID < 32),
// it will be transmitted one more time. frame ID incremented by 32 to incicate this.
//
// if our packet indicates two transmissions, it is discarded without further concern.
//
void packetsStalenessCheck()
{
   //APBU *sptr;  // status frame pointer
   APBU *pptr;  // sms message packet pointer
   //BYTE sfid;  // frame ID from responded packet
   //BYTE pfid;  // frame ID from message packet
   //BYTE cmdstatus;  // status reply for response-type messages
   int pendingPacketCount = wordqCount(smsPktDispQueue);  // how many queued sms messages. we will process only this many items.
   //BOOLEAN foundMatchingFrame;
   U32 timePacketStale;
   //U32 tStampStatusArrived;
   //S32 stampsElapsed;

   while(pendingPacketCount--)  // fetch every frame one time and handle. should be at least one. no more than initial count
   {
      pptr = wordqDequeue(smsPktDispQueue);  // this is a type ftFMTXSMS,  SMS transmit message (frame type 0x1F)
      timePacketStale = pptr->fmtxsms.tStamp + XBEE_API_STALE_PKT_DELTA;
      if (uptimeMilliseconds() > timePacketStale)  // packet needs attention
      {
         if (pptr->fmtxsms.frameid > 32)  // check frame ID to determine whether the waiting packet has been sent once or twice
         {
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
            if (dbpEnabled(LEV3))
            {
               printf(dpo, "\r\n*** packetsStalenessCheck() stale packet discarded  %s\r\n", stringTheDateTimeUptime());
               packetInfoDisplay(pptr);  // display information about this message packet
            }
#endif
            xbfree(pptr);  // out of luck, discard packet
         }
         else  // first transmission status pending, send it again
         {
            pptr->fmtxsms.frameid += 32;  // increment the frame ID for the second transmission
            // if the frame is a detection report, change the payload string "RSN" to "DSN" to indicate the retransmission
            // brute force in ignorance until we have time to make it better
            if ((pptr->fmtxsms.mtxt[0] == 'R') && (pptr->fmtxsms.mtxt[1] == 'S') && (pptr->fmtxsms.mtxt[2] == 'N'))
            {
               pptr->fmtxsms.mtxt[0] = 'D';
            }
            wordqEnqueue(apiXmitQueue, pptr);  // and queue up the packet for transmission.
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
            if (dbpEnabled(LEV3))
            {
               printf(dpo, "\r\n*** packetsStalenessCheck() stale packet retried  %s\r\n", stringTheDateTimeUptime());
               packetInfoDisplay(pptr);  // display information about this message packet
            }
#endif
         }
      }
      else  // packet may keep waiting for status response
      {
         wordqEnqueue(smsPktDispQueue, pptr);  // re-queue the packet
      }
   }  // end while any transmit packets

// lines below are from the copy/pasted function
      
      
//!      pfid = pptr->fmtxst.frameid;  // frame ID of transmitted packet
//!      cmdstatus = sptr->fmtxst.txstat;
//!      // now see whether a transmitted packet matches the frame ID
//!      pendingPacketCount = wordqCount(smsPktDispQueue);
//!      if (pendingPacketCount)  // any packets pending disposition?
//!      {
//!         // for each queued packet, dequeue and check frame id
//!         // if does not match, requeue packet
//!         // if matches, process that packet
//!         foundMatchingFrame = FALSE;
//!         while(pendingPacketCount--)
//!         {
//!            pptr = wordqDequeue(smsPktDispQueue);  // this is a type ftFMTXSMS,  SMS transmit message (frame type 0x1F)
//!            pfid = pptr->fmtxst.frameid;  // frame ID of transmitted packet
//!            if (pfid != sfid)  // not the droid we're looking for
//!            {
//!               wordqEnqueue(smsPktDispQueue, pptr);  // re-queue the packet
//!               continue;  // loop to next queue entry
//!            }
//!            else  // frames match, deal with this message
//!            {
//!               foundMatchingFrame = TRUE;
//!               break;  // stop looping
//!            }
//!         }  // end of 'while(pendingPacketCount--)'
//!         if (foundMatchingFrame)  // we have status frame ID matching the pending transmit frame ID
//!         {
//!            tStampPacketSent = pptr->fmbfr.tStamp;
//!            stampsElapsed = tStampStatusArrived - tStampPacketSent;  // elapsed milliseconds between sms transmission and status response
//!            
//!            // is status good? (zero is good, non-zero is error)
//!            if (cmdstatus)  // error
//!            {
//!               // here we should evaluate whether retry is possible
//!               // leave that for some future day
//!               // just free both packets and loop again
//!#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
//!               if (dbpEnabled(LEV3))
//!               {
//!                  printf(dpo, "\r\n");
//!                  printf(dpo, "%s\r\n", fortyBucks);
//!                  printf(dpo, "%s\r\n", fortyBucks);
//!                  printf(dpo, "$$$$$ packetsMatch() potential retry here  %s\r\n", stringTheDateTimeUptime());
//!                  if (stampsElapsed > 0)  // what we expect to occur
//!                  {
//!                     printf(dpo, "$$$$$ elapsed time [%ld]:%s\r\n", stampsElapsed, stringTheUptime(stampsElapsed));
//!                  }
//!                  else if (stampsElapsed == 0)  // what might possibly occur
//!                  {
//!                     printf(dpo, "$$$$$ packets have equal timestamps\r\n");
//!                  }
//!                  else  // impossible for status to occur before the packet gets sent
//!                  {
//!                     printf(dpo, "$$$$$ packet timestamps have invalid relationship tx [%lu] rx [%lu]\r\n", tStampPacketSent, tStampStatusArrived);
//!                  }
//!                  packetInfoDisplay(sptr);  // display information about this status packet
//!                  printf(dpo, "%s\r\n", fortyBucks);
//!                  printf(dpo, "%s\r\n", fortyBucks);
//!               }
//!#endif
//!               xbfree(sptr);
//!               xbfree(pptr);
//!               continue;
//!            }
//!            else  // good status, free both packets and keep looping
//!            {
//!#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
//!               if (dbpEnabled(LEV3))
//!               {
//!                  printf(dpo, "\r\n*** packetsMatch() matching sms/status packets removed  %s\r\n", stringTheDateTimeUptime());
//!                  packetInfoDisplay(sptr);  // display information about this status packet
//!                  packetInfoDisplay(pptr);  // display information about this message packet
//!                  if (stampsElapsed > 0)  // what we expect to occur
//!                  {
//!                     printf(dpo, "*** elapsed time [%ld]:%s\r\n", stampsElapsed, stringTheUptime(stampsElapsed));
//!                  }
//!                  else if (stampsElapsed == 0)  // what might possibly occur
//!                  {
//!                     printf(dpo, "*** packets have equal timestamps\r\n");
//!                  }
//!                  else  // impossible for status to occur before the packet gets sent
//!                  {
//!                     printf(dpo, "*** packet timestamps have invalid relationship tx [%lu] rx [%lu]\r\n", tStampPacketSent, tStampStatusArrived);
//!                  }
//!               }
//!#endif
//!               xbfree(sptr);
//!               xbfree(pptr);
//!               continue;
//!            }
//!         }
//!         else  // there is no pending transmit frame matching current status frame
//!         {
//!#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
//!            if (dbpEnabled(LEV3))
//!            {
//!               printf(dpo, "\r\n*** packetsMatch() no matching sms packets  %s\r\n", stringTheDateTimeUptime());
//!               packetInfoDisplay(sptr);  // display information about this status packet
//!            }
//!#endif
//!            xbfree(sptr);  // free up the status buffer
//!            continue;  // loop again consuming status packets
//!         }
//!         
//!         
//!      }
//!      else  // no queued packets, ???
//!      {
//!#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
//!         if (dbpEnabled(LEV3))
//!         {
//!            printf(dpo, "\r\n*** packetsMatch() no pending sms packets  %s\r\n", stringTheDateTimeUptime());
//!            packetInfoDisplay(sptr);  // display information about this status packet
//!         }
//!#endif
//!         xbfree(sptr);  // free up the status buffer
//!         continue;  // loop again consuming status packets
//!      }
   
}




//
// process received packets from 'apiRcvQueue'.
// free packet buffers when done, maybe in future this will change.
//
void packetsProcess()
{
   APBU *fptr;  // local frame pointer
   BYTE ftyp;  // frame type is the first byte of the message body
   //BYTE fid;  // frame ID from responded packet
   BYTE flen;  // frame length from responded packet
   BYTE cmdstatus;  // status reply for response-type messages
   BYTE x;  // scratch memory cell
   unsigned int16 currentFrameLength;

   if (wordqCount(apiRcvQueue))  // any frames waiting?
   {
      while(wordqCount(apiRcvQueue))  // fetch all frames and handle
      {
         fptr = (APBU *)wordqDequeue(apiRcvQueue);  // get next frame, always at least one
         packetInfoDisplay(fptr);  // display information about this packet. we want to see every one.
         ftyp = fptr->fmbfrasm.body[0];  // body[0] is frame type
   
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "\r\n*** packetsProcess() new packet arrival fmtyp:0x%02X  %s\r\n", ftyp, stringTheDateTimeUptime());
         }
#endif

         currentFrameLength = fptr->fmbfr.flength_msb * 256 + fptr->fmbfr.flength_lsb;  // technically, the correct length of frame
         flen = currentFrameLength % 256;  // in practice, the length we encounter is always < 256, so only LSB has significance
         switch(ftyp)
         {
            case ftFMMST:  // modem status response 0x8A
               // nothing is waiting for these messages, they are spontaneously
               // generated by the modem. just store the status value, then free the packet buffer.
               //packetInfoDisplay(fptr);  // --> already done // display information about this packet
               x = reportedModemStatus;  // capture present status setting
               reportedModemStatus = fptr->fmmst.mdmstat;  // save new setting
               reportedModemStatusTstamp = uptimeMilliseconds();  // capture time now
               
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
               if (dbpEnabled(LEV3))
               {
                  printf(dpo, "\r\n*** packetsProcess() new modem status 0x%02X old 0x%02X", reportedModemStatus, x);
                  if(reportedModemStatus == x)
                  {
                     printf(dpo, " *** NO CHANGE *** ");
                  }
                  printf(dpo, "  %s\r\n", stringTheDateTimeUptime());
               }
#endif
               
               // show via the onboard LED whether xbee modem is registered with cell network
               if (reportedModemStatus == 3)  // unregistered with cell network
               {
                  output_low(LED);
                  RegisteredToCellNetwork = FALSE;
               }
               if (reportedModemStatus == 2)  // registered with cell network
               {
                  output_high(LED);
                  RegisteredToCellNetwork = TRUE;
               }
               else  // either not yet initialized, or some other value
               {
                  // don't know what to do here as yet
               }
               xbfree((void *)fptr);  // return frame buffer to pool     
               break;
               
            case ftFMATCR:  // AT command response frame  0x88
               // process an AT command response API packet 0x88
               //
               // take notice of such responses in which we are interested
               //
               // there are two different forms of the AT command response API packet.
               // one is used when a value is queried, the other when a value is set.
               //
               // if the response is the result of an RSSI query, store the responded RSSI
               // value and the uptime timestamp passed in some fixed memory locations.
               //
               {
                  BYTE rlen;  // length of response field (if any) from responded packet
                  BFATCR *dbAtRsp = sballoc();  // need extra handling for display of response

                  cmdstatus = fptr->fmatcr.cmdstat;  // it's the transmit status we want
                  if (cmdstatus == 0)  // command was successful
                  {
                     dbAtRsp->respbl = 0;  // assume no response
                     rlen = flen - 5;  // length of response, may be zero
                     if (rlen > 0)  // a response is present
                     {
                        if (rlen > 30)  // too big for small buffer, get a large
                        {
                           sbfree((void *)dbAtRsp);  // return the small buffer
                           dbAtRsp = lballoc();  // get a large instead
                        }
                        memcpy(dbAtRsp->respb, fptr->fmatcr.cmdparam, rlen);  // copy what ever occurs there *NOT A STRING*
                     }  // end 'if (rlen > 0)'
                     dbAtRsp->respbl = rlen;  // save this quantity, may validly be zero
                     dbAtRsp->respb[dbAtRsp->respbl] = 0;  // put null terminator here
                  }  // end 'if (cmdstatus == 0)'
                  else  // command failed
                  {
#if __DEBUG_XBEE_API
                     if (dbpEnabled(LEV3))
                     {
                        printf(dpo, "AT COMMAND TX FAILED:0x%02X  %s\r\n", cmdstatus, stringTheDateTimeUptime());
                     }
#endif
                  }  // end 'command failed'
#if __DEBUG_XBEE_API
                  if (cmdstatus == 0)  // zero is OK transmit status
                  {
                     if (dbpEnabled(LEV3))
                     {
                        printf(dpo, "AT COMMAND TX SUCCESSFUL  %s\r\n", stringTheDateTimeUptime());
                     }
                  }
                  else  // too bad
                  {
                     if (dbpEnabled(LEV3))
                     {
                        printf(dpo, "AT COMMAND TX FAILED:0x%02X  %s\r\n", cmdstatus, stringTheDateTimeUptime());
                     }
                  }
#endif
                  //
                  // save additional information for specific cases
                  //
                  // if the command was RSSI fetch ("DB"), save the response in global memory if
                  // the command was successful, and the value was in a specific range.
                  // otherwise, use the special value to indicate failed/unavailable.
                  //
                  if ((fptr->fmatcr.commandCh1 == 'D') && (fptr->fmatcr.commandCh2 == 'B'))
                  {
                     x = dbAtRsp->respb[0];  // this is the value from the response packet, check it for validity
                     if (cmdstatus == 0)  // check response value
                     {
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
                     }
                     else
                     {
                        x = 0x99;  // failed reply uses special value
                     }
                     // save 'x' as current RSSI in global cell. also save uptime value.
                     savedRSSI = x;
                     savedRSSI_At = uptimeMilliseconds();
                  }
                  
                  xbfree((void *)fptr);  // return frame buffer to pool
                  if (dbAtRsp)  // if pointer is valid, return small buffer to pool
                  {
                     xbfree((void *)dbAtRsp);  // return packet buffer to pool
                  }
               }
               break;
               
            case ftFMTXST:  // SMS transmit status response 0x89
               //packetInfoDisplay(fptr);  // --> already done // display information about this packet
               // packet was timestamped at time of receive, use that value. fptr->fmbfr.tStamp = uptimeMilliseconds();  // timestamp the packet
               //#####################################################################################################
               // while developing, we will simulate a couple of error conditions that we must handle, but do not
               // occur often enough to exercise our code. one condition is a bad error status responded. every so
               // often we turn a good code into a bad one. the other condition is never receiving the status response.
               // every so often, we simulate this by omitting the enqueue of the response packet.
#if __PHONY_BAD_SMS_STATUS
               if (fptr->fmtxst.txstat == 0)  // zero means no error
               {
                  ++__phonyBadSmsStatus;  // add up good status responses
                  if (__phonyBadSmsStatus >= 11)  // every eleventh one is forced bad
                  {
                     fptr->fmtxst.txstat = 0x31;  // sumulate Internal error
                     __phonyBadSmsStatus = 0;  // reset the count
                     printf(dpo, "*** PHONY BAD STATUS  %s\r\n", stringTheDateTimeUptime());
                  }
               }
#endif
#if __PHONY_NO_SMS_STATUS
               ++__phonyNoSmsStatus;  // add up good status responses
               if (__phonyNoSmsStatus >= 7)  // every seventh one is forced bad
               {
                  __phonyNoSmsStatus = 0;  // reset the count
                  printf(dpo, "*** PHONY NO STATUS  %s\r\n", stringTheDateTimeUptime());
                  xbfree((void *)fptr);  // return frame buffer to pool, thus 'throwing it away'     
               }
               else  // process normally
               {
                  wordqEnqueue(smsStatMatchQueue, (UW)fptr);  // add to queue for matching with sent packets
               }
               //#####################################################################################################
#else  // normal code
               wordqEnqueue(smsStatMatchQueue, (UW)fptr);  // add to queue for matching with sent packets
#endif
               break;
               
            case ftFMRXSMS:  // SMS received message frame 0x9F. display it, then throw it away
               packetInfoDisplay(fptr, TRUE);  // display information about this packet including the payload
               xbfree((void *)fptr);  // return frame buffer to pool     
               break;
               
            default:  // not a message type of interest to us
#if __DEBUG_XBEE_API
               if (dbpEnabled(LEV3))
               {
                  printf(dpo, "\r\npacketsProcess() unhandled frame type 0x%02X  %s", ftyp, stringTheDateTimeUptime());
                  dumpToHostWithAddress((BYTE *)fptr, currentFrameLength+3);
               }
#endif
               xbfree((void *)fptr);  // return frame buffer to pool     
               break;
         }  // end of 'ftyp' switch
      }  // end of 'while qcount' loop
   }
#if __DEBUG_XBEE_API || __DEBUG_XBEE_API_PKT
   else  // no packets waiting now
   {
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\n*** packetsProcess() no pending packet  %s\r\n", stringTheDateTimeUptime());
      }
   }
#endif
}



//
// management of the API packet stream read from the XBee modem.
//
// data is read character-by-character and parsed into message packets according
// to a well-defined set of rules.
//
// modem response may be one of several packets.
//
// we accept data bytes one-by-one, looking for the frame delimiter (0x7E) to start a new packet.
// frame delimiter is followed by a 16-bit big-endian message length. Delimiter and length are common to all packets.
// length means the length of frame data field, which is everything following the length except the checksum byte.
//
// next byte up is the frame type, and implies the composition of further data in the packet.
//
// remaining bytes are specific to each frame type. separate code paths will handle.
//

// accept input stream characters and parse into strings.
// <null> characters are valid inputs
//

void packetsParse(char mch)
{
   BYTE ckr, ckc, ckff, ckffo;
   BOOLEAN checksumIsGood;
   
   switch(ppInputState)
   {
      // handle common packet items
      case psIDLE:
         if (mch == XBEE_API_FRAME_DELIMITER)  // frame begins
         {
            ++ppFrameCounter;  // received frame counted
            tempFptr.delim = mch;  // store the frame delimiter in temporary frame header
            ppInputState = psLENGTH_MSB;  // transition to next state, expecting frame length msb
         }
         break;
         
      case psLENGTH_MSB:
         tempFptr.flength_msb = mch;  // store the frame length ms byte in temporary frame header
         ppInputState = psLENGTH_LSB;  // transition to next state, expecting frame length lsb
         break;
         
      case psLENGTH_LSB:
         tempFptr.flength_lsb = mch;  // store the frame length ls byte in temporary frame header
         ppCurrentFrameLength = tempFptr.flength_msb * 256 + tempFptr.flength_lsb;
         // now that we know the length of the impending packet, allocate either
         // a small buffer or a large buffer depending on that length.
         // if the length is > 20 (a SWAG) then use a large buffer
         if (ppCurrentFrameLength > 20)  // long packet lengths will use large buffer
         {
            ppFptr = (APBU *)lballoc();
         }
         else  // shorter lengths may use small buffer
         {
            ppFptr = (APBU *)sballoc();
         }
         if (!ppFptr)  // no buffer available
         {
#if __DEBUG_XBEE_API
            if (dbpEnabled(LEV3))
            {
               printf(dpo, "new frame no buffer, frm:%Lu\r\n", ppFrameCounter);
            }
#endif
            ppInputState = psIDLE;  // transition to idle state, skip input until new packet
         }
         else  // successful allocation, copy temporary fields to allocated buffer, keep going
         {
            ppFptr->fmbfr.delim = tempFptr.delim;  // copy the frame delimiter
            ppFptr->fmbfr.flength_lsb = tempFptr.flength_lsb;  // copy the frame length ls byte
            ppFptr->fmbfr.flength_msb = tempFptr.flength_msb;  // copy the frame length ms byte
            ppCurrentChecksum = 0;  // accumulate sum of all bytes in frame after length
            ppCurrentSubscript = 0;  // array subscript for storing message body bytes
            ppInputState = psFRAMETYPE;  // transition to next state, expecting frame type
         }
         break;
         
      case psFRAMETYPE:
         ppFptr->fmbfrasm.body[ppCurrentSubscript++] = mch;  // store the frame type byte, increment store subscript
         //NOPE ppFptr->fmbfr.frametype = mch;  // store the frame type byte, increment store subscript
         ppCurrentChecksum += mch;  // accumulate sum of all bytes in frame after length

         // set up to consume the balance of this packet.
         // one byte of the field has already come in, the frametype.
         // what is left is ppCurrentFrameLength -1, used as the number of bytes to store
         // what we can do is to receive until the subscript matches the frame length
         ppInputState = psBODY;  // transition to next state
         break;
         
      case psBODY:
         ppFptr->fmbfrasm.body[ppCurrentSubscript++] = mch;  // store the next message byte, increment store subscript
         ppCurrentChecksum += mch;  // add byte to accumulating checksum
         // remain in this state storing characters until all are processed
         if (!(ppCurrentSubscript < ppCurrentFrameLength))  // inelegant but is exactly what we mean, all chars received
         {
            ppInputState = psCKSM;  // transition to next state, expecting checksum
         }
         break;
         
      case psCKSM:
         ppFptr->fmbfrasm.body[ppCurrentSubscript++] = mch;  // store the checksum byte
         // checksum is the last received byte for the packet.
         // checksum algorithm is such that adding the checksum value to the
         // lsb of the accumulated packet byte sum should yield 0xFF.
         // if checksum is OK, invoke processing function for this packet.
         ckr = mch;
         ckc = ppCurrentChecksum & 0xFF;
         ckff = ckr + ckc;
         ckffo = ckr | ckc;
         checksumIsGood = ((ckff == 0xFF) || (ckffo == 0xFF));
                  
         // if using an allocated buffer, and if the checksum is good,
         // queue packet for further processing.
         // if the checksum fails, release buffer here and ignore.
         if (checksumIsGood)  // checksum is valid, timestamp packet and send the frame to the appropriate processing queue
         {
            ppFptr->fmbfr.tStamp = uptimeMilliseconds();
            wordqEnqueue(apiRcvQueue, (UW)ppFptr);
         }
         else  // checksum failed, release the packet buffer
         {
#if __DEBUG_XBEE_API
            if (dbpEnabled(LEV3))
            {
               printf(dpo, "bad packet checksum, frm:%Lu\r\n", ppFrameCounter);
            }
#endif
            xbfree((void *)ppFptr);  // return the frame buffer to pool
         }
         
         ppInputState = psIDLE;  // resume looking for packet
         ppFptr = 0;  // null out the pointer
         break;
         
        
      default:
#if __DEBUG_XBEE_API
         if (dbpEnabled(LEV3))
         {
            printf(dpo, "packet parse bad state:");
            valueToDebug(ppInputState);
            printf(dpo, " input byte:");
            charToDebug(mch);
            printf(dpo, "\r\n");
         }
#endif  // __DEBUG_XBEE_API
         ppInputState = psIDLE;  // attempt recovery by resetting state for new packet
         if (ppFptr != 0)  // if we have an allocated frame buffer
         {
            xbfree((void *)ppFptr);  // return the frame buffer to pool
         }
         break;
   }
   
}


//
// reinitialize state machine when modem sleeps or wakes
//
// set the state code to the initial state (looking for frame start byte)
//
// if the process has allocated a frame buffer, return that buffer to the pool
//
void resetPacketsParse()
{
   BYTE xcpend = rcv_buffer_bytes(SMSport);  // is any data waiting? aside from swallowed frame delimiter
   
#if __DEBUG_XBEE_API
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** PACKET PARSE RESET swallowed:%c pend:%u  %s ***\r\n", (XbeeSwallowedFrameDelimiter) ? 'T' : 'F', xcpend, stringTheDateTimeUptime());
   }
#endif

   // if there is any pending data from xbee modem, try to decode a packet from it.
   // I think it is certain that we are in API mode when the function is called,
   // so any data is packet data.
   if (xcpend)  // is any data waiting? aside from swallowed frame delimiter
   {
      if (XbeeSwallowedFrameDelimiter)  // insert swallowed frame delimiter if appropriate
      {
         packetsParse(XBEE_API_FRAME_DELIMITER);
      }
      while(xcpend--)
      {
         packetsParse(fgetc(SMSport));  // read all the buffered bytes and parse themn
      }
   }

   // we have to clear the swallowed frame delimiter flag if it is set
   if(XbeeSwallowedFrameDelimiter)
   {
      XbeeSwallowedFrameDelimiter = FALSE;
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "resetPacketsParse() cleared \"XbeeSwallowedFrameDelimiter\"  %s\r\n",  stringTheDateTimeUptime());
      }
#endif
   }  
  
   if (ppFptr != 0)  // if presently have an allocated frame buffer
   {
#if __DEBUG_XBEE_API
      if (dbpEnabled(LEV3))
      {
         dumpToHost((BYTE *)ppFptr, 32);  // dump the size of small buffer, will show anything interesting in the packet
         printf(dpo, "\r\n");  // I think we need the trailing crlf
      }
#endif
      xbfree((void *)ppFptr);  // return the frame buffer to pool
   }
   ppInputState = psIDLE;  // transition to next state, expecting new packet
   ppFptr = 0;  // null out the pointer

}


#endif  // APIRCV_XBEE


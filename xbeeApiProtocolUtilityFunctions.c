#ifndef APIUTFXNS_XBEE
#define APIUTFXNS_XBEE
//
// functions to perform odd, handy tasks for the Digi XBee 4G LTE cellular modem (SMS modem)
//




//
// XBee API packet sender
//
// passed a pointer to arbitrary packet frame structure.
// form checksum over the packet contents and then
// transmit to SMS modem (XBee 3G Global)
// free the input buffer once the send is complete.
//
void xbeeApiPacketSend(APBU *fptr)
{
   BYTE *pii;  // scratch
   BYTE aa, bb, cc;  // scratch
   BYTE ftyp;  // API frame type
   
   // now need to compute checksum and set that.
   // variable portion of packet length begins at frametype and ends with checksum byte.
   // so that length is kept in flength_msb:flength_lsb.
   // frame cannot be longer than around 187 bytes so only the lsb will have significance.
   aa = fptr->fmbfr.flength_lsb;
   //pii = (BYTE *)fptr->fmbfrasm.body;  // start of checsummed range in packet
   pii = fptr->fmbfrasm.body;  // start of checsummed range in packet
   ftyp = (BYTE *)fptr->fmbfrasm.body[0];  // start of message body is frame type
   bb = aa + 4;  // transmit length for entire packet, includes frame delimiter, length fields, and checksum
   // computing the checksum involves the summation of all <framelength> bytes in the variable portion
   // of the packet. checksum byte stored immediately following the last message character.
   // interpret the packet using the common buffer structure, which permits addressing the entire
   // variable portion as a byte array.
   //fptr->fmbfr.body[aa] = xbeeChecksum(fptr->fmbfr.body, aa);
   pii[aa] = xbeeChecksum(pii, aa);  // insert checksum at appropriate offset in packet
   // now send the entire packet to the modem
   pii = &fptr->fmbfr.delim;  // point to the start of the frame data in the packet buffer
   cc = bb;  // copy entire length
   
   //packetInfoDisplay(fptr, TRUE);  // display information about this packet, including payload of sms message if present
   
   while (cc--)  // send packet one byte at a time
   {
      fputc(*pii++, SMSport);
   }
   
   // for any non-SMS transmit message, free up the message buffer. use generic free function.
   // queue SMS message packets to the disposition queue. set timestamp field with the current uptime value.
   if (ftyp == ftFMTXSMS)
   {
      fptr->fmbfr.tStamp = uptimeMilliseconds();
      packetInfoDisplay(fptr, TRUE);  // display information about this packet, including payload of sms message if present
      wordqEnqueue(smsPktDispQueue, fptr);
   }
   else
   {
      packetInfoDisplay(fptr);  // display information about this packet
      xbfree((void *)fptr);  // return the frame buffer to pool
   }
   
}   



//
// display XBEE API protocol packet information
//
// for any packet, display the relevant data such as
// frame type, frame ID if appropriate, status code if appropriate, etc.
//
// fptr is a pointer to a message data frame (packet structure)
//
void packetInfoDisplay(APBU *fptr, BOOLEAN displayPayload=FALSE)
{
#if __DEBUG_XBEE_API ||  __DEBUG_XBEE_API_PKT ||  __DEBUG_MISC
   BYTE ftyp;  // frame type is the first byte of the message body
   //BYTE fid;  // frame id is the second byte of the message body, if there is an id in the frame
   BYTE scr;  // scratch cell
   //BYTE *pptr = ((BYTE *)fptr) + 3;  // address of packet within frame
   unsigned int16 currentFrameLength = fptr->fmbfr.flength_msb * 256 + fptr->fmbfr.flength_lsb;  // we need this for some packet types
   unsigned int16 currentResponseLength;
   
   if (currentFrameLength > 5)  // response data is present
   {
      currentResponseLength = currentFrameLength - 5;
   }
   else
   {
      currentResponseLength = 0;
   }

   if (dbpEnabled(LEV3))
   {
      printf(dpo, "*** FRAME 0x%04LX @%s ", fptr, stringTheUptime(fptr->fmbfr.tStamp));
      ftyp = fptr->fmbfrasm.body[0];  // body[0] is frame type
      switch(ftyp)
      {
         case ftFMMST:  // modem status response 0x8A
            scr = fptr->fmmst.mdmstat;  // get the status value from the packet
            printf(dpo, "modem status response fmtyp:0x%02X  stat:0x%02X  %s", ftyp, fptr->fmmst.mdmstat, stringModemStatus(fptr->fmmst.mdmstat));
            break;
            
         case ftFMATC:  // AT command frame  0x08
            //currentFrameLength = fptr->fmbfr.flength_msb * 256 + fptr->fmbfr.flength_lsb;
            // AT responses have two kinds,
            // AT set command and
            // AT query command.
            // the frame length is used to discriminate.
            // query commands have length 4, set commands are >4
            if (currentFrameLength > 4)  // it's a set command
            {
               printf(dpo, "AT set command  fmtyp:0x%02X  fmid:0x%02X  cmd:%c%c  parm val:0x%02X",
                  fptr->fmatcp.frametype, 
                  fptr->fmatcp.frameid, 
                  fptr->fmatcp.command[0], 
                  fptr->fmatcp.command[1], 
                  fptr->fmatcp.cmdvalue);
            }
            else  // it's a query
            {
               printf(dpo, "AT query command  fmtyp:0x%02X  fmid:0x%02X  cmd:%c%c",
                  fptr->fmatcnp.frametype, 
                  fptr->fmatcnp.frameid, 
                  fptr->fmatcnp.command[0], 
                  fptr->fmatcnp.command[1]);
            }
            break;
            
         case ftFMATCR:  // AT command response frame  0x88
            // = fptr->fmbfr.flength_msb * 256 + fptr->fmbfr.flength_lsb;
            // AT responses have two kinds,
            // AT set command response and
            // AT query command response.
            // the frame length is used to discriminate
            // set responses have length 5, query responses are >5
            if (currentResponseLength)  // it's a query response with data returned
            {
               // need extra handling for display of response
               BFATCR *dbAtRsp = sballoc();
               
               if (currentResponseLength > 30)  // too big for small buffer, get a large
               {
                  sbfree((void *)dbAtRsp);  // return the small buffer
                  dbAtRsp = lballoc();  // get a large instead
               }
               memcpy(dbAtRsp->respb, ((APBU *)fptr)->fmatcr.cmdparam, currentResponseLength);  // copy what ever occurs there *NOT A STRING*
               
               dbAtRsp->respbl = currentResponseLength;  // save this quantity
               dbAtRsp->respb[dbAtRsp->respbl] = 0;  // put null terminator here
               
               printf(dpo, "AT query command response  fmtyp:0x%02X  fmid:0x%02X  cmd:%c%c  stat:0x%02X  parm value:",
                  fptr->fmatcr.frametype, 
                  fptr->fmatcr.frameid, 
                  fptr->fmatcr.commandCh1, 
                  fptr->fmatcr.commandCh2, 
                  fptr->fmatcr.cmdstat); 
                  // handle the variable length response
               if(dbAtRsp->respbl > 1)  // multi-byte response, assume it's ASCII
               {
                  for (int i = 0; i < dbAtRsp->respbl; i++)
                  {
                     printf(dpo, "%c", dbAtRsp->respb[i]);
                  }
               }
               else if(dbAtRsp->respbl > 0)  // single-byte response
               {
                  printf(dpo, "0x%02X", dbAtRsp->respb[0]);
               }
               else  // no response
               {
                  printf(dpo, "--");  // error?
               }
               xbfree((BYTE *)dbAtRsp);  // return buffer to pool
            }
            else  // it's a set response
            {
               printf(dpo, "AT set command response  fmtyp:0x%02X  fmid:0x%02X  cmd:%c%c  stat:0x%02X",
                  fptr->fmatcnr.frametype, 
                  fptr->fmatcnr.frameid, 
                  fptr->fmatcnr.commandCh1, 
                  fptr->fmatcnr.commandCh2, 
                  fptr->fmatcnr.cmdstat);
            }
            break;
            
         case ftFMTXST:  // SMS transmit status response 0x89
            printf(dpo, "SMS transmit status response packet fmtyp:0x%02X  fmid:0x%02X  stat:0x%02X  %s", 
               fptr->fmtxst.frametype, 
               fptr->fmtxst.frameid, 
               fptr->fmtxst.txstat,
               stringTransmitStatus(fptr->fmtxst.txstat));
            break;
            
         case ftFMTXSMS:  // SMS transmit message frame 0x1F
            printf(dpo, "SMS transmit message packet fmtyp:0x%02X  fmid:0x%02X",
               fptr->fmtxsms.frametype, 
               fptr->fmtxsms.frameid);
            break;
            
         case ftFMRXSMS:  // SMS received message frame 0x9F
            printf(dpo, "SMS received message packet fmtyp:0x%02X", fptr->fmrxsms.frametype);
            break;
            
         default:  // not a message type of interest to us
            printf(dpo, "packetInfoDisplay() unhandled frame type 0x%02X", ftyp);
            break;
         }  // end of 'ftyp' switch
      printf(dpo, "  %s\r\n", stringTheDateTimeUptime());  // finish the line
      
      // in the case of SMS messages, supply the phone numbers and message texts
      // if the message length is greater than 159, terminate with a null at 160
      if (displayPayload && ((ftyp == 0x1F) || (ftyp == 0x9f)))
      {
         unsigned int8 payloadLength;
         
         if (ftyp == 0x1F)  // it's SMS message for transmit
         {
            payloadLength = strlenb(fptr->fmtxsms.mtxt);
            if (payloadLength > 159)
            {
               fptr->fmtxsms.mtxt[159] = 0;
            }
            printf(dpo, "phnum:%s\r\n", fptr->fmtxsms.phnum);
            printf(dpo, "payload[%u]:\r\n>>>%s<<<\r\n", payloadLength, fptr->fmtxsms.mtxt);
         }
         else  // it's received SMS message
         {
            payloadLength = strlenb(fptr->fmrxsms.mtxt);
            if (payloadLength > 159)
            {
               fptr->fmrxsms.mtxt[159] = 0;
            }
            printf(dpo, "phnum:%s\r\n", fptr->fmrxsms.phnum);
            printf(dpo, "payload[%u]:\r\n>>>%s<<<\r\n", payloadLength, fptr->fmrxsms.mtxt);
         }
      
      }
   }  // end of print enable test

#endif
}



//
// supply strings for the various Transmit Status responses (type 0x89)
//
// unknown (to us) or otherwise unrecognized values resturn error message
//
BYTE *stringTransmitStatus(BYTE mstat)
{
   static BYTE hiddenTransmitStatusString[48];
   
   switch(mstat)
   {
      case 0x0:
         sprintf(hiddenTransmitStatusString, "Successful transmit");
         break;

      case 0x20:
         sprintf(hiddenTransmitStatusString, "Connection not found");
         break;
      
      case 0x21:
         sprintf(hiddenTransmitStatusString, "Failure to transmit to cell network");
         break;
      
      case 0x22:
         sprintf(hiddenTransmitStatusString, "Not registered to cell network");
         break;
      
      case 0x2c:
         sprintf(hiddenTransmitStatusString, "Invalid frame values");
         break;
      
      case 0x31:
         sprintf(hiddenTransmitStatusString, "Internal error");
         break;
      
      case 0x32:
         sprintf(hiddenTransmitStatusString, "Resource error");
         break;

      case 0x74:
         sprintf(hiddenTransmitStatusString, "Message too long");
         break;
      
      case 0x76:
         sprintf(hiddenTransmitStatusString, "Socket closed unexpectedly");
         break;
      
      case 0x78:
         sprintf(hiddenTransmitStatusString, "Invalid UDP port");
         break;
      
      case 0x79:
         sprintf(hiddenTransmitStatusString, "Invalid TCP port");
         break;
      
      case 0x7A:
         sprintf(hiddenTransmitStatusString, "Invalid host address");
         break;
      
      case 0x7B:
         sprintf(hiddenTransmitStatusString, "Invalid data mode");
         break;
      
      case 0x7C:
         sprintf(hiddenTransmitStatusString, "Invalid interface");
         break;
      
      case 0x7D:
         sprintf(hiddenTransmitStatusString, "Interface not accepting frames");
         break;
      
      case 0x7E:
         sprintf(hiddenTransmitStatusString, "A modem update is in progress");
         break;
      
      case 0x80:
         sprintf(hiddenTransmitStatusString, "Connection refused");
         break;
      
      case 0x81:
         sprintf(hiddenTransmitStatusString, "Socket connection lost");
         break;
      
      case 0x82:
         sprintf(hiddenTransmitStatusString, "No server");
         break;
      
      case 0x83:
         sprintf(hiddenTransmitStatusString, "Socket closed");
         break;
      
      case 0x84:
         sprintf(hiddenTransmitStatusString, "Unknown server");
         break;
      
      case 0x85:
         sprintf(hiddenTransmitStatusString, "Unknown error");
         break;
      
      case 0x86:
         sprintf(hiddenTransmitStatusString, "Invalid TLS configuration");
         break;
      
      case 0x87:
         sprintf(hiddenTransmitStatusString, "Socket not connected");
         break;
      
      case 0x88:
         sprintf(hiddenTransmitStatusString, "Socket not bound");
         break;
      
      case 0x8B:
         sprintf(hiddenTransmitStatusString, "TLS Socket Authentication Error ");
         break;

      default:
         sprintf(hiddenTransmitStatusString, "Invalid transmit status [0x%02X]", mstat);
         break;
   }  // end 'mstat' switch
   
   return hiddenTransmitStatusString;
}


//
// supply strings for the various Modem Status responses (type 0x8A)
//
// unknown (to us) or otherwise unrecognized values resturn error message
//
BYTE *stringModemStatus(BYTE mstat)
{
   static BYTE hiddenModemStatusString[48];
   
   switch(mstat)
   {
      case 0:
         sprintf(hiddenModemStatusString, "Hardware reset or power up");
         break;
      
      case 1:
         sprintf(hiddenModemStatusString, "Watchdog timer reset");
         break;
      
      case 2:
         sprintf(hiddenModemStatusString, "Registered with cellular network");
         break;
      
      case 3:
         sprintf(hiddenModemStatusString, "Unregistered with cellular network");
         break;
      
      case 0x0E:
         sprintf(hiddenModemStatusString, "Remote Manager connected");
         break;
      
      case 0x0F:
         sprintf(hiddenModemStatusString, "Remote Manager disconnected");
         break;
      
      case 0x32:
         sprintf(hiddenModemStatusString, "BLE Connect");
         break;
      
      case 0x33:
         sprintf(hiddenModemStatusString, "BLE Disconnect");
         break;
      
      case 0x34:
         sprintf(hiddenModemStatusString, "Bandmask configuration failed");
         break;
      
      case 0x35:
         sprintf(hiddenModemStatusString, "Cellular component update started");
         break;
      
      case 0x36:
         sprintf(hiddenModemStatusString, "Cellular component update failed");
         break;
      
      case 0x37:
         sprintf(hiddenModemStatusString, "Cellular component update completed");
         break;
      
      case 0x38:
         sprintf(hiddenModemStatusString, "XBee firmware update started");
         break;
      
      case 0x39:
         sprintf(hiddenModemStatusString, "XBee firmware update failed");
         break;
      
      case 0x3A:
         sprintf(hiddenModemStatusString, "XBee firmware update applying");
         break;
      
      default:
         sprintf(hiddenModemStatusString, "Invalid or unitialized modem status [0x%02X]", mstat);
         break;
   }  // end 'mstat' switch
   
   return hiddenModemStatusString;
}



#endif  // APIUTFXNS_XBEE

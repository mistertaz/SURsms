#ifndef _streamrevector_c
#define _streamrevector_c

//
// functions which allow revectoring of the data stream used for host interaction
// in the PIC24 product between the BT port uart and the SUR port uart.
//
// this revectoring is necessary when using the SUR port data path for
// console interaction while configuring the SURSMS box.
//
// this capability is kludgy because the compiler forces usage of
// constant values for stream IDs. variables will not work.
//
// hpi();  // use in place of xx = fgetc(SURport);
// hpo(xx);  // use in place of fputch(xx, SURport);
// hpsts(); // use in place of kbhit(SURport);
// hprbb(); // use in place of rcv_buffer_bytes(SURport);
//
// for formatted output, the character out function can be supplied in a 'printf()' statement
// as the first argument. e.g.: printf(hpo,"Hello, World.\r\n");
//

//
// this is host port revector code for PIC24 application.
// normal stream is serial port for BTLE communications.
// alternate stream is serial port for SUR while in development.

//
// host port on BTLE has some issues, we add a
// function to spy on activity on that channel, and
// display it on the ICD port.
//
void BTLE_tagSpyLine();  // forward reference
void BTLE_Spy(BYTE chr, BYTE chrSrc)  // the spied character, was it read or written?
{

//!   if (chrSrc != BTLE_SpyOrientation)  // changed from reading to writing, or the opposite
//!   {
//!      //fprintf(DBGport, "\r\n}}} BTLE_SpyOrientation:%d  chrSrc:%d\r\n", BTLE_SpyOrientation, chrSrc);
//!      BTLE_SpyOrientation = chrSrc;  // reflect the change
//!      BTLE_tagSpyLine();  // go to next line and prefix with direction indication
//!      BTLE_SpyBufCount = 0;  // clear count for the newly-created line
//!   }
//!   
//!   ++BTLE_SpyBufCount;  // count this character
//!   
//!   if (BTLE_SpyBufCount > 80)  // go to next line
//!   {
//!      BTLE_tagSpyLine();  // go to next line and prefix with direction indication
//!      BTLE_SpyBufCount = 0;  // clear count for the newly-created line
//!   }
//!   
//!   if ((chr < 0x20) || (chr > 0x7E)) // ASCII control character or other nondisplayable, show hex value
//!   {
//!      fprintf(DBGport, "\\x%02X", chr);
//!   }
//!   else  // ordinary character, just print it
//!   {
//!      fputc(chr, DBGport);
//!   }
//!   
//!   if (chr == '\r')  // if character was 'enter', go to next line
//!   {
//!      BTLE_tagSpyLine();  // go to next line and prefix with direction indication
//!      BTLE_SpyBufCount = 0;  // clear count for the newly-created line
//!   }
   
}


void BTLE_tagSpyLine()
{

//!   if (BTLE_SpyOrientation == __BTLE_SPY_READING)
//!   {
//!      fprintf(DBGport, "\r\n<<< ");  // data coming "from" btle controller
//!   }
//!   else if (BTLE_SpyOrientation == __BTLE_SPY_WRITING)
//!   {
//!      fprintf(DBGport, "\r\n>>> ");  // data going "to" btle controller
//!   }
//!   else // bad value, indicate the error
//!   {
//!      fprintf(DBGport, "\r\n??? ");  // don't know
//!   }
}


// since we have added the part-time host port accomodation,
// this is a function to tell whether or not it is active now.
BOOLEAN hostPortActive()
{
   return UsingHostSUR || UsingHostBTLE;
}


BOOLEAN hpsts()
{
   if (UsingHostBTLE)
   {
      return kbhit(BTport);
   }
   else if (UsingHostSUR)
   {
      return kbhit(SURport);
   }
   else  // no active host port
   {
      return FALSE;  // no input waiting
   }
}


// this is invalid if no host port is active. return a null in that case.
BYTE hpi()
{
   if (UsingHostBTLE)
   {
      return fgetc(BTport);  // BTLE_Spy(BYTE chr, BYTE chrSrc)
//!      BYTE btleInByte = fgetc(BTport);
//!      BTLE_Spy(btleInByte, __BTLE_SPY_READING);  // snoopy
//!      return btleInByte;
   }
   else if (UsingHostSUR)
   {
      return fgetc(SURport);
   }
   else  // no active host port
   {
      return 0;  // this is an error. should never try to read with no port assigned.
   }
}


// get character but drive the background processes while waiting
// this is invalid if no host port is active. return a null in that case.
BYTE hpi_nblk()
{
   
   while (TRUE)  // loop forever
   {
      if (hpsts())  // character ready?
      {
         return hpi();
      }
      else  // no character, delay a bit and drive the background tasks
      {
         delay_us(550UL);  // wait two character times
         BigLoopMaintenance();
      }
   }  // end of forever loop
}


void hpo(BYTE ch)
{
   if (UsingHostBTLE)
   {
//!      BTLE_Spy(ch, __BTLE_SPY_WRITING);  // snoopy
      fputc(ch, BTport);
   }
   else if (UsingHostSUR)
   {
      fputc(ch, SURport);
   }
   else  // no active host port
   {
      // nothing. character is lost
   }
}



unsigned int16 hprbb()
{
   if (UsingHostBTLE)
   {
      return rcv_buffer_bytes(BTport);
   }
   else if (UsingHostSUR)
   {
      return rcv_buffer_bytes(SURport);
   }
   else  // no active host port
   {
      return 0;  // no input waiting
   }
}



//
// functions which allow revectoring of the data stream used for debug
// printing in the PIC24 product between the BT port uart and the debug port
// provided by the hardware In-Circuit Debugger. (ICD)
//
// this revectoring lets us use the same code for debugging output. 
//
// this capability is kludgy because the compiler forces usage of
// constant values for stream IDs. variables will not work.
//
// this capability is used for output only, so only one
// revectoring function is used. (unlike the host port revector)
//
// dpo(xx);  // use in place of fputch(xx, BTport/DBGport);
//
// for formatted output, the character out function can be supplied in a 'printf()' statement
// as the first argument. e.g.: printf(dpo,"Hello, World.\r\n");
//
// here are the rules:
// always write the character to the ICD port, which may or may not be active.
// echo to the host port, which may or may not be active.
// if using the resident loader, streaming via ICD never works.
//


void dpo(BYTE ch)
{
#if __USING_ICD != 0
   fputc(ch, DBGport);
#else 
   if (UsingHostBTLE)
   {
      fputc(ch, BTport);
   }
   else if (UsingHostSUR)
   {
      fputc(ch, SURport);
   }
   else  // no active host port
   {
      fputc(ch, SURport);  // should be nothing. character is lost. instead use sur while in development
   }
#endif
}


#endif

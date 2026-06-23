#ifndef TOUCH_TIME_C
#define TOUCH_TIME_C
///////////////////////////////////////////////////////////////////////////
////                        Dallas Touch Driver                        ////
//// and DS1904 RTC routines: get_touch_time() set_touch_time()        ////
////                                                                   ////
////                                                                   ////
////  data = touch_read_bit()     Reads one bit from a touch device    ////
////                                                                   ////
////  data = touch_read_BYTE()    Reads one byte from a touch device.  ////
////                                                                   ////
////  ok = touch_write_bit(data)  Writes one bit to a touch device     ////
////                              and returns true if all went ok.     ////
////                              A false indicates a collision with   ////
////                              another device.                      ////
////                                                                   ////
////  ok = touch_write_byte(data) Writes one byte to a touch device    ////
////                              and returns true if all went ok.     ////
////                              A false indicates a collision with   ////
////                              another device.                      ////
////                                                                   ////
////  present = touch_present()   Issues a reset and returns true      ////
////                              if the touch device is there.        ////
////                                                                   ////
////  reset_pulse()               Issues a reset and waits for a       ////
////                              present pulse.                       ////
////                                                                   ////
///////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2010 Custom Computer Services           ////
//// This source code may only be used by licensed users of the CCS C  ////
//// compiler.  This source code may only be distributed to other      ////
//// licensed users of the CCS C compiler.  No other use, reproduction ////
//// or distribution is permitted without written permission.          ////
//// Derivative programs created using this software in object code    ////
//// form are not restricted in any way.                               ////
///////////////////////////////////////////////////////////////////////////


// in the PIC24 device header file, "BYTE" is defined to be "unsigned int8"


#ifndef TOUCH_PIN
#error No definition for one-wire signal TOUCH_PIN
#endif

#include "touchy.c"  // SONOTRONICS modifications to compiler-supplied driver for Dallas 13?? and similar parts

//short set_touch_time()
BOOLEAN set_touch_time()
{
   //int hr,min,sec,mo,da,yr;
   //int8 hr,min,sec,mo,da,yr;
   //int32 xlx;
   BYTE hr,min,sec,mo,da,yr;
   unsigned int32 xlx;
   BOOLEAN DS_Installed;

/*
      seconds since 01-01-01 00:00:00
     tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
    (tm_year-1)*31536000 + ((tm_year)/4)*86400
*/
//   fprintf(UMAIN,"\r\nset1:%x/%x/%x\r\n",MonthBCD,DayBCD,YearBCD);

   sec = (10*(SecondBCD>>4)) + (SecondBCD & 0x0f);
   xlx = sec;
   min = ((10*(MinuteBCD>>4)) + (MinuteBCD & 0x0f));
   xlx = xlx + (int32)min * 60;
   hr = ((10*(HourBCD>>4)) + (HourBCD & 0x0f));
   xlx = xlx + (int32)hr * 3600;
   da = ((10*(DayBCD>>4)) + (DayBCD & 0x0f) - 1);
   xlx = xlx + (int32)da * 86400;
   mo = ((10*(MonthBCD>>4)) + (MonthBCD & 0x0f));
    if (mo == 2)
     xlx = xlx + ((31)*86400);
    if (mo == 3)
     xlx = xlx + ((31+28)*86400);
    if (mo == 4)
     xlx = xlx + ((31+28+31)*86400);
    if (mo == 5)
     xlx = xlx + ((31+28+31+30)*86400);
    if (mo == 6)
     xlx = xlx + ((31+28+31+30+31)*86400);
    if (mo == 7)
     xlx = xlx + ((31+28+31+30+31+30)*86400);
    if (mo == 8)
     xlx = xlx + ((31+28+31+30+31+30+31)*86400);
    if (mo == 9)
     xlx = xlx + ((31+28+31+30+31+30+31+31)*86400);
    if (mo == 10)
     xlx = xlx + ((31+28+31+30+31+30+31+31+30)*86400);
    if (mo == 11)
     xlx = xlx + ((31+28+31+30+31+30+31+31+30+31)*86400);
    if (mo == 12)
     xlx = xlx + ((31+28+31+30+31+30+31+31+30+31+30)*86400);
   yr = (( (10*(YearBCD>>4)) + (YearBCD & 0x0f) ) - 1 );
//   printf("Sec (no yr) = %lu, yr = %u\n\r",xlx,yr);
   xlx = xlx + (yr * 31536000)+ ((yr/4) * 86400);
   if ( ((yr+1)%4 == 0) && (mo > 2))
      xlx = xlx + 86400;                  // account for Feb 29th during leap yr
  //DS_Installed = FALSE;
  DS_Installed = touch_present();
  //if (touch_present())
  if (DS_Installed)
   {
   //DS_Installed = TRUE;
   reset_pulse();
   delay_ms(10);
   touch_write_byte(0xcc);       // skip ROM search
   delay_ms(1);
   touch_write_byte(0x99);
   delay_ms(1);
   touch_write_byte(0xac);
   delay_ms(1);
   touch_write_byte(make8(xlx,0));
   delay_ms(1);
   touch_write_byte(make8(xlx,1));
   delay_ms(1);
   touch_write_byte(make8(xlx,2));
   delay_ms(1);
   touch_write_byte(make8(xlx,3));
   delay_ms(1);
   reset_pulse();
   delay_ms(1);
   }
  else
   {
     printf(hpo,"\n\r   *** touch not present in set_touch_time()\n\r");
   }
  return(DS_Installed);
}

//short get_touch_time(void)
BOOLEAN get_touch_time(void)
{
   //int hr,min,sec,mo,da,yr,i,iR[6];
   //int8 hr,min,sec,mo,da,yr,i,iR[6];
   //int32 xlx;
   BYTE hr,min,sec,mo,da,yr,i,iR[6];
   unsigned int32 xlx;
   BOOLEAN DS_Installed;

   hr = 0;                  // init time to 01/01/01 00:00:00
   min = 0;
   sec = 0;
   mo = 1;
   da = 1;
   yr = 1;
   //DS_Installed = FALSE;
   DS_Installed = touch_present();
   //if (touch_present())
   if (DS_Installed)
      {
      //DS_Installed = TRUE;
         reset_pulse();
         delay_ms(10);
         touch_write_byte(0xcc);       // skip ROM search
         delay_ms(1);
         touch_write_byte(0x66);       // read clock
         delay_ms(1);
         for (i=0;i<5;i++)
               {
               iR[i] = touch_read_byte();
               delay_ms(1);
               }   
      xlx = make32(iR[4],iR[3],iR[2],iR[1]);             // snap shot

      while (xlx >= (365 * 86400))
        {
        if (xlx >= (365 * 86400))
          {
          xlx = xlx - (365 * 86400);
          yr++;
          }
        if (xlx >= (365 * 86400))
          {
          xlx = xlx - (365 * 86400);
          yr++;
          }
        if (xlx >= (365 * 86400))
          {
          xlx = xlx - (365 * 86400);
          yr++;
          }
        if (xlx >= (366 * 86400))
          {
          xlx = xlx - (366 * 86400);
          yr++;
          }
        }
   // note we start with mo = 1 !!
      if ((yr%4) != 0)           // current year is not a leap year...
        {
        if (xlx >= (31+28+31+30+31+30+31+31+30+31+30)*86400)
          {
          xlx = xlx - (31+28+31+30+31+30+31+31+30+31+30)*86400;
          mo = 12;
          }
        if (xlx >= (31+28+31+30+31+30+31+31+30+31)*86400)
          {
          xlx = xlx - (31+28+31+30+31+30+31+31+30+31)*86400;
          mo = 11;
          }
        if (xlx >= (31+28+31+30+31+30+31+31+30)*86400)
          {
          xlx = xlx - (31+28+31+30+31+30+31+31+30)*86400;
          mo = 10;
          }
        if (xlx >= (31+28+31+30+31+30+31+31)*86400)
          {
          xlx = xlx - (31+28+31+30+31+30+31+31)*86400;
          mo = 9;
          }
        if (xlx >= (31+28+31+30+31+30+31)*86400)
          {
          xlx = xlx - (31+28+31+30+31+30+31)*86400;
          mo = 8;
          }
        if (xlx >= (31+28+31+30+31+30)*86400)
          {
          xlx = xlx - (31+28+31+30+31+30)*86400;
          mo = 7;
          }
        if (xlx >= (31+28+31+30+31)*86400)
          {
          xlx = xlx - (31+28+31+30+31)*86400;
          mo = 6;
          }
        if (xlx >= (31+28+31+30)*86400)
          {
          xlx = xlx - (31+28+31+30)*86400;
          mo = 5;
          }
        if (xlx >= (31+28+31)*86400)
          {
          xlx = xlx - (31+28+31)*86400;
          mo = 4;
          }
        if (xlx >= (31+28)*86400)
          {
          xlx = xlx - (31+28)*86400;
          mo = 3;
          }
        if (xlx >= (31)*86400)
          {
          xlx = xlx - (31)*86400;
          mo = 2;
          }
        }
        else        // current year is a leap year
        {
        if (xlx >= (31+29+31+30+31+30+31+31+30+31+30)*86400)
          {
          xlx = xlx - (31+29+31+30+31+30+31+31+30+31+30)*86400;
          mo = 12;
          }
        if (xlx >= (31+29+31+30+31+30+31+31+30+31)*86400)
          {
          xlx = xlx - (31+29+31+30+31+30+31+31+30+31)*86400;
          mo = 11;
          }
        if (xlx >= (31+29+31+30+31+30+31+31+30)*86400)
          {
          xlx = xlx - (31+29+31+30+31+30+31+31+30)*86400;
          mo = 10;
          }
        if (xlx >= (31+29+31+30+31+30+31+31)*86400)
          {
          xlx = xlx - (31+29+31+30+31+30+31+31)*86400;
          mo = 9;
          }
        if (xlx >= (31+29+31+30+31+30+31)*86400)
          {
          xlx = xlx - (31+29+31+30+31+30+31)*86400;
          mo = 8;
          }
        if (xlx >= (31+29+31+30+31+30)*86400)
          {
          xlx = xlx - (31+29+31+30+31+30)*86400;
          mo = 7;
          }
        if (xlx >= (31+29+31+30+31)*86400)
          {
          xlx = xlx - (31+29+31+30+31)*86400;
          mo = 6;
          }
        if (xlx >= (31+29+31+30)*86400)
          {
          xlx = xlx - (31+29+31+30)*86400;
          mo = 5;
          }
        if (xlx >= (31+29+31)*86400)
          {
          xlx = xlx - (31+29+31)*86400;
          mo = 4;
          }
        if (xlx >= (31+29)*86400)
          {
          xlx = xlx - (31+29)*86400;
          mo = 3;
          }
        if (xlx >= (31)*86400)
          {
          xlx = xlx - (31)*86400;
          mo = 2;
          }
        }
        
        
        
      while (xlx >= 86400)
        {
        xlx = xlx - 86400;
        da++;
        }
     while (xlx >= 3600)
        {
        xlx = xlx - 3600;
        hr++;
        }
     while (xlx >= 60)
        {
        xlx = xlx - 60;
        min++;
        }
        
      sec = xlx;           // all that should remin is seconds
     }  // end of 'if touch device is present'
   else
     {
       printf(hpo,"\n\r   *** touch not present in get_touch_time()\n\r");
     }

   SecondBCD = (((sec/10)<<4) + (sec%10));
   MinuteBCD = (((min/10)<<4) + (min%10));
   HourBCD = (((hr/10)<<4) + (hr%10));
   MonthBCD = (((mo/10)<<4) + (mo%10));
   DayBCD = (((da/10)<<4) + (da%10));
   YearBCD = (((yr/10)<<4) + (yr%10));

   return(DS_Installed);
}


#endif

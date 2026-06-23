#ifndef UPTIME_T2
#define UPTIME_T2

//
// use timer 2 to implement an uptime counter with one-millisecond units.
//
// PIC24 case:
// timer 2 counting signal, Fclk, is the PIC instruction cycle clock.
// that signal is the crystal oscillator frequency divided by two.
// in our case, that is 4915200 Hz / 2 = 2457600 Hz, or a period of 406.9 nS.
// if we counted at that frequency for the maximum interval (65536), 
// we would have a period of about 26.67 mS or a frequency of about 37.5 Hz
//
// 406.9 nS period X 2458 counts yields 1 mS.
//
// we write the value 2458 into the Period Match Register (PMR). timer counts its register from 0 to match, 
// then resets to zero and counts up again for the next cycle.
//
//
// note that for the PIC24, the timer used is the "type X" and differs in operation from the "timer 2" in the PIC16/18.
// functions appear in the manual as setup_timerX(), where the timer number replaces the 'X'.
// timer register is 16 bits as opposed to 8 bits for PIC16/18.
//
// a separate count value is maintained to represent seconds of uptime. every 1000 interrupts, 
// that timer is incremented.

void initUptimerMilliseconds()
{
   setup_timer2(TMR_INTERNAL, 2458);  // count for 1 Ms interrupt
}


// 
// timer 2 overflow interrupt handler
//
// just add one to the accumulating uptime counter
//
// PIC24 case:
// this count is close enough to a one-millisecond interrupt that
// we do not need to apply any correction means.
// Big Loop execution time was measured to be 72.46 microseconds,
// so the task-time execution will occur pretty close to when
// it is supposed to be.
//
//
// each time through this service, add one to the accumulating uptime counter.
// set a BOOLEAN flag so that the
// Big Loop will execute the 'tickTask()' for items which need
// to be executed on a fixed time base.
//
//

#INT_TIMER2
void timer2Isr()
{

   ++uptime_milliseconds;
   ++divideBy1000;  // also maintain the uptime-seconds count.
   if (divideBy1000 > 999)
      {
         divideBy1000 = 0;
         ++uptime_seconds;
         FlagHalfHz = !FlagHalfHz;
      }
   ++divideBy500;  // also maintain the half-second flag
   if (divideBy500 > 499)
      {
         divideBy500 = 0;
         FlagOneHz = !FlagOneHz;
      }
   
   // allow for periodic execution of quick tasks
   needTickTask = TRUE;  // trigger a tick task next big loop execution
}
  

// data-hiding functions

U32 uptimeMilliseconds()
{
   return uptime_milliseconds;
}      

 
U32 uptimeSeconds()
{
   return uptime_seconds;
}      


//
// called every one-millisecond timer tick.
// runs at task level, not in the timer ISR.
//
// employs counter prescaling to achieve lower frequencies for some items
//
void tickTask()
{
   --pbPrescale;  // count this down for divide-by-10 prescaling
   if (pbPrescale == 0)  // tenth time is when we act
   {
      pbPrescale = PB_CHECK_MSEC;  // reset for next interval
      DebouncePb();  // call the debouncing code
   }

   // we're going to flag updating for the RTC time/date values
   // and the LCD display of the time to occur every 250 mS.
   // to do this, we're going to time an interval of 125 mS and
   // alternate the action we take each time it occurs.

   --rtcLcdPrescale;  // count this down for divide-by-125 prescaling
   if (rtcLcdPrescale == 0)  // 125th time is when we act
   {
      rtcLcdPrescale = RTC_LCD_CHECK_MSEC;  // reset for next interval
      xorByte ^= 1;  // toggle the ls bit
      if (xorByte)  // use this value to alternate the two actions
      {
         updateLcdNow = TRUE;
      }
      else
      {
         dateTimeNeedsUpdate = TRUE;
      }
   }
}

#endif


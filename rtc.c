#ifndef RTCC
#define RTCC

//
// functions in this file interact with the Real Time Clock values as set into
// and updated in the PIC (PIC24FJ128GA204) processor.
//
// this code is transformed from using Dallas Touch Memory for clock/calendar functions
// to the Micro Crystal RV-3129-C3 Real Time Clock/Calendar module.
//
// that device uses an I2C interface so no special PIC driver is required, as had
// been the case with the Dallas device.
//
// a significant difference is that the MC device maintains the values
// as 8-bit BCD items, while the Dallas device used 8-bit binary numbers.
//
// we need to maintain the usage of the "datetime" stucture because it is used
// by the PIC processor real time clock driver functions. (rtc_read(), rtc_write(), etc.)
//

//
// initialize the time and date items in the PIC internal RTC
//
// inputs are the BCD representations (HourBCD, etc.) 
// get binary conversions and set into the 'datetime' structure,
// also save in the binary time item cells (HourBin, MinuteBin, SecondBin)
// then finally update the MC RV-3129 RTC device with same values
//
void xx_RTC_Write()
{
   datetime.tm_sec = SecondBin = bcd2hex(SecondBCD);
   datetime.tm_min = MinuteBin = bcd2hex(MinuteBCD);
   datetime.tm_hour = HourBin = bcd2hex(HourBCD);
   datetime.tm_mday = bcd2hex(DayBCD);
   datetime.tm_mon = bcd2hex(MonthBCD);
   datetime.tm_year = bcd2hex(YearBCD);
   rtc_write(&datetime);  // <-- sets the values into PIC RTC registers, from the 'datetime' structure members
   set_touch_time();  // <-- sets values into iButton device from HourBCD, MinuteBCD, SecondBCD, DayBCD, MonthBCD, YearBCD
   delay_ms(1); // Wait while data is being written (could poll here instead)
}


//
// this function is now used in configuration only
//
// extended to save binary values for hour, min, sec
//
void xx_RTC_Read(void)
{
   rtc_read(&datetime);
   SecondBin = datetime.tm_sec;
   SecondBCD = hex2bcd(SecondBin);
   MinuteBin = datetime.tm_min;
   MinuteBCD = hex2bcd(MinuteBin);
   HourBin = datetime.tm_hour;
   HourBCD = hex2bcd(HourBin);
   DayBCD = hex2bcd(datetime.tm_mday);
   MonthBCD = hex2bcd(datetime.tm_mon);
   YearBCD = hex2bcd(datetime.tm_year);
}



//
// initialize the PIC real time clock using the date and time stored in the
// Dallas device. ("TouchTime")  If that device is not readable, apply some appropriate
// default values.
//
// ##### in the touch memory version of this code, 'get_touch_time()' returns a BOOLEAN 'TRUE' if
// ##### the touch device initialized OK. but it also has read the time values and stored as BCD
// ##### in the conventional memory cells HourBCD, MinuteBCD, SecondBCD, DayBCD, MonthBCD, YearBCD.
// #####
// ##### for this new CM RV-3129-C3 version, the returned BOOLEAN just indicates device was found and inititlized.
// ##### it is ne3cessary for code here in the init routine to read the initial values and store appropriately.
//
// initialize PIC24 Real Time Clock, date-time structure, and individual values from the CM RV-3129-C3 clock calendar device
//
void xx_RTC_init()
{

   // first call the RM-3129 initialization code. That sets the BCD-formatted cells
   //RTC_InitDevice();
   //ex_RTC_Init();  <-- REPLACE THIS with the old-style initialization
   if (!get_touch_time())  // DALLAS device unresponsive, supply default separate items
      {
      HourBCD = 00;
      MinuteBCD = 00;
      SecondBCD = 01;
      DayBCD = 0x16;
      MonthBCD = 0x11;
      YearBCD = 0x61;
      }
   delay_cycles(1);
   // then use those values to set up the PIC24 real time clock
   xx_RTC_Write();  // form a standard structure from the separate items and set PIC24 RTC, Dallas device
}

//
// simple check for integer representing valid 2-digit BCD
//
// allowable lower digit is 0..9
// 'tensMax' is the allowable upper digit. 
// e.g. upper digit for time can only be 5 or less
//
BOOLEAN vbcd(BYTE putativeBCD, BYTE tensMax)
{
   BYTE unitDigit = putativeBCD & 0x0F;
   BYTE tensDigit = (putativeBCD >> 4) & 0x0F;
   return (unitDigit < 10) && (tensDigit <= tensMax);
}


#endif


#ifndef  TOUCHY_C
#define  TOUCHY_C

//
// the source of this file is the standard Dallas Touch Driver file (touch.c) from the PICC/Drivers directory.
// modified by SONOTRONICS to disable interrupts from the one-millisecond TIMER2 interrupt code while the
// touch memory device is being accessed.
//
// this is done without fear, the code involved is executed *VERY SELDOM* in the course of our application's
// execution: upon initialization, where the stored date and time values are read and used to set the PIC24 RTCC
// and also if setting of the date and time is performed. Neither of these things happens often, so the
// impact of the possible lost interrupts is minimal.
//


///////////////////////////////////////////////////////////////////////////
////                        Dallas Touch Driver                        ////
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



//#ifndef TOUCH_PIN
//   #define TOUCH_PIN  PIN_B0
//#endif

#define TOUCH_PIN_LOW()    output_drive(TOUCH_PIN); output_low(TOUCH_PIN)
#define TOUCH_PIN_HIGH()    output_drive(TOUCH_PIN); output_high(TOUCH_PIN)
#define TOUCH_PIN_FLOAT()  output_float(TOUCH_PIN)
#define TOUCH_PIN_READ()      input_state(TOUCH_PIN)

/////////////////////////////
////                     ////
//// Function Prototypes ////
////                     ////
/////////////////////////////

/*
int1 touch_read_bit()
This will read back a bit from the DS1993
PARAMS: none
RETURNS: A bit from the DS1993
*/
int1 touch_read_bit();

/*
BYTE touch_read_byte()
This will read back a byte from the DS1993
PARAMS: none
RETURNS: A byte from the DS1993
*/
BYTE touch_read_byte();

/*
BOOLEAN touch_write_bit(int1 data)
This will write a bit to the DS1993
PARAMS: The bit to write
RETURNS: True if completed successfully, false if otherwise
*/
BOOLEAN touch_write_bit(int1 data);

/*
BOOLEAN touch_write_byte(BYTE data)
This will write a byte to the DS1993
PARAMS: The byte to write
RETURNS: True if completed successfully, false if otherwise
*/
BOOLEAN touch_write_byte(BYTE data);

/*
BOOLEAN touch_present()
This will evaluate whether or not there is a touch present on the DS1993
PARAMS: none
RETURNS: True if a touch is present, false if otherwise
*/
BOOLEAN touch_present();

/*
void reset_pulse()
This will send the DS1993 a reset pulse
PARAMS: none
RETURNS: none
*/
void reset_pulse();

///////////////////////////////////////////////
////                                       ////
//// SONOTRONICS Added Function Prototypes ////
////                                       ////
///////////////////////////////////////////////



BOOLEAN inhibitTimerInts();
void restoreTimerInts(BOOLEAN);




//////////////////////////////////
////                          ////
//// Function Implementations ////
////                          ////
//////////////////////////////////

/*
int1 touch_read_bit()
This will read back a bit from the DS1993
PARAMS: none
RETURNS: A bit from the DS1993
*/
int1 touch_read_bit()
{
   int1 data;

   TOUCH_PIN_LOW();
   delay_us(14);
   TOUCH_PIN_FLOAT();
   delay_us(5);
   data = TOUCH_PIN_READ();
   delay_us(100);

   return data;
}

/*
BYTE touch_read_byte()
This will read back a byte from the DS1993
PARAMS: none
RETURNS: A byte from the DS1993
*/
BYTE touch_read_byte()
{
   BYTE i,data;

   // protect this time-critical code from timer interrupts while it runs
   BOOLEAN intStat = inhibitTimerInts();  // save currently-enabled status

   for(i=1; i <= 8; ++i)
      shift_right(&data, 1, touch_read_bit());

   // restore timer interrupts if appropriate
   restoreTimerInts(intStat);

   return data;
}

/*
BOOLEAN touch_write_bit(int1 data)
This will write a bit to the DS1993
PARAMS: The bit to write
RETURNS: True if completed successfully, false if otherwise
*/
BOOLEAN touch_write_bit(int1 data)
{
   TOUCH_PIN_LOW();
   delay_us(10);
   if(data)
   {
      TOUCH_PIN_HIGH();
      delay_us(10);
      if(!TOUCH_PIN_READ())
         return FALSE;
   }
   else
   {
      TOUCH_PIN_LOW();
      delay_us(10);
      if(TOUCH_PIN_READ())
         return FALSE;
   }
   delay_us(50);
   TOUCH_PIN_HIGH();
   delay_us(50);
   return TRUE;
}

/*
BOOLEAN touch_write_byte(BYTE data)
This will write a byte to the DS1993
PARAMS: The byte to write
RETURNS: True if completed successfully, false if otherwise
*/
BOOLEAN touch_write_byte(BYTE data)
{
   BYTE i;
   
   // protect this time-critical code from timer interrupts while it runs
   BOOLEAN intStat = inhibitTimerInts();  // save currently-enabled status

   for(i=1; i<=8; ++i)
      if(!touch_write_bit(shift_right(&data, 1, 0)))
      {
         // restore timer interrupts if appropriate
         restoreTimerInts(intStat);
         
         return FALSE;
      }

   // restore timer interrupts if appropriate
   restoreTimerInts(intStat);

   return TRUE;
}

/*
BOOLEAN touch_present()
This will evaluate whether or not there is a touch present on the DS1993
PARAMS: none
RETURNS: True if a touch is present, false if otherwise
*/
BOOLEAN touch_present()
{
   BOOLEAN present;
   
   // protect this time-critical code from timer interrupts while it runs
   BOOLEAN intStat = inhibitTimerInts();  // save currently-enabled status

   TOUCH_PIN_LOW();
   delay_us(500);
   TOUCH_PIN_FLOAT();
   delay_us(5);

   if(!TOUCH_PIN_READ())
   {
      // restore timer interrupts if appropriate
      restoreTimerInts(intStat);
      
      return FALSE;
   }

   delay_us(65);
   present = !TOUCH_PIN_READ();
   delay_us(240);

   // restore timer interrupts if appropriate
   restoreTimerInts(intStat);
   
   return present;
}

/*
void reset_pulse()
This will send the DS1993 a reset pulse
PARAMS: none
RETURNS: none
*/
void reset_pulse()
{
   // protect this time-critical code from timer interrupts while it runs
   BOOLEAN intStat = inhibitTimerInts();  // save currently-enabled status

   TOUCH_PIN_LOW();
   delay_us(500);
   TOUCH_PIN_FLOAT();
   delay_us(5);
   while(!touch_present());

   // restore timer interrupts if appropriate
   restoreTimerInts(intStat);

}

/////////////////////////////////////
////                             ////
//// SONOTRONICS Added Functions ////
////                             ////
/////////////////////////////////////



BOOLEAN inhibitTimerInts()
{
   BOOLEAN timer2InUse = interrupt_enabled(INT_TIMER2);  // check whether timer 2 is enabled now
   if (timer2InUse)
   {
      disable_interrupts(INT_TIMER2);
   }
   return timer2InUse;
}


void restoreTimerInts(BOOLEAN timer2InUse)
{

   if (timer2InUse)
      {
         enable_interrupts(INT_TIMER2);
      }
}


#endif


#ifndef __LCDSMS_C__
#define __LCDSMS_C__
///////////////////////////////////////////////////////////////////////////////
////                         LCD_SMS.C                                     ////
////                      --derived from--                                 ////
////                             LCD.C                                     ////
////                 Driver for common LCD modules                         ////
////                                                                       ////
////  lcd_init()   Must be called before any other function.               ////
////                                                                       ////
////  lcd_putc(c)  Will display c on the next position of the LCD.         ////
////                 \a  Set cursor position to upper left                 ////
////                 \f  Clear display, set cursor to upper left           ////
////                 \n  Go to start of second line                        ////
////                 \b  Move back one position                            ////
////              If LCD_EXTENDED_NEWLINE is defined, the \n character     ////
////              will erase all remanining characters on the current      ////
////              line, and move the cursor to the beginning of the next   ////
////              line.                                                    ////
////              If LCD_EXTENDED_NEWLINE is defined, the \r character     ////
////              will move the cursor to the start of the current         ////
////              line.                                                    ////
////                                                                       ////
////  lcd_gotoxy(x,y) Set write position on LCD (upper left is 1,1)        ////
////                                                                       ////
////  lcd_getc(x,y)   Returns character at position x,y on LCD             ////
////                                                                       ////
////  lcd_cursor_on(int1 on)   Turn the cursor on (on=TRUE) or off         ////
////              (on=FALSE).                                              ////
////                                                                       ////
////  lcd_set_cgram_char(w, *p)   Write a custom character to the CGRAM.   ////
////                                                                       ////
////                                                                       ////
////  CONFIGURATION                                                        ////
////  The LCD can be configured in one of two ways: a.) port access or     ////
////  b.) pin access.  Port access requires the entire 7 bit interface     ////
////  connected to one GPIO port, and the data bits (D4:D7 of the LCD)     ////
////  connected to sequential pins on the GPIO.  Pin access                ////
////  has no requirements, all 7 bits of the control interface can         ////
////  can be connected to any GPIO using several ports.                    ////
////                                                                       ////
////  To use port access, #define LCD_DATA_PORT to the SFR location of     ////
////  of the GPIO port that holds the interface, -AND- edit LCD_PIN_MAP    ////
////  of this file to configure the pin order.  If you are using a         ////
////  baseline PIC (PCB), then LCD_OUTPUT_MAP and LCD_INPUT_MAP also must  ////
////  be defined.                                                          ////
////                                                                       ////
////  Example of port access:                                              ////
////     #define LCD_DATA_PORT getenv("SFR:PORTD")                         ////
////                                                                       ////
////  To use pin access, the following pins must be defined:               ////
////     LCD_ENABLE_PIN                                                    ////
////     LCD_RS_PIN                                                        ////
////     LCD_RW_PIN                                                        ////
////     LCD_DATA4                                                         ////
////     LCD_DATA5                                                         ////
////     LCD_DATA6                                                         ////
////     LCD_DATA7                                                         ////
////                                                                       ////
////  Example of pin access:                                               ////
////     #define LCD_ENABLE_PIN  PIN_E0                                    ////
////     #define LCD_RS_PIN      PIN_E1                                    ////
////     #define LCD_RW_PIN      PIN_E2                                    ////
////     #define LCD_DATA4       PIN_D4                                    ////
////     #define LCD_DATA5       PIN_D5                                    ////
////     #define LCD_DATA6       PIN_D6                                    ////
////     #define LCD_DATA7       PIN_D7                                    ////
////                                                                       ////
///////////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2010 Custom Computer Services           ////
//// This source code may only be used by licensed users of the CCS C  ////
//// compiler.  This source code may only be distributed to other      ////
//// licensed users of the CCS C compiler.  No other use, reproduction ////
//// or distribution is permitted without written permission.          ////
//// Derivative programs created using this software in object code    ////
//// form are not restricted in any way.                               ////
///////////////////////////////////////////////////////////////////////////

//// ##################################################################### ////
////  The hardware pin assignments for LCD operation usually found here    ////
////  have been relocated to the hardware-specific header file for         ////
////  the PIC processor in use.                                            ////
////                                                                       ////
////                                                                       ////
//// ##################################################################### ////

// define the pinout.
// only required if port access is being used.
typedef struct  
{                            // This structure is overlayed
   int1 rs;           // on to an I/O port to gain
   int1 enable;               // access to the LCD pins.
   int1 rw;               // The bits are allocated from
   int1 unused;           // low order up.  ENABLE will
   unsigned int     data : 4;         // be LSB pin of that port.
   unsigned int    reserved: 8;
} LCD_PIN_MAP;


#define lcd_output_enable(x) output_bit(LCD_ENABLE_PIN, x)

#define lcd_output_rs(x) output_bit(LCD_RS_PIN, x)

#define LCD_TYPE 2           // 0=5x7, 1=5x10, 2=2 lines

#define LCD_LINE_TWO 0x40    // LCD RAM address for the second line

#define LCD_LINE_LENGTH 16


void lcd_send_nibble(char n)
{
   output_bit(LCD_DATA4, bit_test(n, 0));
   output_bit(LCD_DATA5, bit_test(n, 1));
   output_bit(LCD_DATA6, bit_test(n, 2));
   output_bit(LCD_DATA7, bit_test(n, 3));
      
   delay_cycles(10);
   lcd_output_enable(1);
   delay_us(20);
   lcd_output_enable(0);
}

void lcd_send_byte(char address, char n)
{
//   lcd_enable_tris();
//   lcd_rs_tris();

   lcd_output_rs(0);
   delay_ms(1);
   lcd_output_rs(address);
   delay_cycles(1);
   //delay_cycles(1);
   lcd_output_enable(0);
   lcd_send_nibble(n >> 4);
   lcd_send_nibble(n & 0xf);
}

void lcd_init(void) 
{
   char i;
   char LCD_INIT_STRING[4] = {0x20 | (LCD_TYPE << 2), 0xc, 1, 6};
                             // These bytes need to be sent to the LCD
                             // to start it up.
   
   output_high(LCD_V);        // apply power to LCD via port pin
   delay_ms(100);             // give it a moment
   lcd_output_enable(0);
   lcd_output_rs(0);

   output_drive(LCD_DATA4);
   output_drive(LCD_DATA5);
   output_drive(LCD_DATA6);
   output_drive(LCD_DATA7);
//   lcd_enable_tris();
//   lcd_rs_tris();
    
   delay_ms(15);
   for(i=1;i<=3;++i)
   {
       lcd_send_nibble(3);
       delay_ms(5);
   }
   
   lcd_send_nibble(2);
   delay_ms(5);
   for(i=0;i<=3;++i)
      lcd_send_byte(0,LCD_INIT_STRING[i]);
}
void lcd_gotoxy(char x, char y)
{
   char address;
   
   if(y!=1)
      address=LCD_LINE_TWO;
   else
      address=0;
     
   address+=x-1;
   lcd_send_byte(0,0x80|address);

}


void lcd_putc(char c)
{
   switch (c)
   {
      case '\a'   :  lcd_gotoxy(1,1);      break;  // TAZ added this back in

      case '\f'   :  lcd_send_byte(0,1);
                     delay_ms(2);
                     break;

      case '\n'   : lcd_gotoxy(1,2);        break;
      
      case '\r'   :  lcd_gotoxy(1,1);       break;
     
      case '\b'   : lcd_send_byte(0,0x10);  break;
     
      default     : lcd_send_byte(1,c);     break;
   }
}
 
// write a custom character to the ram
// which is 0-7 and specifies which character array we are modifying.
// ptr points to an array of 8 bytes, where each byte is the next row of
//    pixels.  only bits 0-4 are used.  the last row is the cursor row, and
//    usually you will want to leave this byte 0x00.
void lcd_set_cgram_char(char which, char *ptr)
{
   unsigned int i;

   which <<= 3;
   which &= 0x38;

   lcd_send_byte(0, 0x40 | which);  //set cgram address

   for(i=0; i<8; i++)
   {
      lcd_send_byte(1, *ptr++);
   }
  
}

void lcd_cursor_on(int1 on)
{
   if (on)
   {
      lcd_send_byte(0,0x0F);           //turn LCD cursor ON
   }
   else
   {
      lcd_send_byte(0,0x0C);           //turn LCD cursor OFF
   }
}

#endif  // needed to add this?? suddenly, why?

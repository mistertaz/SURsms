#ifndef INITS_C
#define INITS_C

//
// common application initialization code
//

//
// pre-configuration initialization
//
// initialization of items which do not depend upon configured values
//
void preConfigInit()
{

#if __USING_ICD > 0
   printf(dpo, "\r\n");
   printf(dpo, "%s\r\n", fortyBucks);
   printf(dpo, "%s\r\n", fortyBucks);
   printf(dpo, "$$$$$$$$$$$$$ ICD ENABLED $$$$$$$$$$$$$$\r\n");
   printf(dpo, "%s\r\n", fortyBucks);
   printf(dpo, "%s\r\n", fortyBucks);
#warning/information $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#warning/information $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#warning/information $$$$$$$$$$$$$ ICD ENABLED $$$$$$$$$$$$$$
#warning/information $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#warning/information $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#endif

   //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
   
   // hardware initializations are done right here
   set_tris_c(0x1A0);
   // Marlin sets a different value, must determine whether it makes a difference
   //set_tris_c(0x0A0);
   // the difference is BIT_C8 which for the PIC24 smsbox is called cCT
   // cCT connects to pin 12 on the XBEE modem
   // modem chip defines that pin as 'Output Clear-to-Send Flow Control'
   // we ignore that signal, but should we define it as an input?
   // if we did, the tris bit should be a '1' for an input, and it is.
   
   
   // need these pullups on I2C signals
   set_pullup(TRUE, EEPROM_SCL);
   set_pullup(TRUE, EEPROM_SDA);
   
   // Marlin also does this
   //i2c_init(TRUE);
   
   // BlueToothLowEnergy
   // if power was lost, reset the controller.
   // otherwise leave it alone so any existing connection might be preserved.
   // this doesn't seem to work as we wished.
   // just clobber the controller every time, connection cannot be preserved
   resetBT();  // formally reset BTLE controller
//!   BTLE_SpyOrientation = __BTLE_SPY_UNDEFINED;  // reset spy so direction is undefined
   
//!   UsingHostSUR = TRUE;  // kludge for development
//!   printf(dpo, "\r\n*****\r\n***** FORCED host port SUR\r\n*****\r\n");

   // XBEE SMS modem
   // create a reset pulse signal
   output_high(Rst);  // deassert /RESET signal
   delay_us(500);  // swag delay of 500 us
   output_low(Rst);  // this is '/RESET' for SMS modem, active low, set high to release reset.
   delay_us(500);  // swag reset pulse of 500 us
   output_high(Rst);  // release from reset
   delay_us(500);  // wait a third time
   set_pulldown(TRUE, PS);  // to get quick response on this input, apply weak pulldown
   //set_pullup(TRUE, PS);  // pulldown made no difference, try pullup. pullup made no difference.
   output_low(PwKy);  // ensure modem pin sleep signal is not asserted so modem will be active once pin-sleep becomes enabled
   
   // check the state of XBEE modem signal CTS/
   // this signal is pin 12 from the XBEE
   // that signal is wired to PIN_C8 on the PIC24, and is defined to be "cCT".
   // if unasserted, modem has not correctly come up
   // system will need to be power-cycled
   xbeeFailedReset = (input(cCT) != 0);  // XBEE CTS is NOT ASSERTED (signal is high)

   // this stuff moved here in the code, before RTC setup is done
   // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#if __DEBUG_INIT || __DEBUG_INIT_CORE
   itsAlive();
#endif
   shadowedEEPROMSetup();

   // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

   setup_rtc(RTC_ENABLE | RTC_CLOCK_SOSC , 0);  // this sets up the RTCC in the PIC24
   
   xx_RTC_init();  // initialize PIC24 Real Time Clock, date-time structure, and individual values from the CM RV-3129-C3 clock calendar device
   

   initUptimerMilliseconds();
   
   enable_interrupts(INT_TIMER2);  // start using timer 2 for one-millisecond time base right away
   
//!#if __USING_SMSISR
//!   // initialize XBEE modem input character handling routines so they are ready when ISR starts
//!   XBeeDivertInput = FALSE;  // has to default to api packet input
//!   stringsParseSetInitialConditions(XBEEstrings);  // setup the string parsing process
//!   resetPacketsParse();  // setup the protocol parsing state machine
//!   clear_interrupt(INT_RDA2);
//!   enable_interrupts(INT_RDA2);  // using this to receive serial data from XBee modem (SMS port)
//!#endif
   
   enable_interrupts(GLOBAL);  // enable interrupt system
    
   output_high(LCD_V);
   delay_ms(2000);
   lcd_init();

   //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

   // set up analog channel 0 in the ADC (12 bit read)
   set_analog_pins(PIN_A0);
   setup_adc_ports(sAN0, VSS_VDD);
   setup_adc(ADC_CLOCK_INTERNAL);
   set_adc_channel(0);
   // battery voltage is an analog input.
   // the expression used to convert the 12-bit ADC reading to a voltage value is:
   // 2 * <adc> / 4096 * 3.3
   // we can do this using floating point computations, but that adds a lot of code
   // which has no other use in this app.
   // what we do instead is keep everything as integers and multiply it all by 100.
   // that gives us a number which we can format as integer and then use character-by-character.
   // we can streamline the computation by rearranging the terms.
   // 2 * 3.3 * 100 * <adc> / 4096 --> 660 * <adc> /4096
   // then we can eliminate the division by right-shifting the result by 12 bits.
   // the computation may be further simplified if we precompute 66/4096 and invert it.
   // that yields 62, and the entire voltage computation becomes <adc>/62. 
   // 16-bit integers are the largest fields we need to use.
   // that result is voltage*10, so we need to accomodate that for display.
   // the results of that computation will always be less than 67, and will be
   // expressible in two decimal digits.
   //
   // read and convert the adc signal right now, determine if power state should be good or low
   BatteryVoltage = read_adc() / 62;  // read and convert it now so initial value is reasonable
   // unconditionally claim voltage is good at this point
   PowerStateCode = POWER_GOOD;
   BatteryVoltageOK = TRUE;

   // initialize display line image buffers
   fillBytes(DisplayLine1String, 20, ' ', TRUE);
   fillBytes(DisplayLine2String, 20, ' ', TRUE);
   fillBytes(DisplayLine3String, 20, ' ', TRUE);
   fillBytes(DisplayLine4String, 20, ' ', TRUE);
   fillBytes(SavedDisplayLine1String, 20, '#', TRUE);
   fillBytes(SavedDisplayLine2String, 20, '#', TRUE);
   fillBytes(SavedDisplayLine3String, 20, '#', TRUE);
   fillBytes(SavedDisplayLine4String, 20, '#', TRUE);
   
   //                          12345678901234567890
   sprintf(DisplayLine1String,"SMS v%X.%X            ", vmsd, vlsd);
   //                         12345678901234567890
   strcpy(DisplayLine2String,"INITIALIZING        ");
   //strcpy(DisplayLine2String,"INITZING            ");
   maintainLCD(FALSE);  // updates display contents not top line
   
   output_low(LED);  // use this LED to indicate xbee modem registered with cell tower when on

   lastSecond = SecondBCD; 

// above here is the processor and peripheral device hardware initialization
// below here is the app program initialization

   setupAWordQueue(apiRcvQueue, apiRcvQueueData, API_RCV_SLOTS+1, lblApiRcv);
   setupAWordQueue(apiXmitQueue, apiXmitQueueData, API_XMIT_SLOTS+1, lblApiXmit);
   setupAWordQueue(smsPktDispQueue, smsPktDispQueueData, SMS_PKT_DISP_SLOTS+1, lblSmsPktDisp);
   setupAWordQueue(smsStatMatchQueue, smsStatMatchQueueData, SMS_STAT_MTCH_SLOTS+1, lblSmsStatMatch);
//!   setupAWordQueue(smsRcvQueue, smsRcvQueueData, SMS_RCV_SLOTS+1, lblSmsHdr);
//!   setupAWordQueue(smsrspRcvQueue, smsrspRcvQueueData, SMS_RSP_RCV_SLOTS+1, lblSmsRspHdr);
//!   setupAWordQueue(atqRcvQueue, atqRcvQueueData, ATQ_RCV_SLOTS+1, lblAtqHdr);
//!   setupAWordQueue(atsRcvQueue, atsRcvQueueData, ATS_RCV_SLOTS+1, lblAtsHdr);
//!   setupAWordQueue(atcRcvQueue, atcRcvQueueData, ATC_RCV_SLOTS+1, lblAtcHdr);

//!#if __DEBUG_INIT
//!   printf(dpo, "CFG_NVMshadow buffers @ 0x%LX, 0x%LX\r\n", &CFG_NVMshadow[0], &CFG_NVMshadow[1]);
//!   printf(dpo, "FLTR_NVMshadow buffers @ 0x%LX, 0x%LX\r\n", &FLTR_NVMshadow[0], &FLTR_NVMshadow[1]);
//!   printf(dpo, "NVRAM Filter Structure size %ld[x%lX]\r\n", (int16)FILTER_NVM_STRUCT_SIZE, (int16)FILTER_NVM_STRUCT_SIZE);
//!   printf(dpo, "TTAG Structure Size %ld\r\n", (int16)sizeof(TTAG));
//!#endif   

   // set up allocatable message buffers

   //createPacketBufferPool();  // this is for API mode packet buffers.
   createLargeBufferPool();  // larger one first
   createSmallBufferPool();
   
   // kludge these pushbutton configuration items so 'buttonCycled()' will work for testing in 'configuration()'
   longPressThresholdSecs = 3;
   longPressThresholdMsecs = 3000UL;
  
   buttonCycled(TRUE);  // initialize the switch debouncing code in the app
   
}




//
// configuration item value initialization
//
void configuredValueInit()
{

   // set pushbutton long-press threshold based on configured value
   longPressThresholdSecs = CFG_NVMshadow[cfg_cur][PBLPRTHR];
   if ((longPressThresholdSecs < 2) || (longPressThresholdSecs > 5))  // allowable range is 2..5, force to 3 if out of range
   {
#if __DEBUG_INIT
      if (dbpEnabled(LEV4))
      {
         printf(dpo, "LONG PRESS THRESHOLD SETTING OUT-OF-RANGE %d\r\n", longPressThresholdSecs);
      }
#endif
      longPressThresholdSecs = 3;
   }
   longPressThresholdMsecs = longPressThresholdMsecVector[longPressThresholdSecs - 2];  // express the minutes duration in milliseconds via table lookup
#if __DEBUG_INIT
   if (dbpEnabled(LEV3))
      {
         printf(dpo, "LONG PRESS THRESHOLD SETTING SEC/MSEC CONFIG %d:%Lu\r\n", longPressThresholdSecs, longPressThresholdMsecs);
      }
#endif
   

}



//
// post-configuration initialization
//
// initialization of items dependent upon configured values
//
void postConfigInit()
{
   BYTE *resp = lballoc();  // allocate large string data buffer
   char SurSimu[] = "SIMULATION";
  
   // send a benign command to the SUR (or the simulated SUR) which will allow us
   // to determine whether the connection is an actual SUR or a simulator.
   relaxedSurTiming = TRUE;  // assumption is a simulator unless disproved
   checkCommAndRecover(resp, LARGE_BUFFER_SIZE);  // will nominally return a string with date & time to 'resp'
   
   if (strlen((char *)resp))  // got something
      {
#if __DEBUG_INIT
         if (dbpEnabled(LEV4))
            {
               printf(dpo, "AT STARTUP, checkCommAndRecover() RETURNED");
               dumpToHost(resp, strlen((char *)resp));
            }
#endif
         // if connected to the simulator, the returned date & time string will also contain "SIMULATION"
         // look for that string and if found, retain the relaxed timing.
         // otherwise, tighten the timing appropriate for the actual SUR.
         if (strstr((char *)resp, SurSimu)) // non-null result means SIMULATE string is present
            {
               relaxedSurTiming = TRUE;  // use relaxed timing
            }
         else
            {
               relaxedSurTiming = FALSE;  // use strict timing
            }
      }
   else
      {
#if __DEBUG_INIT
         if (dbpEnabled(LEV4))
            {
               printf(dpo, "AT STARTUP, checkCommAndRecover() FAILED\r\n");
            }
#endif
         relaxedSurTiming = TRUE;  // no choice but to use the relaxed timing
      }
#if __DEBUG_INIT
   if (dbpEnabled(LEV3))
   {
      if (relaxedSurTiming)
      {
         printf(dpo, "AT STARTUP, SUR COMM TIMING IS RELAXED\r\n");
      }
      else
      {
         printf(dpo, "AT STARTUP, SUR COMM TIMING IS STRICT\r\n");
      }
   }
#endif
   
   // finished with buffer, so free it. this can only be a large buffer
   lbfree(resp);
   
   configuredValueInit();  // set or compute items based on possibly-changed configuration values

   // set up SMS modem based on configured power usage state
   // rely on power-up defaults as factory settings, change only those necessary.
   // don't do this if the modem came up in the wrong CTS state.
   //
   // configure XBee modem hardware for initial startup
   // following system reset.
   // the result of this action is always modem running at full power,
   // and presumably associated with the cell tower.
   //
   // let's do the status spin loop waiting for association to be complete
   //
   // using a new interpretation of modem handling, let's also shut the
   // modem down right here.
   //
   if (isXbeeOkToUse())
   {
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "appInitialModemStartUp()  %s\r\n", stringTheDateTimeUptime());
      }
#endif
   
      xbeeSetPowerControlFlags();  // set flags for low power operating mode
      xbeeWakeUp();  // deassert sleep signal pin now, whether or not it is sleeping
      // this can fail if the modem is unresponsive
      xbeeFailedConfiguration = xbeeRuntimeConfiguration();  // this reply value directly usable to set the flag
      
      if (!xbeeFailedConfiguration)  // if configuration didn't fail, do these other thingsw
      {
         // set status indicating modem is up
         setModemActive();
         
         // 'shut down' modem. this means place it into whichever low power mode is configured.
         appShutDownModem();  // unrecoverable packet parse after this
      }
   }
   
//!#if !__USING_SMSISR  // unnecessary if the interrupt stuff works
   // delete any pending received characters
   drainSmsBufAndTrash();  // appropriate to do this here
//!#endif
   
   if (!isXbeeOkToUse())  // modem has failed prior to this point
   {
      needResetMessage();  // display failed message, should be accurate now
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "appInitialModemStartUp() -- SKIPPED, PRIOR MODEM FAILURE  %s\r\n", stringTheDateTimeUptime());
      }
#endif
   }
   
}


void shadowedEEPROMSetup()
{
   BOOLEAN NeedToDefaultEeprom = FALSE;  // do we need to set default values into configuration EEPROM?

   // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
   // this stuff moved way earlier in the code, before RTC setup is done
   //
   //   if CFG_NVM was initialized in a different version,
   //   or if the current CFG_NVM CRC is incorrect,
   //   store the current version now,
   //   then set all configurable items to default contents.
   //   new CRC is computed and stored.
   //

   init_ext_eeprom();  // initialize vendor-supplied driver for external EEPROM used as configuration memory

   // copy entire CFG_NVM contents to both shadow arrays in RAM 
   fetchShadowCFG_NVM();
   
   // these messages always print out
   if(CFG_NVMshadow[cfg_cur][VERS] != VERSION)
   {
      printf(dpo,"*** preConfigInit() -- CFG_NVM version 0x%02X mismatches program version 0x%02X\r\n", CFG_NVMshadow[cfg_cur][VERS], VERSION);
      NeedToDefaultEeprom = TRUE;  // we need to set default values into configuration EEPROM
   }
   else if (!isShadowCrcCorrect())
   {
      printf(dpo,"*** preConfigInit() -- Internal CFG_NVM CRC is invalid\r\n");
      NeedToDefaultEeprom = TRUE;  // we need to set default values into configuration EEPROM
   }
   
   if (NeedToDefaultEeprom)
   {
      printf(dpo,"*** preConfigInit() -- Internal CFG_NVM set to default initial values...\r\n");
      defaultShadowCFG_NVM();
   }
//!   else
//!   {
//!      printf(dpo,"*** preConfigInit() -- Internal CFG_NVM  version and CRC are valid...\r\n");
//!   }
   
   

//
//   if FLTR_NVM was initialized in a different version, store the current version now,
//   then set all configurable items to default contents.
//   this will invalidate the CRC.
//

// copy entire FLTR_NVM contents to shadow array in RAM 
   fetchShadowFLTR_NVM();

   // these messages are always output
   if (FLTR_NVMshadow[flt_cur].Bv[FLTR_NVM_VERSION] != VERSION)
   {
      printf(dpo,"*** preConfigInit() -- FLTR_NVM version 0x%02X mismatches program version 0x%02X\r\n", FLTR_NVMshadow[flt_cur].Bv[FLTR_NVM_VERSION], VERSION);
      defaultShadowFLTR_NVM();
   }
//!   else
//!   {
//!      printf(dpo,"*** preConfigInit() -- FLTR_NVM version OK\r\n");
//!   }
   

}




#endif

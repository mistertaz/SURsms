/*
 *
 * SURsms configuration management
 *
 * upon entry, look for user typein of three consecutive 'z' characters
 
 * (case irrelevant) to signal that configuration is desired.
 * if entry of three z-s is not completed within 15 seconds, exit.
 *
 * since this code interacts with a console user, always use
 * the SURport redirection unless explicitly necessary to do otherwise.
 *
 */
 
 // command interpretation big loop (modal operation), or single command.
 // command sequence introducer (CSI) is <esc> or ascii '+'
 // if the argument 'calledFromConfig' is TRUE, this code behaves traditionally,
 // looping while awaiting CSI, and processing command(s) until it receives
 // the 'q' command to quit, or the 'p' command to processor reset.
 // if the argument 'calledFromConfig' is FALSE, this code will process one
 // command and then return. the CSI is implied. (called from elsewhere that CSI occurs)
 //
 // operates much like the SUR in that receipt of <esc> signals the start of a command+
 // the <esc> is echoed as a ':', after which the next character or two comprise the command,
 // followed by such data fields as are required.
 //
 // where it makes sense, commands with functions similar to the SUR will be identical
 // to commands in that device. Not possible for all commands, but it's a goal upon which to focus.
 //

void commandProc(BOOLEAN calledFromConfig=FALSE)
{
   BYTE UpdateSuccess[] = "CFG_NVM updated";  // SURsoftSMS looks for this exact string to determine CFG_NVM update successful
   unsigned int16 ii;
   int i,j,k ;  // scratch integers
   BOOLEAN MCond;  // scratch BOOLEAN
   int lengthOfCommandTail;  // string length minus the command character
   BYTE ArgCh;
   BYTE TempCh;
   BYTE CommandCh;
   signed int16 sval;
   BOOLEAN keepGoing = calledFromConfig;  // if called via 'zzz', enable everything
   BOOLEAN inputErrorStartOver = FALSE;

   GlobalCfgNvmChanged = FALSE ;
   GlobalFltrNvmchanged = FALSE ;
   GlobalResetOnExit = FALSE ;
   
   do   //while(keepGoing)((xch == AsciiESC) || (xch == '+'))
   {
      BigLoopMaintenance();  // keep clock and display updated
      
      //
      // if in command mode, we need to wait here for CSI.
      // if in immediate mode, CSI is implied at this point.
      //
      if (calledFromConfig)  // need to receive CSI
      {
         do
         {
            CommandCh = hpi_nblk();  // will call BigLoopMaintenance() to keep clock and display updated
            if (CommandCh == '%')  // filter out the percent-delimited status strings
            {
               skip_until_pct();  // read and trash until a '%' arrives
            }
         } while (!((CommandCh == AsciiESC) || (CommandCh == '+')));  // waiting until CSI is entered
      }
      
      // buffer up command characters until <cr> is entered, then parse command and act on it.
      // receipt of CSI at this point voids the buffer and starts over.
      
      cfgCmdPtr = cfgCommandBuffer;
      inputErrorStartOver = FALSE;
      hpo(':'); // prompt with colon in response to CSI
      
      // read and buffer characters until <cr> is received.
      // receipt of another CSI will terminate this line and start over.
      do
      {
         CommandCh = hpi_nblk();  // will call BigLoopMaintenance() to keep clock and display updated
         if (CommandCh == '%')  // filter out the percent-delimited status strings
         {
            skip_until_pct();  // read and trash until a '%' arrives
         }
         else if ((CommandCh == AsciiESC) || (CommandCh == '+'))  // got another CSI
         {
            printf(hpo, "<ABANDONED>\r\n");
            inputErrorStartOver = TRUE;  // set flag to abandon this line
            break;  // exit this loop
         }
         else if (CommandCh == '\r')  // command terminator
         {
            *cfgCmdPtr = '\0';  // null terminator
            printf(hpo, "\r\n");  // terminate echoed command line
         }
         else  // ordinary character, buffer it and echo to console
         {
            hpo(CommandCh);  // echo command character
            *cfgCmdPtr++ = CommandCh;  // buffer the character
            *cfgCmdPtr = '\0';  // null terminator
            //if (strlen(cfgCommandBuffer) >= CFG_COMMAND_BUFFER_SIZE)  // command is too long
            if (strlenb(cfgCommandBuffer) >= CFG_COMMAND_BUFFER_SIZE)  // command is too long  // custom function
            {
               printf(hpo, "<TOO LONG>\r\n");
               inputErrorStartOver = TRUE;  // set flag to abandon this line
               break;  // exit this loop
            }
         }
         
      } while (CommandCh != '\r');
      
      if (inputErrorStartOver)
      {
         continue;  // re-execute outer command fetch loop
      }
      
      //if (strlen(cfgCommandBuffer) == 0)
      if (strlenb(cfgCommandBuffer) == 0)  // custom function
      {
         printf(hpo, "<NULL STRING>\r\n");
         continue;  // re-execute outer command fetch loop
      }
      
      // parse the command received and act on it
      cfgCmdPtr = cfgCommandBuffer;
      
      CommandCh = *cfgCmdPtr++;   // fetch command character and dispatch on it
      //lengthOfCommandTail = strlen(cfgCmdPtr);  // chars remaining in command string
      lengthOfCommandTail = strlenb(cfgCmdPtr);  // chars remaining in command string  // custom function
      // order these command functions alphabetically, all upper-case before all lower-case or non-printing
      switch (CommandCh)
      {
         case 'D':   // select frequency of detection report. '0' to '4' are valid inputs, anything else is ignored
            ArgCh = *cfgCmdPtr++;   // out of range value is ignored
            if ((isdigit(ArgCh)) && (ArgCh >= '0') && (ArgCh <= '4'))
            {
               prepareShadowCFGForMods();
               CFG_NVMshadow[cfg_fut][DETREPINT] = ArgCh - '0' ;  // update future contents
               shadowCFG_FutureToCurrent();  // make it so
            }   
            break;


         case 'F':   // enable/disable filtering of tag data by match list to prioritize reporting
            ArgCh = *cfgCmdPtr++;   // 0 means disable, 1 means enable, anything else is ignored
            if (ArgCh == '?')  // display current value
            {
               printf(hpo,"\r\n%02X\r\n",CFG_NVMshadow[cfg_cur][USETMATCH]);
            }
            else if ((isdigit(ArgCh)) && ((ArgCh == '0') || (ArgCh == '1')))
            {
               prepareShadowCFGForMods();
               CFG_NVMshadow[cfg_fut][USETMATCH] = ArgCh - '0' ;  // update future contents
               shadowCFG_FutureToCurrent();  // make it so
            }   
            break;


         case 'G':   // set country code for primary cellphone target number, max 7 characters
            prepareShadowCFGForMods();
            // receiving field is 8 bytes, so max null-terminated string is 7
            if (lengthOfCommandTail > 7)
            {
               cfgCmdPtr[7] = 0;  // truncate string at 7 bytes
            }
            strcpy(&CFG_NVMshadow[cfg_fut][PRPHCCD], cfgCmdPtr) ;    // update future contents, no validity checking at all
            CFG_NVMshadow[cfg_fut][PRCCLEN] = strlen(&CFG_NVMshadow[cfg_fut][PRPHCCD]); // length of primary number country code string
            shadowCFG_FutureToCurrent();  // make it so
            break;


         case 'g':   // set country code for secondary cellphone target number, max 7 characters
            prepareShadowCFGForMods();
            // receiving field is 8 bytes, so max null-terminated string is 7
            if (lengthOfCommandTail > 7)
            {
               cfgCmdPtr[7] = 0;  // truncate string at 7 bytes
            }
             strcpy(&CFG_NVMshadow[cfg_fut][SEPHCCD], cfgCmdPtr) ;    // update future contents, no validity checking at all
            CFG_NVMshadow[cfg_fut][SECCLEN] = strlen(&CFG_NVMshadow[cfg_fut][SEPHCCD]); // length of primary number country code string
            shadowCFG_FutureToCurrent();  // make it so
            break;


         case 'H':   // select frequency of health report. '0' and '1' are the only valid inputs. while developing, '2' works as well
            ArgCh = *cfgCmdPtr++;   // 0 means once per day, 1 means twice per day, anything else is ignored
            if ((isdigit(ArgCh)) && ((ArgCh == '0') || (ArgCh == '1') || (ArgCh == '2')))
            {
               prepareShadowCFGForMods();
               CFG_NVMshadow[cfg_fut][HLTHREPINT] = ArgCh - '0' ;  // update future contents
               shadowCFG_FutureToCurrent();  // make it so
            }   
            break;


         //case 'I':   // select pushbutton long-press threshold. '2' to '6' are valid inputs, anything else is ignored
         // change from 'I' to'K' because I want to use 'I' for something else in the future
         case 'K':   // select pushbutton long-press threshold. '2' to '6' are valid inputs, anything else is ignored
            sval = convertForConfig(1, cfgCmdPtr);   // ? means display, 2-6 means set, anything else is ignored
            if (sval == -2)  // display current value
            {
               printf(hpo,"\r\n%d\r\n",CFG_NVMshadow[cfg_cur][PBLPRTHR]);
            }
            else if (sval == -1)  // char not decimal digit
            {
               printf(hpo,"\r\n*ERR DGT*\r\n");
            }
            else if ((sval >= 2) && (sval <= 6))
            {
               prepareShadowCFGForMods();
               CFG_NVMshadow[cfg_fut][PBLPRTHR] = sval ;  // update future contents
               shadowCFG_FutureToCurrent();  // make it so
            }   
            else  // not in valid range
            {
               printf(hpo,"\r\n*ERR RNG*\r\n");
            }
            break;


//! obsolete, no need for this wait.
//!         case 'J':   // select wait mins for external BT for connection.
//!                     // allowed range is 10..60. if value is < 10, it will be set to 10 without error
//!                     // if value is > 60, will be set to 60 without error
//!            sval = convertForConfig(2, cfgCmdPtr);   // ? means display, 10-60 means set, anything else is ignored
//!            if (sval == -2)  // display current value
//!            {
//!               printf(hpo,"\r\n%d\r\n",CFG_NVMshadow[cfg_cur][BT_WAIT_CON]);
//!            }
//!            else if (sval == -1)  // char not decimal digit
//!            {
//!               printf(hpo,"\r\n*ERR DGT*\r\n");
//!            }
//!            else if ((sval >=10) && (sval <= 60))
//!            {
//!               prepareShadowCFGForMods();
//!               CFG_NVMshadow[cfg_fut][BT_WAIT_CON] = sval ;  // update future contents
//!               shadowCFG_FutureToCurrent();  // make it so
//!            }   
//!            else  // not in valid range
//!            {
//!               printf(hpo,"\r\n*ERR RNG*\r\n");
//!            }
//!            break;



//! obsolete, no need for this wait.
//!         case 'K':   // select wait mins before dropping BT for inactivity.
//!                     // allowed range is 10..60. if value is < 10, it will be set to 10 without error
//!                     // if value is > 60, will be set to 60 without error
//!            sval = convertForConfig(2, cfgCmdPtr);   // ? means display, 10-60 means set, anything else is ignored
//!            if (sval == -2)  // display current value
//!            {
//!               printf(hpo,"\r\n%d\r\n",CFG_NVMshadow[cfg_cur][BT_WAIT_DIS]);
//!            }
//!            else if (sval == -1)  // char not decimal digit
//!            {
//!               printf(hpo,"\r\n*ERR DGT*\r\n");
//!            }
//!            else if ((sval >=10) && (sval <= 60))
//!            {
//!               prepareShadowCFGForMods();
//!               CFG_NVMshadow[cfg_fut][BT_WAIT_DIS] = sval ;  // update future contents
//!               shadowCFG_FutureToCurrent();  // make it so
//!            }   
//!            else  // not in valid range
//!            {
//!               printf(hpo,"\r\n*ERR RNG*\r\n");
//!            }
//!            break;
            

         case 'M':   // gross-level match option for tag data. tag types may be 0..15, expressed as single hex digit. filter setting single digit 0..2.
            {
               char chv;
               BOOLEAN keep_going = TRUE;
               ArgCh = *cfgCmdPtr++;   // tag type may be '0'..'9', 'A'..'F', 'a'..'f', anything else is ignored
               if ((ArgCh >= '0') && (ArgCh <= '9'))
               {
                  chv = ArgCh - '0';
               }
               else if ((ArgCh >= 'A') && (ArgCh <= 'F'))
               {
                  chv = ArgCh - 'A' + 10;
               }
               else if ((ArgCh >= 'a') && (ArgCh <= 'f'))
               {
                  chv = ArgCh - 'a' + 10;
               }
               else
               {
                  keep_going = FALSE;  // bad tag type
               }
               if (keep_going)
               {
                  ArgCh = *cfgCmdPtr++;   // tag action may be '0'..'2' to set new value, '?' to display value for selected tag
                  if (ArgCh == '?')  // display current value
                  {
                     printf(hpo,"\r\n%02X\r\n",CFG_NVMshadow[cfg_cur][MATCH_T0+chv]);
                  }
                  else if ((isdigit(ArgCh)) && ((ArgCh == '0') || (ArgCh == '1') || (ArgCh == '2')))
                  {
                     prepareShadowCFGForMods();
                     CFG_NVMshadow[cfg_fut][MATCH_T0+chv] = ArgCh - '0' ;  // update future contents
                     shadowCFG_FutureToCurrent();  // make it so
                  }
               }
            }
            break;


         case 'P':   // set primary cellphone target number (aaaxxxssss)
            prepareShadowCFGForMods();
            // receiving field is 16 bytes, so max null-terminated string is 15
            if (lengthOfCommandTail > 15)
            {
               cfgCmdPtr[15] = 0;  // truncate string at 15 bytes
            }
            strcpy(&CFG_NVMshadow[cfg_fut][PRPHNUM], cfgCmdPtr) ;    // update future contents, no validity checking at all
               CFG_NVMshadow[cfg_fut][PRPHLEN] = strlen(&CFG_NVMshadow[cfg_fut][PRPHNUM]); // length of primary number string
//!            CFG_NVMshadow[cfg_fut][PRPHNUM] = *cfgCmdPtr++ ;    // update future contents, no validity checking at all
//!            CFG_NVMshadow[cfg_fut][PRPHD2] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][PRPHD3] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][PRPHD4] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][PRPHD5] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][PRPHD6] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][PRPHD7] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][PRPHD8] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][PRPHD9] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][PRPHD10] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][PRPHNUL] = 0 ;     // it's a string, needs null termination
            shadowCFG_FutureToCurrent();  // make it so
            break;

         case 'R':   // enable/disable secondary cellphone number usage, has subcommand
            ArgCh = *cfgCmdPtr++;   // 0 means disable, 1 means enable, anything else is ignored
            if ((isdigit(ArgCh)) && ((ArgCh == '0') || (ArgCh == '1')))
            {
               prepareShadowCFGForMods();
               CFG_NVMshadow[cfg_fut][SECONDARY] = ArgCh - '0';  // update future contents
               shadowCFG_FutureToCurrent();  // make it so
            }   
            break;

         case 'S':   // set secondary cellphone target number (aaaxxxssss)
            prepareShadowCFGForMods();
            // receiving field is 16 bytes, so max null-terminated string is 15
            if (lengthOfCommandTail > 15)
            {
               cfgCmdPtr[15] = 0;  // truncate string at 15 bytes
            }
            strcpy(&CFG_NVMshadow[cfg_fut][SEPHNUM], cfgCmdPtr) ;    // update future contents, no validity checking at all
//!            CFG_NVMshadow[cfg_fut][SEPHNUM] = *cfgCmdPtr++ ;    // update future contents, no validity checking at all
//!            CFG_NVMshadow[cfg_fut][SEPHD2] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][SEPHD3] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][SEPHD4] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][SEPHD5] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][SEPHD6] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][SEPHD7] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][SEPHD8] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][SEPHD9] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][SEPHD10] = *cfgCmdPtr++ ;
//!            CFG_NVMshadow[cfg_fut][SEPHNUL] = 0 ;     // it's a string, needs null termination
            shadowCFG_FutureToCurrent();  // make it so
            break;

         case 'U':   // use/dont use all logged detections, has subcommand
            ArgCh = *cfgCmdPtr++;   // 0 means disable, 1 means enable, anything else is ignored
            if ((isdigit(ArgCh)) && ((ArgCh == '0') || (ArgCh == '1')))
            {
               prepareShadowCFGForMods();
               CFG_NVMshadow[cfg_fut][USERAWLOGS] = ArgCh - '0' ;  // update future contents
               shadowCFG_FutureToCurrent();  // make it so
            }   
            break;

         case 'V':   // dump current contents of FLTR_NVM
               dumpToHostNoAscii(FLTR_NVMshadow[flt_cur].Bv, FLTR_NVM_SIZE);
               break;


         case 'W':   // dump current contents of CFG_NVM
            dumpToHostNoAscii(CFG_NVMshadow[cfg_cur], CFG_NVM_SIZE);
            break;


//!         case 'X':   // enable pass-through connection of host serial port to btle controller serial port
//!            fprintf(SURport,"\n\rBTLE <<===>>> SMS Pass-through\n\r") ;  // always use port directly
//!            fprintf(SURport, "\n\rType <ESC> to terminate\n\r");  // always use port directly
//!            //disable_interrupts(INT_TIMER2);  // stop this timer since it isn't used in this function
//!            while (TRUE)   // loop until <esc> occurs from SUR port
//!               {
//!                  if(kbhit(SURport))  // always use port directly
//!                     {
//!                        ArgCh = fgetc(SURport);  // always use port directly
//!                        if (ArgCh == 0x1b)
//!                           break;
//!                        fputc(ArgCh,BTport);  // always use port directly
//!                     }   
//!                  if(kbhit(BTport))  // always use port directly
//!                     {
//!                        ArgCh = fgetc(BTport);  // always use port directly
//!                        fputc(ArgCh,SURport);  // always use port directly
//!                     }   
//!               }
//!            fprintf(SURport,"\n\rBTLE <<===>>> SMS Pass-through Terminated\n\r") ;  // always use port directly
//!            break;


         case 'Y':   // enable/disable filtering of tag data by duplicate detections to prioritize reporting
            ArgCh = *cfgCmdPtr++;   // 0 means disable, 1 means enable, anything else is ignored
            if (ArgCh == '?')  // display current value
            {
               printf(hpo,"\r\n%02X\r\n",CFG_NVMshadow[cfg_cur][USEDMATCH]);
            }
            else if ((isdigit(ArgCh)) && ((ArgCh == '0') || (ArgCh == '1')))
            {
               prepareShadowCFGForMods();
               CFG_NVMshadow[cfg_fut][USEDMATCH] = ArgCh - '0';  // update future contents
               shadowCFG_FutureToCurrent();  // make it so
            }   
            break;


         case 'Z':   // enable/disable promiscuous filtering of tag data (passes ordinarily suppressed tag types 1-3-5-7)
            ArgCh = *cfgCmdPtr++;   // 0 means disable, 1 means enable, anything else is ignored
            if (ArgCh == '?')  // display current value
            {
               printf(hpo,"\r\n%02X\r\n",CFG_NVMshadow[cfg_cur][ALLTAGS]);
            }
            else if ((isdigit(ArgCh)) && ((ArgCh == '0') || (ArgCh == '1')))
            {
               prepareShadowCFGForMods();
               CFG_NVMshadow[cfg_fut][ALLTAGS] = ArgCh - '0';  // update future contents
               shadowCFG_FutureToCurrent();  // make it so
            }   
            break;

               
         case 'p' :  // rerun program from the beginning
            printf(hpo, "...commanded reboot\r\n");
            GlobalResetOnExit = TRUE ;
            keepGoing = FALSE;
            break;
         
         case 'q':   // quit this configuration process
            keepGoing = FALSE;
            break;

         case 't':   // set RTC time & date: hhmmssddmmyy
            // need a 12-character string of valid ascii decimal digits
            if (lengthOfCommandTail == 12)  // length is good
            {
               MCond = TRUE;
               for (k = 0; k < 12; k++)
               {
                  MCond = MCond && isdigit(cfgCmdPtr[k]);  // check that all 12 characters are decimal digits
               }
               if (MCond)  // all digits OK, use string to set time and date
               {
                  set_time(cfgCmdPtr); // this routine does it all and prints response
               }
               else
               {
                  printf(hpo, "<BAD DIGIT>\r\n");
               }
            }
            else  // incorrect argument string length
            {
               printf(hpo, "<BAD LENGTH %d>\r\n", j);
            }
            break;

         case 'T':   // show RTC time & date: hhmmssddmmyy
            tell_time(); // this routine does it all and prints response
            break;

         case 'u':   // write the current content of shadow CFG_NVM to actual CFG_NVM,
                     // after recalculating the CRC. stay in configuration mode.
            if (GlobalCfgNvmChanged)   // only do this if something changed
            {
               storeShadowCFG_NVM();
               configuredValueInit();  // recompute values derived from configuration items
               GlobalCfgNvmChanged = FALSE;
            }
            printf(hpo, "\r\n%s\r\n", UpdateSuccess);  // using this exact string, report this is complete; whether or not it actually was performed
            break;


         case 'v':   // byte-at-a-time access to contents of FLTR_NVM
                     // command to read is: <esc>vhhh< where hhh is the three-digit byte offset
                     // command to write is: <esc>vhhh>hh where hhh is the three-digit byte offset at which the two-digit hex content number is written
            if (lengthOfCommandTail < 4)  // check whether string is long enough
            {
               printf(hpo, "<TOO SHORT>\r\n");
               inputErrorStartOver = TRUE;  // set flag to abandon this line
               break;  // exit this case
            }
            ii = convertHexForConfig(3, cfgCmdPtr);  // offset, 3 hex digits
            cfgCmdPtr += 3;  // advance command pointer to next field
            CommandCh = *cfgCmdPtr++;  // subcommand character
            if(CommandCh == '>')  // write single CFG_NVM location. write to shadow not actual device
               {
                  if (lengthOfCommandTail < 6)  // check whether string is long enough
                  {
                     printf(hpo, "<TOO SHORT>\r\n");
                     inputErrorStartOver = TRUE;  // set flag to abandon this line
                     break;  // exit this case
                  }
                  j = convertHexForConfig(2, cfgCmdPtr);  // data, 2 hex digits
                  //printf(hpo,"\r\nFLTR_NVM write addr:%LX data:%X\r\n",ii,j);
                  printf(hpo,"OK\r\n");  // supply 'OK' because windows config utility looks for it
                  prepareShadowFLTRForMods();
                  //FLTR_NVMshadow[flt_cur].Bv[ ii ] = j ;  // update future contents
                  FLTR_NVMshadow[flt_fut].Bv[ ii ] = j ;  // update future contents
                  shadowFLTR_FutureToCurrent();  // make it so
               }
            else if(CommandCh == '<')  // display single CFG_NVM location. read from shadow not actual device
               {
                  printf(hpo,"\r\n%LX:%X",ii,FLTR_NVMshadow[flt_cur].Bv[ ii ]);
               }
            else  // unrecognized subcommand
               {
                  printf(hpo,"\r\n<BAD SUBCOMMAND:%c[0x%02X]>\r\n", CommandCh, CommandCh);
               }
               //;  // nothing

            break;


         case 'w':   // byte-at-a-time access to contents of CFG_NVM
                     // command to read is: <esc>whh< where hh is the two-digit byte offset
                     // command to write is: <esc>whh>hh where hh is the two-digit byte offset at which the two-digit hex content number is written
            if (lengthOfCommandTail < 3)  // check whether string is long enough
            {
               printf(hpo, "<TOO SHORT>\r\n");
               inputErrorStartOver = TRUE;  // set flag to abandon this line
               break;  // exit this case
            }
            i = convertHexForConfig(2, cfgCmdPtr);  // offset, 2 hex digits
            cfgCmdPtr += 2;  // advance command pointer to next field
            CommandCh = *cfgCmdPtr++;  // subcommand character
            if(CommandCh == '>')  // write single CFG_NVM location. write to shadow not actual device
               {
                  if (lengthOfCommandTail < 5)  // check whether string is long enough
                  {
                     printf(hpo, "<TOO SHORT>\r\n");
                     inputErrorStartOver = TRUE;  // set flag to abandon this line
                     break;  // exit this case
                  }
                  j = convertHexForConfig(2, cfgCmdPtr);  // data, 2 hex digits
                  prepareShadowCFGForMods();
                  CFG_NVMshadow[cfg_fut][ i ] = j ;  // update future contents
                  shadowCFG_FutureToCurrent();  // make it so
               }
            else if(CommandCh == '<')  // display single CFG_NVM location. read from shadow not actual device
               {
                  printf(hpo,"\r\n0x%X:0x%X",i,CFG_NVMshadow[cfg_cur][ i ]);
               }
            else  // unrecognized subcommand
               ;  // nothing

            break;



         case '^':   // early modem start. '0' and '1' are valid inputs, anything else is ignored. default is 0
            sval = convertForConfig(1, cfgCmdPtr);   // ? means display, 0-1 means set, anything else is ignored
            if (sval == -2)  // display current value
            {
               printf(hpo,"\r\n%d\r\n",CFG_NVMshadow[cfg_cur][MDM_EARLY]);
            }
            else if (sval == -1)  // char not decimal digit
            {
               printf(hpo,"\r\n*ERR DGT*\r\n");
            }
            else if (sval < 2)
            {
               prepareShadowCFGForMods();
               CFG_NVMshadow[cfg_fut][MDM_EARLY] = sval ;  // update future contents
               shadowCFG_FutureToCurrent();  // make it so
            }   
            else  // not in valid range
            {
               printf(hpo,"\r\n*ERR RNG*\r\n");
            }
            break;



//! obsolete, from earlier version of XBEE.
//!            case 'i':   // select wait secs after resuming pin sleep.
//!                        // allowed range is 30..60. if value is < 30, it will be set to 30 without error
//!                        // if value is > 60, will be set to 60 without error
//!               sval = convertForConfig(2, cfgCmdPtr);   // ? means display, 30-60 means set, anything else is ignored
//!               if (sval == -2)  // display current value
//!               {
//!                  printf(hpo,"\r\n%d\r\n",CFG_NVMshadow[cfg_cur][LPWR_WAIT]);
//!               }
//!               else if (sval == -1)  // char not decimal digit
//!               {
//!                  printf(hpo,"\r\n*ERR DGT*\r\n");
//!               }
//!               else if ((sval >=30) && (sval <= 60))
//!               {
//!                  prepareShadowCFGForMods();
//!                  CFG_NVMshadow[cfg_fut][LPWR_WAIT] = sval ;  // update future contents
//!                  shadowCFG_FutureToCurrent();  // make it so
//!               }   
//!               else  // not in valid range
//!               {
//!                  printf(hpo,"\r\n*ERR RNG*\r\n");
//!               }
//!               break;
//!


//!            case 'k':   // select wait secs after sending modem command for reply packet. input is range-checked, anything out of range is ignored
//!               sval = convertForConfig(2, cfgCmdPtr);   // ? means display, decimal integer means set, anything else is ignored
//!               if (sval == -2)  // display current value
//!               {
//!                  printf(hpo,"\r\n%d\r\n",CFG_NVMshadow[cfg_cur][MDM_WT_SECS]);
//!               }
//!               else if (sval == -1)  // char not decimal digit
//!               {
//!                  printf(hpo,"\r\n*ERR DGT*\r\n");
//!               }
//!               else if ((sval >= XBEE_WAIT_RESPONSE_SECS_MIN) && (sval <= XBEE_WAIT_RESPONSE_SECS_MAX))
//!               {
//!                  prepareShadowCFGForMods();
//!                  CFG_NVMshadow[cfg_fut][MDM_WT_SECS] = sval ;  // update future contents
//!                  shadowCFG_FutureToCurrent();  // make it so
//!               }   
//!               else  // not in valid range
//!               {
//!                  printf(hpo,"\r\n*ERR RNG*\r\n");
//!               }
//!               break;



            case '?':   // "are you there?" returns '!' 
               printf(hpo,"\r\n!\r\n");
               break;



            case '#':   // select debug print level. '0' to '5' are valid inputs, anything else is ignored
               sval = convertForConfig(1, cfgCmdPtr);   // ? means display, 0-5 means set, anything else is ignored
               if (sval == -2)  // display current value
               {
                  printf(hpo,"\r\n%d\r\n",CFG_NVMshadow[cfg_cur][DBG_LEV]);
               }
               else if (sval == -1)  // char not decimal digit
               {
                  printf(hpo,"\r\n*ERR DGT*\r\n");
               }
               else if (sval < 6)
               {
                  prepareShadowCFGForMods();
                  CFG_NVMshadow[cfg_fut][DBG_LEV] = sval ;  // update future contents
                  shadowCFG_FutureToCurrent();  // make it so
               }   
               else  // not in valid range
               {
                  printf(hpo,"\r\n*ERR RNG*\r\n");
               }
               break;


            
//! obsolete, delay extension is no longer needed.
//!            case '&':   // select modem power down delay in minutes. '1' to '4' are valid inputs, anything else is ignored
//!               sval = convertForConfig(1, cfgCmdPtr);   // ? means display, 1-4 means set, anything else is ignored
//!               if (sval == -2)  // display current value
//!               {
//!                  printf(hpo,"\r\n%d\r\n",CFG_NVMshadow[cfg_cur][MDMTMOMIN]);
//!               }
//!               else if (sval == -1)  // char not decimal digit
//!               {
//!                  printf(hpo,"\r\n*ERR DGT*\r\n");
//!               }
//!               else if ((sval >= 1) && (sval <= 4))
//!               {
//!                  prepareShadowCFGForMods();
//!                  CFG_NVMshadow[cfg_fut][MDMTMOMIN] = sval ;  // update future contents
//!                  shadowCFG_FutureToCurrent();  // make it so
//!               }   
//!               else  // not in valid range
//!               {
//!                  printf(hpo,"\r\n*ERR RNG*\r\n");
//!               }
//!               break;
//!      
            
            
            case '@':   // change modem low power mode parameter. '0', '1', and '2' are valid inputs, anything else is ignored. default is 0
               sval = convertForConfig(1, cfgCmdPtr);   // ? means display, 0-2 means set, anything else is ignored
               if (sval == -2)  // display current value
               {
                  printf(hpo,"\r\n%d\r\n",CFG_NVMshadow[cfg_cur][XBEE_LOPWR]);
               }
               else if (sval == -1)  // char not decimal digit
               {
                  printf(hpo,"\r\n*ERR DGT*\r\n");
               }
               else if (sval < 3)
               {
                  prepareShadowCFGForMods();
                  CFG_NVMshadow[cfg_fut][XBEE_LOPWR] = sval ;  // update future contents
                  shadowCFG_FutureToCurrent();  // make it so
                  xbeeSetPowerControlFlags();  // adjust flags for possible new setting value
               }   
               else  // not in valid range
               {
                  printf(hpo,"\r\n*ERR RNG*\r\n");
               }
               break;
            
            case '0':   // <esc>000 will reinitialize the contents of both the CFG_NVM and the FLTR_NVM.
                        // <esc>001 will reinitialize only the CFG_NVM.
                        // <esc>002 will reinitialize only the FLTR_NVM.
                        // with the 'immediate execution' commands, we need to do the content reset right here
               if (lengthOfCommandTail < 2)  // check whether string is long enough
               {
                  printf(hpo, "<TOO SHORT>\r\n");
                  inputErrorStartOver = TRUE;  // set flag to abandon this line
                  break;  // exit this case
               }
               ArgCh = *cfgCmdPtr++;   // second sequence character must be 0. anything else is error
               if (ArgCh != '0')  // second not zero, error
               {
                  printf(hpo, "<ERR CHAR>\r\n");
                  inputErrorStartOver = TRUE;  // set flag to abandon this line
                  break;  // exit this case
               }
               ArgCh = *cfgCmdPtr++;   // third sequence character must be 0, 1, or 2. anything else is error
               if (ArgCh == '0')  // reinitialize both CFG_NVM and FLTR_NVM
               {
                  defaultShadowCFG_NVM();  // normal product form with no preloaded data
                  defaultShadowFLTR_NVM();  // normal product form with no preloaded data
                  printf(hpo, "both external CFG_NVM and FLTR_NVM updated, resetting now...\r\n");
                  reset_cpu();   // reset program rather thansimply returning
               }
               else if (ArgCh == '1')  // reinitialize both CFG_NVM and FLTR_NVM
               {
                  defaultShadowCFG_NVM();  // normal product form with no preloaded data
                  printf(hpo, "external CFG_NVM updated, resetting now...\r\n");
                  reset_cpu();   // reset program rather than simply returning
               }
               else if (ArgCh == '2')  // reinitialize both CFG_NVM and FLTR_NVM
               {
                  defaultShadowFLTR_NVM();  // normal product form with no preloaded data
                  printf(hpo, "FLTR_NVM updated, resetting now...\r\n");
                  reset_cpu();   // reset program rather than simply returning
               }
               else  // error, complain
               {
               printf(hpo, "<ERR CHAR>\r\n");
               inputErrorStartOver = TRUE;  // set flag to abandon this line
               break;  // exit this case
               }
               break;


            case '~':   // temporary commands while testing
               {
                  int16 TempCh;
                  printf(hpo,"\r\nType CTRL-<key> Sequences to Display Codes\r\n");
                  printf(hpo,"\'q\' to exit\r\n");
                  while(TRUE)
                  {
                     TempCh = hpi_nblk();
                     if (TempCh == 'q')
                        break;
                     printf(hpo,"0x%4X", TempCh);
                     if ((TempCh > 0x1F) && (TempCh < 0x7f))  // printable
                     {
                        printf(hpo,":%c\r\n", TempCh);
                     }
                     else
                     {
                        printf(hpo,"\r\n");
                     }
                  }
               }
               break;
         
         
         case '!':   // enable pass-through connection of host serial port to XBEE modem serial port
            fprintf(SURport,"\n\rHOST <<===>>> SMS Pass-through\n\r") ;  // always use port directly
            fprintf(SURport, "\n\rType <ESC> to terminate\n\r");  // always use port directly
            //disable_interrupts(INT_TIMER2);  // stop this timer since it isn't used in this function
            while (TRUE)   // loop until <esc> occurs from SUR port
               {
                  if(kbhit(SURport))  // always use port directly
                     {
                        ArgCh = fgetc(SURport);  // always use port directly
                        if (ArgCh == 0x1b)
                           break;
                        fputc(ArgCh,SMSport);  // always use port directly
                     }   
                  if(kbhit(SMSport))  // always use port directly
                     {
                        ArgCh = fgetc(SMSport);  // always use port directly
                        fputc(ArgCh,SURport);  // always use port directly
                     }   
               }
            fprintf(SURport,"\n\rHOST <<===>>> SMS Pass-through Terminated\n\r") ;  // always use port directly
            break;

   
            default:  // must be actually unrecognized
               hpo('-');  // indicate unrecognized command
               printf (hpo, "\r\n%02x:%02x:%02x %x/%x/%x\r\n",HourBCD,MinuteBCD,SecondBCD,MonthBCD,DayBCD,YearBCD );
               break;            
      }  // end of command case switch
   
   } while (keepGoing); // end big command process do-while loop

 
 
   if (GlobalResetOnExit)
      {
         fiveSecCountDown();
         printf(hpo, "System Reset Required\r\n");
         disable_interrupts(INT_TIMER2);  // stop using timer 2 for one-millisecond time base
         disable_interrupts(GLOBAL);  // disable interrupt system
         delay_ms(5000);
         reset_cpu();   // reset program rather than simply returning
      }
}
 

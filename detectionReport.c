#ifndef DETREP
#define DETREP
//
// functions to perform odd, handy tasks
// related to producing the tag detection report for the PIC18 architecture product.
//
// As of August 2020, a new process is implemented to produce
// the detection report.
//
// all possible new tags are fetched (max of 15) into a scratch buffer.
// if so enabled, selection criteria are used to choose some or all of them.
//
// the report for return via SMS is formed using a subset of data fields
// from the tag line. as many of the input records which pass the selection criteria
// are reformatted into the output report, up to the maximum size of the SMS reply.
//
// a sequence number of sorts is incremented for each report sent, and transmitted
// as well. this allows the recipient to determine whether any transmitted SMS
// messages have not arrived at the recipient's cell phone.
// sequence number is two decimal digits, and has values 01..99, rolling from
// 99 back to 01 again.

// return 'TRUE' if a detection report is actually sent

//void detRecParse(BYTE* record, BYTE* fields[8]);

BOOLEAN processDetectionReport()
{
   TTAG* tagRecord[15];  // we can have at most 15 records per invocation
   TTAG scratchRecord;
   
   BYTE *cmdp = 0;  // small string data buffer in which to construct SUR command
   BYTE *rsnp = 0;  // small string data buffer in which to construct RSN/RSSI display
   BYTE *rdbp = 0;  // large string data buffer in which to construct data report
   BYTE *ldbp = 0;  // large string data buffer in which to save line image before tokenizing
   TTAG* rptr;  // used in record filtering selection logic
   TTAG* rcptr;  // used in record filtering selection logic
   BYTE SUR_query_cmd;
   int j;
   signed int newRecordCount;
   signed int max_rcount;  // this will never be greater than 15

   unsigned int16 recordTag;
   unsigned int16 tagListTag;
   BYTE thisTag;
   BYTE tagAction;
   BYTE tagListCount;
   BYTE tagListStart;
   BOOLEAN foundIt;
   BOOLEAN applySelectionCriteria;
   BOOLEAN applyTagListCriteria;
   BOOLEAN applyDuplicateCriteria;
   BOOLEAN applyTagAndDupCriteria;
   BOOLEAN passEveryTag;  // "promiscuous mode"
   int matchingLineCount;
   int formattedLineCount;
   int slen;
   int blen;

#define P1 DetectReportFields[0]
#define P2 DetectReportFields[1]
#define P3 DetectReportFields[2]
#define P4 DetectReportFields[3]
#define P5 DetectReportFields[4]
#define P6 DetectReportFields[5]
#define P7 DetectReportFields[6]
#define P8 DetectReportFields[7]
   
   BYTE msgTypeIdent;
   // some SUR commands and quantities vary based on USERAWLOGS setting in CFG_NVM
   BOOLEAN usingOldStyleData;
   BOOLEAN sentReport = FALSE;
   
   BYTE rdbDelimiters[] = { ",\r" };  // string containing comma and <cr>, plus a null terminator
   BYTE CrlfStr[] = { "\r\n" };
   
   // if the battery voltage is too low,
   // transmission of this report cannot occur.
   // short-circuit the process, do not fetch the tags from the SUR,
   // just exit now returning 'FALSE'. no allocatable storage obtained thus far.
   if (!BatteryVoltageOK)
   {
#if __DEBUG_MISC
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\n*** processDetectionReport() -- early exit battery voltage too low\r\n");
      }
#endif
      return sentReport;
   }
   
   // if the XBEE cellular modem has been disabled,
   // transmission of this report cannot occur.
   // short-circuit the process, do not fetch the tags from the SUR,
   // just exit now returning 'FALSE'. no allocatable storage obtained thus far.
   if (!isXbeeOkToUse())
   {
#if __DEBUG_MISC
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\n*** processDetectionReport() -- early exit cellular modem disabled\r\n");
      }
#endif
      return sentReport;
   }
              
   // compute selection style in use, if any.
   // if we are using the raw data format, lines do not contain
   // tag value; tag selection cannot be performed.
   // leverage the if-else logic to set whatever else can be known.
   // some SUR commands and quantities vary based on USERAWLOGS setting in CFG_NVM
   if (CFG_NVMshadow[cfg_cur][USERAWLOGS] != 0)
   {
      usingOldStyleData = TRUE;
      applySelectionCriteria = FALSE;
      applyTagListCriteria = FALSE;
      applyDuplicateCriteria = FALSE;
      applyTagAndDupCriteria = FALSE;
      passEveryTag = TRUE;
      SUR_query_cmd = 'e';   // use the old style query command
      max_rcount = 4L;  // SMS record can hold 4 lines
   }
   else  // set filtering criteria
   {
      usingOldStyleData = FALSE;
      applyTagListCriteria = CFG_NVMshadow[cfg_cur][USETMATCH] != 0;
      applyDuplicateCriteria = CFG_NVMshadow[cfg_cur][USEDMATCH] != 0;
      applySelectionCriteria = TRUE;  // uses settings for ignore all/include all by type
      applyTagAndDupCriteria = applyTagListCriteria && applyDuplicateCriteria;
      passEveryTag = CFG_NVMshadow[cfg_cur][ALLTAGS] != 0;  // selectable feature
      SUR_query_cmd = 'E';   // use the new style query command
      max_rcount = 15L;  // maximum number of tags the SUR can provide at one time
   }

   // query for new detections, fetch data, format, and send.
   // query SUR for time-of-day as communication check.

   // query SUR for version info and stored record count
   BigLoopMaintenance();  //maintainLCD();
   newRecordCount = getSurVersAndRc();
   BigLoopMaintenance();  //maintainLCD();
   if (newRecordCount < 0)   // error fetching version and record count
   {
      //                         12345678901234567890
      strcpy(DisplayLine2String,"VC CHECK FAILED     ");
      //sprintf(DisplayLine2String,"FAILVCHK");
      BigLoopMaintenance();
      return FALSE;  // did not send detection report
   }
   else if (newRecordCount == 0)   // no detection records at present
   {
      //                         12345678901234567890
      strcpy(DisplayLine2String,"OLD DATA!           ");
      //sprintf(DisplayLine2String,"OLDDATA!");
      BigLoopMaintenance();
      return FALSE;  // did not send detection report
   }
   
   //
   // fetch the most recent new messages from SUR.
   // apply filter characteristics to each, accepting those which match
   // until either all new messages are consumed or six matching messages
   // have been accepted.
   //
   //
   // new tag detections are present, fetch all of them
   //
   if (newRecordCount > max_rcount)      // bound fetch count because of limited SMS text message size
   {
      newRecordCount = max_rcount;
   }
   
   //
   // now that it is certain we will read data from SUR, allocate the command buffer that we will use
   //
   cmdp = sballoc();  // allocate small string data buffer in which to construct SUR command
   rsnp = sballoc();  // allocate small string data buffer in which to construct RSN/RSSI report

   // initialize local variables
   strcpy(rdbDelimiters, ",\r");  // string containing comma and <cr>, plus a null terminator
   matchingLineCount = 0;
   msgTypeIdent = (applySelectionCriteria) ? 'F' : 'D';
   
   //
   // query SUR for data records
   //
   sprintf(cmdp, "%c%1X", SUR_query_cmd, (int)newRecordCount);  // encode command character and hexadecimal record count in common buffer
   BigLoopMaintenance();  //maintainLCD();
   // capture detection response strings in parsed data buffer
   if (!SURCommandParsedReply(cmdp, SURstrings)) // read failed
   {
      //                         12345678901234567890
      strcpy(DisplayLine2String,"DATA READ FAILED    ");
      //sprintf(DisplayLine2String,"FAILDATA");
      // free the allocated buffers
      if (cmdp)
         sbfree(cmdp);
      if (rsnp)
         sbfree(rsnp);
      if (rdbp)
         lbfree(rdbp);
      if (ldbp)
         lbfree(ldbp);
      BigLoopMaintenance();
      return FALSE;  // did not send detection report
   }

   j = SURstrings.rstrPtrCt;  // how many lines got parsed? will be at least one

#if __DEBUG_DETREP
   if (dbpEnabled(LEV3))
   {
      printf(dpo,"\r\n%d raw tag records as read by detection report:  %s\r\n", j, stringTheDateTimeUptime());
      for (int jj = 0 ; jj < j ; jj++)
      {
         printf(dpo,"-->|%s|<--\r\n", SURstrings.rstrPtr[jj]);
      }
   }
#endif

   //
   // process each returned tag record line, checking for matches under the selection configuration
   //
   
   //
   // if fetching old-style data, just copy every line unmodified.
   //
   // this loop transfers data line-by-line from the parsed string structure to the reporting SMS message.
   // source line parsing has created null-terminated strings without <cr> or <lf>.
   // we must append <crlf> to each line in the output buffer.
   // earlier logic has ensured that at least one line exists.
   //
   if (usingOldStyleData)
   {
      //
      // at this point we are certain to send a detection report
      // need to startup the modem first

      appStartUpModem();
      
      rdbp = prefixDetectionReportResponse();  // fetch the message buffer, prefix with RSN and RSSI
      
      for (int ll = 0 ; ll < newRecordCount ; ll++)
      {
         strcatb(rdbp, SURstrings.rstrPtr[ll]);
         strcatb(rdbp, CrlfStr);
         ++matchingLineCount;
      }
      blen = strlenb(rdbp);  // find the end of the used buffer now that it's filled
      
      // send the selected messages string
      
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\n*** appStartUpModem() called from processDetectionReport() using old-style data\r\n");
      }
#endif

      BigLoopMaintenance();  //maintainLCD();
      //xbeeApiSendSMSmessage(&CFG_NVMshadow[cfg_cur][PRPHNUM], rdbp);   // primary phone number from configuration CFG_NVM
      xbeeApiSendSMSmessage(formCompletePrimaryNumber(), rdbp);   // primary phone number from configuration CFG_NVM
      // Send secondary message if enabled, and first digit of number is a real digit 1..9
      BigLoopMaintenance();  //maintainLCD();
      if ((CFG_NVMshadow[cfg_cur][SECONDARY] != 0) && (isdigit(CFG_NVMshadow[cfg_cur][SEPHNUM])))
      {  
         //xbeeApiSendSMSmessage(&CFG_NVMshadow[cfg_cur][SEPHNUM], rdbp);   // secondary phone number from configuration CFG_NVM
         xbeeApiSendSMSmessage(formCompleteSecondaryNumber(), rdbp);   // secondary phone number from configuration CFG_NVM
      }
      //                          12345678901234567890
      sprintf(DisplayLine2String,"SENT DATA        %3u", blen);
      //sprintf(DisplayLine2String,"SENTD%3u", blen);
      // free the allocated buffers
      if (cmdp)
         sbfree(cmdp);
      if (rsnp)
         sbfree(rsnp);
      if (rdbp)
         lbfree(rdbp);
      if (ldbp)
         lbfree(ldbp);
      appShutDownModem();  // finished with modem
      BigLoopMaintenance();
      return TRUE;
   }
  
   //
   // all other possible outcomes have been handled earlier.
   // execution proceeds from here, and exit now is
   // by falling out the bottom of the function.
   //
   // if execution reaches this point, we are using new-style tag records
   // and will apply selection filters according to established rules.
   // new-style data is reformatted to yield shorter lines, up to six total
   // in the MMS record
   //
   
   //
   // this loop transfers line-by-line.
   // source line parsing has created null-terminated strings without <cr> or <lf>.
   // we must append <crlf> to each line in the output buffer.
   // also begin the output buffer with an incrementing value which
   // increments each time a detection report is created and sent.
   // that permits the recipient to detect whether any reports were
   // missed at his phone. we'll count up a value for every message,
   // 01..99. we will need tobe careful that the sequence number is
   // not incremented if filtration results in no tags in the report.
   //
   // tag detection data lines occur in reverse-chronological order.
   // the first line in the buffer is the most recent.
   // if we have more than four tags reported, or in the PIC18
   // case, more than six, we bias our selection towards the more recent entries.
   //
   // the selection method used is now the same in both the PIC18 app
   // and the PIC24 app.
   //
   // filtering is always active. pre-set configuration values
   // provide the necessary selection criteria. output tag messages are
   // reformatted to include only those items of significance.
   // 
   // 
   // default filtering: 
   // types 0,1,3,5,7 never reported unless promiscuous mode is set
   // types 2,4,6,8,9 always reported
   // 
   // 
   // filtering by taglist:
   // types 0,1,3,5,7 never reported unless promiscuous mode is set
   // types 2,4,6,8 always reported
   // type 9 reported if ID is contained in recorded list
   // 
   // 
   // filtering by multiple detection:
   // types 0,1,3,5,7 never reported unless promiscuous mode is set
   // types 2,4,6,8 always reported
   // type 9 reported if ID is contained two or more times in detected tags
   // 
   // 
   // filtering by combined taglist and multiple detection:
   // types 0,1,3,5,7 never reported unless promiscuous mode is set
   // types 2,4,6,8 always reported
   // type 9 reported if ID is contained in recorded list, and ID occurs two or more times in detected tags
   // 
   // 
   for (int ll = 0 ; ll < newRecordCount ; ll++)
   {
      BigLoopMaintenance();
      
      //
      // get a large buffer and copy the input record before tokenizing
      //
      if (!ldbp)
         ldbp = lballoc();
      strcpyb(ldbp, SURstrings.rstrPtr[ll]);
#if __DEBUG_DETREP
      if (dbpEnabled(LEV3))
      {
         int k;
         printf(dpo,"\r\ninput line [%d] -->|%s|<--", ll+1, ldbp);
         k = strlenb(ldbp)+2;
         dumpToHost((BYTE *)ldbp, k);  // show data before tokenizing
      }
#endif

      // tokenize source line copy in the allocated large buffer,
      // turning delimiter commas into nulls, for string terminations.
      // the format for a data record is:
      // Rec,date,time,freq,type,id,telemetry,gain
      //
      detRecParse(ldbp);  // note pointer values at the start of each field in the rdbp record.
#if __DEBUG_DETREP
      if (dbpEnabled(LEV3))
      {
         int k;
         printf(dpo,"\r\ntokenizes to:");
         dumpToHost((BYTE *)ldbp, k);  // show data after tokenizing
         printf(dpo,"P1 Rec# -->|%s|<--\r\n", P1);
         printf(dpo,"P2 Time -->|%s|<--\r\n", P2);
         printf(dpo,"P3 Date -->|%s|<--\r\n", P3);
         printf(dpo,"P4 Freq -->|%s|<--\r\n", P4);
         printf(dpo,"P5 Type -->|%s|<--\r\n", P5);
         printf(dpo,"P6 ID -->|%s|<--\r\n", P6);
         printf(dpo,"P7 Telemetry -->|%s|<--\r\n", P7);
         printf(dpo,"P8 Gain -->|%s|<--\r\n", P8);
      }
#endif

      //
      // using the token values, compose a crunched version of
      // the tag data in a structure overlaying the original
      // string storage space. same pointer will
      // identify it. (SURstrings.rstrPtr[ll])
      //
      // form the crunched record in a scratch structure, then copy that
      // record, which will always be shorter, over the original string location.
      //
      // PIC24 system, TTAG structure must reside at even address. PIC18 doesn't matter
      //
      tagRecord[ll] = advanceToEven(SURstrings.rstrPtr[ll]);  // copy the pointer to the start of parsed string
      scratchRecord.tagId = strtoulb(P6, 0, 10);  // store numbers as numerical values
      scratchRecord.tagType = strtoulb(P5, 0, 10);
      strcpyb(scratchRecord.timeString, P2);  // store strings as strings
      strcpyb(scratchRecord.freqString, P4);
      scratchRecord.knockedOut = FALSE;  // initially, every record is a keeper
      scratchRecord.lockedIn = FALSE;  // initially, no record is a certain keeper
      scratchRecord.numOfDups = 0;  // don't count yourself as a duplicate
      for (int qq = 0 ; qq < 16; ++qq)
      {
         scratchRecord.locOfDups[qq] = 0;  // set all the flags to zero
      }
      
      //
      // if this record is tag type 9, compare its ID value to already-processed
      // tags and increment duplicate counts everywhere they occur.
      //
      if ((ll > 0)  // skip this logic if first record in list
         && (scratchRecord.tagType == 9))
      {
         for (int mm = 0; mm < ll; ++mm)
         {
            rcptr = tagRecord[mm];  // point to record under scrutiny
            if ((rcptr->tagType == 9)  // it's the right type
               && (rcptr->tagId == scratchRecord.tagId))  // and the tag IDs match
            {
               ++rcptr->numOfDups;  // increment duplicate count in earlier record
               ++scratchRecord.numOfDups;  // same for record under construction
               rcptr->locOfDups[ll] = 1;  // note current record value in earlier record
               scratchRecord.locOfDups[mm] = 1;  // note earlier record value in record under construction
            }
               
         }  // end of loop finding duplicate type 9 tags in earlier records
      }  // end of loop finding duplicate type 9 tags
      
      
      // debug display structure contents
      slen = sizeof(scratchRecord);

#if __DEBUG_DETREP
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\nline %d crunched structure[%d]:", ll+1, slen);
         dumpToHost((BYTE *)&scratchRecord, slen);
      }
#endif

      //
      // overwrite the string source for this line with
      // the constructed scratch record contents.
      //
      memcpy(tagRecord[ll], &scratchRecord, slen);
      
   }  // end of received message compaction loop

    
   //
   // now run a loop over the crunched records
   // and filter out the ones which do not fit.
   // this is line-by-line, does not check for multiple detections.
   //
   for (int ll = 0 ; ll < newRecordCount ; ll++)
   {
      BigLoopMaintenance();

      //rptr = ((TTAG*)(SURstrings.rstrPtr[ll]));  // point to current crunched record
      rptr = tagRecord[ll];  // point to current crunched record
      if (applySelectionCriteria)
      {
         // do gross check for what to do with the tag type
         BYTE thisTag = rptr->tagType;
         if (thisTag < 16)  // valid range 0..15
         {
            tagAction = CFG_NVMshadow[cfg_cur][MATCH_T0 + thisTag];  // tag disposition code from shadow CFG_NVM
            switch(tagAction)
            {
               default:  // invalid setting, ignore this tag
                  rptr->knockedOut = TRUE;  // knock out of contention
                  break;
                  
               case IGNORE_ALL:  // ignore this tag, unless in promiscuous mode
                  if (passEveryTag)
                  {
                     rptr->knockedOut = FALSE;  // ensure record remains in contention
                  }
                  else
                  {
                     rptr->knockedOut = TRUE;  // knock out of contention
                  }
                  break;
                  
               case INCLUDE_ALL:  // include this tag in output
                  rptr->knockedOut = FALSE;  // ensure record remains in contention
                  break;
                  
               case USE_DETAIL:  // must inspect list of permitted IDs for this tag
                  recordTag = rptr->tagId;
                     tagListCount = (FLTR_NVMshadow[flt_cur].Iv[thisTag + VeCtrlBase] & 0xFF);  // count of desired IDs for this tag type, may be zero
                     if (tagListCount > 0)
                     {
                        tagListStart = ((FLTR_NVMshadow[flt_cur].Iv[thisTag + VeCtrlBase] >> 8) & 0xFF);  // subscript in data array where the list begins for this tag type
                        //printf(hpo, "detected tag type: %Ld  tagListCount:%d  tagListStart:%d\r\n", scratchInteger, tagListCount, tagListStart);
                        if (!(tagListStart > (MATCH_LIST_SIZE - 1)))  //  subscript is in range
                        {
                           foundIt = FALSE;
                           for (int lll = 0 ; lll < tagListCount ; lll++)
                           {
                              tagListTag = FLTR_NVMshadow[flt_cur].Iv[VeFdataBase + tagListStart + lll];  // found ID in list
                              //printf(hpo, "tagList[%d]:%Ld\r\n", lll, scratchInteger2);
                              if (recordTag == tagListTag)  // found ID in list
                              {
                                 foundIt = TRUE;
                                 break;
                              }
                           }
                           // search loop is over, decide what to do based upon value of 'foundIt'
                           if (foundIt)  // we have a winner
                           {
                              rptr->knockedOut = FALSE;  // ensure record remains in contention
                           }
                           else  // ID not found in list, too bad
                           {
                              rptr->knockedOut = TRUE;  // knock out of contention
                           }
                        }
                        else  // start subscript is invalid, too bad
                        {
                           rptr->knockedOut = TRUE;  // knock out of contention
                        }
                     }
                     else  // no list of IDs for this type, too bad
                     {
                        rptr->knockedOut = TRUE;  // knock out of contention
                     }
                  break;
            }  // end of cases
         }
         else  // bad tag number, cannot use this line
         {
             rptr->knockedOut = TRUE;  // knock out of contention
         }
      }  // end of selection selection process
      else
      {
         rptr->knockedOut = FALSE;  // keeping all lines, count this one
      }

   }  // end of received message selection loop
   
   //
   // most record selection via filtration is complete at this point.
   // run a loop and promote all tags not marked as 'knockedOut' to
   // 'lockedIn', except type 9 records if the duplicate filter
   // is in effect.
   //
   for (int ll = 0 ; ll < newRecordCount ; ll++)
   {
      BigLoopMaintenance();

      //rptr = ((TTAG*)(SURstrings.rstrPtr[ll]));  // point to current crunched record
      rptr = tagRecord[ll];  // point to current crunched record
      if (!rptr->knockedOut)  // is this record selected at present?
      {
         if(rptr->tagType == 9)  // type 9 tags might require additional selection
         {
            if(applyDuplicateCriteria)  // requires multiple detections for report inclusion
            {
               //
               // to keep the current record, the duplicate count must be greater than zero,
               // otherwise it's knocked out.  if the duplicate count is greater than zero,
               // then the duplicates later in the list are knocked out.
               //
               if(rptr->numOfDups > 0)  // a winner
               {
                  rptr->lockedIn = TRUE;  // keep this record for sure
                  //
                  // look for set bit flags in record positions *higher* than
                  // the current record, and knock them out. these are
                  // the other type 9 records for the same tag ID.
                  for (int mm = ll + 1; mm < newRecordCount; mm++)  // loop may run zero times
                  {
                     if(rptr->locOfDups[mm] != 0)  // knock out record at ordinal position 'mm'
                     {
                        tagRecord[mm]->knockedOut = TRUE;
                     }
                  }
               }
               else  // a loser
               {
                  rptr->knockedOut = TRUE;
               }
            }
            else  // not requiring multiple type 9 detections for reporting
            {
               rptr->lockedIn = TRUE;  // keep this record for sure
            }
         }
         else  // non-type 9 record is kept
         {
            rptr->lockedIn = TRUE;  // keep this record for sure
         }
      }

   }  // end of locked-in promotion loop
   
   
   // lines which at this point are 'lockedIn' are kept, and will be reformatted into the SMS message buffer
   // run a stupid loop to assess this
   
   matchingLineCount = 0;
   for (int ll = 0 ; ll < newRecordCount ; ll++)
   {
      if (tagRecord[ll]->lockedIn) ++matchingLineCount;
   }
   
   BigLoopMaintenance();

   if (matchingLineCount > 0)
   {
      //
      // at this point we are certain to send a detection report
      // need to startup the modem first

      appStartUpModem();
      
      rdbp = prefixDetectionReportResponse();  // fetch the message buffer, prefix with RSN and RSSI
      

      formattedLineCount = 0;
      
      //
      // create the detection report data strings from
      // the crunched structures in the input string array.
      // use a maximum of 6 records.
      //
      for (int ll = 0 ; ll < newRecordCount ; ll++)
      {
         BigLoopMaintenance();

         //rptr = ((TTAG*)(SURstrings.rstrPtr[ll]));
         rptr = tagRecord[ll];
         if (rptr->lockedIn)
         {
            blen = strlenb(rdbp);  // find the end of the used buffer
            sprintf(&rdbp[blen], "%s,%lu,%d,%s\r\n", 
               rptr->timeString,
               rptr->tagId,
               rptr->tagType,
               rptr->freqString);
               ++formattedLineCount;  // note that another record has been formatted for transmission
         }
         // we have to bound this count at 6 lines
         if (formattedLineCount >= 6)
            break;
      }
      blen = strlenb(rdbp);  // find the end of the used buffer now that it's filled
   
      // send the selected messages string
      
#if __DEBUG_XBEE
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "\r\n*** appStartUpModem() called from processDetectionReport() using new-style data\r\n");
      }
#endif

      BigLoopMaintenance();  //maintainLCD();
      //xbeeApiSendSMSmessage(&CFG_NVMshadow[cfg_cur][PRPHNUM], rdbp, 3);   // primary phone number from configuration CFG_NVM, message on LCD line 3
      xbeeApiSendSMSmessage(formCompletePrimaryNumber(), rdbp, 3);   // primary phone number from configuration CFG_NVM, message on LCD line 3
      // Send secondary message if enabled, and first digit of number is a real digit 1..9
      BigLoopMaintenance();  //maintainLCD();
      if ((CFG_NVMshadow[cfg_cur][SECONDARY] != 0) && (isdigit(CFG_NVMshadow[cfg_cur][SEPHNUM])))
      {  
         //xbeeApiSendSMSmessage(&CFG_NVMshadow[cfg_cur][SEPHNUM], rdbp, 4);   // secondary phone number from configuration CFG_NVM, message on LCD line 4
         xbeeApiSendSMSmessage(formCompleteSecondaryNumber(), rdbp, 4);   // secondary phone number from configuration CFG_NVM, message on LCD line 4
      }
      
      if (applySelectionCriteria)
      {
         //                          12345678901234567890
         sprintf(DisplayLine2String,"SENT FDATA       %3u", blen);
      }
      else
      {
         //                          12345678901234567890
         sprintf(DisplayLine2String,"SENT DATA        %3u", blen);
      }

      sentReport = TRUE;
   }
   else  // no lines met the match criteria
   {
      if (applySelectionCriteria)
      {
         //                         12345678901234567890
         strcpy(DisplayLine2String,"NO FRECORDS TO SEND ");
         //sprintf(DisplayLine2String,"NOFRCRDS");
      }
      else
      {
         //                         12345678901234567890
         strcpy(DisplayLine2String,"NO RECORDS TO SEND  ");
         //sprintf(DisplayLine2String,"NORECRDS");
      }
   }

   // free the allocated buffers
   if (cmdp)
      sbfree(cmdp);
   if (rsnp)
      sbfree(rsnp);
   if (rdbp)
      lbfree(rdbp);
   if (ldbp)
      lbfree(ldbp);
   appShutDownModem();  // finished with modem
   BigLoopMaintenance();
   return sentReport;
}  // end of 'processDetectionReport()'



#endif


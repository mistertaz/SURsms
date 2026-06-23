#ifndef _strpars_c
#define _strpars_c

//
// used to manage the data stream read from the various sources in the SURsms platform:
// FONA modem for the PIC18 product.
// XBEE modem for the PIC24 product (in AT mode).
// SUR detection records.
//
// parses stream into individual lines.
//
// data is read character-by-character and parsed into strings according
// to an arbitrary set of rules.
//
// accept input stream characters and parse into strings
//
// stream data may be one or several strings, delimited by cr-lf pairs.
// we assemble strings of printable ascii only, terminated by zero bytes,
// in the strings buffer "stringAssembly". don't worry about sizes right now.
//
// count the number of strings decoded.
//
// note that the controlling structure is passed by reference.
//
void stringsParse(BYTE mch, SPARSE &sp)
{
   
   // all the interesting stuff happens for non-printing characters
   if ((mch == 0)  // timed out if live read, or else zero if caller wishes to end parsing
      ||(!isprint(mch)))  // non-null, non-printing character terminates string being assembled
      {
         if (sp.inAString)
         {
            sp.sab[sp.sabCt++] = 0;  // store a null to terminate the assembling string
            sp.inAString = FALSE;  // not in a string anymore until get a printing character
            
            //
            // new logic here
            // add some special checks for parsing FONA modem returned strings
            //
            
            
            // ordinary string, remains in the data structure. record pointer, count it
            sp.rstrPtr[sp.rstrPtrCt++] = &sp.sab[sp.sabStartCt];
         }  // end of 'in a string'
      
      }  // end of 'if ch is zero or non-printing'
   else  // got a printing character
      {
         if (!sp.inAString)  // string begins, save pointer to it
         {
            //rstrPtr[rstrPtrCt++] = &sab[sabCt];
            sp.sabStartCt = sp.sabCt;  // rememeber buffer subscript at beginning of current string
            sp.inAString = TRUE;  // in a string now
         }  // end of 'not in a string'
         sp.sab[sp.sabCt++] = mch;  // store this character
      }  // end of 'if ch is a printing character'
}


//
// note that the controlling structure is passed by reference.
//
void stringsParseSetInitialConditions(SPARSE &sp)
{
   sp.rstrPtrCt = 0;
   sp.sabCt = 0;  // buffer subscript for assembling strings
   sp.sabStartCt = 0;  // buffer subscript beginning of current string
   sp.inAString = FALSE;
}
      


//
// detection record parse
//
// split the detection record strings into a number of component tokens delimited by commas
//
// the detection record string is produced by the SUR and has a consistent, predictable format.
// before we see it here, the general string parser has isolated each record line into its own
// null-terminated string. we at this stage will never see <cr> or <lf>.
//
// Examples:
// 09,16:01:52,06/09/21,80.0,0,991,0,6
// 08,16:01:29,06/09/21,80.0,0,991,0,10
// 07,16:01:07,06/09/21,70.0,7,0,6,7
// 06,16:00:44,06/09/21,70.0,9,92,,8
// 05,16:00:21,06/09/21,70.0,7,0,48,8
//
// raw tag records as read by detection report:
// -->|09,16:01:52,06/09/21,80.0,0,991,0,6|<--
// -->|08,16:01:29,06/09/21,80.0,0,991,0,10|<--
// -->|07,16:01:07,06/09/21,70.0,7,0,6,7|<--
// -->|06,16:00:44,06/09/21,70.0,9,92,,8|<--
//
// each line will have these eight fields, delimited by commas, or the final end-of-string null.
// the length of each line will always be less than 128, so use of
// the generic 'int' will always be long enough to contain the length.
//
// note that empty fields can occur, indicated by two successive comman delimiters.
// these will result in null strings.
//
// our action in this function is to make a list of pointers to the start of each field,
// and to replace the comma delimiters with null bytes, to terminate the field strings.
// null fields are returned as pointers to the null bytes.
//
// called with a pointer to the input record, pointer to array of 8 string pointers
// in which will be returned pointers to the component fields.
//
// we will always return 8 pointers, even if fields are missing.
// empty or missing field pointers will point to null byte.
//
// for expediency, use a global array of 8 pointers for the response
//
//void detRecParse(BYTE* record, BYTE* fields[8])
void detRecParse(BYTE* record)  //pointer to detection record for tokenizing
{
   BYTE* recptr = record;  // line cursor
   BYTE* delptr;  // pointer to field delimiter or end-of-line
   int rlength;
   int flength;
   
   for (int i = 0; i < 8; i++)
   {
      DetectReportFields[i] = recptr;  // field string starts here
      rlength = strlenb(recptr);  // length of balance of line
      if (rlength)  // if non-zero
      {
         delptr = strchrb(recptr, ',');  // find the comma delimiter
         if (delptr)  // a delimiter character was found
         {
            flength = strlenb(delptr);  // find string length from delimiter
            if (flength > 1)  // string is delimited, continues after the comma
            {
               // overwrite the delimiter with a null to terminate field.
               *delptr++ = 0;  // advance delptr to start of next field
               recptr = delptr;  // set record pointer for next field
            }
            else if (flength > 0)  // 'flength' == 1, the delimiter is the last character in the string
            {
               // if it should be the case that the delimiter is the last string
               // character, it is OK to overwrite it and advance the
               // record pointer. 'recptr' will then point to the record-terminating null.
               *delptr++ = 0;  // advance delptr to start of next field
               recptr = delptr;  // set record pointer for next field
            }
            else  // pointing to the terminating null of the record
            {
               // 'recptr' points to null, use it for remaining fields
               continue;
            }
         }
         else  // no delimiter character was found
         {
            // so the string length was nonzero, but no delimiter was found.
            // that means the current field is the last one of the record.
            // skip the 'recptr' down to that null character and keep looping.
            // any remaining field pointers will point to that same null character.
            recptr += rlength;
         }
      }
      else  // no characters remain in record
      {
         // 'recptr' points to null, use it for remaining fields
         continue;
      }
   }  // end of for-eight-fields loop
   
}


#endif


#ifndef _strpars_h
#define _strpars_h

//
// definitions and structure used to control parsing of input streams into strings of lines.
//

#define MAX_PARSED_STRINGS 32  // SWAG, SUR needs at most 15 entries

struct SPars {
   BYTE *lab;  // alphabetic label for string function
   BYTE *sab;  // pointer to string assembly buffer
   int16 sabSize;  // size of string assembly buffer in bytes
   BYTE *rstrPtr[MAX_PARSED_STRINGS];  // pointers to the start of the parsed strings
   int16 rstrPtrCt;  // count of parsed strings
   int16 sabCt;  // buffer subscript for assembling strings
   int16 sabStartCt;  // buffer subscript beginning of current string
   BOOLEAN inAString;
};

typedef struct SPars SPARSE;

//
// forward function references
//

void stringsParseSetInitialConditions(SPARSE &x);
void stringsParse(BYTE, SPARSE &x);
//void detRecParse(BYTE* record, BYTE** fields[8]);
void detRecParse(BYTE* record);

#endif

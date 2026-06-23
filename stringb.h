////////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2008 Custom Computer Services            ////
//// This source code may only be used by licensed users of the CCS C   ////
//// compiler.  This source code may only be distributed to other       ////
//// licensed users of the CCS C compiler.  No other use, reproduction  ////
//// or distribution is permitted without written permission.           ////
//// Derivative programs created using this software in object code     ////
//// form are not restricted in any way.                                ////
////////////////////////////////////////////////////////////////////////////

#ifndef _STRINGB
#define _STRINGB
#include <stddef.h>
#include <ctype.h>



//////////////////////////////////////////////
//// Uncomment the following define to    ////
//// allow some functions to use a        ////
//// quicker algorithm, but use more ROM  ////
////                                      ////
//// #define FASTER_BUT_MORE_ROM          ////
//////////////////////////////////////////////



/*Copying functions*/
/* standard template:
   void *memmove(void *s1, void *s2, size_t n).
   Copies max of n characters safely (not following ending '\0')
   from s2 in s1; if s2 has less than n characters, appends 0 */

//unsigned BYTE *memmoveb(void *s1,void *s2,size_t n)
BYTE *memmoveb(void *s1,void *s2,size_t n)
{
   //unsigned BYTE *sc1;
   //unsigned BYTE *sc2;
   BYTE *sc1;
   BYTE *sc2;
   sc1=s1;
   sc2=s2;
   if(sc2<sc1 && sc1 <sc2 +n)
      for(sc1+=n,sc2+=n;0<n;--n)
         *--sc1=*--sc2;
   else
      for(;0<n;--n)
         *sc1++=*sc2++;
  return s1;
  }

/* Standard template: BYTE *strcpyb(BYTE *s1, const BYTE *s2)
   copies the string s2 including the null character to s1.
   'strcpy()' is a compiler built in to handle the different address
   spaces. 
   we supply our own copy function.
*/

#define strcopyb strcpyb

BYTE *strcpyb(BYTE *s1, BYTE *s2)
{
   BYTE *retVal = s1;
   BYTE copiedChar;
   do
   {
      copiedChar = *s2++;
      *s1++ = copiedChar;
   } while (copiedChar);  // keep looping until copy a null character
   return retVal;
}


/* standard template:
   BYTE *strncpyb(BYTE *s1, const BYTE *s2, size_t n).
   Copies max of n characters (not following ending '\0')
   from s2 in s1; if s2 has less than n characters, appends 0 */

BYTE *strncpyb(BYTE *s1, BYTE *s2, size_t n)
{
  BYTE *s;

  for (s = s1; n > 0 && *s2 != '\0'; n--)
     *s++ = *s2++;
  for (; n > 0; n--)
     *s++ = '\0';

  return(s1);
}
/***********************************************************/

/*concatenation functions*/
/* standard template: BYTE *strcatb(BYTE *s1, const BYTE *s2)
appends s2 to s1*/

BYTE *strcatb(BYTE *s1, BYTE *s2)
{
   //unsigned BYTE *s;
   BYTE *s;

   for (s = s1; *s != '\0'; ++s);
   while(*s2 != '\0')
   {
      *s = *s2;
      ++s;
      ++s2;
   }

   *s = '\0';
   return(s1);
}
/* standard template: BYTE *strncatb(BYTE *s1, BYTE *s2,size_t n)
appends not more than n characters from s2 to s1*/

BYTE *strncatb(BYTE *s1, BYTE *s2, size_t n)
{
   BYTE *s;

   for (s = s1; *s != '\0'; ++s);
   while(*s2 != '\0' && 0<n)
   {
      *s = *s2;
      ++s;
      ++s2;
      --n;
   }

   *s = '\0';
   return(s1);
}

/* new function */
/* concatenate single character to string */

size_t strlenb(BYTE *);  // need this forward reference
BYTE *strcatchb(BYTE *s1, BYTE ch)
{
   int x = strlenb(s1);
   
   s1[x] = ch;
   s1[x=1] = 0;
   return(s1);
}


/***********************************************************/


/*comparison functions*/
/* standard template: signed int memcmpb(void *s1, void *s2).
   Compares s1 & s2; returns -1 if s1<s2, 0 if s1=s2, 1 if s1>s2 */

signed int memcmpb(void * s1,void *s2,size_t n)
{
//unsigned BYTE *su1, *su2;
BYTE *su1, *su2;
for(su1=s1, su2=s2; 0<n; ++su1, ++su2, --n)
{
   if(*su1!=*su2)
      return ((*su1<*su2)?-1:1);
}
return 0;
}

/* standard template: int strcmpb(const BYTE *s1, const BYTE *s2).
   Compares s1 & s2; returns -1 if s1<s2, 0 if s1=s2, 1 if s1>s2 */

signed int strcmpb(BYTE *s1, BYTE *s2)
{
   for (; *s1 == *s2; s1++, s2++)
      if (*s1 == '\0')
         return(0);
   return((*s1 < *s2) ? -1: 1);
}
/* standard template: int strcollb(const BYTE *s1, const BYTE *s2).
   Compares s1 & s2; returns -1 if s1<s2, 0 if s1=s2, 1 if s1>s2 */

signed int strcollb(BYTE *s1, BYTE *s2)
{
   for (; *s1 == *s2; s1++, s2++)
      if (*s1 == '\0')
         return(0);
   return((*s1 < *s2) ? -1: 1);
}

/* standard template:
   int strncmpb(const BYTE *s1, const BYTE *s2, size_t n).
   Compares max of n characters (not following 0) from s1 to s2;
   returns same as strcmp */

signed int strncmpb(BYTE *s1, BYTE *s2, size_t n)
{
   for (; n > 0; s1++, s2++, n--)
      if (*s1 != *s2)
         return((*s1 <*s2) ? -1: 1);
      else if (*s1 == '\0')
         return(0);
   return(0);
}
/* standard template:
   int strxfrmb(const BYTE *s1, const BYTE *s2, size_t n).
   transforms maximum of n characters from s2 and places them into s1, returns number of chars written ot s1.
   if s1 is NULL and n is 0, then it returns the length of s2.*/
size_t strxfrmb(BYTE *s1, BYTE *s2, size_t n)
{
   BYTE c;
   unsigned int8 n1;
   
   if (!s1 && !n)
      n = -1; //find length

   n1 = 0;
   
   for (; n1 < n; n1++)
   {
      c = *s2++;
      if (!c)
         break;
      if (s1)
         *s1++ = c;
   }

  return(n1);
}





/***********************************************************/
/*Search functions*/
/* standard template: void *memchrb(const BYTE *s, int c).
   Finds first occurrence of c in n characters of s */

BYTE *memchrb(void *s,unsigned int8 c,size_t n)
{
   BYTE uc;
   BYTE *su;
   uc=c;
   for(su=s;0<n;++su,--n)
      if(*su==uc)
      return su;
   return NULL;
}

/* standard template: BYTE *strchrb(const BYTE *s, int c).
   Finds first occurrence of c in s */

BYTE *strchrb(BYTE *s, BYTE c)
{
   for (; *s != c; s++)
      if (*s == '\0')
         return(0);
   return(s);
}
/* standard template:
   size_t strcspnb(const BYTE *s1, const BYTE *s2).
   Computes length of max initial segment of s1 that
   consists entirely of characters NOT from s2*/

size_t strcspnb(BYTE *s1, BYTE *s2)
{
   BYTE *sc1, *sc2;

   for (sc1 = s1; *sc1 != 0; sc1++)
      for (sc2 = s2; *sc2 != 0; sc2++)
         if (*sc1 == *sc2)
            return(sc1 - s1);
   return(sc1 - s1);
}
/* standard template:
   BYTE *strpbrkb(const BYTE *s1, const BYTE *s2).
   Locates first occurence of any character from s2 in s1;
   returns s1 if s2 is empty string */

BYTE *strpbrkb(BYTE *s1, BYTE *s2)
{
   BYTE *sc1, *sc2;

   for (sc1 = s1; *sc1 != 0; sc1++)
      for (sc2 = s2; *sc2 != 0; sc2++)
         if (*sc1 == *sc2)
            return(sc1);
   return(0);
}


/* standard template: BYTE *strrchrb(const BYTE *s, int c).
   Finds last occurrence of c in s */

BYTE *strrchrb(BYTE *s, BYTE c)
{
   BYTE *p;

   for (p = 0; ; s++)
   {
      if (*s == c)
         p = s;
      if (*s == '\0')
         return(p);
   }
}
/* computes length of max initial segment of s1 consisting
   entirely of characters from s2 */

size_t strspnb(BYTE *s1, BYTE *s2)
{
   BYTE *sc1, *sc2;

   for (sc1 = s1; *sc1 != 0; sc1++)
      for (sc2 = s2; ; sc2++)
    if (*sc2 == '\0')
       return(sc1 - s1);
         else if (*sc1 == *sc2)
            break;
   return(sc1 - s1);
}
/* standard template:
   BYTE *strstrb(const BYTE *s1, const BYTE *s2);
   Locates first occurence of character sequence s2 in s1;
   returns 0 if s2 is empty string

   Uncomment #define FASTER_BUT_MORE_ROM at the top of the
   file to use the faster algorithm */
BYTE *strstrb(BYTE *s1, BYTE *s2)
{
   BYTE *s, *t;

   #ifdef FASTER_BUT_MORE_ROM
   if (*s2 == '\0')
         return(s1);
   #endif

   while (*s1)
   {
      for(s = s1, t = s2; *t && (*s == *t); ++s, ++t);

      if (*t == '\0')
         return s1;
      ++s1;
      #ifdef FASTER_BUT_MORE_ROM
         while(*s1 != '\0' && *s1 != *s2)
            ++s1;
      #endif
   }
   return 0;
}

/* standard template: size_t strlen(const BYTE *s).
   Computes length of s1 (preceding terminating 0) */

size_t strlenb(BYTE *s)
{
   BYTE *sc;

   for (sc = s; *sc != 0; sc++);
   return(sc - s);
}

/* standard template: BYTE *strtokb(BYTE *s1, const BYTE *s2).

   Finds next token in s1 delimited by a character from separator
   string s2 (which can be different from call to call).  First call
   starts at beginning of s1 searching for first character NOT
   contained in s2; returns 0 if none is found.
   If one is found, it is the start of first token (return value).
   Function then searches from there for a character contained in s2.
   If none is found, current token extends to end of s1, and subsequent
   searches for a token will return 0.  If one is found, it is
   overwritten by '\0', which terminates current token.  Function saves
   pointer to following character from which next search will start.
   Each subsequent call, with 0 as first argument, starts searching
   from saved pointer */

BYTE *strtokb(BYTE *s1, BYTE *s2)
{
   BYTE *beg, *end;
   static BYTE *save;

   beg = (s1)? s1: save;
   beg += strspnb(beg, s2);
   if (*beg == '\0')
      return(0);
      
   end = strpbrkb(beg, s2);
   if (end != '\0')
   {
      *end = '\0';
      end++;
      save = end;
   }
   else
      save = beg + strlenb(beg);
   
   return(beg);
}

/*****************************************************************/
/*Miscellaneous functions*/
/* standard template
maps error number in errnum to an error message string
Returns: Pointer to string
*/
#ifdef _ERRNO
BYTE * strerrorb(unsigned int8 errnum)
{
   static unsigned BYTE s[13];
   switch( errnum)
   {
   case 0:
      strcpy(s,"no errors");
      return s;
   case EDOM :
      strcpy(s,"domain error");
      return s;
   case ERANGE:
      strcpy(s,"range error");
      return s;
   }
}
#endif

/* standard template: size_t stricmpb(const BYTE *s1, const BYTE *s2).
   Compares s1 to s2 ignoring case (upper vs. lower) */

signed int stricmpb(BYTE *s1, BYTE *s2)
{
 for(; *s1==*s2||(isalpha(*s1)&&isalpha(*s2)&&((BYTE)*s1==(BYTE)*s2+(BYTE)32||(BYTE)*s2==(BYTE)*s1+(BYTE)32));
    s1++, s2++)
    if (*s1 == '\0')
       return(0);
 return((*s1 < *s2) ? -1: 1);
}


/* standard template: BYTE *strlwrb(BYTE *s).
   Replaces uppercase letters by lowercase;
   returns pointer to new string s */

BYTE *strlwrb(BYTE *s)
{
   BYTE *p;

   for (p = s; *p != '\0'; p++)
      if (*p >= 'A' && *p <='Z')
         *p += 'a' - 'A';
   return(s);
}

/* standard template: BYTE *struprb(BYTE *s).
   Replaces lowercase letters by upercase;
   returns pointer to new string s */

BYTE *struprb(BYTE *s)
{
   BYTE *p;

   for (p = s; *p != '\0'; p++)
      if (*p >= 'a' && *p <='z')
         *p -= 'a' - 'A';
   return(s);
}


/************************************************************/

// another BYTE-modified function, from 'stdlib.h'

unsigned long strtoulb(BYTE *s, BYTE *endptr, signed int base)
{
   BYTE *sc,*s1,*sd;
   unsigned long x=0;
   BYTE sign;
   BYTE digits[]="0123456789abcdefghijklmnopqstuvwxyz";
   for(sc=s;isspace(*sc);++sc);
   sign=*sc=='-'||*sc=='+'?*sc++:'+';
   if(sign=='-' || base <0 || base ==1|| base >36) // invalid base
   goto StrtoulGO;

   else if (base)
   {
      if(base==16 && *sc =='0'&&(sc[1]=='x' || sc[1]=='X'))
         sc+=2;
      if(base==8 && *sc =='0')
         sc+=1;
      if(base==2 && *sc =='0'&&sc[1]=='b')
         sc+=2;

   }
   else if(*sc!='0') // base is 0, find base
      base=10;
   else if (sc[1]=='x' || sc[1]=='X')
      base =16,sc+=2;
   else if(sc[1]=='b')
      base=2,sc+=2;
   else
      base=8;
   for (s1=sc;*sc=='0';++sc);// skip leading zeroes
   sd=memchr(digits,tolower(*sc),base);
   for(; sd!=0; )
   {
      x=x*base+(int16)(sd-digits);
      ++sc;
      sd=memchr(digits,tolower(*sc),base);
   }
   if(s1==sc)
   {
   StrtoulGO:
      if (endptr)
      {
         #IF LONG_POINTERS
         *((int16 *)endptr)= s; 
         #ELSE
         *((BYTE *)endptr)=s;
         #ENDIF
         }
   return 0;
   }
   if (endptr)
   {
         #IF LONG_POINTERS
         *((int16 *)endptr)= sc; 
         #ELSE
         *((BYTE *)endptr)=sc; 
         #ENDIF
   }
   return x;
}


#endif

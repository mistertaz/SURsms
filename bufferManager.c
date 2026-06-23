#ifndef BUFFER_MANAGER
#define BUFFER_MANAGER

//#include <stdlibm.h>  // lets us use malloc()/calloc()

//
// simple buffer allocation/deallocation facility for various often-used buffers
//
// three buffer types are defined:
// small buffers (32 bytes)
// large buffers (192 bytes)
//
// the manager is simple in the extreme.
// a stack of free buffer pointers is created, then 
// popped one at a time for allocation.
// it is the responsibility of the caller to detect
// a null pointer reply indicating allocation failure.
//



//
// small data buffers
//

void createSmallBufferPool()
{
   sbptrDepth = 0;  // depth of free buffer pointer stack, empty if zero. depth is a small number, probably less than 16
   sbptrLeastDepth = 99;  // set to bogus high number, so gets updated on first use
   // create pool entries
   for (int i = 0 ; i < SMALL_BUFFER_POOL_SIZE ; i++)
   {
      sbload(smallPool[i]);  // add the buffer entry to the pool
   }
}

// functions to fetch and release small buffers


// return address of free small buffer from pool, show consumed.
// 0-th item is null pointer, means pool is empty.
BYTE *sballoc()  
{
   static int32 trip_count = 0;
   ++trip_count;
   
   char *retval = sbptrs[sbptrDepth];  // choose top-of-stack entry

#if __DEBUG_BMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "++SB++%04Lu small buffer allo @ 0x%04LX dp:%d %s\r\n", trip_count, (unsigned int16)retval, sbptrDepth, stringTheDateTimeUptime());
   }
#endif

   if (sbptrDepth > 0)
   {
      --sbptrDepth;  // if non-empty, reflect item consumed
   }
#if BALLOC_FAIL_FATAL  // make buffer allocation failure a fatal error
   else  // allocation failure
   {
#if __DEBUG_BMGR
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "xxSBxx%04Lu small buffer allo fail @ 0x%04LX dp:%d %s\r\n", trip_count, (unsigned int16)retval, sbptrDepth, stringTheDateTimeUptime());
         delay_ms(100);  // do we need this to allow time to print message?
      }
#endif
      esad();  // disaster
   }
#endif  // BALLOC_FAIL_FATAL
   
   if (sbptrDepth < sbptrLeastDepth)
      sbptrLeastDepth = sbptrDepth;
   
   memset(retval,0, SMALL_BUFFER_SIZE);  // zero out the buffer
   return retval;
}


void sbfree(BYTE *x)  // return small buffer at 'x' to pool.
{
   static int32 trip_count = 0;
   
   ++trip_count;
   
   sbptrs[++sbptrDepth] = x;  // pre-increment allows depth to be nonzero if any pool entries

#if __DEBUG_BMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "--SB--%04Lu small buffer free @ 0x%04LX dp:%d %s\r\n", trip_count, (unsigned int16)x, sbptrDepth, stringTheDateTimeUptime());
   }
#endif
}


void sbload(BYTE *x)  // add small buffer at 'x' to pool. don't count anything
{
   
   sbptrs[++sbptrDepth] = x;  // pre-increment allows depth to be nonzero if any pool entries

#if __DEBUG_BMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "~~SB~~%04Lu small buffer add @ 0x%04LX %s\r\n", sbptrDepth, (unsigned int16)x, stringTheDateTimeUptime());
   }
#endif
}



//
// large data buffers
//

void createLargeBufferPool()
{
   lbptrDepth = 0;  // depth of free buffer pointer stack, empty if zero. depth is a small number, probably less than 16
   lbptrLeastDepth = 99;  // set to bogus high number, so gets updated on first use
   for (int i = 0 ; i < LARGE_BUFFER_POOL_SIZE ; i++)
   {
      lbload(largePool[i]);  // add the buffer entry to the pool
   }
}

// functions to fetch and release large buffers


// return address of free large buffer from pool, show consumed.
// 0-th item is null pointer, means pool is empty.
BYTE *lballoc()  
{
   static int32 trip_count = 0;
   ++trip_count;
   
   char *retval = lbptrs[lbptrDepth];  // choose top-of-stack entry

#if __DEBUG_BMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "++LB++%04Lu large buffer allo @ 0x%04LX dp:%d %s\r\n", trip_count, (unsigned int16)retval, lbptrDepth, stringTheDateTimeUptime());
   }
#endif
   
   if (lbptrDepth > 0)
   {
      --lbptrDepth;  // if non-empty, reflect item consumed
   }
#if BALLOC_FAIL_FATAL  // make buffer allocation failure a fatal error
   else  // allocation failure
   {
#if __DEBUG_BMGR
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "xxLBxx%04Lu large buffer allo fail @ 0x%04LX dp:%d %s\r\n", trip_count, (unsigned int16)retval, lbptrDepth, stringTheDateTimeUptime());
         delay_ms(100);  // do we need this to allow time to print message?
      }
#endif
      esad();  // disaster
   }
#endif  // BALLOC_FAIL_FATAL
   
   if (lbptrDepth < lbptrLeastDepth)
      lbptrLeastDepth = lbptrDepth;
   
   memset(retval,0, LARGE_BUFFER_SIZE);  // zero out the buffer
   
   return retval;
}


void lbfree(BYTE *x)  // return large buffer at 'x' to pool.
{
   static int32 trip_count = 0;
   
   ++trip_count;
   
   lbptrs[++lbptrDepth] = x;  // pre-increment allows depth to be nonzero if any pool entries

#if __DEBUG_BMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "--LB--%04Lu large buffer free @ 0x%04LX dp:%d %s\r\n", trip_count, (unsigned int16)x, lbptrDepth, stringTheDateTimeUptime());
   }
#endif
}


void lbload(BYTE *x)  // add large buffer at 'x' to pool. don't count anything
{
   lbptrs[++lbptrDepth] = x;  // pre-increment allows depth to be nonzero if any pool entries

#if __DEBUG_BMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "~~LB~~%04Lu large buffer add @ 0x%04LX %s\r\n", lbptrDepth, (unsigned int16)x, stringTheDateTimeUptime());
   }
#endif
}


//
// generic buffer-freeup functions
//
// call with a valid buffer pointer and it will be freed
// to the appropriate pool
//

// returns -1 if address from small pool, +1 if address from large pool, else zero

int xbcheck(void *bp)
{
   int retVal = 0;  // presume not valid buffer pointer
   
   if ((bp >= &smallPool[0]) && (bp < &smallPool[SMALL_BUFFER_POOL_SIZE]) )
   {
      retVal = -1;  // valid address in small pool
   }
   else if ((bp >= &largePool[0]) && (bp < &largePool[LARGE_BUFFER_POOL_SIZE]) )
   {
      retVal = 1;  // valid address in large pool
   }
   
   return retVal;
}


void xbfree(void *bp)
{
   int poolCode = xbcheck(bp);  // is buffer pointer valid, discover which pool
   
   if (poolCode < 0)  // small pool buffer
   {
      sbfree(bp);
   }
   else if (poolCode > 0)  // large pool buffer
   {
      lbfree(bp);
   }
#if __DEBUG_BMGR
   else  // pointer does not address a valid allocatable buffer
   {
      if (dbpEnabled(LEV3))
      {
         printf(dpo, "*** xbfree() -- invalid buffer pointer 0x%04LX %s\r\n", (unsigned int16)bp, stringTheDateTimeUptime());
      }
   }
#endif
}



#endif


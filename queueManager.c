#ifndef _QUEUE_MANAGER_C_
#define _QUEUE_MANAGER_C_

//#include <stdlibm.h>  // lets us use malloc()/calloc()

//
// message queueing facility for various purposes.
// queues are supported for byte values. circular buffers to hold characters.
// queues are supported for word values, data pointers used to implement XBEE API protocol.
//


//
// return number of items in the buffer, words in this case
//
int byteqCount(BYTEQ *qp)
{
   if (qp->in == qp->out)  // empty if so
   {
      return 0;   
   }
   else if (qp->in > qp->out)  // non-empty, easy calculation
   {
      return (qp->in - qp->out);   
   }
   else  // (qp->in < qp->out) non-empty, involved calculation
   {
      return (qp->limit - qp->out + qp->in - qp->first);   
   }
}


//
// initialize queue information, including the data
//
// called with:
// queue pointer -- pointer to queue structure
// queue data -- pointer to queue element storage
// datasize -- size of queue in units of elements. (UW in this case)
// label -- name this queue function for debugging
//
void setupAByteQueue(BYTEQ *qp, UB *bp, UW bsz, BYTE *lbl)
{
   qp->descr = lbl;
   qp->first = qp->in = qp->out = qp->limit = bp;  // set all the data pointers the same
   qp->limit += bsz;  // add size to get 'limit' value
   // show 'em what they won
#if __DEBUG_INIT
   printf(dpo, "Queue: %s @ x%04LX\r\n", qp->descr, qp);
   printf(dpo, "First: x%04LX\r\n", qp->first);
   printf(dpo, "In:    x%04LX\r\n", qp->in);
   printf(dpo, "Out:   x%04LX\r\n", qp->out);
   printf(dpo, "Limit: x%04LX\r\n", qp->limit);
   printf(dpo, "Count: %d\r\n", byteqCount(qp));
#endif
}



void byteqEnqueue(BYTEQ *qp, UB elem)
{
   UB *next;  // local scratch copy of IN pointer
   static int32 trip_count = 0;
   ++trip_count;

   //disable_interrupts(GLOBAL);  // disable interrupt system
   
   next = qp->in;  // copy IN pointer
   *next++ = elem;  // store the value @ IN, advance pointer
   if (next == qp->limit)  // fell out the bottom
   {
      next = qp->first;  // roll to FIRST
   }
   if (next == qp->out)  // is the advanced pointer equal to OUT?
   {
      //enable_interrupts(GLOBAL);  // re-enable interrupt system
      
      return;  // advancing pointer would make it equal to OUT. buffer is full now, throw away new value
   }
   //++qp->count;
   
   qp->in = next;  // save advanced IN pointer
   
   //enable_interrupts(GLOBAL);  // re-enable interrupt system

#if __DEBUG_QMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "$$$$%04Lu %s ENQ 0x%02X count:%d %s\r\n", 
         trip_count, qp->descr, elem, byteqCount(qp), stringTheDateTimeUptime());
   }
#endif
}



// this breaks if the data byte has a zero value
UB byteqDequeue(BYTEQ *qp)  // remove from head of list. return void if list is empty
{
   UB *next;
   UB retv;
   static int32 trip_count = 0;
   ++trip_count;
   
   if (qp->in == qp->out)
      {
         return 0;  // buffer is empty, return zero
      }

   //disable_interrupts(GLOBAL);  // disable interrupt system

   next = qp->out;  // point to oldest item in buffer
   retv = *next++;  // fetch it and advance pointer
   if (next == qp->limit)  // fell out the bottom
   {
      next = qp->first;  // roll to FIRST
   }
   qp->out = next;  // save advanced OUT pointer
   //--qp->count;

   //enable_interrupts(GLOBAL);  // re-enable interrupt system

#if __DEBUG_QMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "$$$$%04Lu %s DEQ 0x%02X count:%d %s\r\n", 
         trip_count, qp->descr, retv, byteqCount(qp), stringTheDateTimeUptime());
   }
#endif
   
   return retv;
}

// byte queues above

//#############################################################################

// word queues below

//
// return number of items in the buffer, words in this case
//
int wordqCount(WORDQ *qp)
{
   if (qp->in == qp->out)  // empty if so
   {
      return 0;   
   }
   else if (qp->in > qp->out)  // non-empty, easy calculation
   {
      return (qp->in - qp->out);   
   }
   else  // (qp->in < qp->out) non-empty, involved calculation
   {
      return (qp->limit - qp->out + qp->in - qp->first);   
   }
}


//
// initialize queue information, including the data
//
// called with:
// queue pointer -- pointer to queue structure
// queue data -- pointer to queue element storage
// datasize -- size of queue in units of elements. (UW in this case)
// label -- name this queue function for debugging
//
void setupAWordQueue(WORDQ *qp, UW *bp, UW bsz, BYTE *lbl)
{
   qp->descr = lbl;
   qp->first = qp->in = qp->out = qp->limit = bp;  // set all the data pointers the same
   qp->limit += bsz;  // add size to get 'limit' value
   // show 'em what they won
#if __DEBUG_QMGR
   printf(dpo, "Queue: %s @ x%04LX\r\n", qp->descr, qp);
   printf(dpo, "First: x%04LX\r\n", qp->first);
   printf(dpo, "In:    x%04LX\r\n", qp->in);
   printf(dpo, "Out:   x%04LX\r\n", qp->out);
   printf(dpo, "Limit: x%04LX\r\n", qp->limit);
   printf(dpo, "Count: %d\r\n", wordqCount(qp));
#endif
}



void wordqEnqueue(WORDQ *qp, UW elem)
{
   UW *next;  // local scratch copy of IN pointer
   static int32 trip_count = 0;
   ++trip_count;

   //disable_interrupts(GLOBAL);  // disable interrupt system
   
   next = qp->in;  // copy IN pointer
   *next++ = elem;  // store the value @ IN, advance pointer
   if (next == qp->limit)  // fell out the bottom
   {
      next = qp->first;  // roll to FIRST
   }
   if (next == qp->out)  // is the advanced pointer equal to OUT?
   {
      //enable_interrupts(GLOBAL);  // re-enable interrupt system
      
      return;  // advancing pointer would make it equal to OUT. buffer is full now, throw away new value
   }
   //++qp->count;
   
   qp->in = next;  // save advanced IN pointer
   
   //enable_interrupts(GLOBAL);  // re-enable interrupt system

#if __DEBUG_QMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "$$$$%04Lu %s ENQ 0x%04LX count:%d %s\r\n", 
         trip_count, qp->descr, elem, wordqCount(qp), stringTheDateTimeUptime());
   }
#endif
}




UW wordqDequeue(WORDQ *qp)  // remove from head of list. return void if list is empty
{
   UW *next;
   UW retv;
   static int32 trip_count = 0;
   ++trip_count;
   
   if (qp->in == qp->out)
      {
         return 0;  // buffer is empty, return zero
      }

   //disable_interrupts(GLOBAL);  // disable interrupt system

   next = qp->out;  // point to oldest item in buffer
   retv = *next++;  // fetch it and advance pointer
   if (next == qp->limit)  // fell out the bottom
   {
      next = qp->first;  // roll to FIRST
   }
   qp->out = next;  // save advanced OUT pointer
   //--qp->count;

   //enable_interrupts(GLOBAL);  // re-enable interrupt system

#if __DEBUG_QMGR
   if (dbpEnabled(LEV3))
   {
      printf(dpo, "$$$$%04Lu %s DEQ 0x%04LX count:%d %s\r\n", 
         trip_count, qp->descr, retv, wordqCount(qp), stringTheDateTimeUptime());
   }
#endif
   
   return retv;
}
//#############################################################################


#endif  // _QUEUE_MANAGER_C_

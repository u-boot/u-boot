/**
 * @file    IxQMgrQAccess.c
 *
 * @author Intel Corporation
 * @date    30-Oct-2001
 *
 * @brief   This file contains functions for putting entries on a queue and
 * removing entries from a queue.
 *
 * 
 * @par
 * IXP400 SW Release version 2.0
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * SPDX-License-Identifier:	BSD-3-Clause
 * @par
 * -- End of Copyright Notice --
*/

/*
 * Inlines are compiled as function when this is defined.
 * N.B. Must be placed before #include of "IxQMgr.h"
 */
#ifndef IXQMGR_H
#    define IXQMGRQACCESS_C
#else
#    error
#endif

/*
 * System defined include files.
 */

/*
 * User defined include files.
 */
#include "IxQMgr.h"
#include "IxQMgrAqmIf_p.h"
#include "IxQMgrQAccess_p.h"
#include "IxQMgrQCfg_p.h"
#include "IxQMgrDefines_p.h"

/*
 * Global variables and extern definitions
 */
extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];

/*
 * Function definitions.
 */
void
ixQMgrQAccessInit (void)
{   
}

IX_STATUS
ixQMgrQReadWithChecks (IxQMgrQId qId,
                       UINT32 *entry)
{
    IxQMgrQEntrySizeInWords entrySizeInWords;
    IxQMgrQInlinedReadWriteInfo *infoPtr;

    if (NULL == entry)
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    /* Check QId */
    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }

    /* Get the q entry size in words */
    entrySizeInWords = ixQMgrQEntrySizeInWordsGet (qId);

    ixQMgrAqmIfQPop (qId, entrySizeInWords, entry);	    

    /* reset the current read count if the counter wrapped around 
    * (unsigned arithmetic)
    */
    infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    if (infoPtr->qReadCount-- > infoPtr->qSizeInEntries)
    {
	infoPtr->qReadCount = 0;
    }

    /* Check if underflow occurred on the read */
    if (ixQMgrAqmIfUnderflowCheck (qId))
    {
	return IX_QMGR_Q_UNDERFLOW;
    }
    
    return IX_SUCCESS;
}

/* this function reads the remaining of the q entry
 * for queues configured with many words.
 * (the first word of the entry is already read 
 * in the inlined function and the entry pointer already
 * incremented
 */
IX_STATUS
ixQMgrQReadMWordsMinus1 (IxQMgrQId qId,
			 UINT32 *entry)
{
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    UINT32 entrySize = infoPtr->qEntrySizeInWords;
    volatile UINT32 *qAccRegAddr = infoPtr->qAccRegAddr;
    
    while (--entrySize)
    {
	/* read the entry and accumulate the result */
	*(++entry) = IX_OSAL_READ_LONG(++qAccRegAddr);
    }
    /* underflow is available for lower queues only */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	/* get the queue status */
	UINT32 status = IX_OSAL_READ_LONG(infoPtr->qUOStatRegAddr);
	
	/* check the underflow status */
	if (status & infoPtr->qUflowStatBitMask)
	{
	    /* the queue is empty 
	     *  clear the underflow status bit if it was set 
	     */
	    IX_OSAL_WRITE_LONG(infoPtr->qUOStatRegAddr,
				 status & ~infoPtr->qUflowStatBitMask);
	    return IX_QMGR_Q_UNDERFLOW;
	}
    }
    return IX_SUCCESS;
}

IX_STATUS
ixQMgrQWriteWithChecks (IxQMgrQId qId,
                        UINT32 *entry)
{
    IxQMgrQEntrySizeInWords entrySizeInWords;
    IxQMgrQInlinedReadWriteInfo *infoPtr;

    if (NULL == entry)
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    /* Check QId */
    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }

    /* Get the q entry size in words */
    entrySizeInWords = ixQMgrQEntrySizeInWordsGet (qId);
    
    ixQMgrAqmIfQPush (qId, entrySizeInWords, entry);

    /* reset the current read count if the counter wrapped around 
    * (unsigned arithmetic)
    */
    infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    if (infoPtr->qWriteCount++ >= infoPtr->qSizeInEntries)
    {
	infoPtr->qWriteCount = infoPtr->qSizeInEntries;
    }

    /* Check if overflow occurred on the write*/
    if (ixQMgrAqmIfOverflowCheck (qId))
    {
	return IX_QMGR_Q_OVERFLOW;
    }
         
    return IX_SUCCESS;
}

IX_STATUS
ixQMgrQPeek (IxQMgrQId qId,
	     unsigned int entryIndex,
	     UINT32 *entry)
{
    unsigned int numEntries;

#ifndef NDEBUG
    if ((NULL == entry) || (entryIndex >= IX_QMGR_Q_SIZE_INVALID))
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
#endif
    
    if (IX_SUCCESS != ixQMgrQNumEntriesGet (qId, &numEntries))
    {
	return IX_FAIL;
    }

    if (entryIndex >= numEntries) /* entryIndex starts at 0 */
    {
	return IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS;
    }

    return ixQMgrAqmIfQPeek (qId, entryIndex, entry);
}

IX_STATUS
ixQMgrQPoke (IxQMgrQId qId,
	     unsigned entryIndex,
	     UINT32 *entry)
{
    unsigned int numEntries;

#ifndef NDEBUG
    if ((NULL == entry) || (entryIndex > 128))
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
#endif
        
    if (IX_SUCCESS != ixQMgrQNumEntriesGet (qId, &numEntries))
    {
	return IX_FAIL;
    }

    if (numEntries < (entryIndex + 1)) /* entryIndex starts at 0 */
    {
	return IX_QMGR_ENTRY_INDEX_OUT_OF_BOUNDS;
    }

    return ixQMgrAqmIfQPoke (qId, entryIndex, entry);
}

IX_STATUS
ixQMgrQStatusGetWithChecks (IxQMgrQId qId,
                            IxQMgrQStatus *qStatus)
{
    if (NULL == qStatus)
    {
	return IX_QMGR_PARAMETER_ERROR;
    }
   
    if (!ixQMgrQIsConfigured (qId)) 
    {
        return IX_QMGR_Q_NOT_CONFIGURED;
    }

    ixQMgrAqmIfQueStatRead (qId, qStatus);

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrQNumEntriesGet (IxQMgrQId qId,
		      unsigned *numEntriesPtr)
{
    UINT32 qPtrs;
    UINT32 qStatus;
    unsigned numEntries;
    IxQMgrQInlinedReadWriteInfo *infoPtr;


#ifndef NDEBUG
    if (NULL == numEntriesPtr)
    {
	return IX_QMGR_PARAMETER_ERROR;
    }

    /* Check QId */
    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
#endif

    /* get fast access data */
    infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];

    /* get snapshot */
    qPtrs = IX_OSAL_READ_LONG(infoPtr->qConfigRegAddr);

    /* Mod subtraction of pointers to get number of words in Q. */
    numEntries = (qPtrs - (qPtrs >> 7)) & 0x7f;
  
    if (numEntries == 0)
    {
	/* 
	 * Could mean either full or empty queue
	 * so look at status
	 */
	ixQMgrAqmIfQueStatRead (qId, &qStatus);

	if (qId < IX_QMGR_MIN_QUEUPP_QID)
	{
	    if (qStatus & IX_QMGR_Q_STATUS_E_BIT_MASK)
	    {
		/* Empty */
		*numEntriesPtr = 0;
	    }
	    else if (qStatus & IX_QMGR_Q_STATUS_F_BIT_MASK)
	    {
		/* Full */
		*numEntriesPtr = infoPtr->qSizeInEntries;
	    }
	    else
	    {	    
		/* 
		 * Queue status and read/write pointers are volatile.
		 * The queue state has changed since we took the
		 * snapshot of the read and write pointers.
		 * Client can retry if they wish
		 */
		*numEntriesPtr = 0;
		return IX_QMGR_WARNING;
	    }
	}
	else /* It is an upper queue which does not have an empty status bit maintained */
	{
	    if (qStatus & IX_QMGR_Q_STATUS_F_BIT_MASK)
	    {
		/* The queue is Full at the time of snapshot. */
		*numEntriesPtr = infoPtr->qSizeInEntries;
	    }
	    else
	    {
	       /* The queue is either empty, either moving,
	        * Client can retry if they wish
	        */
		*numEntriesPtr = 0;
	        return IX_QMGR_WARNING;
	    }
	}
    }
    else
    {
	*numEntriesPtr = (numEntries / infoPtr->qEntrySizeInWords) & (infoPtr->qSizeInEntries - 1);
    }
    
    return IX_SUCCESS;
}

#if defined(__wince) && defined(NO_INLINE_APIS)

PUBLIC IX_STATUS
ixQMgrQRead (IxQMgrQId qId,
      UINT32 *entryPtr)
{
    extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    UINT32 entry, entrySize;

    /* get a new entry */
    entrySize = infoPtr->qEntrySizeInWords;
    entry = IX_OSAL_READ_LONG(infoPtr->qAccRegAddr);

    if (entrySize != IX_QMGR_Q_ENTRY_SIZE1)
    { 
    *entryPtr = entry;
  /* process the remaining part of the entry */
   return ixQMgrQReadMWordsMinus1(qId, entryPtr);
    }

    /* underflow is available for lower queues only */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
 /* the counter of queue entries is decremented. In happy 
    * day scenario there are many entries in the queue
  * and the counter does not reach zero.
  */
     if (infoPtr->qReadCount-- == 0)
 {
       /* There is maybe no entry in the queue
      * qReadCount is now negative, but will be corrected before
      * the function returns.
         */
     UINT32 qPtrs; /* queue internal pointers */

     /* when a queue is empty, the hw guarantees to return 
       * a null value. If the value is not null, the queue is
      * not empty.
        */
     if (entry == 0)
     {
       /* get the queue status */
      UINT32 status = IX_OSAL_READ_LONG(infoPtr->qUOStatRegAddr);
   
        /* check the underflow status */
        if (status & infoPtr->qUflowStatBitMask)
        {
           /* the queue is empty 
          *  clear the underflow status bit if it was set 
            */
          IX_OSAL_WRITE_LONG(infoPtr->qUOStatRegAddr,
                    status & ~infoPtr->qUflowStatBitMask);
         *entryPtr = 0;
          infoPtr->qReadCount = 0;
            return IX_QMGR_Q_UNDERFLOW;
     }
       }
       /* store the result */
      *entryPtr = entry;

      /* No underflow occured : someone is filling the queue
       * or the queue contains null entries.
       * The current counter needs to be
       * updated from the current number of entries in the queue
       */

     /* get snapshot of queue pointers */
        qPtrs = IX_OSAL_READ_LONG(infoPtr->qConfigRegAddr);

       /* Mod subtraction of pointers to get number of words in Q. */
      qPtrs = (qPtrs - (qPtrs >> 7)) & 0x7f; 
  
       if (qPtrs == 0)
     {
       /* no entry in the queue */
     infoPtr->qReadCount = 0;
        }
       else
        {
       /* convert the number of words inside the queue
      * to a number of entries 
       */
     infoPtr->qReadCount = qPtrs & (infoPtr->qSizeInEntries - 1);
        }
       return IX_SUCCESS;
  }
    }
    *entryPtr = entry;
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixQMgrQBurstRead (IxQMgrQId qId,
          UINT32 numEntries,
          UINT32 *entries)
{
    extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    UINT32 nullCheckEntry;

    if (infoPtr->qEntrySizeInWords == IX_QMGR_Q_ENTRY_SIZE1)
    {
    volatile UINT32 *qAccRegAddr = infoPtr->qAccRegAddr;

    /* the code is optimized to take care of data dependencies:
  * Durig a read, there are a few cycles needed to get the 
   * read complete. During these cycles, it is poossible to
    * do some CPU, e.g. increment pointers and decrement 
   * counters.
     */

 /* fetch a queue entry */
   nullCheckEntry = IX_OSAL_READ_LONG(infoPtr->qAccRegAddr);

 /* iterate the specified number of queue entries */ 
    while (--numEntries)
    {
       /* check the result of the previous read */
     if (nullCheckEntry == 0)
        {
       /* if we read a NULL entry, stop. We have underflowed */
        break;
      }
       else
        {
       /* write the entry */
       *entries = nullCheckEntry;
      /* fetch next entry */
      nullCheckEntry = IX_OSAL_READ_LONG(qAccRegAddr);
      /* increment the write address */
       entries++;
      }
   }
   /* write the pre-fetched entry */
   *entries = nullCheckEntry;
    }
    else
    {
    IxQMgrQEntrySizeInWords entrySizeInWords = infoPtr->qEntrySizeInWords;
  /* read the specified number of queue entries */
    nullCheckEntry = 0;
 while (numEntries--)
    {
       int i;

      for (i = 0; i < entrySizeInWords; i++)
      {
       *entries = IX_OSAL_READ_LONG(infoPtr->qAccRegAddr + i);
       nullCheckEntry |= *entries++;
       }

       /* if we read a NULL entry, stop. We have underflowed */
        if (nullCheckEntry == 0)
        {
       break;
      }
       nullCheckEntry = 0;
 }
    }

    /* reset the current read count : next access to the read function 
     * will force a underflow status check 
     */
    infoPtr->qWriteCount = 0;

    /* Check if underflow occurred on the read */
    if (nullCheckEntry == 0 && qId < IX_QMGR_MIN_QUEUPP_QID)
    {
  /* get the queue status */
  UINT32 status = IX_OSAL_READ_LONG(infoPtr->qUOStatRegAddr);

   if (status & infoPtr->qUflowStatBitMask)
    {
       /* clear the underflow status bit if it was set */
      IX_OSAL_WRITE_LONG(infoPtr->qUOStatRegAddr,
                status & ~infoPtr->qUflowStatBitMask);
     return IX_QMGR_Q_UNDERFLOW;
 }
    }

    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixQMgrQWrite (IxQMgrQId qId,
         UINT32 *entry)
{
    extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    UINT32 entrySize;

    /* write the entry */
    IX_OSAL_WRITE_LONG(infoPtr->qAccRegAddr, *entry);
    entrySize = infoPtr->qEntrySizeInWords;

    if (entrySize != IX_QMGR_Q_ENTRY_SIZE1)
    {   
    /* process the remaining part of the entry */
   volatile UINT32 *qAccRegAddr = infoPtr->qAccRegAddr;
    while (--entrySize)
 {
       ++entry;
        IX_OSAL_WRITE_LONG(++qAccRegAddr, *entry);
    }
   entrySize = infoPtr->qEntrySizeInWords;
    }

    /* overflow is available for lower queues only */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {   
  UINT32 qSize = infoPtr->qSizeInEntries;
 /* increment the current number of entries in the queue
  * and check for overflow 
   */
 if (infoPtr->qWriteCount++ == qSize)
    {
       /* the queue may have overflow */
       UINT32 qPtrs; /* queue internal pointers */
  
       /* get the queue status */
      UINT32 status = IX_OSAL_READ_LONG(infoPtr->qUOStatRegAddr);

       /* read the status twice because the status may 
         * not be immediately ready after the write operation
        */
     if ((status & infoPtr->qOflowStatBitMask) ||
        ((status = IX_OSAL_READ_LONG(infoPtr->qUOStatRegAddr))
         & infoPtr->qOflowStatBitMask))
     {
       /* the queue is full, clear the overflow status
      *  bit if it was set 
       */
     IX_OSAL_WRITE_LONG(infoPtr->qUOStatRegAddr,
                    status & ~infoPtr->qOflowStatBitMask);
     infoPtr->qWriteCount = infoPtr->qSizeInEntries;
     return IX_QMGR_Q_OVERFLOW;
      }
       /* No overflow occured : someone is draining the queue
       * and the current counter needs to be
       * updated from the current number of entries in the queue
       */

     /* get q pointer snapshot */
        qPtrs = IX_OSAL_READ_LONG(infoPtr->qConfigRegAddr);

       /* Mod subtraction of pointers to get number of words in Q. */
      qPtrs = (qPtrs - (qPtrs >> 7)) & 0x7f; 

     if (qPtrs == 0)
     {
       /* the queue may be full at the time of the 
         * snapshot. Next access will check 
         * the overflow status again.
        */
     infoPtr->qWriteCount = qSize;
       }
       else 
       {
       /* convert the number of words to a number of entries */
        if (entrySize == IX_QMGR_Q_ENTRY_SIZE1)
     {
           infoPtr->qWriteCount = qPtrs & (qSize - 1);
     }
       else
        {
           infoPtr->qWriteCount = (qPtrs / entrySize) & (qSize - 1);
       }
       }
   }
    }
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixQMgrQBurstWrite (IxQMgrQId qId,
          unsigned numEntries,
        UINT32 *entries)
{
    extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
    UINT32 status;

    /* update the current write count */
    infoPtr->qWriteCount += numEntries;

    if (infoPtr->qEntrySizeInWords == IX_QMGR_Q_ENTRY_SIZE1)
    {
    volatile UINT32 *qAccRegAddr = infoPtr->qAccRegAddr;
    while (numEntries--)
    {
       IX_OSAL_WRITE_LONG(qAccRegAddr, *entries);
        entries++;
  }
    }
    else
    {
 IxQMgrQEntrySizeInWords entrySizeInWords = infoPtr->qEntrySizeInWords;
  int i;

  /* write each queue entry */
    while (numEntries--)
    {
       /* write the queueEntrySize number of words for each entry */
       for (i = 0; i < entrySizeInWords; i++)
      {
       IX_OSAL_WRITE_LONG((infoPtr->qAccRegAddr + i), *entries);
     entries++;
      }
   }
    }

    /* check if the write count overflows */
    if (infoPtr->qWriteCount > infoPtr->qSizeInEntries)
    {
  /* reset the current write count */
 infoPtr->qWriteCount = infoPtr->qSizeInEntries;
    }

    /* Check if overflow occurred on the write operation */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
   /* get the queue status */
  status = IX_OSAL_READ_LONG(infoPtr->qUOStatRegAddr);

  /* read the status twice because the status may 
     * not be ready at the time of the write
     */
 if ((status & infoPtr->qOflowStatBitMask) ||
        ((status = IX_OSAL_READ_LONG(infoPtr->qUOStatRegAddr))
         & infoPtr->qOflowStatBitMask))
 {
       /* clear the underflow status bit if it was set */
      IX_OSAL_WRITE_LONG(infoPtr->qUOStatRegAddr,
                status & ~infoPtr->qOflowStatBitMask);
     return IX_QMGR_Q_OVERFLOW;
  }
    }

    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixQMgrQStatusGet (IxQMgrQId qId,
          IxQMgrQStatus *qStatus)
{
    /* read the status of a queue in the range 0-31 */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
  extern UINT32 ixQMgrAqmIfQueLowStatRegAddr[];
   extern UINT32 ixQMgrAqmIfQueLowStatBitsOffset[];
    extern UINT32 ixQMgrAqmIfQueLowStatBitsMask;
    extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];
    IxQMgrQInlinedReadWriteInfo *infoPtr = &ixQMgrQInlinedReadWriteInfo[qId];
   volatile UINT32 *lowStatRegAddr = (UINT32*)ixQMgrAqmIfQueLowStatRegAddr[qId];
   volatile UINT32 *qUOStatRegAddr = infoPtr->qUOStatRegAddr;

  UINT32 lowStatBitsOffset = ixQMgrAqmIfQueLowStatBitsOffset[qId];
    UINT32 lowStatBitsMask   = ixQMgrAqmIfQueLowStatBitsMask;
   UINT32 underflowBitMask  = infoPtr->qUflowStatBitMask;
  UINT32 overflowBitMask   = infoPtr->qOflowStatBitMask;

  /* read the status register for this queue */
   *qStatus = IX_OSAL_READ_LONG(lowStatRegAddr);
 /* mask out the status bits relevant only to this queue */
  *qStatus = (*qStatus >> lowStatBitsOffset) & lowStatBitsMask;

   /* Check if the queue has overflowed */
 if (IX_OSAL_READ_LONG(qUOStatRegAddr) & overflowBitMask)
  {
       /* clear the overflow status bit if it was set */
       IX_OSAL_WRITE_LONG(qUOStatRegAddr,
                 (IX_OSAL_READ_LONG(qUOStatRegAddr) &
               ~overflowBitMask));
       *qStatus |= IX_QMGR_Q_STATUS_OF_BIT_MASK;
   }

   /* Check if the queue has underflowed */
        if (IX_OSAL_READ_LONG(qUOStatRegAddr) & underflowBitMask)
 {
       /* clear the underflow status bit if it was set */
      IX_OSAL_WRITE_LONG(qUOStatRegAddr,
                 (IX_OSAL_READ_LONG(qUOStatRegAddr) &
               ~underflowBitMask));
      *qStatus |= IX_QMGR_Q_STATUS_UF_BIT_MASK;
   }
    }
    else /* read status of a queue in the range 32-63 */
    {
 extern UINT32 ixQMgrAqmIfQueUppStat0RegAddr;
    extern UINT32 ixQMgrAqmIfQueUppStat1RegAddr;
    extern UINT32 ixQMgrAqmIfQueUppStat0BitMask[];
  extern UINT32 ixQMgrAqmIfQueUppStat1BitMask[];

  volatile UINT32 *qNearEmptyStatRegAddr = (UINT32*)ixQMgrAqmIfQueUppStat0RegAddr;
    volatile UINT32 *qFullStatRegAddr      = (UINT32*)ixQMgrAqmIfQueUppStat1RegAddr;
    int maskIndex = qId - IX_QMGR_MIN_QUEUPP_QID;
   UINT32 qNearEmptyStatBitMask = ixQMgrAqmIfQueUppStat0BitMask[maskIndex];
    UINT32 qFullStatBitMask      = ixQMgrAqmIfQueUppStat1BitMask[maskIndex];

    /* Reset the status bits */
 *qStatus = 0;

   /* Check if the queue is nearly empty */
    if (IX_OSAL_READ_LONG(qNearEmptyStatRegAddr) & qNearEmptyStatBitMask)
 {
       *qStatus |= IX_QMGR_Q_STATUS_NE_BIT_MASK;
   }

   /* Check if the queue is full */
    if (IX_OSAL_READ_LONG(qFullStatRegAddr) & qFullStatBitMask)
   {
       *qStatus |= IX_QMGR_Q_STATUS_F_BIT_MASK;
    }
    }
    return IX_SUCCESS;
}
#endif /* def NO_INLINE_APIS */

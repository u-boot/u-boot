/**
 * @file IxOsalBufferMgt.c
 *
 * @brief Default buffer pool management and buffer management
 *        Implementation.
 *
 * Design Notes:
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
 * OS may choose to use default bufferMgt by defining
 * IX_OSAL_USE_DEFAULT_BUFFER_MGT in IxOsalOsBufferMgt.h  
 */

#include "IxOsal.h"

#define IX_OSAL_BUFFER_FREE_PROTECTION  /* Define this to enable Illegal MBuf Freed Protection*/

/*
 * The implementation is only used when the following
 * is defined.
 */
#ifdef IX_OSAL_USE_DEFAULT_BUFFER_MGT


#define IX_OSAL_MBUF_SYS_SIGNATURE				(0x8BADF00D)
#define IX_OSAL_MBUF_SYS_SIGNATURE_MASK				(0xEFFFFFFF)
#define IX_OSAL_MBUF_USED_FLAG					(0x10000000)
#define IX_OSAL_MBUF_SYS_SIGNATURE_INIT(bufPtr)        		IX_OSAL_MBUF_SIGNATURE (bufPtr) = (UINT32)IX_OSAL_MBUF_SYS_SIGNATURE

/* 
*  This implementation is protect, the buffer pool management's  ixOsalMBufFree 
*  against an invalid MBUF pointer argument that already has been freed earlier 
*  or in other words resides in the free pool of MBUFs. This added feature, 
*  checks the MBUF "USED" FLAG. The Flag tells if the MBUF is still not freed 
*  back to the Buffer Pool.
*  Disable this feature for performance reasons by undef 
*  IX_OSAL_BUFFER_FREE_PROTECTION macro.
*/
#ifdef IX_OSAL_BUFFER_FREE_PROTECTION  /*IX_OSAL_BUFFER_FREE_PROTECTION With Buffer Free protection*/

#define IX_OSAL_MBUF_GET_SYS_SIGNATURE(bufPtr)		(IX_OSAL_MBUF_SIGNATURE (bufPtr)&(IX_OSAL_MBUF_SYS_SIGNATURE_MASK) )
#define IX_OSAL_MBUF_SET_SYS_SIGNATURE(bufPtr)    do {																											\
																									IX_OSAL_MBUF_SIGNATURE (bufPtr)&(~IX_OSAL_MBUF_SYS_SIGNATURE_MASK);\
														    									IX_OSAL_MBUF_SIGNATURE (bufPtr)|=IX_OSAL_MBUF_SYS_SIGNATURE;			\
																									}while(0)

#define IX_OSAL_MBUF_SET_USED_FLAG(bufPtr)   IX_OSAL_MBUF_SIGNATURE (bufPtr)|=IX_OSAL_MBUF_USED_FLAG
#define IX_OSAL_MBUF_CLEAR_USED_FLAG(bufPtr) IX_OSAL_MBUF_SIGNATURE (bufPtr)&=~IX_OSAL_MBUF_USED_FLAG
#define IX_OSAL_MBUF_ISSET_USED_FLAG(bufPtr) (IX_OSAL_MBUF_SIGNATURE (bufPtr)&IX_OSAL_MBUF_USED_FLAG)

#else

#define IX_OSAL_MBUF_GET_SYS_SIGNATURE(bufPtr)	 IX_OSAL_MBUF_SIGNATURE (bufPtr)
#define IX_OSAL_MBUF_SET_SYS_SIGNATURE(bufPtr)   IX_OSAL_MBUF_SIGNATURE (bufPtr) = IX_OSAL_MBUF_SYS_SIGNATURE

#endif /*IX_OSAL_BUFFER_FREE_PROTECTION With Buffer Free protection*/
/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

/* 
 * A unit of 32, used to provide bit-shift for pool
 * management. Needs some work if users want more than 32 pools.
 */
#define IX_OSAL_BUFF_FREE_BITS 32

PRIVATE UINT32 ixOsalBuffFreePools[IX_OSAL_MBUF_MAX_POOLS /
    IX_OSAL_BUFF_FREE_BITS];

PUBLIC IX_OSAL_MBUF_POOL ixOsalBuffPools[IX_OSAL_MBUF_MAX_POOLS];

static int ixOsalBuffPoolsInUse = 0;

#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY
PRIVATE IX_OSAL_MBUF *
ixOsalBuffPoolMbufInit (UINT32 mbufSizeAligned,
                      UINT32 dataSizeAligned,
                      IX_OSAL_MBUF_POOL *poolPtr);
#endif

PRIVATE IX_OSAL_MBUF_POOL * ixOsalPoolAlloc (void);                      

/*
 * Function definition: ixOsalPoolAlloc
 */

/****************************/

PRIVATE IX_OSAL_MBUF_POOL *
ixOsalPoolAlloc (void)
{
    register unsigned int i = 0;

    /*
     * Scan for the first free buffer. Free buffers are indicated by 0
     * on the corrsponding bit in ixOsalBuffFreePools. 
     */
    if (ixOsalBuffPoolsInUse >= IX_OSAL_MBUF_MAX_POOLS)
    {
        /*
         * Fail to grab a ptr this time 
         */
        return NULL;
    }

    while (ixOsalBuffFreePools[i / IX_OSAL_BUFF_FREE_BITS] &
        (1 << (i % IX_OSAL_BUFF_FREE_BITS)))
        i++;
    /*
     * Free buffer found. Mark it as busy and initialize. 
     */
    ixOsalBuffFreePools[i / IX_OSAL_BUFF_FREE_BITS] |=
        (1 << (i % IX_OSAL_BUFF_FREE_BITS));

    memset (&ixOsalBuffPools[i], 0, sizeof (IX_OSAL_MBUF_POOL));

    ixOsalBuffPools[i].poolIdx = i;
    ixOsalBuffPoolsInUse++;

    return &ixOsalBuffPools[i];
}


#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY
PRIVATE IX_OSAL_MBUF *
ixOsalBuffPoolMbufInit (UINT32 mbufSizeAligned,
                      UINT32 dataSizeAligned,
                      IX_OSAL_MBUF_POOL *poolPtr)
{
    UINT8 *dataPtr;
    IX_OSAL_MBUF *realMbufPtr;
    /* Allocate cache-aligned memory for mbuf header */
    realMbufPtr = (IX_OSAL_MBUF *) IX_OSAL_CACHE_DMA_MALLOC (mbufSizeAligned);
    IX_OSAL_ASSERT (realMbufPtr != NULL);
    memset (realMbufPtr, 0, mbufSizeAligned);

    /* Allocate cache-aligned memory for mbuf data */
    dataPtr = (UINT8 *) IX_OSAL_CACHE_DMA_MALLOC (dataSizeAligned);
    IX_OSAL_ASSERT (dataPtr != NULL);
    memset (dataPtr, 0, dataSizeAligned);

    /* Fill in mbuf header fields */
    IX_OSAL_MBUF_MDATA (realMbufPtr) = dataPtr;
    IX_OSAL_MBUF_ALLOCATED_BUFF_DATA (realMbufPtr) = (UINT32)dataPtr;

    IX_OSAL_MBUF_MLEN (realMbufPtr) = dataSizeAligned;
    IX_OSAL_MBUF_ALLOCATED_BUFF_LEN (realMbufPtr) = dataSizeAligned;

    IX_OSAL_MBUF_NET_POOL (realMbufPtr) = (IX_OSAL_MBUF_POOL *) poolPtr;

    IX_OSAL_MBUF_SYS_SIGNATURE_INIT(realMbufPtr);

    /* update some statistical information */
    poolPtr->mbufMemSize += mbufSizeAligned;
    poolPtr->dataMemSize += dataSizeAligned;

    return realMbufPtr;
}
#endif /* #ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY */

/*
 * Function definition: ixOsalBuffPoolInit
 */

PUBLIC IX_OSAL_MBUF_POOL *
ixOsalPoolInit (UINT32 count, UINT32 size, const char *name)
{

    /* These variables are only used if UX_OSAL_BUFFER_ALLOC_SEPERATELY
     * is defined .
     */
#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY
    UINT32 i, mbufSizeAligned, dataSizeAligned;
    IX_OSAL_MBUF *currentMbufPtr = NULL;
#else
    void *poolBufPtr;
    void *poolDataPtr;
    int mbufMemSize;
    int dataMemSize;
#endif

    IX_OSAL_MBUF_POOL *poolPtr = NULL;
    
    if (count <= 0)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalPoolInit(): " "count = 0 \n", 0, 0, 0, 0, 0, 0);
        return NULL;        
    }

    if (name == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalPoolInit(): " "NULL name \n", 0, 0, 0, 0, 0, 0);
        return NULL;        
    }
    
    if (strlen (name) > IX_OSAL_MBUF_POOL_NAME_LEN)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalPoolInit(): "
            "ERROR - name length should be no greater than %d  \n",
            IX_OSAL_MBUF_POOL_NAME_LEN, 0, 0, 0, 0, 0);
        return NULL;
    }

/* OS can choose whether to allocate all buffers all together (if it 
 * can handle a huge single alloc request), or to allocate buffers 
 * separately by the defining IX_OSAL_BUFFER_ALLOC_SEPARATELY.
 */
#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY
    /* Get a pool Ptr */
    poolPtr = ixOsalPoolAlloc ();

    if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalPoolInit(): " "Fail to Get PoolPtr \n", 0, 0, 0, 0, 0, 0);    
        return NULL;
    }

    mbufSizeAligned = IX_OSAL_MBUF_POOL_SIZE_ALIGN (sizeof (IX_OSAL_MBUF));
    dataSizeAligned = IX_OSAL_MBUF_POOL_SIZE_ALIGN(size);

    poolPtr->nextFreeBuf = NULL;    
    poolPtr->mbufMemPtr = NULL;    
    poolPtr->dataMemPtr = NULL;
    poolPtr->bufDataSize = dataSizeAligned;
    poolPtr->totalBufsInPool = count;
    poolPtr->poolAllocType = IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC;
    strcpy (poolPtr->name, name);


    for (i = 0; i < count; i++)
    {
	    /* create an mbuf */
	    currentMbufPtr = ixOsalBuffPoolMbufInit (mbufSizeAligned,
					         dataSizeAligned,
					         poolPtr);

#ifdef IX_OSAL_BUFFER_FREE_PROTECTION 		
/* Set the Buffer USED Flag. If not, ixOsalMBufFree will fail.
   ixOsalMbufFree used here is in a special case whereby, it's 
   used to add MBUF to the Pool. By specification, ixOsalMbufFree 
   deallocates an allocated MBUF from Pool.
*/ 			         
      IX_OSAL_MBUF_SET_USED_FLAG(currentMbufPtr);
#endif                             
	    /* Add it to the pool */
	    ixOsalMbufFree (currentMbufPtr);

	    /* flush the pool information to RAM */
	    IX_OSAL_CACHE_FLUSH (currentMbufPtr, mbufSizeAligned);
    }
    
    /*
     * update the number of free buffers in the pool 
     */
    poolPtr->freeBufsInPool = count;

#else 
/* Otherwise allocate buffers in a continuous block fashion */    
    poolBufPtr = IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC (count, mbufMemSize);
    IX_OSAL_ASSERT (poolBufPtr != NULL);
    poolDataPtr =
        IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC (count, size, dataMemSize);
    IX_OSAL_ASSERT (poolDataPtr != NULL);

    poolPtr = ixOsalNoAllocPoolInit (poolBufPtr, poolDataPtr,
        count, size, name);
    if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalPoolInit(): " "Fail to get pool ptr \n", 0, 0, 0, 0, 0, 0);
        return NULL;
    }

    poolPtr->poolAllocType = IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC;

#endif /* IX_OSAL_BUFFER_ALLOC_SEPARATELY */
    return poolPtr;
}

PUBLIC IX_OSAL_MBUF_POOL *
ixOsalNoAllocPoolInit (void *poolBufPtr,
    void *poolDataPtr, UINT32 count, UINT32 size, const char *name)
{
    UINT32 i,  mbufSizeAligned, sizeAligned;
    IX_OSAL_MBUF *currentMbufPtr = NULL;
    IX_OSAL_MBUF *nextMbufPtr = NULL;
    IX_OSAL_MBUF_POOL *poolPtr = NULL;

    /*
     * check parameters 
     */
    if (poolBufPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalNoAllocPoolInit(): "
            "ERROR - NULL poolBufPtr \n", 0, 0, 0, 0, 0, 0);
        return NULL;
    }

    if (count <= 0)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalNoAllocPoolInit(): "
            "ERROR - count must > 0   \n", 0, 0, 0, 0, 0, 0);
        return NULL;
    }

    if (name == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalNoAllocPoolInit(): "
            "ERROR - NULL name ptr  \n", 0, 0, 0, 0, 0, 0);
        return NULL;
    }

    if (strlen (name) > IX_OSAL_MBUF_POOL_NAME_LEN)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalNoAllocPoolInit(): "
            "ERROR - name length should be no greater than %d  \n",
            IX_OSAL_MBUF_POOL_NAME_LEN, 0, 0, 0, 0, 0);
        return NULL;
    }

    poolPtr = ixOsalPoolAlloc ();

    if (poolPtr == NULL)
    {
        return NULL;
    }

    /*
     * Adjust sizes to ensure alignment on cache line boundaries 
     */
    mbufSizeAligned =
        IX_OSAL_MBUF_POOL_SIZE_ALIGN (sizeof (IX_OSAL_MBUF));
    /*
     * clear the mbuf memory area 
     */
    memset (poolBufPtr, 0, mbufSizeAligned * count);

    if (poolDataPtr != NULL)
    {
        /*
         * Adjust sizes to ensure alignment on cache line boundaries 
         */
        sizeAligned = IX_OSAL_MBUF_POOL_SIZE_ALIGN (size);
        /*
         * clear the data memory area 
         */
        memset (poolDataPtr, 0, sizeAligned * count);
    }
    else
    {
        sizeAligned = 0;
    }

    /*
     * initialise pool fields 
     */
    strcpy ((poolPtr)->name, name);

    poolPtr->dataMemPtr = poolDataPtr;
    poolPtr->mbufMemPtr = poolBufPtr;
    poolPtr->bufDataSize = sizeAligned;
    poolPtr->totalBufsInPool = count;
    poolPtr->mbufMemSize = mbufSizeAligned * count;
    poolPtr->dataMemSize = sizeAligned * count;

    currentMbufPtr = (IX_OSAL_MBUF *) poolBufPtr;

    poolPtr->nextFreeBuf = currentMbufPtr;

    for (i = 0; i < count; i++)
    {
        if (i < (count - 1))
        {
            nextMbufPtr =
                (IX_OSAL_MBUF *) ((unsigned) currentMbufPtr +
                mbufSizeAligned);
        }
        else
        {                       /* last mbuf in chain */
            nextMbufPtr = NULL;
        }
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (currentMbufPtr) = nextMbufPtr;
        IX_OSAL_MBUF_NET_POOL (currentMbufPtr) = poolPtr;

        IX_OSAL_MBUF_SYS_SIGNATURE_INIT(currentMbufPtr);

        if (poolDataPtr != NULL)
        {
            IX_OSAL_MBUF_MDATA (currentMbufPtr) = poolDataPtr;
            IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(currentMbufPtr) = (UINT32) poolDataPtr;

            IX_OSAL_MBUF_MLEN (currentMbufPtr) = sizeAligned;
            IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(currentMbufPtr) = sizeAligned;

            poolDataPtr = (void *) ((unsigned) poolDataPtr + sizeAligned);
        }

        currentMbufPtr = nextMbufPtr;
    }

    /*
     * update the number of free buffers in the pool 
     */
    poolPtr->freeBufsInPool = count;

    poolPtr->poolAllocType = IX_OSAL_MBUF_POOL_TYPE_USER_ALLOC;

    return poolPtr;
}

/* 
 * Get a mbuf ptr from the pool
 */
PUBLIC IX_OSAL_MBUF *
ixOsalMbufAlloc (IX_OSAL_MBUF_POOL * poolPtr)
{
    int lock;
    IX_OSAL_MBUF *newBufPtr = NULL;

    /*
     * check parameters 
     */
    if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMbufAlloc(): "
            "ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);
        return NULL;
    }

    lock = ixOsalIrqLock ();

    newBufPtr = poolPtr->nextFreeBuf;
    if (newBufPtr)
    {
        poolPtr->nextFreeBuf =
            IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (newBufPtr);
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (newBufPtr) = NULL;

        /*
         * update the number of free buffers in the pool 
         */
        poolPtr->freeBufsInPool--;
    }
    else
    {
        /* Return NULL to indicate to caller that request is denied. */
        ixOsalIrqUnlock (lock);

        return NULL;
    }

#ifdef IX_OSAL_BUFFER_FREE_PROTECTION
	/* Set Buffer Used Flag to indicate state.*/
    IX_OSAL_MBUF_SET_USED_FLAG(newBufPtr);
#endif

    ixOsalIrqUnlock (lock);

    return newBufPtr;
}

PUBLIC IX_OSAL_MBUF *
ixOsalMbufFree (IX_OSAL_MBUF * bufPtr)
{
    int lock;
    IX_OSAL_MBUF_POOL *poolPtr;

    IX_OSAL_MBUF *nextBufPtr = NULL;

    /*
     * check parameters 
     */
    if (bufPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMbufFree(): "
            "ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);
        return NULL;
    }



    lock = ixOsalIrqLock ();

#ifdef IX_OSAL_BUFFER_FREE_PROTECTION
	
	/* Prevention for Buffer freed more than once*/
    if(!IX_OSAL_MBUF_ISSET_USED_FLAG(bufPtr))
    {
   	return NULL;
    }
    IX_OSAL_MBUF_CLEAR_USED_FLAG(bufPtr);
#endif
	
    poolPtr = IX_OSAL_MBUF_NET_POOL (bufPtr);

    /*
     * check the mbuf wrapper signature (if mbuf wrapper was used) 
     */
    if (poolPtr->poolAllocType == IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC)
    {
        IX_OSAL_ENSURE ( (IX_OSAL_MBUF_GET_SYS_SIGNATURE(bufPtr) == IX_OSAL_MBUF_SYS_SIGNATURE),
            "ixOsalBuffPoolBufFree: ERROR - Invalid mbuf signature.");
    }

    nextBufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (bufPtr);

    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR (bufPtr) = poolPtr->nextFreeBuf;
    poolPtr->nextFreeBuf = bufPtr;

    /*
     * update the number of free buffers in the pool 
     */
    poolPtr->freeBufsInPool++;

    ixOsalIrqUnlock (lock);

    return nextBufPtr;
}

PUBLIC void
ixOsalMbufChainFree (IX_OSAL_MBUF * bufPtr)
{
    while ((bufPtr = ixOsalMbufFree (bufPtr)));
}

/*
 * Function definition: ixOsalBuffPoolShow
 */
PUBLIC void
ixOsalMbufPoolShow (IX_OSAL_MBUF_POOL * poolPtr)
{
    IX_OSAL_MBUF *nextBufPtr;
    int count = 0;
    int lock;

    /*
     * check parameters 
     */
    if (poolPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalBuffPoolShow(): "
            "ERROR - Invalid Parameter", 0, 0, 0, 0, 0, 0);
        /*
         * return IX_FAIL; 
         */
        return;
    }

    lock = ixOsalIrqLock ();
    count = poolPtr->freeBufsInPool;
    nextBufPtr = poolPtr->nextFreeBuf;
    ixOsalIrqUnlock (lock);

    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
        IX_OSAL_LOG_DEV_STDOUT, "=== POOL INFORMATION ===\n", 0, 0, 0,
        0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Pool Name:                   %s\n",
        (unsigned int) poolPtr->name, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Pool Allocation Type:        %d\n",
        (unsigned int) poolPtr->poolAllocType, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Pool Mbuf Mem Usage (bytes): %d\n",
        (unsigned int) poolPtr->mbufMemSize, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Pool Data Mem Usage (bytes): %d\n",
        (unsigned int) poolPtr->dataMemSize, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Mbuf Data Capacity  (bytes): %d\n",
        (unsigned int) poolPtr->bufDataSize, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Total Mbufs in Pool:         %d\n",
        (unsigned int) poolPtr->totalBufsInPool, 0, 0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Available Mbufs:             %d\n", (unsigned int) count, 0,
        0, 0, 0, 0);
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "Next Available Mbuf:         %p\n", (unsigned int) nextBufPtr,
        0, 0, 0, 0, 0);

    if (poolPtr->poolAllocType == IX_OSAL_MBUF_POOL_TYPE_USER_ALLOC)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
            IX_OSAL_LOG_DEV_STDOUT,
            "Mbuf Mem Area Start address: %p\n",
            (unsigned int) poolPtr->mbufMemPtr, 0, 0, 0, 0, 0);
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
            "Data Mem Area Start address: %p\n",
            (unsigned int) poolPtr->dataMemPtr, 0, 0, 0, 0, 0);
    }
}

PUBLIC void
ixOsalMbufDataPtrReset (IX_OSAL_MBUF * bufPtr)
{
    IX_OSAL_MBUF_POOL *poolPtr;
    UINT8 *poolDataPtr;

    if (bufPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalBuffPoolBufDataPtrReset"
            ": ERROR - Invalid Parameter\n", 0, 0, 0, 0, 0, 0);
        return;
    }

    poolPtr = (IX_OSAL_MBUF_POOL *) IX_OSAL_MBUF_NET_POOL (bufPtr);
    poolDataPtr = poolPtr->dataMemPtr;

    if (poolPtr->poolAllocType == IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC)
    {
        if (IX_OSAL_MBUF_GET_SYS_SIGNATURE(bufPtr) != IX_OSAL_MBUF_SYS_SIGNATURE)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalBuffPoolBufDataPtrReset"
                ": invalid mbuf, cannot reset mData pointer\n", 0, 0,
                0, 0, 0, 0);
            return;
        }
        IX_OSAL_MBUF_MDATA (bufPtr) = (UINT8*)IX_OSAL_MBUF_ALLOCATED_BUFF_DATA (bufPtr);
    }
    else
    {
        if (poolDataPtr)
        {
            unsigned int bufSize = poolPtr->bufDataSize;
            unsigned int bufDataAddr =
                (unsigned int) IX_OSAL_MBUF_MDATA (bufPtr);
            unsigned int poolDataAddr = (unsigned int) poolDataPtr;

            /*
             * the pointer is still pointing somewhere in the mbuf payload.
             * This operation moves the pointer to the beginning of the 
             * mbuf payload
             */
            bufDataAddr = ((bufDataAddr - poolDataAddr) / bufSize) * bufSize;
            IX_OSAL_MBUF_MDATA (bufPtr) = &poolDataPtr[bufDataAddr];
        }
        else
        {
            ixOsalLog (IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalBuffPoolBufDataPtrReset"
                ": cannot be used if user supplied NULL pointer for pool data area "
                "when pool was created\n", 0, 0, 0, 0, 0, 0);
            return;
        }
    }

}

/*
 * Function definition: ixOsalBuffPoolUninit
 */
PUBLIC IX_STATUS
ixOsalBuffPoolUninit (IX_OSAL_MBUF_POOL * pool)
{
    if (!pool)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalBuffPoolUninit: NULL ptr \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (pool->freeBufsInPool != pool->totalBufsInPool)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalBuffPoolUninit: need to return all ptrs to the pool first! \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (pool->poolAllocType == IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC)
    {
#ifdef IX_OSAL_BUFFER_ALLOC_SEPARATELY
				UINT32 i;
				IX_OSAL_MBUF* pBuf;
				
				pBuf = pool->nextFreeBuf;
				/* Freed the Buffer one by one till all the Memory is freed*/
				for (i= pool->freeBufsInPool; i >0 && pBuf!=NULL ;i--){
						IX_OSAL_MBUF* pBufTemp;
						pBufTemp = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(pBuf);
						/* Freed MBUF Data Memory area*/
						IX_OSAL_CACHE_DMA_FREE( (void *) (IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(pBuf)) );
						/* Freed MBUF Struct Memory area*/
						IX_OSAL_CACHE_DMA_FREE(pBuf);
						pBuf = pBufTemp;
				}
				
#else    	
        IX_OSAL_CACHE_DMA_FREE (pool->mbufMemPtr);
        IX_OSAL_CACHE_DMA_FREE (pool->dataMemPtr);
#endif        
    }

    ixOsalBuffFreePools[pool->poolIdx / IX_OSAL_BUFF_FREE_BITS] &=
        ~(1 << (pool->poolIdx % IX_OSAL_BUFF_FREE_BITS));
    ixOsalBuffPoolsInUse--;
    return IX_SUCCESS;
}

/*
 * Function definition: ixOsalBuffPoolDataAreaSizeGet
 */
PUBLIC UINT32
ixOsalBuffPoolDataAreaSizeGet (int count, int size)
{
    UINT32 memorySize;
    memorySize = count * IX_OSAL_MBUF_POOL_SIZE_ALIGN (size);
    return memorySize;
}

/*
 * Function definition: ixOsalBuffPoolMbufAreaSizeGet
 */
PUBLIC UINT32
ixOsalBuffPoolMbufAreaSizeGet (int count)
{
    UINT32 memorySize;
    memorySize =
        count * IX_OSAL_MBUF_POOL_SIZE_ALIGN (sizeof (IX_OSAL_MBUF));
    return memorySize;
}

/*
 * Function definition: ixOsalBuffPoolFreeCountGet
 */
PUBLIC UINT32 ixOsalBuffPoolFreeCountGet(IX_OSAL_MBUF_POOL * poolPtr)

{

   return poolPtr->freeBufsInPool;

}

#endif /* IX_OSAL_USE_DEFAULT_BUFFER_MGT */

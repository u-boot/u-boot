/**
 * @file IxEthDataPlane.c
 *
 * @author Intel Corporation
 * @date 12-Feb-2002
 *
 * @brief This file contains the implementation of the IXPxxx
 * Ethernet Access Data plane component
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
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * @par
 * -- End of Copyright Notice --
 */

#include "IxNpeMh.h"
#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxOsal.h"
#include "IxEthDBPortDefs.h"
#include "IxFeatureCtrl.h"
#include "IxEthAcc_p.h"
#include "IxEthAccQueueAssign_p.h"

extern PUBLIC IxEthAccMacState ixEthAccMacState[];
extern PUBLIC UINT32 ixEthAccNewSrcMask;

/**
 * private functions prototype
 */
PRIVATE IX_OSAL_MBUF *
ixEthAccEntryFromQConvert(UINT32 qEntry, UINT32 mask);

PRIVATE UINT32
ixEthAccMbufRxQPrepare(IX_OSAL_MBUF *mbuf);

PRIVATE UINT32
ixEthAccMbufTxQPrepare(IX_OSAL_MBUF *mbuf);

PRIVATE IxEthAccStatus
ixEthAccTxSwQHighestPriorityGet(IxEthAccPortId portId,
				IxEthAccTxPriority *priorityPtr);

PRIVATE IxEthAccStatus
ixEthAccTxFromSwQ(IxEthAccPortId portId,
		  IxEthAccTxPriority priority);

PRIVATE IxEthAccStatus
ixEthAccRxFreeFromSwQ(IxEthAccPortId portId);

PRIVATE void
ixEthAccMbufFromTxQ(IX_OSAL_MBUF *mbuf);

PRIVATE void
ixEthAccMbufFromRxQ(IX_OSAL_MBUF *mbuf);

PRIVATE IX_STATUS
ixEthAccQmgrLockTxWrite(IxEthAccPortId portId,
			UINT32 qBuffer);

PRIVATE IX_STATUS
ixEthAccQmgrLockRxWrite(IxEthAccPortId portId,
			UINT32 qBuffer);

PRIVATE IX_STATUS
ixEthAccQmgrTxWrite(IxEthAccPortId portId,
		    UINT32 qBuffer,
		    UINT32 priority);

/**
 * @addtogroup IxEthAccPri
 *@{
 */

/* increment a counter only when stats are enabled */
#define TX_STATS_INC(port,field) \
        IX_ETH_ACC_STATS_INC(ixEthAccPortData[port].ixEthAccTxData.stats.field)
#define RX_STATS_INC(port,field) \
        IX_ETH_ACC_STATS_INC(ixEthAccPortData[port].ixEthAccRxData.stats.field)

/* always increment the counter (mainly used for unexpected errors) */
#define TX_INC(port,field) \
        ixEthAccPortData[port].ixEthAccTxData.stats.field++
#define RX_INC(port,field) \
        ixEthAccPortData[port].ixEthAccRxData.stats.field++

PRIVATE IxEthAccDataPlaneStats     ixEthAccDataStats;

extern IxEthAccPortDataInfo   ixEthAccPortData[];
extern IxEthAccInfo   ixEthAccDataInfo;

PRIVATE IxOsalFastMutex txWriteMutex[IX_ETH_ACC_NUMBER_OF_PORTS];
PRIVATE IxOsalFastMutex rxWriteMutex[IX_ETH_ACC_NUMBER_OF_PORTS];

/**
 *
 * @brief Mbuf header conversion macros : they implement the
 *  different conversions using a temporary value. They also double-check
 *  that the parameters can be converted to/from NPE format.
 *
 */
#if defined(__wince) && !defined(IN_KERNEL)
#define PTR_VIRT2NPE(ptrSrc,dst) \
  do { UINT32 temp; \
      IX_OSAL_ENSURE(sizeof(ptrSrc) == sizeof(UINT32), "Wrong parameter type"); \
      IX_OSAL_ENSURE(sizeof(dst) == sizeof(UINT32), "Wrong parameter type"); \
      temp = (UINT32)IX_OSAL_MBUF_MBUF_VIRTUAL_TO_PHYSICAL_TRANSLATION((IX_OSAL_MBUF*)ptrSrc); \
      (dst) = IX_OSAL_SWAP_BE_SHARED_LONG(temp); } \
  while(0)

#define PTR_NPE2VIRT(type,src,ptrDst) \
  do { void *temp; \
      IX_OSAL_ENSURE(sizeof(type) == sizeof(UINT32), "Wrong parameter type"); \
      IX_OSAL_ENSURE(sizeof(src) == sizeof(UINT32), "Wrong parameter type"); \
      IX_OSAL_ENSURE(sizeof(ptrDst) == sizeof(UINT32), "Wrong parameter type"); \
      temp = (void *)IX_OSAL_SWAP_BE_SHARED_LONG(src); \
      (ptrDst) = (type)IX_OSAL_MBUF_MBUF_PHYSICAL_TO_VIRTUAL_TRANSLATION(temp); } \
  while(0)
#else
#define PTR_VIRT2NPE(ptrSrc,dst) \
  do { UINT32 temp; \
      IX_OSAL_ENSURE(sizeof(ptrSrc) == sizeof(UINT32), "Wrong parameter type"); \
      IX_OSAL_ENSURE(sizeof(dst) == sizeof(UINT32), "Wrong parameter type"); \
      temp = (UINT32)IX_OSAL_MMU_VIRT_TO_PHYS(ptrSrc); \
      (dst) = IX_OSAL_SWAP_BE_SHARED_LONG(temp); } \
  while(0)

#define PTR_NPE2VIRT(type,src,ptrDst) \
  do { void *temp; \
      IX_OSAL_ENSURE(sizeof(type) == sizeof(UINT32), "Wrong parameter type"); \
      IX_OSAL_ENSURE(sizeof(src) == sizeof(UINT32), "Wrong parameter type"); \
      IX_OSAL_ENSURE(sizeof(ptrDst) == sizeof(UINT32), "Wrong parameter type"); \
      temp = (void *)IX_OSAL_SWAP_BE_SHARED_LONG(src); \
      (ptrDst) = (type)IX_OSAL_MMU_PHYS_TO_VIRT(temp); } \
  while(0)
#endif

/**
 *
 * @brief Mbuf payload pointer conversion macros : Wince has its own
 *  method to convert the buffer pointers
 */
#if defined(__wince) && !defined(IN_KERNEL)
#define DATAPTR_VIRT2NPE(ptrSrc,dst) \
  do { UINT32 temp; \
      temp = (UINT32)IX_OSAL_MBUF_DATA_VIRTUAL_TO_PHYSICAL_TRANSLATION(ptrSrc); \
      (dst) = IX_OSAL_SWAP_BE_SHARED_LONG(temp); } \
  while(0)

#else
#define DATAPTR_VIRT2NPE(ptrSrc,dst) PTR_VIRT2NPE(IX_OSAL_MBUF_MDATA(ptrSrc),dst)
#endif


/* Flush the shared part of the mbuf header */
#define IX_ETHACC_NE_CACHE_FLUSH(mbufPtr) \
  do { \
      IX_OSAL_CACHE_FLUSH(IX_ETHACC_NE_SHARED(mbufPtr), \
			      sizeof(IxEthAccNe)); \
    } \
  while(0)

/* Invalidate the shared part of the mbuf header */
#define IX_ETHACC_NE_CACHE_INVALIDATE(mbufPtr) \
  do { \
      IX_OSAL_CACHE_INVALIDATE(IX_ETHACC_NE_SHARED(mbufPtr), \
				   sizeof(IxEthAccNe)); \
    } \
  while(0)

/* Preload one cache line (shared mbuf headers are aligned
 * and their size is 1 cache line)
 *
 * IX_OSAL_CACHED  is defined when the mbuf headers are
 * allocated from cached memory.
 *
 * Other processor on emulation environment may not implement
 * preload function
 */
#ifdef IX_OSAL_CACHED
	#if (CPU!=SIMSPARCSOLARIS) && !defined (__wince)
		#define IX_ACC_DATA_CACHE_PRELOAD(ptr) \
		do { /* preload a cache line (Xscale Processor) */ \
			__asm__ (" pld [%0]\n": : "r" (ptr)); \
		} \
		while(0)
	#else
		/* preload not implemented on different processor */
		#define IX_ACC_DATA_CACHE_PRELOAD(mbufPtr) \
		do { /* nothing */ } while (0)
	#endif
#else
	/* preload not needed if cache is not enabled */
	#define IX_ACC_DATA_CACHE_PRELOAD(mbufPtr) \
	do { /* nothing */ } while (0)
#endif

/**
 *
 * @brief function to retrieve the correct pointer from
 * a queue entry posted by the NPE
 *
 * @param qEntry : entry from qmgr queue
 *        mask : applicable mask for this queue
 *        (4 most significant bits are used for additional informations)
 *
 * @return IX_OSAL_MBUF * pointer to mbuf header
 *
 * @internal
 */
PRIVATE IX_OSAL_MBUF *
ixEthAccEntryFromQConvert(UINT32 qEntry, UINT32 mask)
{
    IX_OSAL_MBUF *mbufPtr;

    if (qEntry != 0)
    {
        /* mask NPE bits (e.g. priority, port ...) */
        qEntry &= mask;

#if IX_ACC_DRAM_PHYS_OFFSET != 0
        /* restore the original address pointer (if PHYS_OFFSET is not 0) */
        qEntry |= (IX_ACC_DRAM_PHYS_OFFSET & ~IX_ETHNPE_QM_Q_RXENET_ADDR_MASK);
#endif
        /* get the mbuf pointer address from the npe-shared address */
        qEntry -= offsetof(IX_OSAL_MBUF,ix_ne);

        /* phys2virt mbuf */
        mbufPtr = (IX_OSAL_MBUF *)IX_OSAL_MMU_PHYS_TO_VIRT(qEntry);

        /* preload the cacheline shared with NPE */
        IX_ACC_DATA_CACHE_PRELOAD(IX_ETHACC_NE_SHARED(mbufPtr));

        /* preload the cacheline used by xscale */
        IX_ACC_DATA_CACHE_PRELOAD(mbufPtr);
    }
    else
    {
	mbufPtr = NULL;
    }

    return mbufPtr;
}

/* Convert the mbuf header for NPE transmission */
PRIVATE UINT32
ixEthAccMbufTxQPrepare(IX_OSAL_MBUF *mbuf)
{
    UINT32 qbuf;
    UINT32 len;

    /* endianess swap for tci and flags
       note: this is done only once, even for chained buffers */
    IX_ETHACC_NE_FLAGS(mbuf)   = IX_OSAL_SWAP_BE_SHARED_SHORT(IX_ETHACC_NE_FLAGS(mbuf));
    IX_ETHACC_NE_VLANTCI(mbuf) = IX_OSAL_SWAP_BE_SHARED_SHORT(IX_ETHACC_NE_VLANTCI(mbuf));

    /* test for unchained mbufs */
    if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbuf) == NULL)
    {
	/* "best case" scenario : unchained mbufs */
	IX_ETH_ACC_STATS_INC(ixEthAccDataStats.unchainedTxMBufs);

	/* payload pointer conversion */
	DATAPTR_VIRT2NPE(mbuf, IX_ETHACC_NE_DATA(mbuf));

	/* unchained mbufs : the frame length is the mbuf length
	 * and the 2 identical lengths are stored in the same
	 * word.
	 */
	len = IX_OSAL_MBUF_MLEN(mbuf);

	/* set the length in both length and pktLen 16-bits fields */
	len |= (len << IX_ETHNPE_ACC_LENGTH_OFFSET);
	IX_ETHACC_NE_LEN(mbuf) = IX_OSAL_SWAP_BE_SHARED_LONG(len);

	/* unchained mbufs : next contains 0 */
	IX_ETHACC_NE_NEXT(mbuf) = 0;

	/* flush shared header after all address conversions */
	IX_ETHACC_NE_CACHE_FLUSH(mbuf);
    }
    else
    {
	/* chained mbufs */
	IX_OSAL_MBUF *ptr = mbuf;
	IX_OSAL_MBUF *nextPtr;
	UINT32 frmLen;

	/* get the frame length from the header of the first buffer */
	frmLen = IX_OSAL_MBUF_PKT_LEN(mbuf);

	do
	{
	    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.chainedTxMBufs);

	    /* payload pointer */
	    DATAPTR_VIRT2NPE(ptr,IX_ETHACC_NE_DATA(ptr));
	    /* Buffer length and frame length are stored in the same word */
	    len = IX_OSAL_MBUF_MLEN(ptr);
	    len = frmLen | (len << IX_ETHNPE_ACC_LENGTH_OFFSET);
	    IX_ETHACC_NE_LEN(ptr) = IX_OSAL_SWAP_BE_SHARED_LONG(len);

	    /* get the virtual next chain pointer */
	    nextPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr);
	    if (nextPtr != NULL)
	    {
		/* shared pointer of the next buffer is chained */
		PTR_VIRT2NPE(IX_ETHACC_NE_SHARED(nextPtr),
			     IX_ETHACC_NE_NEXT(ptr));
	    }
	    else
	    {
		IX_ETHACC_NE_NEXT(ptr) = 0;
	    }

	    /* flush shared header after all address conversions */
	    IX_ETHACC_NE_CACHE_FLUSH(ptr);

	    /* move to next buffer */
	    ptr = nextPtr;

	    /* the frame length field is set only in the first buffer
	     * and is zeroed in the next buffers
	     */
	    frmLen = 0;
	}
	while(ptr != NULL);

    }

    /* virt2phys mbuf itself */
    qbuf = (UINT32)IX_OSAL_MMU_VIRT_TO_PHYS(
		  IX_ETHACC_NE_SHARED(mbuf));

    /* Ensure the bits which are reserved to exchange information with
     * the NPE are cleared
     *
     * If the mbuf address is not correctly aligned, or from an
     * incompatible memory range, there is no point to continue
     */
    IX_OSAL_ENSURE(((qbuf & ~IX_ETHNPE_QM_Q_TXENET_ADDR_MASK) == 0),
	      "Invalid address range");

    return qbuf;
}

/* Convert the mbuf header for NPE reception */
PRIVATE UINT32
ixEthAccMbufRxQPrepare(IX_OSAL_MBUF *mbuf)
{
    UINT32 len;
    UINT32 qbuf;

    if (IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbuf) == NULL)
    {
	/* "best case" scenario : unchained mbufs */
	IX_ETH_ACC_STATS_INC(ixEthAccDataStats.unchainedRxFreeMBufs);

	/* unchained mbufs : payload pointer */
	DATAPTR_VIRT2NPE(mbuf, IX_ETHACC_NE_DATA(mbuf));

	/* unchained mbufs : set the buffer length
	* and the frame length field is zeroed
	*/
	len = (IX_OSAL_MBUF_MLEN(mbuf) << IX_ETHNPE_ACC_LENGTH_OFFSET);
	IX_ETHACC_NE_LEN(mbuf) = IX_OSAL_SWAP_BE_SHARED_LONG(len);

	/* unchained mbufs : next pointer is null */
	IX_ETHACC_NE_NEXT(mbuf) = 0;

	/* flush shared header after all address conversions */
	IX_ETHACC_NE_CACHE_FLUSH(mbuf);

	/* remove shared header cache line */
	IX_ETHACC_NE_CACHE_INVALIDATE(mbuf);
    }
    else
    {
	/* chained mbufs */
	IX_OSAL_MBUF *ptr = mbuf;
	IX_OSAL_MBUF *nextPtr;

	do
	{
	    /* chained mbufs */
	    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.chainedRxFreeMBufs);

	    /* we must save virtual next chain pointer */
	    nextPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr);

	    if (nextPtr != NULL)
	    {
		/* chaining pointer for NPE */
		PTR_VIRT2NPE(IX_ETHACC_NE_SHARED(nextPtr),
			     IX_ETHACC_NE_NEXT(ptr));
	    }
	    else
	    {
		IX_ETHACC_NE_NEXT(ptr) = 0;
	    }

	    /* payload pointer */
	    DATAPTR_VIRT2NPE(ptr,IX_ETHACC_NE_DATA(ptr));

	    /* buffer length */
	    len = (IX_OSAL_MBUF_MLEN(ptr) << IX_ETHNPE_ACC_LENGTH_OFFSET);
	    IX_ETHACC_NE_LEN(ptr) = IX_OSAL_SWAP_BE_SHARED_LONG(len);

	    /* flush shared header after all address conversions */
	    IX_ETHACC_NE_CACHE_FLUSH(ptr);

	    /* remove shared header cache line */
	    IX_ETHACC_NE_CACHE_INVALIDATE(ptr);

	    /* next mbuf in the chain */
	    ptr = nextPtr;
	}
	while(ptr != NULL);
    }

    /* virt2phys mbuf itself */
    qbuf = (UINT32)IX_OSAL_MMU_VIRT_TO_PHYS(
		  IX_ETHACC_NE_SHARED(mbuf));

    /* Ensure the bits which are reserved to exchange information with
     * the NPE are cleared
     *
     * If the mbuf address is not correctly aligned, or from an
     * incompatible memory range, there is no point to continue
     */
    IX_OSAL_ENSURE(((qbuf & ~IX_ETHNPE_QM_Q_RXENET_ADDR_MASK) == 0),
	      "Invalid address range");

    return qbuf;
}

/* Convert the mbuf header after NPE transmission
 * Since there is nothing changed by the NPE, there is no need
 * to process anything but the update of internal stats
 * when they are enabled
*/
PRIVATE void
ixEthAccMbufFromTxQ(IX_OSAL_MBUF *mbuf)
{
#ifndef NDEBUG
    /* test for unchained mbufs */
    if (IX_ETHACC_NE_NEXT(mbuf) == 0)
    {
	/* unchained mbufs : update the stats */
	IX_ETH_ACC_STATS_INC(ixEthAccDataStats.unchainedTxDoneMBufs);
    }
    else
    {
	/* chained mbufs : walk the chain and update the stats */
	IX_OSAL_MBUF *ptr = mbuf;

	do
	{
	    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.chainedTxDoneMBufs);
	    ptr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr);
	}
	while (ptr != NULL);
    }
#endif
}

/* Convert the mbuf header after NPE reception */
PRIVATE void
ixEthAccMbufFromRxQ(IX_OSAL_MBUF *mbuf)
{
    UINT32 len;

    /* endianess swap for tci and flags
       note: this is done only once, even for chained buffers */
    IX_ETHACC_NE_FLAGS(mbuf)   = IX_OSAL_SWAP_BE_SHARED_SHORT(IX_ETHACC_NE_FLAGS(mbuf));
    IX_ETHACC_NE_VLANTCI(mbuf) = IX_OSAL_SWAP_BE_SHARED_SHORT(IX_ETHACC_NE_VLANTCI(mbuf));

    /* test for unchained mbufs */
    if (IX_ETHACC_NE_NEXT(mbuf) == 0)
    {
	/* unchained mbufs */
	IX_ETH_ACC_STATS_INC(ixEthAccDataStats.unchainedRxMBufs);

	/* get the frame length. it is the same than the buffer length */
	len = IX_OSAL_SWAP_BE_SHARED_LONG(IX_ETHACC_NE_LEN(mbuf));
	len &= IX_ETHNPE_ACC_PKTLENGTH_MASK;
	IX_OSAL_MBUF_PKT_LEN(mbuf) = IX_OSAL_MBUF_MLEN(mbuf) = len;

        /* clears the next packet field */
	IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbuf) = NULL;
    }
    else
    {
	IX_OSAL_MBUF *ptr = mbuf;
	IX_OSAL_MBUF *nextPtr;
	UINT32 frmLen;

	/* convert the frame length */
	frmLen = IX_OSAL_SWAP_BE_SHARED_LONG(IX_ETHACC_NE_LEN(mbuf));
	IX_OSAL_MBUF_PKT_LEN(mbuf) = (frmLen & IX_ETHNPE_ACC_PKTLENGTH_MASK);

        /* chained mbufs */
	do
	{
	    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.chainedRxMBufs);

	    /* convert the length */
	    len = IX_OSAL_SWAP_BE_SHARED_LONG(IX_ETHACC_NE_LEN(ptr));
	    IX_OSAL_MBUF_MLEN(ptr) = (len >> IX_ETHNPE_ACC_LENGTH_OFFSET);

            /* get the next pointer */
 	    PTR_NPE2VIRT(IX_OSAL_MBUF *,IX_ETHACC_NE_NEXT(ptr), nextPtr);
	    if (nextPtr != NULL)
	    {
		nextPtr = (IX_OSAL_MBUF *)((UINT8 *)nextPtr - offsetof(IX_OSAL_MBUF,ix_ne));
	    }
	    /* set the next pointer */
	    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(ptr) = nextPtr;

	    /* move to the next buffer */
	    ptr = nextPtr;
	}
	while (ptr != NULL);
    }
}

/* write to qmgr if possible and report an overflow if not possible
 * Use a fast lock to protect the queue write.
 * This way, the tx feature is reentrant.
 */
PRIVATE IX_STATUS
ixEthAccQmgrLockTxWrite(IxEthAccPortId portId, UINT32 qBuffer)
{
    IX_STATUS qStatus;
    if (ixOsalFastMutexTryLock(&txWriteMutex[portId]) == IX_SUCCESS)
    {
	qStatus = ixQMgrQWrite(
	       IX_ETH_ACC_PORT_TO_TX_Q_ID(portId),
	       &qBuffer);
#ifndef NDEBUG
	if (qStatus != IX_SUCCESS)
	{
	    TX_STATS_INC(portId, txOverflow);
	}
#endif
	ixOsalFastMutexUnlock(&txWriteMutex[portId]);
    }
    else
    {
	TX_STATS_INC(portId, txLock);
	qStatus = IX_QMGR_Q_OVERFLOW;
    }
    return qStatus;
}

/* write to qmgr if possible and report an overflow if not possible
 * Use a fast lock to protect the queue write.
 * This way, the Rx feature is reentrant.
 */
PRIVATE IX_STATUS
ixEthAccQmgrLockRxWrite(IxEthAccPortId portId, UINT32 qBuffer)
{
    IX_STATUS qStatus;
    if (ixOsalFastMutexTryLock(&rxWriteMutex[portId]) == IX_SUCCESS)
    {
	qStatus = ixQMgrQWrite(
	       IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId),
	       &qBuffer);
#ifndef NDEBUG
	if (qStatus != IX_SUCCESS)
	{
	    RX_STATS_INC(portId, rxFreeOverflow);
	}
#endif
	ixOsalFastMutexUnlock(&rxWriteMutex[portId]);
    }
    else
    {
	RX_STATS_INC(portId, rxFreeLock);
	qStatus = IX_QMGR_Q_OVERFLOW;
    }
    return qStatus;
}

/*
 * Set the priority and write to a qmgr queue.
 */
PRIVATE IX_STATUS
ixEthAccQmgrTxWrite(IxEthAccPortId portId, UINT32 qBuffer, UINT32 priority)
{
    /* fill the priority field */
    qBuffer |= (priority << IX_ETHNPE_QM_Q_FIELD_PRIOR_R);

    return ixEthAccQmgrLockTxWrite(portId, qBuffer);
}

/**
 *
 * @brief This function will discover the highest priority S/W Tx Q that
 *        has entries in it
 *
 * @param portId - (in) the id of the port whose S/W Tx queues are to be searched
 *        priorityPtr - (out) the priority of the highest priority occupied q will be written
 *                      here
 *
 * @return IX_ETH_ACC_SUCCESS if an occupied Q is found
 *         IX_ETH_ACC_FAIL if no Q has entries
 *
 * @internal
 */
PRIVATE IxEthAccStatus
ixEthAccTxSwQHighestPriorityGet(IxEthAccPortId portId,
				IxEthAccTxPriority *priorityPtr)
{
    if (ixEthAccPortData[portId].ixEthAccTxData.schDiscipline
	== FIFO_NO_PRIORITY)
    {
	if(IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(ixEthAccPortData[portId].
	       ixEthAccTxData.txQ[IX_ETH_ACC_TX_DEFAULT_PRIORITY]))
	{
	    return IX_ETH_ACC_FAIL;
	}
	else
	{
	    *priorityPtr = IX_ETH_ACC_TX_DEFAULT_PRIORITY;
	    TX_STATS_INC(portId,txPriority[*priorityPtr]);
	    return IX_ETH_ACC_SUCCESS;
	}
    }
    else
    {
	IxEthAccTxPriority highestPriority = IX_ETH_ACC_TX_PRIORITY_7;
	while(1)
	{
	    if(!IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(ixEthAccPortData[portId].
	       ixEthAccTxData.txQ[highestPriority]))
	    {

		*priorityPtr = highestPriority;
		TX_STATS_INC(portId,txPriority[highestPriority]);
		return IX_ETH_ACC_SUCCESS;

	    }
	    if (highestPriority == IX_ETH_ACC_TX_PRIORITY_0)
	    {
		return IX_ETH_ACC_FAIL;
	    }
	    highestPriority--;
	}
    }
}

/**
 *
 * @brief This function will take a buffer from a TX S/W Q and attempt
 *        to add it to the relevant TX H/W Q
 *
 * @param portId - the port whose TX queue is to be written to
 *        priority - identifies the queue from which the entry is to be read
 *
 * @internal
 */
PRIVATE IxEthAccStatus
ixEthAccTxFromSwQ(IxEthAccPortId portId,
		  IxEthAccTxPriority priority)
{
    IX_OSAL_MBUF        *mbuf;
    IX_STATUS	   qStatus;

    IX_OSAL_ENSURE((UINT32)priority <= (UINT32)7, "Invalid priority");

    IX_ETH_ACC_DATAPLANE_REMOVE_MBUF_FROM_Q_HEAD(
	ixEthAccPortData[portId].ixEthAccTxData.txQ[priority],
	mbuf);

    if (mbuf != NULL)
    {
	/*
	 * Add the Tx buffer to the H/W Tx Q
	 * We do not need to flush here as it is already done
	 * in TxFrameSubmit().
	 */
	qStatus = ixEthAccQmgrTxWrite(
	      portId,
	      IX_OSAL_MMU_VIRT_TO_PHYS((UINT32)IX_ETHACC_NE_SHARED(mbuf)),
	      priority);

	if (qStatus == IX_SUCCESS)
	{
	    TX_STATS_INC(portId,txFromSwQOK);
	    return IX_SUCCESS;
	}
	else if (qStatus == IX_QMGR_Q_OVERFLOW)
	{
	    /*
	     * H/W Q overflow, need to save the buffer
	     * back on the s/w Q.
	     * we must put it back on the head of the q to avoid
	     * reordering packet tx
	     */
	    TX_STATS_INC(portId,txFromSwQDelayed);
	    IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(
		ixEthAccPortData[portId].ixEthAccTxData.txQ[priority],
		mbuf);

	    /*enable Q notification*/
	    qStatus = ixQMgrNotificationEnable(
		IX_ETH_ACC_PORT_TO_TX_Q_ID(portId),
		IX_ETH_ACC_PORT_TO_TX_Q_SOURCE(portId));

            if (qStatus != IX_SUCCESS && qStatus != IX_QMGR_WARNING)
            {
		TX_INC(portId,txUnexpectedError);
		IX_ETH_ACC_FATAL_LOG(
	            "ixEthAccTxFromSwQ:Unexpected Error: %u\n",
	            qStatus, 0, 0, 0, 0, 0);
            }
	}
	else
	{
	    TX_INC(portId,txUnexpectedError);

	    /* recovery attempt */
	    IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(
		ixEthAccPortData[portId].ixEthAccTxData.txQ[priority],
		mbuf);

	    IX_ETH_ACC_FATAL_LOG(
		"ixEthAccTxFromSwQ:Error: unexpected QM status 0x%08X\n",
		qStatus, 0, 0, 0, 0, 0);
	}
    }
    else
    {
	/* sw queue is empty */
    }
    return IX_ETH_ACC_FAIL;
}

/**
 *
 * @brief This function will take a buffer from a RXfree S/W Q and attempt
 *        to add it to the relevant RxFree H/W Q
 *
 * @param portId - the port whose RXFree queue is to be written to
 *
 * @internal
 */
PRIVATE IxEthAccStatus
ixEthAccRxFreeFromSwQ(IxEthAccPortId portId)
{
    IX_OSAL_MBUF        *mbuf;
    IX_STATUS	   qStatus = IX_SUCCESS;

    IX_ETH_ACC_DATAPLANE_REMOVE_MBUF_FROM_Q_HEAD(
	  ixEthAccPortData[portId].ixEthAccRxData.freeBufferList,
	  mbuf);
    if (mbuf != NULL)
    {
	/*
	 * Add The Rx Buffer to the H/W Free buffer Q if possible
	 */
	qStatus = ixEthAccQmgrLockRxWrite(portId,
		  IX_OSAL_MMU_VIRT_TO_PHYS(
			 (UINT32)IX_ETHACC_NE_SHARED(mbuf)));

	if (qStatus == IX_SUCCESS)
	{
	    RX_STATS_INC(portId,rxFreeRepFromSwQOK);
	    /*
	     * Buffer added to h/w Q.
	     */
	    return IX_SUCCESS;
	}
	else if (qStatus == IX_QMGR_Q_OVERFLOW)
	{
	    /*
	     * H/W Q overflow, need to save the buffer back on the s/w Q.
	     */
	    RX_STATS_INC(portId,rxFreeRepFromSwQDelayed);

	    IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(
		   ixEthAccPortData[portId].ixEthAccRxData.freeBufferList,
		   mbuf);
	}
	else
	{
	    /* unexpected qmgr error */
	    RX_INC(portId,rxUnexpectedError);

	    IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_HEAD(
		    ixEthAccPortData[portId].ixEthAccRxData.freeBufferList,
		    mbuf);

	    IX_ETH_ACC_FATAL_LOG("IxEthAccRxFreeFromSwQ:Error: unexpected QM status 0x%08X\n",
				 qStatus, 0, 0, 0, 0, 0);
	}
    }
    else
    {
	/* sw queue is empty */
    }
    return IX_ETH_ACC_FAIL;
}


IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccInitDataPlane()
{
    UINT32 portId;

    /*
     * Initialize the service and register callback to other services.
     */

    IX_ETH_ACC_MEMSET(&ixEthAccDataStats,
		      0,
		      sizeof(ixEthAccDataStats));

    for(portId=0; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
    {
	ixOsalFastMutexInit(&txWriteMutex[portId]);
	ixOsalFastMutexInit(&rxWriteMutex[portId]);

	IX_ETH_ACC_MEMSET(&ixEthAccPortData[portId],
			  0,
			  sizeof(ixEthAccPortData[portId]));

	ixEthAccPortData[portId].ixEthAccTxData.schDiscipline = FIFO_NO_PRIORITY;
    }

    return (IX_ETH_ACC_SUCCESS);
}


IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccPortTxDoneCallbackRegister(IxEthAccPortId portId,
						  IxEthAccPortTxDoneCallback
						  txCallbackFn,
						  UINT32 callbackTag)
{
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }

/* HACK: removing this code to enable NPE-A preliminary testing
 *    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
 *    {
 *        IX_ETH_ACC_WARNING_LOG("ixEthAccPortTxDoneCallbackRegister: Unavailable Eth %d: Cannot register TxDone Callback.\n",(INT32)portId,0,0,0,0,0);
 *        return IX_ETH_ACC_SUCCESS ;
 *    }
 */

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    if (txCallbackFn == 0)
	/* Check for null function pointer here. */
    {
	return (IX_ETH_ACC_INVALID_ARG);
    }
    ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn = txCallbackFn;
    ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag = callbackTag;
    return (IX_ETH_ACC_SUCCESS);
}


IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccPortRxCallbackRegister(IxEthAccPortId portId,
					      IxEthAccPortRxCallback
					      rxCallbackFn,
					      UINT32 callbackTag)
{
    IxEthAccPortId port;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("ixEthAccPortRxCallbackRegister: Unavailable Eth %d: Cannot register Rx Callback.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* Check for null function pointer here. */
    if (rxCallbackFn == NULL)
    {
	return (IX_ETH_ACC_INVALID_ARG);
    }

    /* Check the user is not changing the callback type
     * when the port is enabled.
    */
    if (ixEthAccMacState[portId].portDisableState == ACTIVE)
    {
	for (port = 0; port < IX_ETH_ACC_NUMBER_OF_PORTS; port++)
	{
	    if ((ixEthAccMacState[port].portDisableState == ACTIVE)
		&& (ixEthAccPortData[port].ixEthAccRxData.rxMultiBufferCallbackInUse == TRUE))
	    {
		/* one of the active ports has a different rx callback type.
		 * Changing the callback type when the port is enabled
		 * is not safe
		 */
		return (IX_ETH_ACC_INVALID_ARG);
	    }
	}
    }

    /* update the callback pointer : this is done before
     * registering the new qmgr callback
     */
    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn = rxCallbackFn;
    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag = callbackTag;

    /* update the qmgr callback for rx queues */
    if (ixEthAccQMgrRxCallbacksRegister(ixEthRxFrameQMCallback)
	!= IX_ETH_ACC_SUCCESS)
    {
	/* unexpected qmgr error */
        IX_ETH_ACC_FATAL_LOG("ixEthAccPortRxCallbackRegister: unexpected QMgr error, " \
            "could not register Rx single-buffer callback\n", 0, 0, 0, 0, 0, 0);

	RX_INC(portId,rxUnexpectedError);
	return (IX_ETH_ACC_INVALID_ARG);
    }

    ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackInUse = FALSE;

    return (IX_ETH_ACC_SUCCESS);
}

IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccPortMultiBufferRxCallbackRegister(
			 IxEthAccPortId portId,
			 IxEthAccPortMultiBufferRxCallback
			 rxCallbackFn,
			 UINT32 callbackTag)
{
    IxEthAccPortId port;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("ixEthAccPortMultiBufferRxCallbackRegister: Unavailable Eth %d: Cannot register Rx Callback.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    /* Check for null function pointer here. */
    if (rxCallbackFn == NULL)
    {
	return (IX_ETH_ACC_INVALID_ARG);
    }

    /* Check the user is not changing the callback type
     * when the port is enabled.
    */
    if (ixEthAccMacState[portId].portDisableState == ACTIVE)
    {
	for (port = 0; port < IX_ETH_ACC_NUMBER_OF_PORTS; port++)
	{
	    if ((ixEthAccMacState[port].portDisableState == ACTIVE)
		&& (ixEthAccPortData[port].ixEthAccRxData.rxMultiBufferCallbackInUse == FALSE))
	    {
		/* one of the active ports has a different rx callback type.
		 * Changing the callback type when the port is enabled
		 * is not safe
		 */
		return (IX_ETH_ACC_INVALID_ARG);
	    }
	}
    }

    /* update the callback pointer : this is done before
     * registering the new qmgr callback
     */
    ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackFn = rxCallbackFn;
    ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackTag = callbackTag;

    /* update the qmgr callback for rx queues */
    if (ixEthAccQMgrRxCallbacksRegister(ixEthRxMultiBufferQMCallback)
	!= IX_ETH_ACC_SUCCESS)
    {
	/* unexpected qmgr error */
	RX_INC(portId,rxUnexpectedError);

        IX_ETH_ACC_FATAL_LOG("ixEthAccPortMultiBufferRxCallbackRegister: unexpected QMgr error, " \
            "could not register Rx multi-buffer callback\n", 0, 0, 0, 0, 0, 0);

	return (IX_ETH_ACC_INVALID_ARG);
    }

    ixEthAccPortData[portId].ixEthAccRxData.rxMultiBufferCallbackInUse = TRUE;

    return (IX_ETH_ACC_SUCCESS);
}

IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccPortTxFrameSubmit(IxEthAccPortId portId,
					 IX_OSAL_MBUF *buffer,
					 IxEthAccTxPriority priority)
{
    IX_STATUS	qStatus = IX_SUCCESS;
    UINT32      qBuffer;
    IxEthAccTxPriority highestPriority;
    IxQMgrQStatus txQStatus;

#ifndef NDEBUG
    if (buffer == NULL)
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_FATAL_LOG("ixEthAccPortTxFrameSubmit: Unavailable Eth %d: Cannot submit Tx Frame.\n",
			     (INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_PORT_UNINITIALIZED ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    if ((UINT32)priority > (UINT32)IX_ETH_ACC_TX_PRIORITY_7)
    {
	return (IX_ETH_ACC_INVALID_ARG);
    }
#endif

    /*
     * Need to Flush the MBUF and its contents (data) as it may be
     * read from the NPE. Convert virtual addresses to physical addresses also.
     */
    qBuffer = ixEthAccMbufTxQPrepare(buffer);

    /*
     * If no fifo priority set on Xscale ...
     */
    if (ixEthAccPortData[portId].ixEthAccTxData.schDiscipline ==
	FIFO_NO_PRIORITY)
    {
	/*
	 * Add The Tx Buffer to the H/W Tx Q if possible
	 * (the priority is passed to the NPE, because
	 * the NPE is able to reorder the frames
	 * before transmission to the underlying hardware)
	 */
	qStatus = ixEthAccQmgrTxWrite(portId,
				      qBuffer,
				      IX_ETH_ACC_TX_DEFAULT_PRIORITY);

	if (qStatus == IX_SUCCESS)
	{
	    TX_STATS_INC(portId,txQOK);

	    /*
	     * "best case" scenario : Buffer added to h/w Q.
	     */
	    return (IX_SUCCESS);
	}
	else if (qStatus == IX_QMGR_Q_OVERFLOW)
	{
	    /*
	     * We were unable to write the buffer to the
	     * appropriate H/W Q,  Save it in the sw Q.
	     * (use the default priority queue regardless of
	     * input parameter)
	     */
	    priority = IX_ETH_ACC_TX_DEFAULT_PRIORITY;
	}
	else
	{
	    /* unexpected qmgr error */
	    TX_INC(portId,txUnexpectedError);
	    IX_ETH_ACC_FATAL_LOG(
		"ixEthAccPortTxFrameSubmit:Error: qStatus = %u\n",
		(UINT32)qStatus, 0, 0, 0, 0, 0);
	    return (IX_ETH_ACC_FAIL);
	}
    }
    else if (ixEthAccPortData[portId].ixEthAccTxData.schDiscipline ==
	     FIFO_PRIORITY)
    {

	/*
	 * For priority transmission, put the frame directly on the H/W queue
	 * if the H/W queue is empty, otherwise, put it in a S/W Q
	 */
	ixQMgrQStatusGet(IX_ETH_ACC_PORT_TO_TX_Q_ID(portId), &txQStatus);
	if((txQStatus & IX_QMGR_Q_STATUS_E_BIT_MASK) != 0)
	{
	    /*The tx queue is empty, check whether there are buffers on the s/w queues*/
	    if(ixEthAccTxSwQHighestPriorityGet(portId,  &highestPriority)
	       !=IX_ETH_ACC_FAIL)
	    {
		/*there are buffers on the s/w queues, submit them*/
		ixEthAccTxFromSwQ(portId, highestPriority);

		/* the queue was empty, 1 buffer is already supplied
		 * but is likely to be immediately transmitted and the
		 * hw queue is likely to be empty again, so submit
		 * more from the sw queues
		 */
		if(ixEthAccTxSwQHighestPriorityGet(portId,  &highestPriority)
		   !=IX_ETH_ACC_FAIL)
		{
		    ixEthAccTxFromSwQ(portId, highestPriority);
		    /*
		     * and force the buffer supplied to be placed
		     * on a priority queue
		     */
		    qStatus = IX_QMGR_Q_OVERFLOW;
		}
		else
		{
		    /*there are no buffers in the s/w queues, submit directly*/
		    qStatus = ixEthAccQmgrTxWrite(portId, qBuffer, priority);
		}
	    }
	    else
	    {
		/*there are no buffers in the s/w queues, submit directly*/
		qStatus = ixEthAccQmgrTxWrite(portId, qBuffer, priority);
	    }
	}
	else
	{
	    qStatus = IX_QMGR_Q_OVERFLOW;
	}
    }
    else
    {
	TX_INC(portId,txUnexpectedError);
	IX_ETH_ACC_FATAL_LOG(
	    "ixEthAccPortTxFrameSubmit:Error: wrong schedule discipline setup\n",
	    0, 0, 0, 0, 0, 0);
	return (IX_ETH_ACC_FAIL);
    }

    if(qStatus == IX_SUCCESS )
    {
	TX_STATS_INC(portId,txQOK);
	return IX_ETH_ACC_SUCCESS;
    }
    else if(qStatus == IX_QMGR_Q_OVERFLOW)
    {
	TX_STATS_INC(portId,txQDelayed);
	/*
	 * We were unable to write the buffer to the
	 * appropriate H/W Q,  Save it in a s/w Q.
	 */
	IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_TAIL(
		ixEthAccPortData[portId].
		ixEthAccTxData.txQ[priority],
		buffer);

	qStatus = ixQMgrNotificationEnable(
		IX_ETH_ACC_PORT_TO_TX_Q_ID(portId),
		IX_ETH_ACC_PORT_TO_TX_Q_SOURCE(portId));

        if (qStatus != IX_SUCCESS)
	{
	    if (qStatus == IX_QMGR_WARNING)
	    {
		/* notification is enabled for a queue
		 * which is already empty (the condition is already met)
		 * and there will be no more queue event to drain the sw queue
		 */
		TX_STATS_INC(portId,txLateNotificationEnabled);

		/* pull a buffer from the sw queue */
		if(ixEthAccTxSwQHighestPriorityGet(portId,  &highestPriority)
		   !=IX_ETH_ACC_FAIL)
		{
		    /*there are buffers on the s/w queues, submit from them*/
		    ixEthAccTxFromSwQ(portId, highestPriority);
		}
	    }
	    else
	    {
		TX_INC(portId,txUnexpectedError);
		IX_ETH_ACC_FATAL_LOG(
		     "ixEthAccPortTxFrameSubmit: unexpected Error: %u\n",
		     qStatus, 0, 0, 0, 0, 0);
	    }
        }
    }
    else
    {
	TX_INC(portId,txUnexpectedError);
	IX_ETH_ACC_FATAL_LOG(
	     "ixEthAccPortTxFrameSubmit: unexpected Error: %u\n",
	     qStatus, 0, 0, 0, 0, 0);
	return (IX_ETH_ACC_FAIL);
    }

    return (IX_ETH_ACC_SUCCESS);
}


/**
 *
 * @brief replenish: convert a chain of mbufs to the format
 *        expected by the NPE
 *
  */

IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccPortRxFreeReplenish(IxEthAccPortId portId,
					   IX_OSAL_MBUF *buffer)
{
    IX_STATUS	qStatus = IX_SUCCESS;
    UINT32      qBuffer;

    /*
     * Check buffer is valid.
     */

#ifndef NDEBUG
    /* check parameter value */
    if (buffer == 0)
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return (IX_ETH_ACC_FAIL);
    }
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }

    /* check initialisation is done */
    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_FATAL_LOG(" ixEthAccPortRxFreeReplenish: Unavailable Eth %d: Cannot replenish Rx Free Q.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_PORT_UNINITIALIZED ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }
    /* check boundaries and constraints */
    if (IX_OSAL_MBUF_MLEN(buffer) < IX_ETHNPE_ACC_RXFREE_BUFFER_LENGTH_MIN)
    {
	return (IX_ETH_ACC_FAIL);
    }
#endif

    qBuffer = ixEthAccMbufRxQPrepare(buffer);

    /*
     * Add The Rx Buffer to the H/W Free buffer Q if possible
     */
    qStatus = ixEthAccQmgrLockRxWrite(portId, qBuffer);

    if (qStatus == IX_SUCCESS)
    {
	RX_STATS_INC(portId,rxFreeRepOK);
	/*
	 * Buffer added to h/w Q.
	 */
	return (IX_SUCCESS);
    }
    else if (qStatus == IX_QMGR_Q_OVERFLOW)
    {
	RX_STATS_INC(portId,rxFreeRepDelayed);
	/*
	 * We were unable to write the buffer to the approprate H/W Q,
	 * Save it in a s/w Q.
	 */
	IX_ETH_ACC_DATAPLANE_ADD_MBUF_TO_Q_TAIL(
	    ixEthAccPortData[portId].ixEthAccRxData.freeBufferList,
	    buffer);

	qStatus = ixQMgrNotificationEnable(
	    IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId),
	    IX_ETH_ACC_PORT_TO_RX_FREE_Q_SOURCE(portId));

        if (qStatus != IX_SUCCESS)
	{
	    if (qStatus == IX_QMGR_WARNING)
	    {
		/* notification is enabled for a queue
		 * which is already empty (the condition is already met)
		 * and there will be no more queue event to drain the sw queue
		 * move an entry from the sw queue to the hw queue */
		RX_STATS_INC(portId,rxFreeLateNotificationEnabled);
		ixEthAccRxFreeFromSwQ(portId);
	    }
	    else
	    {
		RX_INC(portId,rxUnexpectedError);
		IX_ETH_ACC_FATAL_LOG(
		     "ixEthAccRxPortFreeReplenish:Error: %u\n",
		     qStatus, 0, 0, 0, 0, 0);
	    }
        }
    }
    else
    {
	RX_INC(portId,rxUnexpectedError);
	IX_ETH_ACC_FATAL_LOG(
	    "ixEthAccRxPortFreeReplenish:Error: qStatus = %u\n",
	    (UINT32)qStatus, 0, 0, 0, 0, 0);
        return(IX_ETH_ACC_FAIL);
    }
    return (IX_ETH_ACC_SUCCESS);
}


IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccTxSchedulingDisciplineSetPriv(IxEthAccPortId portId,
						 IxEthAccSchedulerDiscipline
						 sched)
{
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	return (IX_ETH_ACC_INVALID_PORT);
    }

    if (IX_ETH_ACC_SUCCESS != ixEthAccSingleEthNpeCheck(portId))
    {
        IX_ETH_ACC_WARNING_LOG("ixEthAccTxSchedulingDisciplineSet: Unavailable Eth %d: Cannot set Tx Scheduling Discipline.\n",(INT32)portId,0,0,0,0,0);
        return IX_ETH_ACC_SUCCESS ;
    }

    if (!IX_ETH_IS_PORT_INITIALIZED(portId))
    {
	return (IX_ETH_ACC_PORT_UNINITIALIZED);
    }

    if (sched != FIFO_PRIORITY && sched != FIFO_NO_PRIORITY)
    {
	return (IX_ETH_ACC_INVALID_ARG);
    }

    ixEthAccPortData[portId].ixEthAccTxData.schDiscipline = sched;
    return (IX_ETH_ACC_SUCCESS);
}

IX_ETH_ACC_PUBLIC
IxEthAccStatus ixEthAccRxSchedulingDisciplineSetPriv(IxEthAccSchedulerDiscipline
						 sched)
{
    if (sched != FIFO_PRIORITY && sched != FIFO_NO_PRIORITY)
    {
	return (IX_ETH_ACC_INVALID_ARG);
    }

    ixEthAccDataInfo.schDiscipline = sched;

    return (IX_ETH_ACC_SUCCESS);
}


/**
 * @fn ixEthRxFrameProcess(IxEthAccPortId portId, IX_OSAL_MBUF *mbufPtr)
 *
 * @brief process incoming frame :
 *
 * @param @ref IxQMgrCallback IxQMgrMultiBufferCallback
 *
 * @return none
 *
 * @internal
 *
 */
IX_ETH_ACC_PRIVATE BOOL
ixEthRxFrameProcess(IxEthAccPortId portId, IX_OSAL_MBUF *mbufPtr)
{
    UINT32 flags;
    IxEthDBStatus result;

#ifndef NDEBUG
    /* Prudent to at least check the port is within range */
    if (portId >= IX_ETH_ACC_NUMBER_OF_PORTS)
    {
	ixEthAccDataStats.unexpectedError++;
	IX_ETH_ACC_FATAL_LOG(
	     "ixEthRxFrameProcess: Illegal port: %u\n",
	     (UINT32)portId, 0, 0, 0, 0, 0);
	return FALSE;
    }
#endif

    /* convert fields from mbuf header */
    ixEthAccMbufFromRxQ(mbufPtr);

    /* check about any special processing for this frame */
    flags = IX_ETHACC_NE_FLAGS(mbufPtr);
    if ((flags & (IX_ETHACC_NE_FILTERMASK | IX_ETHACC_NE_NEWSRCMASK)) == 0)
    {
	/* "best case" scenario : nothing special to do for this frame */
	return TRUE;
    }

#ifdef CONFIG_IXP425_COMPONENT_ETHDB
    /* if a new source MAC address is detected by the NPE,
     * update IxEthDB with the portId and the MAC address.
     */
    if ((flags & IX_ETHACC_NE_NEWSRCMASK & ixEthAccNewSrcMask) != 0)
    {
        result = ixEthDBFilteringDynamicEntryProvision(portId,
			  (IxEthDBMacAddr *) IX_ETHACC_NE_SOURCEMAC(mbufPtr));

	if (result != IX_ETH_DB_SUCCESS && result != IX_ETH_DB_FEATURE_UNAVAILABLE)
	{
            if ((ixEthAccMacState[portId].portDisableState == ACTIVE) && (result != IX_ETH_DB_BUSY))
            {
	        RX_STATS_INC(portId, rxUnexpectedError);
                IX_ETH_ACC_FATAL_LOG("ixEthRxFrameProcess: Failed to add source MAC \
                                    to the Learning/Filtering database\n", 0, 0, 0, 0, 0, 0);
            }
            else
            {
                /* we expect this to fail during PortDisable, as EthDB is disabled for
                 * that port and will refuse to learn new addresses
		 */
            }
	}
	else
	{
	    RX_STATS_INC(portId, rxUnlearnedMacAddress);
	}
    }
#endif

    /* check if this frame should have been filtered
     * by the NPE and take the appropriate action
     */
    if (((flags & IX_ETHACC_NE_FILTERMASK) != 0)
        && (ixEthAccMacState[portId].portDisableState == ACTIVE))
    {
        /* If the mbuf was allocated with a small data size, or the current data pointer is not
         * within the allocated data area, then the buffer is non-standard and has to be
         * replenished with the minimum size only
         */
        if( (IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(mbufPtr) < IX_ETHNPE_ACC_RXFREE_BUFFER_LENGTH_MIN)
           || ((UINT8 *)IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(mbufPtr) > IX_OSAL_MBUF_MDATA(mbufPtr))
           || ((UINT8 *)(IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(mbufPtr) +
              IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(mbufPtr))
               < IX_OSAL_MBUF_MDATA(mbufPtr)) )
        {
            /* set to minimum length */
            IX_OSAL_MBUF_MLEN(mbufPtr) = IX_OSAL_MBUF_PKT_LEN(mbufPtr) =
                IX_ETHNPE_ACC_RXFREE_BUFFER_LENGTH_MIN;
        }
        else
        {
            /* restore original length */
            IX_OSAL_MBUF_MLEN(mbufPtr) = IX_OSAL_MBUF_PKT_LEN(mbufPtr) =
                ( IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(mbufPtr) -
                 (IX_OSAL_MBUF_MDATA(mbufPtr) - (UINT8 *)IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(mbufPtr)) );
        }

        /* replenish from here */
        if (ixEthAccPortRxFreeReplenish(portId, mbufPtr) != IX_ETH_ACC_SUCCESS)
        {
                IX_ETH_ACC_FATAL_LOG("ixEthRxFrameProcess: Failed to replenish with filtered frame\
                                      on port %d\n", portId, 0, 0, 0, 0, 0);
        }

        RX_STATS_INC(portId, rxFiltered);

        /* indicate that frame should not be subjected to further processing */
        return FALSE;
    }

    return TRUE;
}


/**
 * @fn ixEthRxFrameQMCallback
 *
 * @brief receive callback for Frame receive Q from NPE
 *
 * Frames are passed one-at-a-time to the user
 *
 * @param @ref IxQMgrCallback
 *
 * @return none
 *
 * @internal
 *
 * Design note : while processing the entry X, entry X+1 is preloaded
 * into memory to reduce the number of stall cycles
 *
 */
void ixEthRxFrameQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId)
{
    IX_OSAL_MBUF    *mbufPtr;
    IX_OSAL_MBUF    *nextMbufPtr;
    UINT32     qEntry;
    UINT32     nextQEntry;
    UINT32     *qEntryPtr;
    UINT32     portId;
    UINT32     destPortId;
    UINT32     npeId;
    UINT32     rxQReadStatus;

    /*
     * Design note : entries are read in a buffer, This buffer contains
     * an extra zeroed entry so the loop will
     * always terminate on a null entry, whatever the result of Burst read is.
     */
    UINT32 rxQEntry[IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK + 1];

    /*
     * Indication of the number of times the callback is used.
     */
    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.rxCallbackCounter);

    do
    {
	/*
	 * Indication of the number of times the queue is drained
	 */
	IX_ETH_ACC_STATS_INC(ixEthAccDataStats.rxCallbackBurstRead);

	/* ensure the last entry of the array contains a zeroed value */
	qEntryPtr = rxQEntry;
	qEntryPtr[IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK] = 0;

	rxQReadStatus = ixQMgrQBurstRead(qId,
		 IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK,
		 qEntryPtr);

#ifndef NDEBUG
	if ((rxQReadStatus != IX_QMGR_Q_UNDERFLOW)
	    && (rxQReadStatus != IX_SUCCESS))
	{
	    ixEthAccDataStats.unexpectedError++;
	    /*major error*/
	    IX_ETH_ACC_FATAL_LOG(
		"ixEthRxFrameQMCallback:Error: %u\n",
		(UINT32)rxQReadStatus, 0, 0, 0, 0, 0);
	    return;
	}
#endif

	/* convert and preload the next entry
	 * (the conversion function takes care about null pointers which
	 * are used to mark the end of the loop)
	 */
	nextQEntry = *qEntryPtr;
	nextMbufPtr = ixEthAccEntryFromQConvert(nextQEntry,
			  IX_ETHNPE_QM_Q_RXENET_ADDR_MASK);

	while(nextQEntry != 0)
	{
	    /* get the next entry */
	    qEntry = nextQEntry;
	    mbufPtr = nextMbufPtr;

#ifndef NDEBUG
	    if (mbufPtr == NULL)
	    {
		ixEthAccDataStats.unexpectedError++;
		IX_ETH_ACC_FATAL_LOG(
		    "ixEthRxFrameQMCallback: Null Mbuf Ptr\n",
		    0, 0, 0, 0, 0, 0);
		return;
	    }
#endif

	    /* convert the next entry
	     * (the conversion function takes care about null pointers which
	     * are used to mark the end of the loop)
	     */
	    nextQEntry = *(++qEntryPtr);
	    nextMbufPtr = ixEthAccEntryFromQConvert(nextQEntry,
			      IX_ETHNPE_QM_Q_RXENET_ADDR_MASK);

	    /*
	     * Get Port and Npe ID from message.
	     */
	    npeId = ((IX_ETHNPE_QM_Q_RXENET_NPEID_MASK &
		      qEntry) >> IX_ETHNPE_QM_Q_FIELD_NPEID_R);
	    portId = IX_ETH_ACC_NPE_TO_PORT_ID(npeId);

	    /* process frame, check the return code and skip the remaining of
	     * the loop if the frame is to be filtered out
	     */
            if (ixEthRxFrameProcess(portId, mbufPtr))
            {
	        /* destination portId for this packet */
	        destPortId = IX_ETHACC_NE_DESTPORTID(mbufPtr);

                if (destPortId != IX_ETH_DB_UNKNOWN_PORT)
                {
                    destPortId = IX_ETH_DB_NPE_LOGICAL_ID_TO_PORT_ID(destPortId);
                }

	        /* test if QoS is enabled in ethAcc
	        */
	        if (ixEthAccDataInfo.schDiscipline == FIFO_PRIORITY)
	        {
		    /* check if there is a higher priority queue
		    * which may require processing and then process it.
		    */
		    if (ixEthAccDataInfo.higherPriorityQueue[qId] < IX_QMGR_MAX_NUM_QUEUES)
		    {
		        ixEthRxFrameQMCallback(ixEthAccDataInfo.higherPriorityQueue[qId],
					    callbackId);
		    }
	        }

	        /*
	        * increment priority stats
	        */
	        RX_STATS_INC(portId,rxPriority[IX_ETHACC_NE_QOS(mbufPtr)]);

	        /*
	        * increment callback count stats
	        */
	        RX_STATS_INC(portId,rxFrameClientCallback);

	        /*
	        * Call user level callback.
	        */
	        ixEthAccPortData[portId].ixEthAccRxData.rxCallbackFn(
		    ixEthAccPortData[portId].ixEthAccRxData.rxCallbackTag,
		    mbufPtr,
		    destPortId);
            }
	}
    } while (rxQReadStatus == IX_SUCCESS);
}

/**
 * @fn ixEthRxMultiBufferQMCallback
 *
 * @brief receive callback for Frame receive Q from NPE
 *
 * Frames are passed as an array to the user
 *
 * @param @ref IxQMgrCallback
 *
 * @return none
 *
 * @internal
 *
 * Design note : while processing the entry X, entry X+1 is preloaded
 * into memory to reduce the number of stall cycles
 *
 */
void ixEthRxMultiBufferQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId)
{
    IX_OSAL_MBUF    *mbufPtr;
    IX_OSAL_MBUF    *nextMbufPtr;
    UINT32     qEntry;
    UINT32     nextQEntry;
    UINT32     *qEntryPtr;
    UINT32     portId;
    UINT32     npeId;
    UINT32     rxQReadStatus;
    /*
     * Design note : entries are read in a static buffer, This buffer contains
     * an extra zeroed entry so the loop will
     * always terminate on a null entry, whatever the result of Burst read is.
     */
    static UINT32 rxQEntry[IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK + 1];
    static IX_OSAL_MBUF *rxMbufPortArray[IX_ETH_ACC_NUMBER_OF_PORTS][IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK + 1];
    IX_OSAL_MBUF **rxMbufPtr[IX_ETH_ACC_NUMBER_OF_PORTS];

    for (portId = 0; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
    {
	rxMbufPtr[portId] = rxMbufPortArray[portId];
    }

    /*
     * Indication of the number of times the callback is used.
     */
    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.rxCallbackCounter);

    do
    {
	/*
	 * Indication of the number of times the queue is drained
	 */
	IX_ETH_ACC_STATS_INC(ixEthAccDataStats.rxCallbackBurstRead);

	/* ensure the last entry of the array contains a zeroed value */
	qEntryPtr = rxQEntry;
	qEntryPtr[IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK] = 0;

	rxQReadStatus = ixQMgrQBurstRead(qId,
		 IX_ETH_ACC_MAX_RX_FRAME_CONSUME_PER_CALLBACK,
		 qEntryPtr);

#ifndef NDEBUG
	if ((rxQReadStatus != IX_QMGR_Q_UNDERFLOW)
	    && (rxQReadStatus != IX_SUCCESS))
	{
	    ixEthAccDataStats.unexpectedError++;
	    /*major error*/
	    IX_ETH_ACC_FATAL_LOG(
		"ixEthRxFrameMultiBufferQMCallback:Error: %u\n",
		(UINT32)rxQReadStatus, 0, 0, 0, 0, 0);
	    return;
	}
#endif

	/* convert and preload the next entry
	 * (the conversion function takes care about null pointers which
	 * are used to mark the end of the loop)
	 */
	nextQEntry = *qEntryPtr;
	nextMbufPtr = ixEthAccEntryFromQConvert(nextQEntry,
			  IX_ETHNPE_QM_Q_RXENET_ADDR_MASK);

	while(nextQEntry != 0)
	{
	    /* get the next entry */
	    qEntry = nextQEntry;
	    mbufPtr = nextMbufPtr;

#ifndef NDEBUG
	    if (mbufPtr == NULL)
	    {
		ixEthAccDataStats.unexpectedError++;
		IX_ETH_ACC_FATAL_LOG(
		    "ixEthRxFrameMultiBufferQMCallback:Error: Null Mbuf Ptr\n",
		    0, 0, 0, 0, 0, 0);
		return;
	    }
#endif

	    /* convert the next entry
	     * (the conversion function takes care about null pointers which
	     * are used to mark the end of the loop)
	     */
	    nextQEntry = *(++qEntryPtr);
	    nextMbufPtr = ixEthAccEntryFromQConvert(nextQEntry,
			      IX_ETHNPE_QM_Q_RXENET_ADDR_MASK);

	    /*
	     * Get Port and Npe ID from message.
	     */
	    npeId = ((IX_ETHNPE_QM_Q_RXENET_NPEID_MASK &
		      qEntry) >>
		     IX_ETHNPE_QM_Q_FIELD_NPEID_R);
	    portId = IX_ETH_ACC_NPE_TO_PORT_ID(npeId);

	    /* skip the remaining of the loop if the frame is
	     * to be filtered out
	     */
	    if (ixEthRxFrameProcess(portId, mbufPtr))
	    {
		/* store a mbuf pointer in an array */
		*rxMbufPtr[portId]++ = mbufPtr;

		/*
		 * increment priority stats
		 */
		RX_STATS_INC(portId,rxPriority[IX_ETHACC_NE_QOS(mbufPtr)]);
	    }

	    /* test for QoS enabled in ethAcc */
	    if (ixEthAccDataInfo.schDiscipline == FIFO_PRIORITY)
	    {
		/* check if there is a higher priority queue
		 * which may require processing and then process it.
		 */
		if (ixEthAccDataInfo.higherPriorityQueue[qId] < IX_QMGR_MAX_NUM_QUEUES)
		{
		    ixEthRxMultiBufferQMCallback(ixEthAccDataInfo.higherPriorityQueue[qId],
						 callbackId);
		}
	    }
	}

	/* check if any of the the arrays contains any entry */
	for (portId = 0; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
	{
	    if (rxMbufPtr[portId] != rxMbufPortArray[portId])
	    {
		/* add a last NULL pointer at the end of the
		 * array of mbuf pointers
		 */
		*rxMbufPtr[portId] = NULL;

		/*
		 * increment callback count stats
		 */
		RX_STATS_INC(portId,rxFrameClientCallback);

		/*
		 * Call user level callback with an array of
		 * buffers (NULL terminated)
		 */
		ixEthAccPortData[portId].ixEthAccRxData.
		    rxMultiBufferCallbackFn(
			    ixEthAccPortData[portId].ixEthAccRxData.
			           rxMultiBufferCallbackTag,
			    rxMbufPortArray[portId]);

		/* reset the buffer pointer to the beginning of
		 * the array
		 */
		rxMbufPtr[portId] = rxMbufPortArray[portId];
	    }
	}

    } while (rxQReadStatus == IX_SUCCESS);
}


/**
 * @brief  rxFree low event handler
 *
 */
void ixEthRxFreeQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId)
{
    IxEthAccPortId	portId = (IxEthAccPortId) callbackId;
    int		        lockVal;
    UINT32		maxQWritesToPerform = IX_ETH_ACC_MAX_RX_FREE_BUFFERS_LOAD;
    IX_STATUS	        qStatus = IX_SUCCESS;

    /*
     * We have reached a low threshold on one of the Rx Free Qs
     */

    /*note that due to the fact that we are working off an Empty threshold, this callback
      need only write a single entry to the Rx Free queue in order to re-arm the notification
    */

    RX_STATS_INC(portId,rxFreeLowCallback);

    /*
     * Get buffers from approprite S/W Rx freeBufferList Q.
     */

#ifndef NDEBUG
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	ixEthAccDataStats.unexpectedError++;
	IX_ETH_ACC_FATAL_LOG(
	    "ixEthRxFreeQMCallback:Error: Invalid Port 0x%08X\n",
	    portId, 0, 0, 0, 0, 0);
	return;
    }
#endif
    IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);
    if (IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(ixEthAccPortData[portId].
					ixEthAccRxData.freeBufferList))
    {
	/*
	 * Turn off Q callback notification for Q in Question.
	 */
	qStatus = ixQMgrNotificationDisable(
	    IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId));


	IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);

	if (qStatus != IX_SUCCESS)
	{
	    RX_INC(portId,rxUnexpectedError);
	    IX_ETH_ACC_FATAL_LOG(
		"ixEthRxFreeQMCallback:Error: unexpected QM status 0x%08X\n",
		qStatus, 0, 0, 0, 0, 0);
	    return;
	}
    }
    else
    {
	IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);
	/*
	 * Load the H/W Q with buffers from the s/w Q.
	 */

	do
	{
	    /*
	     * Consume Q entries. - Note Q contains Physical addresss,
	     * and have already been flushed to memory,
	     * And endianess converted if required.
	     */
	    if (ixEthAccRxFreeFromSwQ(portId) != IX_SUCCESS)
	    {
		/*
		 * No more entries in s/w Q.
		 * Turn off Q callback indication
		 */

		IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);
		if (IX_ETH_ACC_DATAPLANE_IS_Q_EMPTY(ixEthAccPortData[portId].
		    ixEthAccRxData.freeBufferList))
		{
		    qStatus = ixQMgrNotificationDisable(
			IX_ETH_ACC_PORT_TO_RX_FREE_Q_ID(portId));
		}
		IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);
		break;
	    }
	}
	while (--maxQWritesToPerform);
    }
}
/**
 * @fn Tx queue low event handler
 *
 */
void
ixEthTxFrameQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId)
{
    IxEthAccPortId portId = (IxEthAccPortId) callbackId;
    int		   lockVal;
    UINT32	   maxQWritesToPerform = IX_ETH_ACC_MAX_TX_FRAME_TX_CONSUME_PER_CALLBACK;
    IX_STATUS	   qStatus = IX_SUCCESS;
    IxEthAccTxPriority highestPriority;


    /*
     * We have reached a low threshold on the Tx Q, and are being asked to
     * supply a buffer for transmission from our S/W TX queues
     */
    TX_STATS_INC(portId,txLowThreshCallback);

    /*
     * Get buffers from approprite Q.
     */

#ifndef NDEBUG
    if (!IX_ETH_ACC_IS_PORT_VALID(portId))
    {
	ixEthAccDataStats.unexpectedError++;
	IX_ETH_ACC_FATAL_LOG(
	    "ixEthTxFrameQMCallback:Error: Invalid Port 0x%08X\n",
	    portId, 0, 0, 0, 0, 0);
	return;
    }
#endif

    do
    {
	/*
	 * Consume Q entries. - Note Q contains Physical addresss,
	 * and have already been flushed to memory,
	 * and endianess already sone if required.
	 */

	IX_ETH_ACC_DATA_PLANE_LOCK(lockVal);

	if(ixEthAccTxSwQHighestPriorityGet(portId, &highestPriority) ==
	   IX_ETH_ACC_FAIL)
	{
	    /*
	     * No more entries in s/w Q.
	     * Turn off Q callback indication
	     */
	    qStatus = ixQMgrNotificationDisable(
		IX_ETH_ACC_PORT_TO_TX_Q_ID(portId));

	    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);

	    if (qStatus != IX_SUCCESS)
	    {
		ixEthAccDataStats.unexpectedError++;
		IX_ETH_ACC_FATAL_LOG(
		    "ixEthTxFrameQMCallback:Error: unexpected QM status 0x%08X\n",
		    qStatus, 0, 0, 0, 0, 0);
	    }

	    return;
	}
	else
	{
	    IX_ETH_ACC_DATA_PLANE_UNLOCK(lockVal);
	    if (ixEthAccTxFromSwQ(portId,highestPriority)!=IX_SUCCESS)
	    {
                /* nothing left in the sw queue or the hw queues are
                * full. There is no point to continue to drain the
                * sw queues
                */
		return;
	    }
	}
    }
    while (--maxQWritesToPerform);
}

/**
 * @brief TxDone event handler
 *
 * Design note : while processing the entry X, entry X+1 is preloaded
 * into memory to reduce the number of stall cycles
 *
 */

void
ixEthTxFrameDoneQMCallback(IxQMgrQId qId, IxQMgrCallbackId callbackId)
{
    IX_OSAL_MBUF    *mbufPtr;
    UINT32     qEntry;
    UINT32     *qEntryPtr;
    UINT32     txDoneQReadStatus;
    UINT32     portId;
    UINT32     npeId;

    /*
     * Design note : entries are read in a static buffer, This buffer contains
     * an extra entyry (which is zeroed by the compiler), so the loop will
     * always terminate on a null entry, whatever the result of Burst read is.
     */
    static UINT32 txDoneQEntry[IX_ETH_ACC_MAX_TX_FRAME_DONE_CONSUME_PER_CALLBACK + 1];

    /*
     * Indication that Tx frames have been transmitted from the NPE.
     */

    IX_ETH_ACC_STATS_INC(ixEthAccDataStats.txDoneCallbackCounter);

    do{
	qEntryPtr = txDoneQEntry;
	txDoneQReadStatus = ixQMgrQBurstRead(IX_ETH_ACC_TX_FRAME_DONE_ETH_Q,
		     IX_ETH_ACC_MAX_TX_FRAME_DONE_CONSUME_PER_CALLBACK,
		     qEntryPtr);

#ifndef NDEBUG
	if (txDoneQReadStatus != IX_QMGR_Q_UNDERFLOW
	    && (txDoneQReadStatus != IX_SUCCESS))
	{
	    /*major error*/
	    ixEthAccDataStats.unexpectedError++;
	    IX_ETH_ACC_FATAL_LOG(
		"ixEthTxFrameDoneQMCallback:Error: %u\n",
		(UINT32)txDoneQReadStatus, 0, 0, 0, 0, 0);
	    return;
	}
#endif

	qEntry = *qEntryPtr;

	while(qEntry != 0)
	{
	    mbufPtr = ixEthAccEntryFromQConvert(qEntry,
		      IX_ETHNPE_QM_Q_TXENET_ADDR_MASK);

#ifndef NDEBUG
	    if (mbufPtr == NULL)
	    {
		ixEthAccDataStats.unexpectedError++;
		IX_ETH_ACC_FATAL_LOG(
		    "ixEthTxFrameDoneQMCallback:Error: Null Mbuf Ptr\n",
		    0, 0, 0, 0, 0, 0);
		return;
	    }
#endif

	    /* endianness conversions and stats updates */
	    ixEthAccMbufFromTxQ(mbufPtr);

	    /*
	     * Get NPE id from message, then convert to portId.
	     */
	    npeId = ((IX_ETHNPE_QM_Q_TXENETDONE_NPEID_MASK &
		       qEntry) >>
		      IX_ETHNPE_QM_Q_FIELD_NPEID_R);
	    portId = IX_ETH_ACC_NPE_TO_PORT_ID(npeId);

#ifndef NDEBUG
	    /* Prudent to at least check the port is within range */
	    if (portId >= IX_ETH_ACC_NUMBER_OF_PORTS)
	    {
		ixEthAccDataStats.unexpectedError++;
		IX_ETH_ACC_FATAL_LOG(
		    "ixEthTxFrameDoneQMCallback: Illegal port: %u\n",
		    (UINT32)portId, 0, 0, 0, 0, 0);
		return;
	    }
#endif

	    TX_STATS_INC(portId,txDoneClientCallback);

	    /*
	     * Call user level callback.
	     */
	    ixEthAccPortData[portId].ixEthAccTxData.txBufferDoneCallbackFn(
		ixEthAccPortData[portId].ixEthAccTxData.txCallbackTag,
		mbufPtr);

	    /* move to next queue entry */
	    qEntry = *(++qEntryPtr);

	}
    } while( txDoneQReadStatus == IX_SUCCESS );
}

IX_ETH_ACC_PUBLIC
void ixEthAccDataPlaneShow(void)
{
    UINT32 numTx0Entries;
    UINT32 numTx1Entries;
    UINT32 numTxDoneEntries;
    UINT32 numRxEntries;
    UINT32 numRxFree0Entries;
    UINT32 numRxFree1Entries;
    UINT32 portId;
#ifdef __ixp46X
    UINT32 numTx2Entries;
    UINT32 numRxFree2Entries;
#endif
#ifndef NDEBUG
    UINT32 priority;
    UINT32 numBuffersInRx=0;
    UINT32 numBuffersInTx=0;
    UINT32 numBuffersInSwQ=0;
    UINT32 totalBuffers=0;
    UINT32 rxFreeCallbackCounter = 0;
    UINT32 txCallbackCounter = 0;
#endif
    UINT32 key;

    /* snapshot of stats */
    IxEthAccTxDataStats tx[IX_ETH_ACC_NUMBER_OF_PORTS];
    IxEthAccRxDataStats rx[IX_ETH_ACC_NUMBER_OF_PORTS];
    IxEthAccDataPlaneStats stats;

    if (!IX_ETH_ACC_IS_SERVICE_INITIALIZED())
    {
	return;
    }

    /* get a reliable snapshot */
    key = ixOsalIrqLock();

    numTx0Entries = 0;
    ixQMgrQNumEntriesGet(IX_ETH_ACC_TX_FRAME_ENET0_Q, &numTx0Entries);
    numTx1Entries = 0;
    ixQMgrQNumEntriesGet(IX_ETH_ACC_TX_FRAME_ENET1_Q, &numTx1Entries);
    numTxDoneEntries = 0;
    ixQMgrQNumEntriesGet( IX_ETH_ACC_TX_FRAME_DONE_ETH_Q, &numTxDoneEntries);
    numRxEntries = 0;
    ixEthAccQMgrRxQEntryGet(&numRxEntries);
    numRxFree0Entries = 0;
    ixQMgrQNumEntriesGet(IX_ETH_ACC_RX_FREE_BUFF_ENET0_Q, &numRxFree0Entries);
    numRxFree1Entries = 0;
    ixQMgrQNumEntriesGet(IX_ETH_ACC_RX_FREE_BUFF_ENET1_Q, &numRxFree1Entries);

#ifdef __ixp46X
    numTx2Entries = 0;
    ixQMgrQNumEntriesGet(IX_ETH_ACC_TX_FRAME_ENET2_Q, &numTx2Entries);
    numRxFree2Entries = 0;
    ixQMgrQNumEntriesGet(IX_ETH_ACC_RX_FREE_BUFF_ENET2_Q, &numRxFree2Entries);
#endif

    for(portId=IX_ETH_PORT_1; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
    {
	memcpy(&tx[portId],
	       &ixEthAccPortData[portId].ixEthAccTxData.stats,
	       sizeof(tx[portId]));
	memcpy(&rx[portId],
	       &ixEthAccPortData[portId].ixEthAccRxData.stats,
	       sizeof(rx[portId]));
    }
    memcpy(&stats, &ixEthAccDataStats, sizeof(stats));

    ixOsalIrqUnlock(key);

#ifdef NDEBUG
    printf("Detailed statistics collection not supported in this load\n");
#endif

    /* print snapshot */
    for(portId=0; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
    {
        /* If not IXP42X A0 stepping, proceed to check for existence of coprocessors */
        if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 !=
	     (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
	    || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
        {
                if ((IX_ETH_PORT_1 == portId) &&
                    (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED))
                {
                   continue ;
                }
                if ((IX_ETH_PORT_2 == portId) &&
                    (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED))
                {
                    continue ;
                }
                if ((IX_ETH_PORT_3 == portId) &&
                    (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_NPEA_ETH) ==
                     IX_FEATURE_CTRL_COMPONENT_DISABLED))
                {
                    continue ;
                }
        }

	printf("PORT %u --------------------------------\n",
	       portId);
#ifndef NDEBUG
	printf("Tx Done Frames                : %u\n",
	       tx[portId].txDoneClientCallback +
	       tx[portId].txDoneSwQDuringDisable +
	       tx[portId].txDoneDuringDisable);
	printf("Tx Frames                     : %u\n",
	       tx[portId].txQOK + tx[portId].txQDelayed);
	printf("Tx H/W Q Added OK             : %u\n",
	       tx[portId].txQOK);
	printf("Tx H/W Q Delayed              : %u\n",
	       tx[portId].txQDelayed);
	printf("Tx From S/W Q Added OK        : %u\n",
	       tx[portId].txFromSwQOK);
	printf("Tx From S/W Q Delayed         : %u\n",
	       tx[portId].txFromSwQDelayed);
	printf("Tx Overflow                   : %u\n",
	       tx[portId].txOverflow);
	printf("Tx Mutual Lock                : %u\n",
	       tx[portId].txLock);
	printf("Tx Late Ntf Enabled           : %u\n",
	       tx[portId].txLateNotificationEnabled);
	printf("Tx Low Thresh CB              : %u\n",
	       tx[portId].txLowThreshCallback);
	printf("Tx Done from H/W Q (Disable)  : %u\n",
	       tx[portId].txDoneDuringDisable);
	printf("Tx Done from S/W Q (Disable)  : %u\n",
	       tx[portId].txDoneSwQDuringDisable);
	for (priority = IX_ETH_ACC_TX_PRIORITY_0;
	     priority <= IX_ETH_ACC_TX_PRIORITY_7;
	     priority++)
	{
	    if (tx[portId].txPriority[priority])
	    {
		printf("Tx Priority %u                 : %u\n",
		       priority,
		       tx[portId].txPriority[priority]);
	    }
	}
#endif
	printf("Tx unexpected errors          : %u (should be 0)\n",
	       tx[portId].txUnexpectedError);

#ifndef NDEBUG
	printf("Rx Frames                     : %u\n",
	       rx[portId].rxFrameClientCallback +
	       rx[portId].rxSwQDuringDisable+
	       rx[portId].rxDuringDisable);
	printf("Rx Free Replenish             : %u\n",
	       rx[portId].rxFreeRepOK + rx[portId].rxFreeRepDelayed);
	printf("Rx Free H/W Q Added OK        : %u\n",
	       rx[portId].rxFreeRepOK);
	printf("Rx Free H/W Q Delayed         : %u\n",
	       rx[portId].rxFreeRepDelayed);
	printf("Rx Free From S/W Q Added OK   : %u\n",
	       rx[portId].rxFreeRepFromSwQOK);
	printf("Rx Free From S/W Q Delayed    : %u\n",
	       rx[portId].rxFreeRepFromSwQDelayed);
	printf("Rx Free Overflow              : %u\n",
	       rx[portId].rxFreeOverflow);
	printf("Rx Free Mutual Lock           : %u\n",
	       rx[portId].rxFreeLock);
	printf("Rx Free Late Ntf Enabled      : %u\n",
	       rx[portId].rxFreeLateNotificationEnabled);
	printf("Rx Free Low CB                : %u\n",
	       rx[portId].rxFreeLowCallback);
	printf("Rx From H/W Q (Disable)       : %u\n",
	       rx[portId].rxDuringDisable);
	printf("Rx From S/W Q (Disable)       : %u\n",
	       rx[portId].rxSwQDuringDisable);
	printf("Rx unlearned Mac Address      : %u\n",
	       rx[portId].rxUnlearnedMacAddress);
        printf("Rx Filtered (Rx => RxFree)    : %u\n",
            rx[portId].rxFiltered);

	for (priority = IX_ETH_ACC_TX_PRIORITY_0;
	     priority <= IX_ETH_ACC_TX_PRIORITY_7;
	     priority++)
	{
	    if (rx[portId].rxPriority[priority])
	    {
		printf("Rx Priority %u                 : %u\n",
		       priority,
		       rx[portId].rxPriority[priority]);
	    }
	}
#endif
	printf("Rx unexpected errors          : %u (should be 0)\n",
	       rx[portId].rxUnexpectedError);

#ifndef NDEBUG
	numBuffersInTx = tx[portId].txQOK +
	    tx[portId].txQDelayed -
	    tx[portId].txDoneClientCallback -
	    tx[portId].txDoneSwQDuringDisable -
	    tx[portId].txDoneDuringDisable;

	printf("# Tx Buffers currently for transmission : %u\n",
	       numBuffersInTx);

	numBuffersInRx = rx[portId].rxFreeRepOK +
	    rx[portId].rxFreeRepDelayed -
	    rx[portId].rxFrameClientCallback -
	    rx[portId].rxSwQDuringDisable -
	    rx[portId].rxDuringDisable;

	printf("# Rx Buffers currently for reception    : %u\n",
	       numBuffersInRx);

	totalBuffers += numBuffersInRx + numBuffersInTx;
#endif
    }

    printf("---------------------------------------\n");

#ifndef NDEBUG
    printf("\n");
    printf("Mbufs :\n");
    printf("Tx Unchained mbufs            : %u\n",
	   stats.unchainedTxMBufs);
    printf("Tx Chained bufs               : %u\n",
	   stats.chainedTxMBufs);
    printf("TxDone Unchained mbufs        : %u\n",
	   stats.unchainedTxDoneMBufs);
    printf("TxDone Chained bufs           : %u\n",
	   stats.chainedTxDoneMBufs);
    printf("RxFree Unchained mbufs        : %u\n",
	   stats.unchainedRxFreeMBufs);
    printf("RxFree Chained bufs           : %u\n",
	   stats.chainedRxFreeMBufs);
    printf("Rx Unchained mbufs            : %u\n",
	   stats.unchainedRxMBufs);
    printf("Rx Chained bufs               : %u\n",
	   stats.chainedRxMBufs);

    printf("\n");
    printf("Software queue usage :\n");
    printf("Buffers added to S/W Q        : %u\n",
	   stats.addToSwQ);
    printf("Buffers removed from S/W Q    : %u\n",
	   stats.removeFromSwQ);

    printf("\n");
    printf("Hardware queues callbacks :\n");

    for(portId=0; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
    {
	rxFreeCallbackCounter += rx[portId].rxFreeLowCallback;
	txCallbackCounter += tx[portId].txLowThreshCallback;
    }
    printf("Tx Done QM Callback invoked   : %u\n",
	   stats.txDoneCallbackCounter);
    printf("Tx QM Callback invoked        : %u\n",
	   txCallbackCounter);
    printf("Rx QM Callback invoked        : %u\n",
	   stats.rxCallbackCounter);
    printf("Rx QM Callback burst read     : %u\n",
	   stats.rxCallbackBurstRead);
    printf("Rx Free QM Callback invoked   : %u\n",
	   rxFreeCallbackCounter);
#endif
    printf("Unexpected errors in CB       : %u (should be 0)\n",
	   stats.unexpectedError);
    printf("\n");

    printf("Hardware queues levels :\n");
    printf("Transmit Port 1 Q             : %u \n",numTx0Entries);
    printf("Transmit Port 2 Q             : %u \n",numTx1Entries);
#ifdef __ixp46X
    printf("Transmit Port 3 Q             : %u \n",numTx2Entries);
#endif
    printf("Transmit Done Q               : %u \n",numTxDoneEntries);
    printf("Receive Q                     : %u \n",numRxEntries);
    printf("Receive Free Port 1 Q         : %u \n",numRxFree0Entries);
    printf("Receive Free Port 2 Q         : %u \n",numRxFree1Entries);
#ifdef __ixp46X
    printf("Receive Free Port 3 Q         : %u \n",numRxFree2Entries);
#endif

#ifndef NDEBUG
    printf("\n");
    printf("# Total Buffers accounted for : %u\n",
	   totalBuffers);

    numBuffersInSwQ = ixEthAccDataStats.addToSwQ -
	ixEthAccDataStats.removeFromSwQ;

    printf("    Buffers in S/W Qs         : %u\n",
	   numBuffersInSwQ);
    printf("    Buffers in H/W Qs or NPEs : %u\n",
	   totalBuffers - numBuffersInSwQ);
#endif

    printf("Rx QoS Discipline             : %s\n",
	   (ixEthAccDataInfo.schDiscipline ==
	    FIFO_PRIORITY ) ? "Enabled" : "Disabled");

    for(portId=0; portId < IX_ETH_ACC_NUMBER_OF_PORTS; portId++)
    {
	printf("Tx QoS Discipline port %u      : %s\n",
	       portId,
	       (ixEthAccPortData[portId].ixEthAccTxData.schDiscipline ==
		FIFO_PRIORITY ) ? "Enabled" : "Disabled");
    }
    printf("\n");
}






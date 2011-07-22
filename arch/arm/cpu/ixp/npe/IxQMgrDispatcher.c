/**
 * @file    IxQMgrDispatcher.c
 *
 * @author Intel Corporation
 * @date    20-Dec-2001
 *    
 * @brief   This file contains the implementation of the Dispatcher sub component
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

/*
 * User defined include files.
 */
#include "IxQMgr.h"
#include "IxQMgrAqmIf_p.h"
#include "IxQMgrQCfg_p.h"
#include "IxQMgrDispatcher_p.h"
#include "IxQMgrLog_p.h"
#include "IxQMgrDefines_p.h"
#include "IxFeatureCtrl.h"
#include "IxOsal.h"



/*
 * #defines and macros used in this file.
 */


/*
 * This constant is used to indicate the number of priority levels supported
 */
#define IX_QMGR_NUM_PRIORITY_LEVELS 3

/* 
 * This constant is used to set the size of the array of status words
 */
#define MAX_Q_STATUS_WORDS      4

/*
 * This macro is used to check if a given priority is valid
 */
#define IX_QMGR_DISPATCHER_PRIORITY_CHECK(priority) \
(((priority) >= IX_QMGR_Q_PRIORITY_0) && ((priority) <= IX_QMGR_Q_PRIORITY_2))

/*
 * This macto is used to check that a given interrupt source is valid
 */
#define IX_QMGR_DISPATCHER_SOURCE_ID_CHECK(srcSel) \
(((srcSel) >= IX_QMGR_Q_SOURCE_ID_E) && ((srcSel) <= IX_QMGR_Q_SOURCE_ID_NOT_F))

/*
 * Number of times a dummy callback is called before logging a trace
 * message
 */
#define LOG_THROTTLE_COUNT 1000000

/* Priority tables limits */
#define IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX (0)
#define IX_QMGR_MID_LOW_QUE_PRIORITY_TABLE_INDEX (16)
#define IX_QMGR_MAX_LOW_QUE_PRIORITY_TABLE_INDEX (31)
#define IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX (32)
#define IX_QMGR_MID_UPP_QUE_PRIORITY_TABLE_INDEX (48)
#define IX_QMGR_MAX_UPP_QUE_PRIORITY_TABLE_INDEX (63)
 
/*
 * This macro is used to check if a given callback type is valid
 */
#define IX_QMGR_DISPATCHER_CALLBACK_TYPE_CHECK(type) \
            (((type) >= IX_QMGR_TYPE_REALTIME_OTHER) && \
            ((type) <= IX_QMGR_TYPE_REALTIME_SPORADIC))

/* 
 * define max index in lower queue to use in loops 
 */
#define IX_QMGR_MAX_LOW_QUE_TABLE_INDEX (31)

/*
 * Typedefs whose scope is limited to this file.
 */

/*
 * Information on a queue needed by the Dispatcher
 */
typedef struct 
{
    IxQMgrCallback callback;       /* Notification callback                  */
    IxQMgrCallbackId callbackId;   /* Notification callback identifier       */
    unsigned dummyCallbackCount;   /* Number of times runs of dummy callback */
    IxQMgrPriority priority;       /* Dispatch priority                      */
    unsigned int statusWordOffset; /* Offset to the status word to check     */
    UINT32 statusMask;             /* Status mask                            */    
    UINT32 statusCheckValue;       /* Status check value                     */
    UINT32 intRegCheckMask;	   /* Interrupt register check mask          */
} IxQMgrQInfo;

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */

/* 
 * Flag to keep record of what dispatcher set in featureCtrl when ixQMgrInit()
 * is called. This is needed because it is possible that a client might
 * change whether the live lock prevention dispatcher is used between
 * calls to ixQMgrInit() and ixQMgrDispatcherLoopGet(). 
 */
PRIVATE IX_STATUS ixQMgrOrigB0Dispatcher = IX_FEATURE_CTRL_COMPONENT_ENABLED;

/* 
 * keep record of Q types - not in IxQMgrQInfo for performance as
 * it is only used with ixQMgrDispatcherLoopRunB0LLP()
 */
PRIVATE IxQMgrType ixQMgrQTypes[IX_QMGR_MAX_NUM_QUEUES];

/*
 * This array contains a list of queue identifiers ordered by priority. The table
 * is split logically between queue identifiers 0-31 and 32-63.
 */
static IxQMgrQId priorityTable[IX_QMGR_MAX_NUM_QUEUES];

/*
 * This flag indicates to the dispatcher that the priority table needs to be rebuilt.
 */
static BOOL rebuildTable = FALSE;

/* Dispatcher statistics */
static IxQMgrDispatcherStats dispatcherStats;

/* Table of queue information */
static IxQMgrQInfo dispatchQInfo[IX_QMGR_MAX_NUM_QUEUES];

/* Masks use to identify the first queues in the priority tables 
*  when comparing with the interrupt register
*/
static unsigned int lowPriorityTableFirstHalfMask;
static unsigned int uppPriorityTableFirstHalfMask;

/*
 * Static function prototypes
 */

/*
 * This function is the default callback for all queues
 */
PRIVATE void
dummyCallback (IxQMgrQId qId,	      
	       IxQMgrCallbackId cbId);

PRIVATE void
ixQMgrDispatcherReBuildPriorityTable (void);

/*
 * Function definitions.
 */
void
ixQMgrDispatcherInit (void)
{
    int i;
    IxFeatureCtrlProductId productId = 0;
    IxFeatureCtrlDeviceId deviceId = 0;
    BOOL stickyIntSilicon = TRUE; 

    /* Set default priorities */
    for (i=0; i< IX_QMGR_MAX_NUM_QUEUES; i++)
    {
	dispatchQInfo[i].callback = dummyCallback;
	dispatchQInfo[i].callbackId = 0;
	dispatchQInfo[i].dummyCallbackCount = 0;
	dispatchQInfo[i].priority = IX_QMGR_Q_PRIORITY_2;
	dispatchQInfo[i].statusWordOffset = 0;
	dispatchQInfo[i].statusCheckValue = 0;
	dispatchQInfo[i].statusMask = 0;  
        /* 
	 * There are two interrupt registers, 32 bits each. One for the lower
	 * queues(0-31) and one for the upper queues(32-63). Therefore need to
	 * mod by 32 i.e the min upper queue identifier.
	 */
	dispatchQInfo[i].intRegCheckMask = (1<<(i%(IX_QMGR_MIN_QUEUPP_QID)));

        /* 
         * Set the Q types - will only be used with livelock 
         */
        ixQMgrQTypes[i] = IX_QMGR_TYPE_REALTIME_OTHER;

	/* Reset queue statistics */
	dispatcherStats.queueStats[i].callbackCnt = 0;
	dispatcherStats.queueStats[i].priorityChangeCnt = 0;
	dispatcherStats.queueStats[i].intNoCallbackCnt = 0;
	dispatcherStats.queueStats[i].intLostCallbackCnt = 0;
        dispatcherStats.queueStats[i].notificationEnabled = FALSE;
        dispatcherStats.queueStats[i].srcSel = 0;

    }

    /* Priority table. Order the table from queue 0 to 63 */
    ixQMgrDispatcherReBuildPriorityTable();

    /* Reset statistics */
    dispatcherStats.loopRunCnt = 0;

    /* Get the device ID for the underlying silicon */
    deviceId = ixFeatureCtrlDeviceRead();
    
    /* Get the product ID for the underlying silicon */
    productId = ixFeatureCtrlProductIdRead();

    /* 
     * Check featureCtrl to see if Livelock prevention is required 
     */
    ixQMgrOrigB0Dispatcher = ixFeatureCtrlSwConfigurationCheck( 
                                 IX_FEATURECTRL_ORIGB0_DISPATCHER);

    /*
     * Check if the silicon supports the sticky interrupt feature.
     * IF (IXP42X AND A0) -> No sticky interrupt feature supported 
     */
    if ((IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X == 
        (IX_FEATURE_CTRL_DEVICE_TYPE_MASK & deviceId)) &&
        (IX_FEATURE_CTRL_SILICON_TYPE_A0 == 
        (IX_FEATURE_CTRL_SILICON_STEPPING_MASK & productId))) 
    {
       stickyIntSilicon = FALSE;
    }

    /*
     * IF user wants livelock prev option AND silicon supports sticky interrupt 
     * feature -> enable the sticky interrupt bit
     */
    if ((IX_FEATURE_CTRL_SWCONFIG_DISABLED == ixQMgrOrigB0Dispatcher) &&
         stickyIntSilicon)  
    {
        ixQMgrStickyInterruptRegEnable();
    }
}

IX_STATUS
ixQMgrDispatcherPrioritySet (IxQMgrQId qId,
			     IxQMgrPriority priority)
{   
    int ixQMgrLockKey;

    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
    
    if (!IX_QMGR_DISPATCHER_PRIORITY_CHECK(priority))
    {
	return IX_QMGR_Q_INVALID_PRIORITY;
    }

    ixQMgrLockKey = ixOsalIrqLock();
    
    /* Change priority */
    dispatchQInfo[qId].priority = priority;
    /* Set flag */
    rebuildTable = TRUE;

    ixOsalIrqUnlock(ixQMgrLockKey);

#ifndef NDEBUG
    /* Update statistics */
    dispatcherStats.queueStats[qId].priorityChangeCnt++;
#endif

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrNotificationCallbackSet (IxQMgrQId qId,
			       IxQMgrCallback callback,
			       IxQMgrCallbackId callbackId)
{
    if (!ixQMgrQIsConfigured(qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }

    if (NULL == callback)
    {
	/* Reset to dummy callback */
	dispatchQInfo[qId].callback = dummyCallback;
	dispatchQInfo[qId].dummyCallbackCount = 0;
	dispatchQInfo[qId].callbackId = 0;
    }
    else 
    {
	dispatchQInfo[qId].callback = callback;
	dispatchQInfo[qId].callbackId = callbackId;
    }

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrNotificationEnable (IxQMgrQId qId, 
			  IxQMgrSourceId srcSel)
{
    IxQMgrQStatus qStatusOnEntry;/* The queue status on entry/exit */
    IxQMgrQStatus qStatusOnExit; /* to this function               */
    int ixQMgrLockKey;

#ifndef NDEBUG
    if (!ixQMgrQIsConfigured (qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }

    if ((qId < IX_QMGR_MIN_QUEUPP_QID) &&
       !IX_QMGR_DISPATCHER_SOURCE_ID_CHECK(srcSel))
    {
	/* QId 0-31 source id invalid */
	return IX_QMGR_INVALID_INT_SOURCE_ID;
    }

    if ((IX_QMGR_Q_SOURCE_ID_NE != srcSel) &&
	(qId >= IX_QMGR_MIN_QUEUPP_QID))
    {
	/*
	 * For queues 32-63 the interrupt source is fixed to the Nearly
	 * Empty status flag and therefore should have a srcSel of NE.
	 */
	return IX_QMGR_INVALID_INT_SOURCE_ID;
    }
#endif

#ifndef NDEBUG
    dispatcherStats.queueStats[qId].notificationEnabled = TRUE;
    dispatcherStats.queueStats[qId].srcSel = srcSel;
#endif

    /* Get the current queue status */
    ixQMgrAqmIfQueStatRead (qId, &qStatusOnEntry);
  
    /* 
     * Enabling interrupts results in Read-Modify-Write
     * so need critical section
     */

    ixQMgrLockKey = ixOsalIrqLock();

    /* Calculate the checkMask and checkValue for this q */
    ixQMgrAqmIfQStatusCheckValsCalc (qId,
				     srcSel,
				     &dispatchQInfo[qId].statusWordOffset,
				     &dispatchQInfo[qId].statusCheckValue,
				     &dispatchQInfo[qId].statusMask);


    /* Set the interrupt source is this queue is in the range 0-31 */
    if (qId < IX_QMGR_MIN_QUEUPP_QID)
    {
	ixQMgrAqmIfIntSrcSelWrite (qId, srcSel);
    }

    /* Enable the interrupt */
    ixQMgrAqmIfQInterruptEnable (qId);

    ixOsalIrqUnlock(ixQMgrLockKey);
    
    /* Get the current queue status */
    ixQMgrAqmIfQueStatRead (qId, &qStatusOnExit);
  
    /* If the status has changed return a warning */
    if (qStatusOnEntry != qStatusOnExit)
    {
	return IX_QMGR_WARNING;
    }
    
    return IX_SUCCESS;
}


IX_STATUS
ixQMgrNotificationDisable (IxQMgrQId qId)
{
    int ixQMgrLockKey;

#ifndef NDEBUG
    /* Validate parameters */
    if (!ixQMgrQIsConfigured (qId))
    {
	return IX_QMGR_Q_NOT_CONFIGURED;
    }
#endif
  
    /* 
     * Enabling interrupts results in Read-Modify-Write
     * so need critical section
     */
#ifndef NDEBUG
    dispatcherStats.queueStats[qId].notificationEnabled = FALSE;
#endif

    ixQMgrLockKey = ixOsalIrqLock();

    ixQMgrAqmIfQInterruptDisable (qId);
    
    ixOsalIrqUnlock(ixQMgrLockKey);

    return IX_SUCCESS;    
}

void 
ixQMgrStickyInterruptRegEnable(void)
{
 /* Use Aqm If function to set Interrupt Register0 Bit-3 */ 
 ixQMgrAqmIfIntSrcSelReg0Bit3Set ();   
}

#if !defined __XSCALE__ || defined __linux

/* Count the number of leading zero bits in a word,
 * and return the same value than the CLZ instruction.
 *
 * word (in)    return value (out)
 * 0x80000000   0
 * 0x40000000   1
 * ,,,          ,,,
 * 0x00000002   30
 * 0x00000001   31
 * 0x00000000   32
 *
 * The C version of this function is used as a replacement 
 * for system not providing the equivalent of the CLZ 
 * assembly language instruction.
 *
 * Note that this version is big-endian
 */
unsigned int
ixQMgrCountLeadingZeros(UINT32 word)
{
  unsigned int leadingZerosCount = 0;

  if (word == 0)
  {
      return 32;
  }
  /* search the first bit set by testing the MSB and shifting the input word */
  while ((word & 0x80000000) == 0)
  {
      word <<= 1;
      leadingZerosCount++;
  }
  return leadingZerosCount;
}
#endif /* not  __XSCALE__ or __linux */

void
ixQMgrDispatcherLoopGet (IxQMgrDispatcherFuncPtr *qDispatcherFuncPtr)
{
  IxFeatureCtrlProductId productId = 0;
  IxFeatureCtrlDeviceId deviceId = 0;
  
  /* Get the device ID for the underlying silicon */
  deviceId = ixFeatureCtrlDeviceRead();

  /* Get the product ID for the underlying silicon */
  productId = ixFeatureCtrlProductIdRead ();

  /* IF (IXP42X AND A0 silicon) -> use ixQMgrDispatcherLoopRunA0 */
  if ((IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X ==
      (IX_FEATURE_CTRL_DEVICE_TYPE_MASK & deviceId)) &&
      (IX_FEATURE_CTRL_SILICON_TYPE_A0 ==  
      (IX_FEATURE_CTRL_SILICON_STEPPING_MASK & productId)))  
  {
    /*For IXP42X A0 silicon */
    *qDispatcherFuncPtr = &ixQMgrDispatcherLoopRunA0 ;
  } 
  else /*For IXP42X B0 or IXP46X silicon*/ 
  { 
    if (IX_FEATURE_CTRL_SWCONFIG_ENABLED == ixQMgrOrigB0Dispatcher)
    {
        /* Default for IXP42X B0 and IXP46X silicon */
        *qDispatcherFuncPtr = &ixQMgrDispatcherLoopRunB0;
    }
    else 
    {
        /* FeatureCtrl indicated that livelock dispatcher be used */
        *qDispatcherFuncPtr = &ixQMgrDispatcherLoopRunB0LLP;
    }
  }
}

void
ixQMgrDispatcherLoopRunA0 (IxQMgrDispatchGroup group)
{
    UINT32 intRegVal;                /* Interrupt reg val */
    UINT32 intRegValAfterWrite;      /* Interrupt reg val after writing back */
    UINT32 intRegCheckMask;          /* Mask for checking interrupt bits */
    UINT32 qStatusWordsB4Write[MAX_Q_STATUS_WORDS];  /* Status b4 interrupt write */
    UINT32 qStatusWordsAfterWrite[MAX_Q_STATUS_WORDS]; /* Status after interrupt write */
    IxQMgrQInfo *currDispatchQInfo;
    BOOL statusChangeFlag;

    int priorityTableIndex;/* Priority table index */
    int qIndex;            /* Current queue being processed */
    int endIndex;          /* Index of last queue to process */

#ifndef NDEBUG
    IX_OSAL_ASSERT((group == IX_QMGR_QUEUPP_GROUP) || 
	      (group == IX_QMGR_QUELOW_GROUP));
#endif

    /* Read Q status registers before interrupt status read/write */
    ixQMgrAqmIfQStatusRegsRead (group, qStatusWordsB4Write);

    /* Read the interrupt register */
    ixQMgrAqmIfQInterruptRegRead (group, &intRegVal);

    /* No bit set : nothing to process (the reaminder of the algorithm is
    * based on the fact that the interrupt register value contains at
    * least one bit set
    */
    if (intRegVal == 0) 
    {
#ifndef NDEBUG
	/* Update statistics */
	dispatcherStats.loopRunCnt++;
#endif

	/* Rebuild the priority table if needed */
	if (rebuildTable)
	{
	    ixQMgrDispatcherReBuildPriorityTable ();
	}

	return;
    }
   
    /* Write it back to clear the interrupt */
    ixQMgrAqmIfQInterruptRegWrite (group, intRegVal);

    /* Read Q status registers after interrupt status read/write */
    ixQMgrAqmIfQStatusRegsRead (group, qStatusWordsAfterWrite);
 
    /* get the first queue Id from the interrupt register value */
    qIndex = (BITS_PER_WORD - 1) - ixQMgrCountLeadingZeros(intRegVal);

    /* check if any change occured during hw register modifications */ 
    if (IX_QMGR_QUELOW_GROUP == group)
    {
	statusChangeFlag = 
	    (qStatusWordsB4Write[0] != qStatusWordsAfterWrite[0]) ||
	    (qStatusWordsB4Write[1] != qStatusWordsAfterWrite[1]) ||
	    (qStatusWordsB4Write[2] != qStatusWordsAfterWrite[2]) ||
	    (qStatusWordsB4Write[3] != qStatusWordsAfterWrite[3]);
    }
    else
    {
	statusChangeFlag = 
	    (qStatusWordsB4Write[0] != qStatusWordsAfterWrite[0]);
	/* Set the queue range based on the queue group to proccess */
	qIndex += IX_QMGR_MIN_QUEUPP_QID;
    }

    if (statusChangeFlag == FALSE)
    {
	/* check if the interrupt register contains 
	 * only 1 bit set (happy day scenario)
	 */
	currDispatchQInfo = &dispatchQInfo[qIndex];
	if (intRegVal == currDispatchQInfo->intRegCheckMask)
	{
	    /* only 1 queue event triggered a notification *
	     * Call the callback function for this queue 
	     */
	    currDispatchQInfo->callback (qIndex,
					 currDispatchQInfo->callbackId);  
#ifndef NDEBUG
	    /* Update statistics */
	    dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif
	}
	else 
	{
	    /* the event is triggered by more than 1 queue, 
	     * the queue search will be starting from the beginning
	     * or the middle of the priority table
	     *
	     * the serach will end when all the bits of the interrupt
	     * register are cleared. There is no need to maintain
	     * a seperate value and test it at each iteration.
	     */
	    if (IX_QMGR_QUELOW_GROUP == group)
	    {
		/* check if any bit related to queues in the first
		 * half of the priority table is set
		 */
		if (intRegVal & lowPriorityTableFirstHalfMask)
		{
		    priorityTableIndex = IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX;
		}
		else
		{
		    priorityTableIndex = IX_QMGR_MID_LOW_QUE_PRIORITY_TABLE_INDEX;
		}
	    }
	    else 
	    {
		/* check if any bit related to queues in the first
		 * half of the priority table is set
		 */
		if (intRegVal & uppPriorityTableFirstHalfMask)
		{
		    priorityTableIndex = IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX;
		}
		else
		{
		    priorityTableIndex = IX_QMGR_MID_UPP_QUE_PRIORITY_TABLE_INDEX;
		}
	    }
	    
	    /* iterate following the priority table until all the bits 
	     * of the interrupt register are cleared.
	     */
	    do
	    {
		qIndex = priorityTable[priorityTableIndex++];
		currDispatchQInfo = &dispatchQInfo[qIndex];
		intRegCheckMask = currDispatchQInfo->intRegCheckMask;
		
		/* If this queue caused this interrupt to be raised */
		if (intRegVal & intRegCheckMask)
		{
		    /* Call the callback function for this queue */
		    currDispatchQInfo->callback (qIndex,
						 currDispatchQInfo->callbackId);
#ifndef NDEBUG
		    /* Update statistics */
		    dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif
		    
		    /* Clear the interrupt register bit */
		    intRegVal &= ~intRegCheckMask;
		}
	    }
	    while(intRegVal);
	}
    }
    else
    {
    /* A change in queue status occured during the hw interrupt
     * register update. To maintain the interrupt consistency, it
     * is necessary to iterate through all queues of the queue group.
     */

    /* Read interrupt status again */
    ixQMgrAqmIfQInterruptRegRead (group, &intRegValAfterWrite);

    if (IX_QMGR_QUELOW_GROUP == group)
    {
	priorityTableIndex = IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX;
	endIndex = IX_QMGR_MAX_LOW_QUE_PRIORITY_TABLE_INDEX;
    }
    else
    {
	priorityTableIndex = IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX;
	endIndex = IX_QMGR_MAX_UPP_QUE_PRIORITY_TABLE_INDEX;
    }

    for ( ; priorityTableIndex<=endIndex; priorityTableIndex++)
    {
	qIndex = priorityTable[priorityTableIndex];
	currDispatchQInfo = &dispatchQInfo[qIndex];
	intRegCheckMask = currDispatchQInfo->intRegCheckMask;

	/* If this queue caused this interrupt to be raised */
	if (intRegVal & intRegCheckMask)
	{  
	    /* Call the callback function for this queue */
	    currDispatchQInfo->callback (qIndex,
					 currDispatchQInfo->callbackId);
#ifndef NDEBUG
	    /* Update statistics */
	    dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif
	    
	} /* if (intRegVal .. */

	/* 
	 * If interrupt bit is set in intRegValAfterWrite don't
	 * proceed as this will be caught in next interrupt
	 */
	else if ((intRegValAfterWrite & intRegCheckMask) == 0)
	{
	    /* Check if an interrupt was lost for this Q */
	    if (ixQMgrAqmIfQStatusCheck(qStatusWordsB4Write,
					qStatusWordsAfterWrite,
					currDispatchQInfo->statusWordOffset,
					currDispatchQInfo->statusCheckValue,
					currDispatchQInfo->statusMask))
	    {
		/* Call the callback function for this queue */
		currDispatchQInfo->callback (qIndex, 
					     dispatchQInfo[qIndex].callbackId);                 
#ifndef NDEBUG
		/* Update statistics */
		dispatcherStats.queueStats[qIndex].callbackCnt++;
		dispatcherStats.queueStats[qIndex].intLostCallbackCnt++;
#endif
	    } /* if ixQMgrAqmIfQStatusCheck(.. */
	} /* else if ((intRegValAfterWrite ... */
    } /* for (priorityTableIndex=0 ... */
    }

    /* Rebuild the priority table if needed */
    if (rebuildTable)
    {
	ixQMgrDispatcherReBuildPriorityTable ();
    }

#ifndef NDEBUG
    /* Update statistics */
    dispatcherStats.loopRunCnt++;
#endif
}



void
ixQMgrDispatcherLoopRunB0 (IxQMgrDispatchGroup group)
{
    UINT32 intRegVal;                /* Interrupt reg val */
    UINT32 intRegCheckMask;          /* Mask for checking interrupt bits */
    IxQMgrQInfo *currDispatchQInfo;


    int priorityTableIndex; /* Priority table index */
    int qIndex;             /* Current queue being processed */

#ifndef NDEBUG
    IX_OSAL_ASSERT((group == IX_QMGR_QUEUPP_GROUP) ||
              (group == IX_QMGR_QUELOW_GROUP));
    IX_OSAL_ASSERT((group == IX_QMGR_QUEUPP_GROUP) || 
	      (group == IX_QMGR_QUELOW_GROUP));
#endif

    /* Read the interrupt register */
    ixQMgrAqmIfQInterruptRegRead (group, &intRegVal);


    /* No queue has interrupt register set */
    if (intRegVal != 0)
    {

            /* Write it back to clear the interrupt */
            ixQMgrAqmIfQInterruptRegWrite (group, intRegVal);

            /* get the first queue Id from the interrupt register value */
            qIndex = (BITS_PER_WORD - 1) - ixQMgrCountLeadingZeros(intRegVal);

            if (IX_QMGR_QUEUPP_GROUP == group)
            {
                /* Set the queue range based on the queue group to proccess */
                qIndex += IX_QMGR_MIN_QUEUPP_QID;
            }

            /* check if the interrupt register contains
             * only 1 bit set
             * For example:
             *                                        intRegVal = 0x0010
             *               currDispatchQInfo->intRegCheckMask = 0x0010
             *    intRegVal == currDispatchQInfo->intRegCheckMask is TRUE.
             */
             currDispatchQInfo = &dispatchQInfo[qIndex];
             if (intRegVal == currDispatchQInfo->intRegCheckMask)
             {
                /* only 1 queue event triggered a notification *
                 * Call the callback function for this queue
                 */
                currDispatchQInfo->callback (qIndex,
                                     currDispatchQInfo->callbackId);
#ifndef NDEBUG
                /* Update statistics */
                dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif
             }
             else
             {
                 /* the event is triggered by more than 1 queue,
                  * the queue search will be starting from the beginning
                  * or the middle of the priority table
                  *
                  * the serach will end when all the bits of the interrupt
                  * register are cleared. There is no need to maintain
                  * a seperate value and test it at each iteration.
                  */
                 if (IX_QMGR_QUELOW_GROUP == group)
                 {
                     /* check if any bit related to queues in the first
                      * half of the priority table is set
                      */
                     if (intRegVal & lowPriorityTableFirstHalfMask)
                     {
                         priorityTableIndex = IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX;
                     }
                     else
                     {
                         priorityTableIndex = IX_QMGR_MID_LOW_QUE_PRIORITY_TABLE_INDEX;
                     }
                 }
                else
                 {
                     /* check if any bit related to queues in the first
                      * half of the priority table is set
                      */
                     if (intRegVal & uppPriorityTableFirstHalfMask)
                     {
                         priorityTableIndex = IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX;
                     }
                     else
                     {
                         priorityTableIndex = IX_QMGR_MID_UPP_QUE_PRIORITY_TABLE_INDEX;
                     }
                 }

                 /* iterate following the priority table until all the bits
                  * of the interrupt register are cleared.
                  */
                 do
                 {
                     qIndex = priorityTable[priorityTableIndex++];
                     currDispatchQInfo = &dispatchQInfo[qIndex];
                     intRegCheckMask = currDispatchQInfo->intRegCheckMask;

                     /* If this queue caused this interrupt to be raised */
                     if (intRegVal & intRegCheckMask)
                     {
                         /* Call the callback function for this queue */
                         currDispatchQInfo->callback (qIndex,
                                              currDispatchQInfo->callbackId);
#ifndef NDEBUG
                         /* Update statistics */
                         dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif

                         /* Clear the interrupt register bit */
                         intRegVal &= ~intRegCheckMask;
                     }
                  }
                  while(intRegVal);
             } /*End of intRegVal == currDispatchQInfo->intRegCheckMask */
     } /* End of intRegVal != 0 */

#ifndef NDEBUG
    /* Update statistics */
    dispatcherStats.loopRunCnt++;
#endif

    /* Rebuild the priority table if needed */
    if (rebuildTable)
    {
        ixQMgrDispatcherReBuildPriorityTable ();
    }
}

void
ixQMgrDispatcherLoopRunB0LLP (IxQMgrDispatchGroup group)
{
    UINT32 intRegVal =0;                /* Interrupt reg val */
    UINT32 intRegCheckMask;          /* Mask for checking interrupt bits */
    IxQMgrQInfo *currDispatchQInfo;

    int priorityTableIndex; /* Priority table index */
    int qIndex;             /* Current queue being processed */

    UINT32 intRegValCopy = 0;
    UINT32 intEnableRegVal = 0;
    UINT8 i = 0;

#ifndef NDEBUG
    IX_OSAL_ASSERT((group == IX_QMGR_QUEUPP_GROUP) ||
              (group == IX_QMGR_QUELOW_GROUP));
#endif

    /* Read the interrupt register */
    ixQMgrAqmIfQInterruptRegRead (group, &intRegVal);

    /* 
     * mask any interrupts that are not enabled 
     */
    ixQMgrAqmIfQInterruptEnableRegRead (group, &intEnableRegVal);
    intRegVal &= intEnableRegVal;

    /* No queue has interrupt register set */
    if (intRegVal != 0)
    {
        if (IX_QMGR_QUELOW_GROUP == group)
        {
            /*
             * As the sticky bit is set, the interrupt register will 
             * not clear if write back at this point because the condition
             * has not been cleared. Take a copy and write back later after
             * the condition has been cleared
             */
            intRegValCopy = intRegVal;
        }
        else
        {
            /* no sticky for upper Q's, so write back now */
            ixQMgrAqmIfQInterruptRegWrite (group, intRegVal);
        }

        /* get the first queue Id from the interrupt register value */
        qIndex = (BITS_PER_WORD - 1) - ixQMgrCountLeadingZeros(intRegVal);

        if (IX_QMGR_QUEUPP_GROUP == group)
        {
            /* Set the queue range based on the queue group to proccess */
            qIndex += IX_QMGR_MIN_QUEUPP_QID;
        }

        /* check if the interrupt register contains
        * only 1 bit set
        * For example:
        *                                        intRegVal = 0x0010
        *               currDispatchQInfo->intRegCheckMask = 0x0010
        *    intRegVal == currDispatchQInfo->intRegCheckMask is TRUE.
        */
        currDispatchQInfo = &dispatchQInfo[qIndex];
        if (intRegVal == currDispatchQInfo->intRegCheckMask)
        {

            /* 
             * check if Q type periodic -  only lower queues can
             * have there type set to periodic 
             */
            if (IX_QMGR_TYPE_REALTIME_PERIODIC == ixQMgrQTypes[qIndex])
            {
                /* 
                 * Disable the notifications on any sporadics 
                 */
                for (i=0; i <= IX_QMGR_MAX_LOW_QUE_TABLE_INDEX; i++)
                {
                    if (IX_QMGR_TYPE_REALTIME_SPORADIC == ixQMgrQTypes[i])
                    {
                        ixQMgrNotificationDisable(i);
#ifndef NDEBUG
                        /* Update statistics */
                        dispatcherStats.queueStats[i].disableCount++;
#endif
                    }
                }
            }

            currDispatchQInfo->callback (qIndex,
                                         currDispatchQInfo->callbackId);
#ifndef NDEBUG
            /* Update statistics */
            dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif
        }
        else
        {
            /* the event is triggered by more than 1 queue,
            * the queue search will be starting from the beginning
            * or the middle of the priority table
            *
            * the serach will end when all the bits of the interrupt
            * register are cleared. There is no need to maintain
            * a seperate value and test it at each iteration.
            */
            if (IX_QMGR_QUELOW_GROUP == group)
            {
                /* check if any bit related to queues in the first
                 * half of the priority table is set
                 */
                if (intRegVal & lowPriorityTableFirstHalfMask)
                {
                    priorityTableIndex =
                                       IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX;
                }
                else
                {
                    priorityTableIndex =
                                       IX_QMGR_MID_LOW_QUE_PRIORITY_TABLE_INDEX;
                }
            }
            else
            {
                /* check if any bit related to queues in the first
                 * half of the priority table is set
                 */
                if (intRegVal & uppPriorityTableFirstHalfMask)
                {
                    priorityTableIndex =
                                       IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX;
                }
                else
                {
                    priorityTableIndex =
                                       IX_QMGR_MID_UPP_QUE_PRIORITY_TABLE_INDEX;
                }
            }

            /* iterate following the priority table until all the bits
             * of the interrupt register are cleared.
             */
            do
            {
                qIndex = priorityTable[priorityTableIndex++];
                currDispatchQInfo = &dispatchQInfo[qIndex];
                intRegCheckMask = currDispatchQInfo->intRegCheckMask;

                /* If this queue caused this interrupt to be raised */
                if (intRegVal & intRegCheckMask)
                {
                    /* 
                     * check if Q type periodic - only lower queues can
                     * have there type set to periodic. There can only be one
                     * periodic queue, so the sporadics are only disabled once.
                     */
                    if (IX_QMGR_TYPE_REALTIME_PERIODIC == ixQMgrQTypes[qIndex])
                    {
                        /* 
                         * Disable the notifications on any sporadics 
                         */
                        for (i=0; i <= IX_QMGR_MAX_LOW_QUE_TABLE_INDEX; i++)
                        {
                            if (IX_QMGR_TYPE_REALTIME_SPORADIC == 
                                    ixQMgrQTypes[i])
                            {
                                ixQMgrNotificationDisable(i);
                                /* 
                                 * remove from intRegVal as we don't want 
                                 * to service any sporadics now
                                 */
                                intRegVal &= ~dispatchQInfo[i].intRegCheckMask;
#ifndef NDEBUG
                                /* Update statistics */
                                dispatcherStats.queueStats[i].disableCount++;
#endif
                            }
                        }
                    }

                    currDispatchQInfo->callback (qIndex,
                                                 currDispatchQInfo->callbackId);
#ifndef NDEBUG
                    /* Update statistics */
                    dispatcherStats.queueStats[qIndex].callbackCnt++;
#endif
                    /* Clear the interrupt register bit */
                    intRegVal &= ~intRegCheckMask;
                }
            }
            while(intRegVal);
        } /*End of intRegVal == currDispatchQInfo->intRegCheckMask */
    } /* End of intRegVal != 0 */

#ifndef NDEBUG
    /* Update statistics */
    dispatcherStats.loopRunCnt++;
#endif

    if ((intRegValCopy != 0) && (IX_QMGR_QUELOW_GROUP == group))
    {
        /* 
         * lower groups (therefore sticky) AND at least one enabled interrupt
         * Write back to clear the interrupt 
         */
        ixQMgrAqmIfQInterruptRegWrite (IX_QMGR_QUELOW_GROUP, intRegValCopy);
    }

    /* Rebuild the priority table if needed */
    if (rebuildTable)
    {
        ixQMgrDispatcherReBuildPriorityTable ();
    }
}

PRIVATE void
ixQMgrDispatcherReBuildPriorityTable (void)
{
    UINT32 qIndex;
    UINT32 priority;
    int lowQuePriorityTableIndex = IX_QMGR_MIN_LOW_QUE_PRIORITY_TABLE_INDEX;
    int uppQuePriorityTableIndex = IX_QMGR_MIN_UPP_QUE_PRIORITY_TABLE_INDEX;

    /* Reset the rebuild flag */
    rebuildTable = FALSE;

    /* initialize the mak used to identify the queues in the first half
     * of the priority table
     */
    lowPriorityTableFirstHalfMask = 0;
    uppPriorityTableFirstHalfMask = 0;
    
    /* For each priority level */
    for(priority=0; priority<IX_QMGR_NUM_PRIORITY_LEVELS; priority++)
    {
	/* Foreach low queue in this priority */
	for(qIndex=0; qIndex<IX_QMGR_MIN_QUEUPP_QID; qIndex++)
	{
	    if (dispatchQInfo[qIndex].priority == priority)
	    { 
		/* build the priority table bitmask which match the
		 * queues of the first half of the priority table 
		 */
		if (lowQuePriorityTableIndex < IX_QMGR_MID_LOW_QUE_PRIORITY_TABLE_INDEX) 
		{
		    lowPriorityTableFirstHalfMask |= dispatchQInfo[qIndex].intRegCheckMask;
		}
		/* build the priority table */
		priorityTable[lowQuePriorityTableIndex++] = qIndex;
	    }
	}
	/* Foreach upp queue */
	for(qIndex=IX_QMGR_MIN_QUEUPP_QID; qIndex<=IX_QMGR_MAX_QID; qIndex++)
	{
	    if (dispatchQInfo[qIndex].priority == priority)
	    {
		/* build the priority table bitmask which match the
		 * queues of the first half of the priority table 
		 */
		if (uppQuePriorityTableIndex < IX_QMGR_MID_UPP_QUE_PRIORITY_TABLE_INDEX) 
		{
		    uppPriorityTableFirstHalfMask |= dispatchQInfo[qIndex].intRegCheckMask;
		}
		/* build the priority table */
		priorityTable[uppQuePriorityTableIndex++] = qIndex;
	    }
	}
    }
}

IxQMgrDispatcherStats*
ixQMgrDispatcherStatsGet (void)
{
    return &dispatcherStats;
}

PRIVATE void
dummyCallback (IxQMgrQId qId,
	       IxQMgrCallbackId cbId)
{
    /* Throttle the trace message */
    if ((dispatchQInfo[qId].dummyCallbackCount % LOG_THROTTLE_COUNT) == 0)
    {
	IX_QMGR_LOG_WARNING2("--> dummyCallback: qId (%d), callbackId (%d)\n",qId,cbId);
    }
    dispatchQInfo[qId].dummyCallbackCount++;

#ifndef NDEBUG
    /* Update statistcs */
    dispatcherStats.queueStats[qId].intNoCallbackCnt++;
#endif
}
void
ixQMgrLLPShow (int resetStats)
{
#ifndef NDEBUG
    UINT8 i = 0;
    IxQMgrQInfo *q;
    UINT32 intEnableRegVal = 0;

    printf ("Livelock statistics are printed on the fly.\n");
    printf ("qId Type     EnableCnt DisableCnt IntEnableState Callbacks\n");
    printf ("=== ======== ========= ========== ============== =========\n");

    for (i=0; i<= IX_QMGR_MAX_LOW_QUE_TABLE_INDEX; i++)
    {
        q = &dispatchQInfo[i];

        if (ixQMgrQTypes[i] != IX_QMGR_TYPE_REALTIME_OTHER)
        {
            printf (" %2d ", i);

            if (ixQMgrQTypes[i] == IX_QMGR_TYPE_REALTIME_SPORADIC)
            {
                printf ("Sporadic");
            }
            else
            {
                printf ("Periodic");
            }

           
            ixQMgrAqmIfQInterruptEnableRegRead (IX_QMGR_QUELOW_GROUP, 
                                                    &intEnableRegVal);
            	

	    intEnableRegVal &= dispatchQInfo[i].intRegCheckMask;
            intEnableRegVal = intEnableRegVal >> i;

            printf (" %10d %10d %10d %10d\n",
                    dispatcherStats.queueStats[i].enableCount,
                    dispatcherStats.queueStats[i].disableCount,
                    intEnableRegVal,
                    dispatcherStats.queueStats[i].callbackCnt);

            if (resetStats)
            {
                dispatcherStats.queueStats[i].enableCount =
                dispatcherStats.queueStats[i].disableCount = 
                dispatcherStats.queueStats[i].callbackCnt = 0;
            }
        }
    }
#else
    IX_QMGR_LOG0("Livelock Prevention statistics are only collected in debug mode\n");
#endif
}

void
ixQMgrPeriodicDone (void)
{
    UINT32 i = 0;
    UINT32 ixQMgrLockKey = 0;

    /* 
     * for the lower queues
     */
    for (i=0; i <= IX_QMGR_MAX_LOW_QUE_TABLE_INDEX; i++)
    {
        /*
         * check for sporadics 
         */
        if (IX_QMGR_TYPE_REALTIME_SPORADIC == ixQMgrQTypes[i])
        {
             /* 
              * enable any sporadics 
              */
             ixQMgrLockKey = ixOsalIrqLock();
             ixQMgrAqmIfQInterruptEnable(i);
             ixOsalIrqUnlock(ixQMgrLockKey);
#ifndef NDEBUG
             /* 
              * Update statistics 
              */
             dispatcherStats.queueStats[i].enableCount++;
             dispatcherStats.queueStats[i].notificationEnabled = TRUE;
#endif
        }
    }
}
IX_STATUS
ixQMgrCallbackTypeSet (IxQMgrQId qId, 
                       IxQMgrType type)
{
    UINT32 ixQMgrLockKey = 0;
    IxQMgrType ixQMgrOldType =0;

#ifndef NDEBUG
    if (!ixQMgrQIsConfigured(qId))
    {
        return IX_QMGR_Q_NOT_CONFIGURED;
    }
    if (qId >= IX_QMGR_MIN_QUEUPP_QID)
    {
        return IX_QMGR_PARAMETER_ERROR;
    }
    if(!IX_QMGR_DISPATCHER_CALLBACK_TYPE_CHECK(type))
    {
        return IX_QMGR_PARAMETER_ERROR;
    }
#endif

    ixQMgrOldType = ixQMgrQTypes[qId];
    ixQMgrQTypes[qId] = type;

    /*
     * check if Q has been changed from type SPORADIC
     */
    if (IX_QMGR_TYPE_REALTIME_SPORADIC == ixQMgrOldType)
    {
       /* 
        * previously Q was a SPORADIC, this means that LLP
        * might have had it disabled. enable it now.
        */
       ixQMgrLockKey = ixOsalIrqLock();
       ixQMgrAqmIfQInterruptEnable(qId);
       ixOsalIrqUnlock(ixQMgrLockKey);

#ifndef NDEBUG
       /* 
        * Update statistics 
        */
       dispatcherStats.queueStats[qId].enableCount++;
#endif
    }

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrCallbackTypeGet (IxQMgrQId qId, 
                       IxQMgrType *type)
{
#ifndef NDEBUG
    if (!ixQMgrQIsConfigured(qId))
    {
        return IX_QMGR_Q_NOT_CONFIGURED;
    }
    if (qId >= IX_QMGR_MIN_QUEUPP_QID)
    {
        return IX_QMGR_PARAMETER_ERROR;
    }
    if(type == NULL)
    {
         return IX_QMGR_PARAMETER_ERROR;
    }
#endif

    *type = ixQMgrQTypes[qId];
    return IX_SUCCESS;
}

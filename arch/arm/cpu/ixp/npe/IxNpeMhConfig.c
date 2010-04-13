/**
 * @file IxNpeMhConfig.c
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the implementation of the private API for the
 * Configuration module.
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
 * Put the system defined include files required.
 */


/*
 * Put the user defined include files required.
 */

#include "IxOsal.h"

#include "IxNpeMhMacros_p.h"

#include "IxNpeMhConfig_p.h"

/*
 * #defines and macros used in this file.
 */
#define IX_NPE_MH_MAX_NUM_OF_RETRIES 1000000 /**< Maximum number of
                                              * retries before
                                              * timeout
					                          */  

/*
 * Typedefs whose scope is limited to this file.
 */

/**
 * @struct IxNpeMhConfigStats
 *
 * @brief This structure is used to maintain statistics for the
 * Configuration module.
 */

typedef struct
{
    UINT32 outFifoReads;        /**< outFifo reads */
    UINT32 inFifoWrites;        /**< inFifo writes */
    UINT32 maxInFifoFullRetries;   /**< max retries if inFIFO full   */
    UINT32 maxOutFifoEmptyRetries; /**< max retries if outFIFO empty */
} IxNpeMhConfigStats;

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

IxNpeMhConfigNpeInfo ixNpeMhConfigNpeInfo[IX_NPEMH_NUM_NPES] =
{
    {
        0,
        IX_NPEMH_NPEA_INT,
	0,
        0,
        0,
        0,
        0,
        NULL,
        FALSE
    },
    {
        0,
        IX_NPEMH_NPEB_INT,
	0,
	0,
        0,
        0,
        0,
        NULL,
        FALSE
    },
    {
        0,
        IX_NPEMH_NPEC_INT,
        0,
        0,
        0,
        0,
        0,
        NULL,
        FALSE
    }
};

PRIVATE IxNpeMhConfigStats ixNpeMhConfigStats[IX_NPEMH_NUM_NPES];

/*
 * Extern function prototypes.
 */

/*
 * Static function prototypes.
 */
PRIVATE
void ixNpeMhConfigIsr (void *parameter);

/*
 * Function definition: ixNpeMhConfigIsr
 */

PRIVATE
void ixNpeMhConfigIsr (void *parameter)
{
    IxNpeMhNpeId npeId = (IxNpeMhNpeId)parameter;
    UINT32 ofint;
    volatile UINT32 *statusReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].statusRegister;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigIsr\n");

    /* get the OFINT (OutFifo interrupt) bit of the status register */
    IX_NPEMH_REGISTER_READ_BITS (statusReg, &ofint, IX_NPEMH_NPE_STAT_OFINT);

    /* if the OFINT status bit is set */
    if (ofint)
    {
        /* if there is an ISR registered for this NPE */
        if (ixNpeMhConfigNpeInfo[npeId].isr != NULL)
        {
            /* invoke the ISR routine */
            ixNpeMhConfigNpeInfo[npeId].isr (npeId);
        }
        else
        {
            /* if we don't service the interrupt the NPE will continue */
            /* to trigger the interrupt indefinitely */
            IX_NPEMH_ERROR_REPORT ("No ISR registered to service "
                                   "interrupt\n");
        }
    }

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigIsr\n");
}

/*
 * Function definition: ixNpeMhConfigInitialize
 */

void ixNpeMhConfigInitialize (
    IxNpeMhNpeInterrupts npeInterrupts)
{
    IxNpeMhNpeId npeId;
    UINT32 virtualAddr[IX_NPEMH_NUM_NPES];

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigInitialize\n");

    /* Request a mapping for the NPE-A config register address space */
    virtualAddr[IX_NPEMH_NPEID_NPEA] =
	(UINT32) IX_OSAL_MEM_MAP (IX_NPEMH_NPEA_BASE,
				     IX_OSAL_IXP400_NPEA_MAP_SIZE);
    IX_OSAL_ASSERT (virtualAddr[IX_NPEMH_NPEID_NPEA]);

    /* Request a mapping for the NPE-B config register address space */
    virtualAddr[IX_NPEMH_NPEID_NPEB] =
	(UINT32) IX_OSAL_MEM_MAP (IX_NPEMH_NPEB_BASE,
				     IX_OSAL_IXP400_NPEB_MAP_SIZE);
    IX_OSAL_ASSERT (virtualAddr[IX_NPEMH_NPEID_NPEB]);
    
    /* Request a mapping for the NPE-C config register address space */
    virtualAddr[IX_NPEMH_NPEID_NPEC] =
	(UINT32) IX_OSAL_MEM_MAP (IX_NPEMH_NPEC_BASE,
				     IX_OSAL_IXP400_NPEC_MAP_SIZE);
    IX_OSAL_ASSERT (virtualAddr[IX_NPEMH_NPEID_NPEC]);

    /* for each NPE ... */
    for (npeId = 0; npeId < IX_NPEMH_NUM_NPES; npeId++)
    {
        /* declare a convenience pointer */
        IxNpeMhConfigNpeInfo *npeInfo = &ixNpeMhConfigNpeInfo[npeId];
	
	/* store the virtual addresses of the NPE registers for later use */
	npeInfo->virtualRegisterBase  = virtualAddr[npeId];
	npeInfo->statusRegister  = virtualAddr[npeId] + IX_NPEMH_NPESTAT_OFFSET;
	npeInfo->controlRegister = virtualAddr[npeId] + IX_NPEMH_NPECTL_OFFSET;
	npeInfo->inFifoRegister  = virtualAddr[npeId] + IX_NPEMH_NPEFIFO_OFFSET;
	npeInfo->outFifoRegister = virtualAddr[npeId] + IX_NPEMH_NPEFIFO_OFFSET;

        /* for test purposes - to verify the register addresses */
        IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "NPE %d status register  = "
                         "0x%08X\n", npeId, npeInfo->statusRegister);
        IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "NPE %d control register = "
                         "0x%08X\n", npeId, npeInfo->controlRegister);
        IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "NPE %d inFifo register  = "
                         "0x%08X\n", npeId, npeInfo->inFifoRegister);
        IX_NPEMH_TRACE2 (IX_NPEMH_DEBUG, "NPE %d outFifo register = "
                         "0x%08X\n", npeId, npeInfo->outFifoRegister);

        /* connect our ISR to the NPE interrupt */
        (void) ixOsalIrqBind (
            npeInfo->interruptId, ixNpeMhConfigIsr, (void *)npeId);

        /* initialise a mutex for this NPE */
        (void) ixOsalMutexInit (&npeInfo->mutex);

        /* if we should service the NPE's "outFIFO not empty" interrupt */
        if (npeInterrupts == IX_NPEMH_NPEINTERRUPTS_YES)
        {
            /* enable the NPE's "outFIFO not empty" interrupt */
            ixNpeMhConfigNpeInterruptEnable (npeId);
        }
        else
        {
            /* disable the NPE's "outFIFO not empty" interrupt */
            ixNpeMhConfigNpeInterruptDisable (npeId);
        }
    }

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigInitialize\n");
}

/*
 * Function definition: ixNpeMhConfigUninit
 */

void ixNpeMhConfigUninit (void)
{
    IxNpeMhNpeId npeId;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigUninit\n");

    /* for each NPE ... */
    for (npeId = 0; npeId < IX_NPEMH_NUM_NPES; npeId++)
    {
        /* declare a convenience pointer */
        IxNpeMhConfigNpeInfo *npeInfo = &ixNpeMhConfigNpeInfo[npeId];
        
        /* disconnect ISR */
        ixOsalIrqUnbind(npeInfo->interruptId);

        /* destroy mutex associated with this NPE */
        ixOsalMutexDestroy(&npeInfo->mutex);
	
	IX_OSAL_MEM_UNMAP (npeInfo->virtualRegisterBase);

	npeInfo->virtualRegisterBase  = 0;
	npeInfo->statusRegister  = 0;
	npeInfo->controlRegister = 0;
	npeInfo->inFifoRegister  = 0;
	npeInfo->outFifoRegister = 0;
    }

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigUninit\n");
}

/*
 * Function definition: ixNpeMhConfigIsrRegister
 */

void ixNpeMhConfigIsrRegister (
    IxNpeMhNpeId npeId,
    IxNpeMhConfigIsr isr)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigIsrRegister\n");

    /* check if there is already an ISR registered for this NPE */
    if (ixNpeMhConfigNpeInfo[npeId].isr != NULL)
    {
        IX_NPEMH_TRACE0 (IX_NPEMH_DEBUG, "Over-writing registered NPE ISR\n");
    }

    /* save the ISR routine with the NPE info */
    ixNpeMhConfigNpeInfo[npeId].isr = isr;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigIsrRegister\n");
}

/*
 * Function definition: ixNpeMhConfigNpeInterruptEnable
 */

BOOL ixNpeMhConfigNpeInterruptEnable (
    IxNpeMhNpeId npeId)
{
    UINT32 ofe;
    volatile UINT32 *controlReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].controlRegister;

    /* get the OFE (OutFifoEnable) bit of the control register */
    IX_NPEMH_REGISTER_READ_BITS (controlReg, &ofe, IX_NPEMH_NPE_CTL_OFE);

    /* if the interrupt is disabled then we must enable it */
    if (!ofe)
    {
        /* set the OFE (OutFifoEnable) bit of the control register */
        /* we must set the OFEWE (OutFifoEnableWriteEnable) at the same */
        /* time for the write to have effect */
        IX_NPEMH_REGISTER_WRITE_BITS (controlReg,
                                      (IX_NPEMH_NPE_CTL_OFE |
                                       IX_NPEMH_NPE_CTL_OFEWE),
                                      (IX_NPEMH_NPE_CTL_OFE |
                                       IX_NPEMH_NPE_CTL_OFEWE));
    }

    /* return the previous state of the interrupt */
    return (ofe != 0);
}

/*
 * Function definition: ixNpeMhConfigNpeInterruptDisable
 */

BOOL ixNpeMhConfigNpeInterruptDisable (
    IxNpeMhNpeId npeId)
{
    UINT32 ofe;
    volatile UINT32 *controlReg =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].controlRegister;

    /* get the OFE (OutFifoEnable) bit of the control register */
    IX_NPEMH_REGISTER_READ_BITS (controlReg, &ofe, IX_NPEMH_NPE_CTL_OFE);

    /* if the interrupt is enabled then we must disable it */
    if (ofe)
    {
        /* unset the OFE (OutFifoEnable) bit of the control register */
        /* we must set the OFEWE (OutFifoEnableWriteEnable) at the same */
        /* time for the write to have effect */
        IX_NPEMH_REGISTER_WRITE_BITS (controlReg,
                                      (0                    |
                                       IX_NPEMH_NPE_CTL_OFEWE),
                                      (IX_NPEMH_NPE_CTL_OFE |
                                       IX_NPEMH_NPE_CTL_OFEWE));
    }

    /* return the previous state of the interrupt */
    return (ofe != 0);
}

/*
 * Function definition: ixNpeMhConfigMessageIdGet
 */

IxNpeMhMessageId ixNpeMhConfigMessageIdGet (
    IxNpeMhMessage message)
{
    /* return the most-significant byte of the first word of the */
    /* message */
    return ((IxNpeMhMessageId) ((message.data[0] >> 24) & 0xFF));
}

/*
 * Function definition: ixNpeMhConfigNpeIdIsValid
 */

BOOL ixNpeMhConfigNpeIdIsValid (
    IxNpeMhNpeId npeId)
{
    /* check that the npeId parameter is within the range of valid IDs */
    return (npeId >= 0 && npeId < IX_NPEMH_NUM_NPES);
}

/*
 * Function definition: ixNpeMhConfigLockGet
 */

void ixNpeMhConfigLockGet (
    IxNpeMhNpeId npeId)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigLockGet\n");

    /* lock the mutex for this NPE */
    (void) ixOsalMutexLock (&ixNpeMhConfigNpeInfo[npeId].mutex, 
			    IX_OSAL_WAIT_FOREVER);

    /* disable the NPE's "outFIFO not empty" interrupt */
    ixNpeMhConfigNpeInfo[npeId].oldInterruptState =
        ixNpeMhConfigNpeInterruptDisable (npeId);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigLockGet\n");
}

/*
 * Function definition: ixNpeMhConfigLockRelease
 */

void ixNpeMhConfigLockRelease (
    IxNpeMhNpeId npeId)
{
    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhConfigLockRelease\n");

    /* if the interrupt was previously enabled */
    if (ixNpeMhConfigNpeInfo[npeId].oldInterruptState)
    {
        /* enable the NPE's "outFIFO not empty" interrupt */
        ixNpeMhConfigNpeInfo[npeId].oldInterruptState =
            ixNpeMhConfigNpeInterruptEnable (npeId);
    }

    /* unlock the mutex for this NPE */
    (void) ixOsalMutexUnlock (&ixNpeMhConfigNpeInfo[npeId].mutex);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhConfigLockRelease\n");
}

/*
 * Function definition: ixNpeMhConfigInFifoWrite
 */

IX_STATUS ixNpeMhConfigInFifoWrite (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message)
{
    volatile UINT32 *npeInFifo =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].inFifoRegister;
    UINT32 retriesCount = 0;

    /* write the first word of the message to the NPE's inFIFO */
    IX_NPEMH_REGISTER_WRITE (npeInFifo, message.data[0]);

    /* need to wait for room to write second word - see SCR #493,
       poll for maximum number of retries, if exceed maximum
       retries, exit from while loop */
    while ((IX_NPE_MH_MAX_NUM_OF_RETRIES > retriesCount)
        && ixNpeMhConfigInFifoIsFull (npeId))
    {
        retriesCount++;
    }

    /* Return TIMEOUT status to caller, indicate that NPE Hang / Halt */
    if (IX_NPE_MH_MAX_NUM_OF_RETRIES == retriesCount)
    {
        return IX_NPEMH_CRITICAL_NPE_ERR;   
    }    
    
    /* write the second word of the message to the NPE's inFIFO */
    IX_NPEMH_REGISTER_WRITE (npeInFifo, message.data[1]);

    /* record in the stats the maximum number of retries needed */
    if (ixNpeMhConfigStats[npeId].maxInFifoFullRetries < retriesCount)
    {
	ixNpeMhConfigStats[npeId].maxInFifoFullRetries = retriesCount;
    }

    /* update statistical info */
    ixNpeMhConfigStats[npeId].inFifoWrites++;
    
    return IX_SUCCESS;
}

/*
 * Function definition: ixNpeMhConfigOutFifoRead
 */

IX_STATUS ixNpeMhConfigOutFifoRead (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage *message)
{
    volatile UINT32 *npeOutFifo =
        (UINT32 *)ixNpeMhConfigNpeInfo[npeId].outFifoRegister;
    UINT32 retriesCount = 0;

    /* read the first word of the message from the NPE's outFIFO */
    IX_NPEMH_REGISTER_READ (npeOutFifo, &message->data[0]);

    /* need to wait for NPE to write second word - see SCR #493 
       poll for maximum number of retries, if exceed maximum
       retries, exit from while loop */
    while ((IX_NPE_MH_MAX_NUM_OF_RETRIES > retriesCount)
        && ixNpeMhConfigOutFifoIsEmpty (npeId))
    {
        retriesCount++;
    }

    /* Return TIMEOUT status to caller, indicate that NPE Hang / Halt */
    if (IX_NPE_MH_MAX_NUM_OF_RETRIES == retriesCount)
    {
        return IX_NPEMH_CRITICAL_NPE_ERR;   
    } 
    
    /* read the second word of the message from the NPE's outFIFO */
    IX_NPEMH_REGISTER_READ (npeOutFifo, &message->data[1]);

    /* record in the stats the maximum number of retries needed */
    if (ixNpeMhConfigStats[npeId].maxOutFifoEmptyRetries < retriesCount)
    {
	ixNpeMhConfigStats[npeId].maxOutFifoEmptyRetries = retriesCount;
    }

    /* update statistical info */
    ixNpeMhConfigStats[npeId].outFifoReads++;
    
    return IX_SUCCESS;
}

/*
 * Function definition: ixNpeMhConfigShow
 */

void ixNpeMhConfigShow (
    IxNpeMhNpeId npeId)
{
    /* show the message fifo read counter */
    IX_NPEMH_SHOW ("Message FIFO reads",
                   ixNpeMhConfigStats[npeId].outFifoReads);

    /* show the message fifo write counter */
    IX_NPEMH_SHOW ("Message FIFO writes",
                   ixNpeMhConfigStats[npeId].inFifoWrites);

    /* show the max retries performed when inFIFO full */
    IX_NPEMH_SHOW ("Max inFIFO Full retries",
		   ixNpeMhConfigStats[npeId].maxInFifoFullRetries);

    /* show the max retries performed when outFIFO empty */
    IX_NPEMH_SHOW ("Max outFIFO Empty retries",
		   ixNpeMhConfigStats[npeId].maxOutFifoEmptyRetries);

    /* show the current status of the inFifo */
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
	       "InFifo is %s and %s\n",
	       (ixNpeMhConfigInFifoIsEmpty (npeId) ? 
		(int) "EMPTY" : (int) "NOT EMPTY"),
	       (ixNpeMhConfigInFifoIsFull (npeId) ? 
		(int) "FULL" : (int) "NOT FULL"),
	       0, 0, 0, 0);

    /* show the current status of the outFifo */
    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
	       "OutFifo is %s and %s\n",
	       (ixNpeMhConfigOutFifoIsEmpty (npeId) ? 
		(int) "EMPTY" : (int) "NOT EMPTY"),
	       (ixNpeMhConfigOutFifoIsFull (npeId) ? 
		(int) "FULL" : (int) "NOT FULL"),
	       0, 0, 0, 0);
}

/*
 * Function definition: ixNpeMhConfigShowReset
 */

void ixNpeMhConfigShowReset (
    IxNpeMhNpeId npeId)
{
    /* reset the message fifo read counter */
    ixNpeMhConfigStats[npeId].outFifoReads = 0;

    /* reset the message fifo write counter */
    ixNpeMhConfigStats[npeId].inFifoWrites = 0;

    /* reset the max inFIFO Full retries counter */
    ixNpeMhConfigStats[npeId].maxInFifoFullRetries = 0;

    /* reset the max outFIFO empty retries counter */
    ixNpeMhConfigStats[npeId].maxOutFifoEmptyRetries = 0;
}



/**
 * @file IxNpeMhSend.c
 *
 * @author Intel Corporation
 * @date 18 Jan 2002
 *
 * @brief This file contains the implementation of the private API for the
 * Send module.
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

#include "IxNpeMhMacros_p.h"

#include "IxNpeMhConfig_p.h"
#include "IxNpeMhSend_p.h"
#include "IxNpeMhSolicitedCbMgr_p.h"

/*
 * #defines and macros used in this file.
 */

/**
 * @def IX_NPEMH_INFIFO_RETRY_DELAY_US
 *
 * @brief Amount of time (uSecs) to delay between retries
 * while inFIFO is Full when attempting to send a message
 */
#define IX_NPEMH_INFIFO_RETRY_DELAY_US (1)


/*
 * Typedefs whose scope is limited to this file.
 */

/**
 * @struct IxNpeMhSendStats
 *
 * @brief This structure is used to maintain statistics for the Send
 * module.
 */

typedef struct
{
    UINT32 sends;             /**< send invocations */
    UINT32 sendWithResponses; /**< send with response invocations */
    UINT32 queueFulls;        /**< fifo queue full occurrences */
    UINT32 queueFullRetries;  /**< fifo queue full retry occurrences */
    UINT32 maxQueueFullRetries; /**< max fifo queue full retries */
    UINT32 callbackFulls;     /**< callback list full occurrences */
} IxNpeMhSendStats;

/*
 * Variable declarations global to this file only.  Externs are followed by
 * static variables.
 */

PRIVATE IxNpeMhSendStats ixNpeMhSendStats[IX_NPEMH_NUM_NPES];

/*
 * Extern function prototypes.
 */

/*
 * Static function prototypes.
 */
PRIVATE
BOOL ixNpeMhSendInFifoIsFull(
    IxNpeMhNpeId npeId,
    UINT32 maxSendRetries);

/*
 * Function definition: ixNpeMhSendInFifoIsFull
 */

PRIVATE
BOOL ixNpeMhSendInFifoIsFull(
    IxNpeMhNpeId npeId,
    UINT32 maxSendRetries)
{
    BOOL isFull = FALSE;
    UINT32 numRetries = 0;

    /* check the NPE's inFIFO */
    isFull = ixNpeMhConfigInFifoIsFull (npeId);

    /* we retry a few times, just to give the NPE a chance to read from */
    /* the FIFO if the FIFO is currently full */
    while (isFull && (numRetries++ < maxSendRetries))
    {
	if (numRetries >= IX_NPEMH_SEND_RETRIES_DEFAULT)
	{
	    /* Delay here for as short a time as possible (1 us). */
	    /* Adding a delay here should ensure we are not hogging */
	    /* the AHB bus while we are retrying                    */
	    ixOsalBusySleep (IX_NPEMH_INFIFO_RETRY_DELAY_US);
	}

        /* re-check the NPE's inFIFO */
        isFull = ixNpeMhConfigInFifoIsFull (npeId);

        /* update statistical info */
        ixNpeMhSendStats[npeId].queueFullRetries++;
    }

    /* record the highest number of retries that occurred */
    if (ixNpeMhSendStats[npeId].maxQueueFullRetries < numRetries)
    {
	ixNpeMhSendStats[npeId].maxQueueFullRetries = numRetries;
    }

    if (isFull)
    {
        /* update statistical info */
        ixNpeMhSendStats[npeId].queueFulls++;
    }

    return isFull;
}

/*
 * Function definition: ixNpeMhSendMessageSend
 */

IX_STATUS ixNpeMhSendMessageSend (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message,
    UINT32 maxSendRetries)
{
    IX_STATUS status;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhSendMessageSend\n");

    /* update statistical info */
    ixNpeMhSendStats[npeId].sends++;

    /* check if the NPE's inFIFO is full - if so return an error */
    if (ixNpeMhSendInFifoIsFull (npeId, maxSendRetries))
    {
        IX_NPEMH_TRACE0 (IX_NPEMH_WARNING, "NPE's inFIFO is full\n");
        return IX_FAIL;
    }

    /* write the message to the NPE's inFIFO */
    status = ixNpeMhConfigInFifoWrite (npeId, message);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhSendMessageSend\n");

    return status;
}

/*
 * Function definition: ixNpeMhSendMessageWithResponseSend
 */

IX_STATUS ixNpeMhSendMessageWithResponseSend (
    IxNpeMhNpeId npeId,
    IxNpeMhMessage message,
    IxNpeMhMessageId solicitedMessageId,
    IxNpeMhCallback solicitedCallback,
    UINT32 maxSendRetries)
{
    IX_STATUS status = IX_SUCCESS;

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Entering "
                     "ixNpeMhSendMessageWithResponseSend\n");

    /* update statistical info */
    ixNpeMhSendStats[npeId].sendWithResponses++;

    /* sr: this sleep will call the receive routine (no interrupts used!!!) */
    ixOsalSleep (IX_NPEMH_INFIFO_RETRY_DELAY_US);

    /* check if the NPE's inFIFO is full - if so return an error */
    if (ixNpeMhSendInFifoIsFull (npeId, maxSendRetries))
    {
        IX_NPEMH_TRACE0 (IX_NPEMH_WARNING, "NPE's inFIFO is full\n");
        return IX_FAIL;
    }

    /* save the solicited callback */
    status = ixNpeMhSolicitedCbMgrCallbackSave (
        npeId, solicitedMessageId, solicitedCallback);
    if (status != IX_SUCCESS)
    {
        IX_NPEMH_ERROR_REPORT ("Failed to save solicited callback\n");

        /* update statistical info */
        ixNpeMhSendStats[npeId].callbackFulls++;

        return status;
    }

    /* write the message to the NPE's inFIFO */
    status = ixNpeMhConfigInFifoWrite (npeId, message);

    IX_NPEMH_TRACE0 (IX_NPEMH_FN_ENTRY_EXIT, "Exiting "
                     "ixNpeMhSendMessageWithResponseSend\n");

    return status;
}

/*
 * Function definition: ixNpeMhSendShow
 */

void ixNpeMhSendShow (
    IxNpeMhNpeId npeId)
{
    /* show the message send invocation counter */
    IX_NPEMH_SHOW ("Send invocations",
                   ixNpeMhSendStats[npeId].sends);

    /* show the message send with response invocation counter */
    IX_NPEMH_SHOW ("Send with response invocations",
                   ixNpeMhSendStats[npeId].sendWithResponses);

    /* show the fifo queue full occurrence counter */
    IX_NPEMH_SHOW ("Fifo queue full occurrences",
                   ixNpeMhSendStats[npeId].queueFulls);

    /* show the fifo queue full retry occurrence counter */
    IX_NPEMH_SHOW ("Fifo queue full retry occurrences",
                   ixNpeMhSendStats[npeId].queueFullRetries);

    /* show the fifo queue full maximum retries counter */
    IX_NPEMH_SHOW ("Maximum fifo queue full retries",
                   ixNpeMhSendStats[npeId].maxQueueFullRetries);

    /* show the callback list full occurrence counter */
    IX_NPEMH_SHOW ("Solicited callback list full occurrences",
                   ixNpeMhSendStats[npeId].callbackFulls);
}

/*
 * Function definition: ixNpeMhSendShowReset
 */

void ixNpeMhSendShowReset (
    IxNpeMhNpeId npeId)
{
    /* reset the message send invocation counter */
    ixNpeMhSendStats[npeId].sends = 0;

    /* reset the message send with response invocation counter */
    ixNpeMhSendStats[npeId].sendWithResponses = 0;

    /* reset the fifo queue full occurrence counter */
    ixNpeMhSendStats[npeId].queueFulls = 0;

    /* reset the fifo queue full retry occurrence counter */
    ixNpeMhSendStats[npeId].queueFullRetries = 0;

    /* reset the max fifo queue full retries counter */
    ixNpeMhSendStats[npeId].maxQueueFullRetries = 0;

    /* reset the callback list full occurrence counter */
    ixNpeMhSendStats[npeId].callbackFulls = 0;
}

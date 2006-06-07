/**
 * @file IxOsalOsServices.c (linux)
 *
 * @brief Implementation for Irq, Mem, sleep.
 *
 *
 * @par
 * IXP400 SW Release version 1.5
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

#include <config.h>
#include <common.h>
#include "IxOsal.h"
#include <IxEthAcc.h>
#include <IxEthDB.h>
#include <IxNpeDl.h>
#include <IxQMgr.h>
#include <IxNpeMh.h>

static char *traceHeaders[] = {
    "",
    "[fatal] ",
    "[error] ",
    "[warning] ",
    "[message] ",
    "[debug1] ",
    "[debug2] ",
    "[debug3] ",
    "[all]"
};

/* by default trace all but debug message */
PRIVATE int ixOsalCurrLogLevel = IX_OSAL_LOG_LVL_MESSAGE;

/**************************************
 * Irq services
 *************************************/

PUBLIC IX_STATUS
ixOsalIrqBind (UINT32 vector, IxOsalVoidFnVoidPtr routine, void *parameter)
{
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalIrqUnbind (UINT32 vector)
{
    return IX_FAIL;
}

PUBLIC UINT32
ixOsalIrqLock ()
{
    return 0;
}

/* Enable interrupts and task scheduling,
 * input parameter: irqEnable status returned
 * by ixOsalIrqLock().
 */
PUBLIC void
ixOsalIrqUnlock (UINT32 lockKey)
{
}

PUBLIC UINT32
ixOsalIrqLevelSet (UINT32 level)
{
    return IX_FAIL;
}

PUBLIC void
ixOsalIrqEnable (UINT32 irqLevel)
{
}

PUBLIC void
ixOsalIrqDisable (UINT32 irqLevel)
{
}

/*********************
 * Log function
 *********************/

INT32
ixOsalLog (IxOsalLogLevel level,
    IxOsalLogDevice device,
    char *format, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
{
    /*
     * Return -1 for custom display devices
     */
    if ((device != IX_OSAL_LOG_DEV_STDOUT)
        && (device != IX_OSAL_LOG_DEV_STDERR))
    {
        debug("ixOsalLog: only IX_OSAL_LOG_DEV_STDOUT and IX_OSAL_LOG_DEV_STDERR are supported \n");
        return (IX_OSAL_LOG_ERROR);
    }

    if (level <= ixOsalCurrLogLevel && level != IX_OSAL_LOG_LVL_NONE)
    {
#if 0 /* sr: U-Boots printf or debug doesn't return a length */
        int headerByteCount = (level == IX_OSAL_LOG_LVL_USER) ? 0 : diag_printf(traceHeaders[level - 1]);

        return headerByteCount + diag_printf (format, arg1, arg2, arg3, arg4, arg5, arg6);
#else
        int headerByteCount = (level == IX_OSAL_LOG_LVL_USER) ? 0 : strlen(traceHeaders[level - 1]);

        return headerByteCount + strlen(format);
#endif
    }
    else
    {
        /*
         * Return error
         */
        return (IX_OSAL_LOG_ERROR);
    }
}

PUBLIC UINT32
ixOsalLogLevelSet (UINT32 level)
{
    UINT32 oldLevel;

    /*
     * Check value first
     */
    if ((level < IX_OSAL_LOG_LVL_NONE) || (level > IX_OSAL_LOG_LVL_ALL))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalLogLevelSet: Log Level is between %d and%d \n",
            IX_OSAL_LOG_LVL_NONE, IX_OSAL_LOG_LVL_ALL, 0, 0, 0, 0);
        return IX_OSAL_LOG_LVL_NONE;
    }
    oldLevel = ixOsalCurrLogLevel;

    ixOsalCurrLogLevel = level;

    return oldLevel;
}

/**************************************
 * Task services
 *************************************/

PUBLIC void
ixOsalBusySleep (UINT32 microseconds)
{
	udelay(microseconds);
}

PUBLIC void
ixOsalSleep (UINT32 milliseconds)
{
    if (milliseconds != 0) {
#if 1
	/*
	 * sr: We poll while we wait because interrupts are off in U-Boot
	 * and CSR expects messages, etc to be dispatched while sleeping.
	 */
	int i;
	IxQMgrDispatcherFuncPtr qDispatcherFunc;

	ixQMgrDispatcherLoopGet(&qDispatcherFunc);

	while (milliseconds--) {
		for (i = 1; i <= 2; i++)
			ixNpeMhMessagesReceive(i);
		(*qDispatcherFunc)(IX_QMGR_QUELOW_GROUP);

		udelay(1000);
	}
#endif
    }
}

/**************************************
 * Memory functions
 *************************************/

void *
ixOsalMemAlloc (UINT32 size)
{
    return (void *)0;
}

void
ixOsalMemFree (void *ptr)
{
}

/*
 * Copy count bytes from src to dest ,
 * returns pointer to the dest mem zone.
 */
void *
ixOsalMemCopy (void *dest, void *src, UINT32 count)
{
    IX_OSAL_ASSERT (dest != NULL);
    IX_OSAL_ASSERT (src != NULL);
    return (memcpy (dest, src, count));
}

/*
 * Fills a memory zone with a given constant byte,
 * returns pointer to the memory zone.
 */
void *
ixOsalMemSet (void *ptr, UINT8 filler, UINT32 count)
{
    IX_OSAL_ASSERT (ptr != NULL);
    return (memset (ptr, filler, count));
}

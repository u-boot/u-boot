/**
 * @file    IxQMgrDispatcher_p.h
 *
 * @author Intel Corporation
 * @date    07-Feb-2002
 *
 * @brief   This file contains the internal functions for dispatcher
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

#ifndef IXQMGRDISPATCHER_P_H
#define IXQMGRDISPATCHER_P_H

/*
 * User defined include files
 */
#include "IxQMgr.h"

/*
 * This structure defines the statistic data for a queue
 */
typedef struct
{
    unsigned callbackCnt;       /* Call count of callback                    */
    unsigned priorityChangeCnt; /* Priority change count                     */
    unsigned intNoCallbackCnt;  /* Interrupt fired but no callback set count */
    unsigned intLostCallbackCnt;  /* Interrupt lost and detected ;  SCR541   */
    BOOL notificationEnabled;    /* Interrupt enabled for this queue         */
    IxQMgrSourceId srcSel;       /* interrupt source                         */
    unsigned enableCount;        /* num times notif enabled by LLP           */
    unsigned disableCount;       /* num of times notif disabled by LLP       */
} IxQMgrDispatcherQStats;

/*
 * This structure defines statistic data for the disatcher
 */
typedef struct
 {
    unsigned loopRunCnt;       /* ixQMgrDispatcherLoopRun count */

    IxQMgrDispatcherQStats queueStats[IX_QMGR_MAX_NUM_QUEUES];

} IxQMgrDispatcherStats;

/*
 * Initialise the dispatcher component
 */
void
ixQMgrDispatcherInit (void);

/*
 * Get the dispatcher statistics
 */
IxQMgrDispatcherStats*
ixQMgrDispatcherStatsGet (void);

/**
 * Retrieve the number of leading zero bits starting from the MSB 
 * This function is implemented as an (extremely fast) asm routine 
 * for XSCALE processor (see clz instruction) and as a (slower) C 
 * function for other systems.
 */
unsigned int
ixQMgrCountLeadingZeros(unsigned int value);

#endif/*IXQMGRDISPATCHER_P_H*/



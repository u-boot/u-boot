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
 * SPDX-License-Identifier:	BSD-3-Clause
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



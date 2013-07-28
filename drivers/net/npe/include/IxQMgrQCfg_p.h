/**
 * @file    IxQMgrQCfg_p.h
 *
 * @author Intel Corporation
 * @date    07-Feb-2002
 *
 * @brief   This file contains the internal functions for config
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

#ifndef IXQMGRQCFG_P_H
#define IXQMGRQCFG_P_H

/*
 * User defined header files
 */
#include "IxQMgr.h"

/*
 * Typedefs
 */
typedef struct
{
    unsigned wmSetCnt;    

    struct
    {
	char *qName;
	BOOL isConfigured;	
	unsigned int qSizeInWords;
	unsigned int qEntrySizeInWords;
	unsigned int ne;
	unsigned int nf;
	unsigned int numEntries;
	UINT32 baseAddress;
	UINT32 readPtr;
	UINT32 writePtr;
    } qStats[IX_QMGR_MAX_NUM_QUEUES];

} IxQMgrQCfgStats;

/*
 * Initialize the QCfg subcomponent
 */ 
void
ixQMgrQCfgInit (void);

/*
 * Uninitialize the QCfg subcomponent
 */ 
void
ixQMgrQCfgUninit (void);

/*
 * Get the Q size in words
 */ 
IxQMgrQSizeInWords
ixQMgrQSizeInWordsGet (IxQMgrQId qId);

/*
 * Get the Q entry size in words
 */ 
IxQMgrQEntrySizeInWords
ixQMgrQEntrySizeInWordsGet (IxQMgrQId qId);

/*
 * Get the generic cfg stats
 */
IxQMgrQCfgStats*
ixQMgrQCfgStatsGet (void);

/*
 * Get queue specific stats
 */
IxQMgrQCfgStats*
ixQMgrQCfgQStatsGet (IxQMgrQId qId);

/*
 * Check is the queue configured
 */
BOOL
ixQMgrQIsConfigured(IxQMgrQId qId);
 
#endif /* IX_QMGRQCFG_P_H */

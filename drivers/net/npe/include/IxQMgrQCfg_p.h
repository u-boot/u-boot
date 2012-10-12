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

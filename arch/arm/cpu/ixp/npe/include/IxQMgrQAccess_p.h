/**
 * @file    IxQMgrQAccess_p.h
 *
 * @author Intel Corporation
 * @date    30-Oct-2001
 *
 * @brief   QAccess private header file
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

#ifndef IXQMGRQACCESS_P_H
#define IXQMGRQACCESS_P_H

/*
 * User defined header files
 */
#include "IxQMgr.h"

/* 
 * Global variables declarations.
 */
extern volatile UINT32 * ixQMgrAqmIfQueAccRegAddr[]; 

/* 
 * Initialise the Queue Access component
 */
void
ixQMgrQAccessInit (void);

/*
 * read the remainder of a multi-word queue entry 
 * (the first word is already read)
 */
IX_STATUS
ixQMgrQReadMWordsMinus1 (IxQMgrQId qId,
                         UINT32 *entry);

/*
 * Fast access : pop a q entry from a single word queue
 */
extern __inline__ UINT32 ixQMgrQAccessPop(IxQMgrQId qId);

extern __inline__ UINT32 ixQMgrQAccessPop(IxQMgrQId qId)
{
  return *(ixQMgrAqmIfQueAccRegAddr[qId]);
}

/*
 * Fast access : push a q entry in a single word queue
 */
extern __inline__ void ixQMgrQAccessPush(IxQMgrQId qId, UINT32 entry);

extern __inline__ void ixQMgrQAccessPush(IxQMgrQId qId, UINT32 entry)
{
  *(ixQMgrAqmIfQueAccRegAddr[qId]) = entry;
}

#endif/*IXQMGRQACCESS_P_H*/

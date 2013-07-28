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
 * SPDX-License-Identifier:	BSD-3-Clause
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

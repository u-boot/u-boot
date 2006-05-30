/**
 * @file IxOsalIxpOem.h
 *
 * @brief this file contains platform-specific defines.
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

#ifndef IxOsalOem_H
#define IxOsalOem_H

#include "IxOsalTypes.h"

/* OS-specific header for Platform package */
#include "IxOsalOsIxp400.h"

/*
 * Platform Name
 */
#define IX_OSAL_PLATFORM_NAME ixp400

/*
 * Cache line size
 */
#define IX_OSAL_CACHE_LINE_SIZE (32)


/* Platform-specific fastmutex implementation */
PUBLIC IX_STATUS ixOsalOemFastMutexTryLock (IxOsalFastMutex * mutex);

/* Platform-specific init (MemMap) */
PUBLIC IX_STATUS
ixOsalOemInit (void);

/* Platform-specific unload (MemMap) */
PUBLIC void
ixOsalOemUnload (void);

/* Default implementations */

PUBLIC UINT32
ixOsalIxp400SharedTimestampGet (void);


UINT32
ixOsalIxp400SharedTimestampRateGet (void);

UINT32
ixOsalIxp400SharedSysClockRateGet (void);

void
ixOsalIxp400SharedTimeGet (IxOsalTimeval * tv);


INT32
ixOsalIxp400SharedLog (UINT32 level, UINT32 device, char *format, 
                       int arg1, int arg2, int arg3, int arg4, 
                       int arg5, int arg6);

#endif /* IxOsal_Oem_H */

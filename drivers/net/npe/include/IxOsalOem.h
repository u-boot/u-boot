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
 * SPDX-License-Identifier:	BSD-3-Clause
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

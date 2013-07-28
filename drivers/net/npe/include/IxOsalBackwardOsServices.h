/** 
 * This file is intended to provide backward 
 * compatibility for main osService/OSSL 
 * APIs. 
 *
 * It shall be phased out gradually and users
 * are strongly recommended to use IX_OSAL API.
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

#ifndef IX_OSAL_BACKWARD_OSSERVICES_H
#define IX_OSAL_BACKWARD_OSSERVICES_H

#ifndef __vxworks
typedef UINT32 IX_IRQ_STATUS;
#else
typedef int IX_IRQ_STATUS;
#endif

typedef IxOsalMutex IxMutex;

typedef IxOsalFastMutex IxFastMutex;

typedef IxOsalVoidFnVoidPtr IxVoidFnVoidPtr;

typedef IxOsalVoidFnPtr IxVoidFnPtr;


#define LOG_NONE 	IX_OSAL_LOG_LVL_NONE
#define LOG_USER 	IX_OSAL_LOG_LVL_USER
#define LOG_FATAL 	IX_OSAL_LOG_LVL_FATAL
#define LOG_ERROR  	IX_OSAL_LOG_LVL_ERROR
#define LOG_WARNING	IX_OSAL_LOG_LVL_WARNING
#define LOG_MESSAGE	IX_OSAL_LOG_LVL_MESSAGE
#define LOG_DEBUG1  IX_OSAL_LOG_LVL_DEBUG1
#define LOG_DEBUG2	IX_OSAL_LOG_LVL_DEBUG2
#define LOG_DEBUG3 	IX_OSAL_LOG_LVL_DEBUG3
#ifndef __vxworks
#define LOG_ALL 	IX_OSAL_LOG_LVL_ALL
#endif

PUBLIC IX_STATUS
ixOsServIntBind (int level, void (*routine) (void *), void *parameter);

PUBLIC IX_STATUS ixOsServIntUnbind (int level);


PUBLIC int ixOsServIntLock (void);

PUBLIC void ixOsServIntUnlock (int lockKey);


PUBLIC int ixOsServIntLevelSet (int level);

PUBLIC IX_STATUS ixOsServMutexInit (IxMutex * mutex);

PUBLIC IX_STATUS ixOsServMutexLock (IxMutex * mutex);

PUBLIC IX_STATUS ixOsServMutexUnlock (IxMutex * mutex);

PUBLIC IX_STATUS ixOsServMutexDestroy (IxMutex * mutex);

PUBLIC IX_STATUS ixOsServFastMutexInit (IxFastMutex * mutex);

PUBLIC IX_STATUS ixOsServFastMutexTryLock (IxFastMutex * mutex);

PUBLIC IX_STATUS ixOsServFastMutexUnlock (IxFastMutex * mutex);

PUBLIC int
ixOsServLog (int level, char *format, int arg1, int arg2, int arg3, int arg4,
	     int arg5, int arg6);


PUBLIC int ixOsServLogLevelSet (int level);

PUBLIC void ixOsServSleep (int microseconds);

PUBLIC void ixOsServTaskSleep (int milliseconds);

PUBLIC unsigned int ixOsServTimestampGet (void);


PUBLIC void ixOsServUnload (void);

PUBLIC void ixOsServYield (void);

#endif
/* IX_OSAL_BACKWARD_OSSERVICES_H */

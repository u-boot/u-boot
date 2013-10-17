/**
 * @file IxOsalTypes.h
 *
 * @brief Define OSAL basic data types.
 *
 * This file contains fundamental data types used by OSAL.
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


#ifndef IxOsalTypes_H
#define IxOsalTypes_H

#include <config.h>
#include <common.h>

#define __ixp42X			/* sr: U-Boot needs this define */
#define IXP425_EXP_CFG_BASE	0xC4000000
#define diag_printf		debug

#undef SIMSPARCSOLARIS
#define SIMSPARCSOLARIS		0xaffe	/* sr: U-Boot gets confused with this solaris define */

/*
 * Include the OS-specific type definitions
 */
#include "IxOsalOsTypes.h"
/**
 * @defgroup IxOsalTypes Osal basic data types.
 *
 * @brief Basic data types for Osal
 *
 * @{
 */

/**
 * @brief OSAL status
 *
 * @note Possible OSAL return status include IX_SUCCESS and IX_FAIL.
 */
typedef UINT32 IX_STATUS;

/**
 * @brief VUINT32
 *
 * @note volatile UINT32
 */
typedef volatile UINT32 VUINT32;

/**
 * @brief VINT32
 *
 * @note volatile INT32
 */
typedef volatile INT32 VINT32;

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_BILLION
 *
 * @brief  Alias for 1,000,000,000
 *
 */
#define IX_OSAL_BILLION (1000000000)

#ifndef NULL
#define NULL       0L
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_SUCCESS
 *
 * @brief Success status
 *
 */
#ifndef IX_SUCCESS
#define IX_SUCCESS 0L /**< #defined as 0L */
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_FAIL
 *
 * @brief Failure status
 *
 */
#ifndef IX_FAIL
#define IX_FAIL    1L /**< #defined as 1L */
#endif


#ifndef PRIVATE
#ifdef IX_PRIVATE_OFF
#define PRIVATE			/* nothing */
#else
#define PRIVATE static	/**< #defined as static, except for debug builds */
#endif /* IX_PRIVATE_OFF */
#endif /* PRIVATE */


/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_INLINE
 *
 * @brief Alias for __inline
 *
 */
#ifndef IX_OSAL_INLINE
#define IX_OSAL_INLINE __inline
#endif /* IX_OSAL_INLINE */


#ifndef __inline__
#define __inline__	IX_OSAL_INLINE
#endif


/* Each OS can define its own PUBLIC, otherwise it will be empty. */
#ifndef PUBLIC
#define PUBLIC
#endif /* PUBLIC */


/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_INLINE_EXTERN
 *
 * @brief Alias for __inline extern
 *
 */
#ifndef IX_OSAL_INLINE_EXTERN
#define IX_OSAL_INLINE_EXTERN IX_OSAL_INLINE extern
#endif

/**
 * @ingroup IxOsalTypes
 * @enum IxOsalLogDevice
 * @brief This is an emum for OSAL log devices.
 */
typedef enum
{
    IX_OSAL_LOG_DEV_STDOUT = 0,	       /**< standard output (implemented by default) */
    IX_OSAL_LOG_DEV_STDERR = 1,	       /**< standard error (implemented */
    IX_OSAL_LOG_DEV_HEX_DISPLAY = 2,   /**< hexadecimal display (not implemented) */
    IX_OSAL_LOG_DEV_ASCII_DISPLAY = 3  /**< ASCII-capable display (not implemented) */
} IxOsalLogDevice;


/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_LOG_ERROR
 *
 * @brief Alias for -1, used as log function error status
 *
 */
#define IX_OSAL_LOG_ERROR (-1)

/**
 * @ingroup IxOsalTypes
 * @enum IxOsalLogLevel
 * @brief This is an emum for OSAL log trace level.
 */
typedef enum
{
    IX_OSAL_LOG_LVL_NONE = 0,	 /**<No trace level */
    IX_OSAL_LOG_LVL_USER = 1,	 /**<Set trace level to user */
    IX_OSAL_LOG_LVL_FATAL = 2,	 /**<Set trace level to fatal */
    IX_OSAL_LOG_LVL_ERROR = 3,	 /**<Set trace level to error */
    IX_OSAL_LOG_LVL_WARNING = 4, /**<Set trace level to warning */
    IX_OSAL_LOG_LVL_MESSAGE = 5, /**<Set trace level to message */
    IX_OSAL_LOG_LVL_DEBUG1 = 6,	 /**<Set trace level to debug1 */
    IX_OSAL_LOG_LVL_DEBUG2 = 7,	 /**<Set trace level to debug2 */
    IX_OSAL_LOG_LVL_DEBUG3 = 8,	 /**<Set trace level to debug3 */
    IX_OSAL_LOG_LVL_ALL	/**<Set trace level to all */
} IxOsalLogLevel;


/**
 * @ingroup IxOsalTypes
 * @brief Void function pointer prototype
 *
 * @note accepts a void pointer parameter
 * and does not return a value.
 */
typedef void (*IxOsalVoidFnVoidPtr) (void *);

typedef void (*IxOsalVoidFnPtr) (void);


/**
 * @brief Timeval structure
 *
 * @note Contain subfields of seconds and nanoseconds..
 */
typedef struct
{
    UINT32 secs;		/**< seconds */
    UINT32 nsecs;		/**< nanoseconds */
} IxOsalTimeval;


/**
 * @ingroup IxOsalTypes
 * @brief IxOsalTimer
 *
 * @note OSAL timer handle
 *
 */
typedef UINT32 IxOsalTimer;


/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_WAIT_FOREVER
 *
 * @brief Definition for timeout forever, OS-specific.
 *
 */
#define IX_OSAL_WAIT_FOREVER IX_OSAL_OS_WAIT_FOREVER

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_WAIT_NONE
 *
 * @brief Definition for timeout 0, OS-specific.
 *
 */
#define IX_OSAL_WAIT_NONE    IX_OSAL_OS_WAIT_NONE


/**
 * @ingroup IxOsalTypes
 * @brief IxOsalMutex
 *
 * @note Mutex handle, OS-specific
 *
 */
typedef IxOsalOsMutex IxOsalMutex;

/**
 * @ingroup IxOsalTypes
 * @brief IxOsalFastMutex
 *
 * @note FastMutex handle, OS-specific
 *
 */
typedef IxOsalOsFastMutex IxOsalFastMutex;

/**
 * @ingroup IxOsalTypes
 * @brief IxOsalThread
 *
 * @note Thread handle, OS-specific
 *
 */
typedef IxOsalOsThread IxOsalThread;

/**
 * @ingroup IxOsalTypes
 * @brief IxOsalSemaphore
 *
 * @note Semaphore handle, OS-specific
 *
 */
typedef IxOsalOsSemaphore IxOsalSemaphore;

/**
 * @ingroup IxOsalTypes
 * @brief IxOsalMessageQueue
 *
 * @note Message Queue handle, OS-specific
 *
 */
typedef IxOsalOsMessageQueue IxOsalMessageQueue;


/**
 * @brief Thread Attribute
 * @note Default thread attribute
 */
typedef struct
{
    char *name;        /**< name */
    UINT32 stackSize;  /**< stack size */
    UINT32 priority;   /**< priority */
} IxOsalThreadAttr;

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_THREAD_DEFAULT_STACK_SIZE
 *
 * @brief Default thread stack size, OS-specific.
 *
 */
#define IX_OSAL_THREAD_DEFAULT_STACK_SIZE (IX_OSAL_OS_THREAD_DEFAULT_STACK_SIZE)

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_THREAD_MAX_STACK_SIZE
 *
 * @brief Max stack size, OS-specific.
 *
 */
#define IX_OSAL_THREAD_MAX_STACK_SIZE (IX_OSAL_OS_THREAD_MAX_STACK_SIZE)

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_DEFAULT_THREAD_PRIORITY
 *
 * @brief Default thread priority, OS-specific.
 *
 */
#define IX_OSAL_DEFAULT_THREAD_PRIORITY (IX_OSAL_OS_DEFAULT_THREAD_PRIORITY)

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_MAX_THREAD_PRIORITY
 *
 * @brief Max thread priority, OS-specific.
 *
 */
#define IX_OSAL_MAX_THREAD_PRIORITY (IX_OSAL_OS_MAX_THREAD_PRIORITY)

/**
 * @} IxOsalTypes
 */


#endif /* IxOsalTypes_H */

/**
 * @file IxEthAccDBLocks_p.h
 *
 * @brief Definition of transaction lock stacks and lock utility macros
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

#ifndef IxEthAccDBLocks_p_H
#define IxEthAccDBLocks_p_H

#include "IxOsPrintf.h"

/* Lock and lock stacks */
typedef struct
{
    IxOsalFastMutex* locks[MAX_LOCKS];
    UINT32 stackPointer, basePointer;
} LockStack;

#define TRY_LOCK(mutex) \
    { \
        if (ixOsalFastMutexTryLock(mutex) != IX_SUCCESS) \
        { \
            return IX_ETH_DB_BUSY; \
        } \
    }


#define UNLOCK(mutex) { ixOsalFastMutexUnlock(mutex); }

#define INIT_STACK(stack) \
    { \
        (stack)->basePointer  = 0; \
        (stack)->stackPointer = 0; \
    }

#define PUSH_LOCK(stack, lock) \
    { \
        if ((stack)->stackPointer == MAX_LOCKS) \
        { \
            ERROR_LOG("Ethernet DB: maximum number of elements in a lock stack has been exceeded on push, heavy chaining?\n"); \
            UNROLL_STACK(stack); \
            \
            return IX_ETH_DB_NOMEM; \
        } \
        \
        if (ixOsalFastMutexTryLock(lock) == IX_SUCCESS) \
        { \
            (stack)->locks[(stack)->stackPointer++] = (lock); \
        } \
        else \
        { \
            UNROLL_STACK(stack); \
            \
            return IX_ETH_DB_BUSY; \
        } \
    }

#define POP_LOCK(stack) \
    { \
        ixOsalFastMutexUnlock((stack)->locks[--(stack)->stackPointer]); \
    }

#define UNROLL_STACK(stack) \
    { \
        while ((stack)->stackPointer > (stack)->basePointer) \
        { \
            POP_LOCK(stack); \
        } \
    }

#define SHIFT_STACK(stack) \
    { \
        if ((stack)->basePointer == MAX_LOCKS - 1) \
        { \
            ERROR_LOG("Ethernet DB: maximum number of elements in a lock stack has been exceeded on shift, heavy chaining?\n"); \
            UNROLL_STACK(stack); \
            \
            return IX_ETH_DB_BUSY; \
        } \
        \
        ixOsalFastMutexUnlock((stack)->locks[(stack)->basePointer++]); \
    }

#endif /* IxEthAccDBLocks_p_H */

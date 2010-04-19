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

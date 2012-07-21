/**
 * @file IxOsalBufferMgtDefault.h
 *
 * @brief Default buffer pool management and buffer management
 *        definitions.
 *
 * Design Notes:
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

#ifndef IX_OSAL_BUFFER_MGT_DEFAULT_H
#define IX_OSAL_BUFFER_MGT_DEFAULT_H

/**
 * @enum IxMbufPoolAllocationType
 * @brief Used to indicate how the pool memory was allocated
 */

typedef enum
{
    IX_OSAL_MBUF_POOL_TYPE_SYS_ALLOC = 0, /**< mbuf pool allocated by the system */
    IX_OSAL_MBUF_POOL_TYPE_USER_ALLOC	 /**< mbuf pool allocated by the user */
} IxOsalMbufPoolAllocationType;


/**
 * @brief Implementation of buffer pool structure for use with non-VxWorks OS
 */

typedef struct
{
    IX_OSAL_MBUF *nextFreeBuf;	/**< Pointer to the next free mbuf              */
    void *mbufMemPtr;	   /**< Pointer to the mbuf memory area            */
    void *dataMemPtr;	   /**< Pointer to the data memory area            */
    int bufDataSize;	   /**< The size of the data portion of each mbuf  */
    int totalBufsInPool;   /**< Total number of mbufs in the pool          */
    int freeBufsInPool;	   /**< Number of free mbufs currently in the pool */
    int mbufMemSize;	   /**< The size of the pool mbuf memory area      */
    int dataMemSize;	   /**< The size of the pool data memory area      */
    char name[IX_OSAL_MBUF_POOL_NAME_LEN + 1];	 /**< Descriptive name for pool */
    IxOsalMbufPoolAllocationType poolAllocType;
    unsigned int poolIdx;  /**< Pool Index */ 
} IxOsalMbufPool;

typedef IxOsalMbufPool IX_OSAL_MBUF_POOL;


PUBLIC IX_STATUS ixOsalBuffPoolUninit (IX_OSAL_MBUF_POOL * pool);


#endif /* IX_OSAL_BUFFER_MGT_DEFAULT_H */

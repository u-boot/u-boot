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
 * SPDX-License-Identifier:	BSD-3-Clause
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

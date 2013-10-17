/**
 * @file IxOsalBufferMgt.h
 *
 * @brief OSAL Buffer pool management and buffer management definitions.
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
/* @par
 * -- Copyright Notice --
 *
 * @par
 * Copyright 1979, 1980, 1983, 1986, 1988, 1989, 1991, 1992, 1993, 1994 
 *      The Regents of the University of California. All rights reserved.
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxOsalBufferMgt_H
#define IxOsalBufferMgt_H

#include "IxOsal.h"
/**
 * @defgroup IxOsalBufferMgt OSAL Buffer Management Module.
 *
 * @brief Buffer management module for IxOsal
 *
 * @{ 
 */

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_MAX_POOLS
 *
 * @brief The maximum number of pools that can be allocated, must be 
 *        a multiple of 32 as required by implementation logic.
 * @note  This can safely be increased if more pools are required.
 */
#define IX_OSAL_MBUF_MAX_POOLS      32

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_NAME_LEN
 *
 * @brief The maximum string length of the pool name
 */
#define IX_OSAL_MBUF_POOL_NAME_LEN  64



/**
 *  Define IX_OSAL_MBUF
 */


/* forward declaration of internal structure */
struct __IXP_BUF;

/* 
 * OS can define it in IxOsalOs.h to skip the following
 * definition.
 */
#ifndef IX_OSAL_ATTRIBUTE_ALIGN32
#define IX_OSAL_ATTRIBUTE_ALIGN32 __attribute__ ((aligned(32)))
#endif

/* release v1.4 backward compatible definitions */
struct __IX_MBUF
{
    struct __IXP_BUF *ix_next IX_OSAL_ATTRIBUTE_ALIGN32;
    struct __IXP_BUF *ix_nextPacket;
    UINT8 *ix_data;
    UINT32 ix_len;  
    unsigned char ix_type;
    unsigned char ix_flags;
    unsigned short ix_reserved;
    UINT32 ix_rsvd;
    UINT32 ix_PktLen; 
    void *ix_priv;     
};

struct __IX_CTRL
{
    UINT32 ix_reserved[2];        /**< Reserved field */
    UINT32 ix_signature;          /**< Field to indicate if buffers are allocated by the system */    
    UINT32 ix_allocated_len;      /**< Allocated buffer length */  
    UINT32 ix_allocated_data;     /**< Allocated buffer data pointer */  
    void *ix_pool;                /**< pointer to the buffer pool */
    struct __IXP_BUF *ix_chain;   /**< chaining */ 
    void *ix_osbuf_ptr;           /**< Storage for OS-specific buffer pointer */
};

struct __IX_NE_SHARED
{
    UINT32 reserved[8] IX_OSAL_ATTRIBUTE_ALIGN32;   /**< Reserved area for NPE Service-specific usage */
};


/* 
 * IXP buffer structure 
 */
typedef struct __IXP_BUF
{
    struct __IX_MBUF ix_mbuf IX_OSAL_ATTRIBUTE_ALIGN32; /**< buffer header */
    struct __IX_CTRL ix_ctrl;                           /**< buffer management */
    struct __IX_NE_SHARED ix_ne;                        /**< Reserved area for NPE Service-specific usage*/
} IXP_BUF;



/**
 * @ingroup IxOsalBufferMgt
 *
 * @def typedef IX_OSAL_MBUF
 *
 * @brief Generic IXP mbuf format.
 */
typedef IXP_BUF IX_OSAL_MBUF;


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_IXP_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)
 *
 * @brief Return pointer to the next mbuf in a single packet
 */
#define IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)  \
        (m_blk_ptr)->ix_mbuf.ix_next


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)
 *
 * @brief Return pointer to the next packet in the chain
 */
#define IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)  \
        (m_blk_ptr)->ix_mbuf.ix_nextPacket


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_MDATA(m_blk_ptr)
 *
 * @brief Return pointer to the data in the mbuf
 */
#define IX_OSAL_MBUF_MDATA(m_blk_ptr)       (m_blk_ptr)->ix_mbuf.ix_data

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_MLEN(m_blk_ptr)
 *
 * @brief Return the data length
 */
#define IX_OSAL_MBUF_MLEN(m_blk_ptr) \
    (m_blk_ptr)->ix_mbuf.ix_len

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_MTYPE(m_blk_ptr)
 *
 * @brief Return the data type in the mbuf
 */
#define IX_OSAL_MBUF_MTYPE(m_blk_ptr) \
    (m_blk_ptr)->ix_mbuf.ix_type


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_FLAGS(m_blk_ptr)
 *
 * @brief Return the buffer flags
 */
#define IX_OSAL_MBUF_FLAGS(m_blk_ptr)       \
        (m_blk_ptr)->ix_mbuf.ix_flags


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_NET_POOL(m_blk_ptr)
 *
 * @brief Return pointer to a network pool
 */
#define IX_OSAL_MBUF_NET_POOL(m_blk_ptr)	\
        (m_blk_ptr)->ix_ctrl.ix_pool



/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_PKT_LEN(m_blk_ptr)
 *
 * @brief Return the total length of all the data in
 * the mbuf chain for this packet
 */
#define IX_OSAL_MBUF_PKT_LEN(m_blk_ptr) \
        (m_blk_ptr)->ix_mbuf.ix_PktLen




/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_PRIV(m_blk_ptr)
 *
 * @brief Return the private field
 */
#define IX_OSAL_MBUF_PRIV(m_blk_ptr)        \
        (m_blk_ptr)->ix_mbuf.ix_priv



/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_SIGNATURE(m_blk_ptr)
 *
 * @brief Return the signature field of IX_OSAL_MBUF
 */
#define IX_OSAL_MBUF_SIGNATURE(m_blk_ptr)  \
        (m_blk_ptr)->ix_ctrl.ix_signature


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_OSBUF_PTR(m_blk_ptr)
 *
 * @brief Return ix_osbuf_ptr field of IX_OSAL_MBUF, which is used to store OS-specific buffer pointer during a buffer conversion.
 */
#define IX_OSAL_MBUF_OSBUF_PTR(m_blk_ptr)  \
        (m_blk_ptr)->ix_ctrl.ix_osbuf_ptr


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)
 *
 * @brief Return the allocated buffer size
 */
#define IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)  \
        (m_blk_ptr)->ix_ctrl.ix_allocated_len

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(m_blk_ptr)
 *
 * @brief Return the allocated buffer pointer
 */
#define IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(m_blk_ptr)  \
        (m_blk_ptr)->ix_ctrl.ix_allocated_data



/* Name length */
#define IX_OSAL_MBUF_POOL_NAME_LEN  64


/****************************************************
 * Macros for buffer pool management
 ****************************************************/

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_FREE_COUNT(m_pool_ptr
 *
 * @brief Return the total number of freed buffers left in the pool.
 */
#define IX_OSAL_MBUF_POOL_FREE_COUNT(m_pool_ptr) \
                    ixOsalBuffPoolFreeCountGet(m_pool_ptr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_SIZE_ALIGN
 *
 * @brief This macro takes an integer as an argument and
 * rounds it up to be a multiple of the memory cache-line 
 * size.
 *
 * @param int [in] size - the size integer to be rounded up
 *
 * @return int - the size, rounded up to a multiple of
 *               the cache-line size
 */
#define IX_OSAL_MBUF_POOL_SIZE_ALIGN(size)                 \
    ((((size) + (IX_OSAL_CACHE_LINE_SIZE - 1)) /      \
        IX_OSAL_CACHE_LINE_SIZE) *                  \
            IX_OSAL_CACHE_LINE_SIZE)

/* Don't use this directly, use macro */
PUBLIC UINT32 ixOsalBuffPoolMbufAreaSizeGet (int count);


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED
 *
 * @brief This macro calculates, from the number of mbufs required, the 
 * size of the memory area required to contain the mbuf headers for the
 * buffers in the pool.  The size to be used for each mbuf header is 
 * rounded up to a multiple of the cache-line size, to ensure
 * each mbuf header aligns on a cache-line boundary.
 * This macro is used by IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC()
 *
 * @param int [in] count - the number of buffers the pool will contain
 *
 * @return int - the total size required for the pool mbuf area (aligned)
 */
#define IX_OSAL_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count) \
        ixOsalBuffPoolMbufAreaSizeGet(count)


/* Don't use this directly, use macro */
PUBLIC UINT32 ixOsalBuffPoolDataAreaSizeGet (int count, int size);


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_DATA_AREA_SIZE_ALIGNED
 *
 * @brief This macro calculates, from the number of mbufs required and the
 * size of the data portion for each mbuf, the size of the data memory area
 * required. The size is adjusted to ensure alignment on cache line boundaries.
 * This macro is used by IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC()
 *
 *
 * @param int [in] count - The number of mbufs in the pool.
 * @param int [in] size  - The desired size for each mbuf data portion.
 *                         This size will be rounded up to a multiple of the
 *                         cache-line size to ensure alignment on cache-line
 *                         boundaries for each data block.
 *
 * @return int - the total size required for the pool data area (aligned)
 */
#define IX_OSAL_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count, size) \
        ixOsalBuffPoolDataAreaSizeGet((count), (size))


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC
 *
 * @brief Allocates the memory area needed for the number of mbuf headers
 * specified by <i>count</i>.
 * This macro ensures the mbuf headers align on cache line boundaries.
 * This macro evaluates to a pointer to the memory allocated.
 *
 * @param int [in] count - the number of mbufs the pool will contain
 * @param int [out] memAreaSize - the total amount of memory allocated
 *
 * @return void * - a pointer to the allocated memory area
 */
#define IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC(count, memAreaSize) \
    IX_OSAL_CACHE_DMA_MALLOC((memAreaSize =                 \
        IX_OSAL_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count)))

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC
 *
 * @brief Allocates the memory pool for the data portion of the pool mbufs.
 * The number of mbufs is specified by <i>count</i>.  The size of the data
 * portion of each mbuf is specified by <i>size</i>.
 * This macro ensures the mbufs are aligned on cache line boundaries
 * This macro evaluates to a pointer to the memory allocated.
 *
 * @param int [in] count - the number of mbufs the pool will contain
 * @param int [in] size - the desired size (in bytes) required for the data
 *                        portion of each mbuf.  Note that this size may be
 *                        rounded up to ensure alignment on cache-line
 *                        boundaries.
 * @param int [out] memAreaSize - the total amount of memory allocated
 *
 * @return void * - a pointer to the allocated memory area
 */
#define IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC(count, size, memAreaSize) \
    IX_OSAL_CACHE_DMA_MALLOC((memAreaSize =                     \
        IX_OSAL_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count,size)))



/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_INIT
 *
 * @brief Wrapper macro for ixOsalPoolInit() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_INIT(count, size, name) \
    ixOsalPoolInit((count), (size), (name))

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_NO_ALLOC_POOL_INIT
 *
 * @return Pointer to the new pool or NULL if the initialization failed.
 *
 * @brief Wrapper macro for ixOsalNoAllocPoolInit() 
 * See function description below for details.
 * 
 */
#define IX_OSAL_MBUF_NO_ALLOC_POOL_INIT(bufPtr, dataPtr, count, size, name) \
    ixOsalNoAllocPoolInit( (bufPtr), (dataPtr), (count), (size), (name))

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_GET
 *
 * @brief Wrapper macro for ixOsalMbufAlloc() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_GET(poolPtr) \
        ixOsalMbufAlloc(poolPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_PUT
 *
 * @brief Wrapper macro for ixOsalMbufFree() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_PUT(bufPtr) \
    ixOsalMbufFree(bufPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_PUT_CHAIN
 *
 * @brief Wrapper macro for ixOsalMbufChainFree() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_PUT_CHAIN(bufPtr) \
    ixOsalMbufChainFree(bufPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_SHOW
 *
 * @brief Wrapper macro for ixOsalMbufPoolShow() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_SHOW(poolPtr) \
    ixOsalMbufPoolShow(poolPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_MDATA_RESET
 *
 * @brief Wrapper macro for ixOsalMbufDataPtrReset() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_MDATA_RESET(bufPtr) \
    ixOsalMbufDataPtrReset(bufPtr)

/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_MBUF_POOL_UNINIT
 *
 * @brief Wrapper macro for ixOsalBuffPoolUninit() 
 * See function description below for details.
 */
#define IX_OSAL_MBUF_POOL_UNINIT(m_pool_ptr)  \
        ixOsalBuffPoolUninit(m_pool_ptr)

/* 
 * Include OS-specific bufferMgt definitions 
 */
#include "IxOsalOsBufferMgt.h"


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_CONVERT_OSBUF_TO_IXPBUF( osBufPtr, ixpBufPtr)
 *
 * @brief Convert pre-allocated os-specific buffer format to OSAL IXP_BUF (IX_OSAL_MBUF) format. 
 * It is users' responsibility to provide pre-allocated and valid buffer pointers.
 * @param osBufPtr (in) - a pre-allocated os-specific buffer pointer.
 * @param ixpBufPtr (in)- a pre-allocated OSAL IXP_BUF pointer
 * @return None
 */
#define IX_OSAL_CONVERT_OSBUF_TO_IXPBUF( osBufPtr, ixpBufPtr) \
        IX_OSAL_OS_CONVERT_OSBUF_TO_IXPBUF( osBufPtr, ixpBufPtr)        


/**
 * @ingroup IxOsalBufferMgt
 *
 * @def IX_OSAL_CONVERT_IXPBUF_TO_OSBUF( ixpBufPtr, osBufPtr)
 *
 * @brief Convert pre-allocated OSAL IXP_BUF (IX_OSAL_MBUF) format to os-specific buffer pointers.
 * @param ixpBufPtr (in) - OSAL IXP_BUF pointer
 * @param osBufPtr (out) - os-specific buffer pointer.
 * @return None
 */

#define IX_OSAL_CONVERT_IXPBUF_TO_OSBUF( ixpBufPtr, osBufPtr)  \
        IX_OSAL_OS_CONVERT_IXPBUF_TO_OSBUF( ixpBufPtr, osBufPtr)


PUBLIC IX_OSAL_MBUF_POOL *ixOsalPoolInit (UINT32 count,
                      UINT32 size, const char *name);

PUBLIC IX_OSAL_MBUF_POOL *ixOsalNoAllocPoolInit (void *poolBufPtr,
                         void *poolDataPtr,
						 UINT32 count,
						 UINT32 size,
						 const char *name);

PUBLIC IX_OSAL_MBUF *ixOsalMbufAlloc (IX_OSAL_MBUF_POOL * pool);

PUBLIC IX_OSAL_MBUF *ixOsalMbufFree (IX_OSAL_MBUF * mbuf);

PUBLIC void ixOsalMbufChainFree (IX_OSAL_MBUF * mbuf);

PUBLIC void ixOsalMbufDataPtrReset (IX_OSAL_MBUF * mbuf);

PUBLIC void ixOsalMbufPoolShow (IX_OSAL_MBUF_POOL * pool);

PUBLIC IX_STATUS ixOsalBuffPoolUninit (IX_OSAL_MBUF_POOL * pool);

PUBLIC UINT32 ixOsalBuffPoolFreeCountGet(IX_OSAL_MBUF_POOL * pool);


/**
 * @} IxOsalBufferMgt
 */


#endif /* IxOsalBufferMgt_H */

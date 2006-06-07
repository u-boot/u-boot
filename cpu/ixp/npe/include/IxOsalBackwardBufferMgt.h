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

#ifndef IX_OSAL_BACKWARD_BUFFER_MGT_H
#define IX_OSAL_BACKWARD_BUFFER_MGT_H

typedef IX_OSAL_MBUF IX_MBUF;

typedef IX_OSAL_MBUF_POOL IX_MBUF_POOL;


#define IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)  \
		IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)


#define IX_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)  \
		IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)


#define IX_MBUF_MDATA(m_blk_ptr)  \
		IX_OSAL_MBUF_MDATA(m_blk_ptr)


#define IX_MBUF_MLEN(m_blk_ptr) \
		IX_OSAL_MBUF_MLEN(m_blk_ptr)


#define IX_MBUF_TYPE(m_blk_ptr) \
		IX_OSAL_MBUF_MTYPE(m_blk_ptr)

/* Same as IX_MBUF_TYPE */
#define IX_MBUF_MTYPE(m_blk_ptr) \
                IX_OSAL_MBUF_MTYPE(m_blk_ptr)

#define IX_MBUF_FLAGS(m_blk_ptr)   	\
		IX_OSAL_MBUF_FLAGS(m_blk_ptr)


#define IX_MBUF_NET_POOL(m_blk_ptr)	\
		IX_OSAL_MBUF_NET_POOL(m_blk_ptr)


#define IX_MBUF_PKT_LEN(m_blk_ptr)	\
		IX_OSAL_MBUF_PKT_LEN(m_blk_ptr)


#define IX_MBUF_PRIV(m_blk_ptr)		\
		IX_OSAL_MBUF_PRIV(m_blk_ptr)


#define IX_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)  \
		IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)


#define IX_MBUF_ALLOCATED_BUFF_DATA(m_blk_ptr)  \
		IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(m_blk_ptr)


#define IX_MBUF_POOL_SIZE_ALIGN(size)   \
		IX_OSAL_MBUF_POOL_SIZE_ALIGN(size)


#define IX_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count)	\
		IX_OSAL_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count)


#define IX_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count, size) \
		IX_OSAL_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count, size)


#define IX_MBUF_POOL_MBUF_AREA_ALLOC(count, memAreaSize) \
		IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC(count, memAreaSize)


#define IX_MBUF_POOL_DATA_AREA_ALLOC(count, size, memAreaSize) \
		IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC(count, size, memAreaSize)

IX_STATUS
ixOsalOsIxp400BackwardPoolInit (IX_OSAL_MBUF_POOL ** poolPtrPtr,
				UINT32 count, UINT32 size, const char *name);


/* This one needs extra steps*/
#define IX_MBUF_POOL_INIT(poolPtr, count, size, name) \
		ixOsalOsIxp400BackwardPoolInit( poolPtr, count,  size, name)


#define IX_MBUF_POOL_INIT_NO_ALLOC(poolPtrPtr, bufPtr, dataPtr, count, size, name) \
		(*poolPtrPtr = IX_OSAL_MBUF_NO_ALLOC_POOL_INIT(bufPtr, dataPtr, count, size, name))


IX_STATUS
ixOsalOsIxp400BackwardMbufPoolGet (IX_OSAL_MBUF_POOL * poolPtr,
				   IX_OSAL_MBUF ** newBufPtrPtr);

#define IX_MBUF_POOL_GET(poolPtr, bufPtrPtr) \
		ixOsalOsIxp400BackwardMbufPoolGet(poolPtr, bufPtrPtr)


#define IX_MBUF_POOL_PUT(bufPtr) \
		IX_OSAL_MBUF_POOL_PUT(bufPtr)


#define IX_MBUF_POOL_PUT_CHAIN(bufPtr) \
		IX_OSAL_MBUF_POOL_PUT_CHAIN(bufPtr)


#define IX_MBUF_POOL_SHOW(poolPtr) \
		IX_OSAL_MBUF_POOL_SHOW(poolPtr)


#define IX_MBUF_POOL_MDATA_RESET(bufPtr) \
		IX_OSAL_MBUF_POOL_MDATA_RESET(bufPtr)

#endif /* IX_OSAL_BACKWARD_BUFFER_MGT_H */

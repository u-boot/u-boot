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

#ifndef IX_OSAL_BACKWARD_CACHE_MMU_H
#define IX_OSAL_BACKWARD_CACHE_MMU_H

#ifdef IX_OSAL_CACHED
#define IX_ACC_CACHE_ENABLED
#endif

#define IX_XSCALE_CACHE_LINE_SIZE IX_OSAL_CACHE_LINE_SIZE

#define IX_ACC_DRV_DMA_MALLOC(size) IX_OSAL_CACHE_DMA_MALLOC(size)

#define IX_ACC_DRV_DMA_FREE(ptr,size) IX_OSAL_CACHE_DMA_FREE(ptr)

#define IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(addr) IX_OSAL_MMU_VIRT_TO_PHYS(addr)

#define IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(addr) IX_OSAL_MMU_PHYS_TO_VIRT(addr)

#define IX_ACC_DATA_CACHE_INVALIDATE(addr,size) IX_OSAL_CACHE_INVALIDATE(addr, size)

#define IX_ACC_DATA_CACHE_FLUSH(addr,size) IX_OSAL_CACHE_FLUSH(addr,size)

#endif /* IX_OSAL_BACKWARD_CACHE_MMU_H */

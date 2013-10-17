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

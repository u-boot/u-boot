/*
 * Copyright (c) 2015 Google, Inc
 *
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef __ALIGNMEM_H
#define __ALIGNMEM_H

/*
 * ARCH_DMA_MINALIGN is defined in asm/cache.h for each architecture.  It
 * is used to align DMA buffers.
 */
#ifndef __ASSEMBLY__
#include <asm/cache.h>

#include <malloc.h>

static inline void *malloc_cache_aligned(size_t size)
{
	return memalign(ARCH_DMA_MINALIGN, ALIGN(size, ARCH_DMA_MINALIGN));
}
#endif

#endif /* __ALIGNMEM_H */

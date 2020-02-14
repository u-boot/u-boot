/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2007
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */
#ifndef __ASM_ARM_DMA_MAPPING_H
#define __ASM_ARM_DMA_MAPPING_H

#include <common.h>
#include <asm/cache.h>
#include <cpu_func.h>
#include <linux/dma-direction.h>
#include <linux/types.h>
#include <malloc.h>

#define	dma_mapping_error(x, y)	0

static inline void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	*handle = (unsigned long)memalign(ARCH_DMA_MINALIGN, ROUND(len, ARCH_DMA_MINALIGN));
	return (void *)*handle;
}

static inline void dma_free_coherent(void *addr)
{
	free(addr);
}

static inline dma_addr_t dma_map_single(void *vaddr, size_t len,
					enum dma_data_direction dir)
{
	unsigned long addr = (unsigned long)vaddr;

	len = ALIGN(len, ARCH_DMA_MINALIGN);

	if (dir == DMA_FROM_DEVICE)
		invalidate_dcache_range(addr, addr + len);
	else
		flush_dcache_range(addr, addr + len);

	return addr;
}

static inline void dma_unmap_single(dma_addr_t addr, size_t len,
				    enum dma_data_direction dir)
{
	len = ALIGN(len, ARCH_DMA_MINALIGN);

	if (dir != DMA_TO_DEVICE)
		invalidate_dcache_range(addr, addr + len);
}

#endif /* __ASM_ARM_DMA_MAPPING_H */

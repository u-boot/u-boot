/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Andes Technology Corporation
 * Ken Kuo, Andes Technology Corporation <ken_kuo@andestech.com>
 */
#ifndef __ASM_NDS_DMA_MAPPING_H
#define __ASM_NDS_DMA_MAPPING_H

#include <common.h>
#include <asm/cache.h>
#include <cpu_func.h>
#include <linux/dma-direction.h>
#include <linux/types.h>
#include <malloc.h>

static void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	*handle = (unsigned long)memalign(ARCH_DMA_MINALIGN, len);
	return (void *)*handle;
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

static inline void dma_unmap_single(volatile void *vaddr, size_t len,
				    enum dma_data_direction dir)
{
	unsigned long addr = (unsigned long)vaddr;

	len = ALIGN(len, ARCH_DMA_MINALIGN);

	if (dir != DMA_TO_DEVICE)
		invalidate_dcache_range(addr, addr + len);
}

#endif /* __ASM_NDS_DMA_MAPPING_H */

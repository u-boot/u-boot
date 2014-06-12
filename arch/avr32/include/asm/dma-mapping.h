/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_AVR32_DMA_MAPPING_H
#define __ASM_AVR32_DMA_MAPPING_H

#include <asm/io.h>
#include <asm/arch/cacheflush.h>

enum dma_data_direction {
	DMA_BIDIRECTIONAL	= 0,
	DMA_TO_DEVICE		= 1,
	DMA_FROM_DEVICE		= 2,
};
extern void *dma_alloc_coherent(size_t len, unsigned long *handle);

static inline unsigned long dma_map_single(volatile void *vaddr, size_t len,
					   enum dma_data_direction dir)
{
	extern void __bad_dma_data_direction(void);

	switch (dir) {
	case DMA_BIDIRECTIONAL:
		flush_dcache_range((unsigned long)vaddr,
				   (unsigned long)vaddr + len);
		break;
	case DMA_TO_DEVICE:
		dcache_clean_range(vaddr, len);
		break;
	case DMA_FROM_DEVICE:
		invalidate_dcache_range((unsigned long)vaddr,
					(unsigned long)vaddr + len);
		break;
	default:
		/* This will cause a linker error */
		__bad_dma_data_direction();
	}

	return virt_to_phys(vaddr);
}

static inline void dma_unmap_single(volatile void *vaddr, size_t len,
				    unsigned long paddr)
{

}

#endif /* __ASM_AVR32_DMA_MAPPING_H */

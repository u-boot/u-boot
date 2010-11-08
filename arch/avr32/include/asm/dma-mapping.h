/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
		dcache_flush_range(vaddr, len);
		break;
	case DMA_TO_DEVICE:
		dcache_clean_range(vaddr, len);
		break;
	case DMA_FROM_DEVICE:
		dcache_invalidate_range(vaddr, len);
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

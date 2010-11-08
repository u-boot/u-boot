#ifndef __ASM_NIOS2_DMA_MAPPING_H
#define __ASM_NIOS2_DMA_MAPPING_H

/* dma_alloc_coherent() return cache-line aligned allocation which is mapped
 * to uncached io region.
 *
 * IO_REGION_BASE should be defined in board config header file
 *   0x80000000 for nommu, 0xe0000000 for mmu
 */

static inline void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	void *addr = malloc(len + CONFIG_SYS_DCACHELINE_SIZE);
	if (!addr)
		return 0;
	flush_dcache((unsigned long)addr, len + CONFIG_SYS_DCACHELINE_SIZE);
	*handle = ((unsigned long)addr +
		   (CONFIG_SYS_DCACHELINE_SIZE - 1)) &
		~(CONFIG_SYS_DCACHELINE_SIZE - 1) & ~(IO_REGION_BASE);
	return (void *)(*handle | IO_REGION_BASE);
}

#endif /* __ASM_NIOS2_DMA_MAPPING_H */

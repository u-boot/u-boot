/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_NIOS2_CACHE_H_
#define __ASM_NIOS2_CACHE_H_

extern void flush_dcache (unsigned long start, unsigned long size);
extern void flush_icache (unsigned long start, unsigned long size);

/*
 * Valid L1 data cache line sizes for the NIOS2 architecture are 4, 16, and 32
 * bytes.  If the board configuration has not specified one we default to the
 * largest of these values for alignment of DMA buffers.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	32
#endif

#endif /* __ASM_NIOS2_CACHE_H_ */

/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_OPENRISC_CACHE_H_
#define __ASM_OPENRISC_CACHE_H_

/*
 * Valid L1 data cache line sizes for the OpenRISC architecture are
 * 16 and 32 bytes.
 * If the board configuration has not specified one we default to the
 * largest of these values for alignment of DMA buffers.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN       CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN       32
#endif

#endif /* __ASM_OPENRISC_CACHE_H_ */

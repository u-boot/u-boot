/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARC_CACHE_H
#define __ASM_ARC_CACHE_H

#include <config.h>

/*
 * The current upper bound for ARC L1 data cache line sizes is 128 bytes.
 * We use that value for aligning DMA buffers unless the board config has
 * specified an alternate cache line size.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	128
#endif

#endif /* __ASM_ARC_CACHE_H */

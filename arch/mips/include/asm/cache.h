/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MIPS_CACHE_H__
#define __MIPS_CACHE_H__

/*
 * The maximum L1 data cache line size on MIPS seems to be 128 bytes.  We use
 * that as a default for aligning DMA buffers unless the board config has
 * specified another cache line size.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	128
#endif

#endif /* __MIPS_CACHE_H__ */

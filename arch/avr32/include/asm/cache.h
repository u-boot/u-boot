/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AVR32_CACHE_H__
#define __AVR32_CACHE_H__

/*
 * Since the AVR32 architecture has a queryable cacheline size with a maximum
 * value of 256 we set the DMA buffer alignemnt requirement to this maximum
 * value.  The board config can override this if it knows that the cacheline
 * size is a smaller value.  AVR32 boards use the CONFIG_SYS_DCACHE_LINESZ
 * macro to specify cache line size, so if it is set we use it instead.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#elif defined(CONFIG_SYS_DCACHE_LINESZ)
#define ARCH_DMA_MINALIGN	CONFIG_SYS_DCACHE_LINESZ
#else
#define ARCH_DMA_MINALIGN	256
#endif

#endif /* __AVR32_CACHE_H__ */

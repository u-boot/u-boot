/*
 * (C) Copyright 2008,
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SPARC_CACHE_H__
#define __SPARC_CACHE_H__

#include <linux/config.h>
#include <asm/processor.h>

/*
 * If CONFIG_SYS_CACHELINE_SIZE is defined use it for DMA alignment.  Otherwise
 * use 32-bytes, the cacheline size for Sparc.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	32
#endif

#endif

/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _TEST_BURST_H
#define _TEST_BURST_H

/* Cache line size */
#define CACHE_LINE_SIZE		16
/* Binary logarithm of the cache line size */
#define LG_CACHE_LINE_SIZE	4

#ifndef __ASSEMBLY__
extern void mmu_init(void);
extern void caches_init(void);
extern void flush_dcache_range(unsigned long start, unsigned long stop);
#endif

#endif /* _TEST_BURST_H */

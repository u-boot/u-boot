/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* for now: just dummy functions to satisfy the linker */

#include <common.h>

void  __flush_cache(unsigned long start, unsigned long size)
{
#if defined(CONFIG_ARM1136)

#if !defined(CONFIG_SYS_ICACHE_OFF)
	asm("mcr p15, 0, r1, c7, c5, 0"); /* invalidate I cache */
#endif

#if !defined(CONFIG_SYS_DCACHE_OFF)
	asm("mcr p15, 0, r1, c7, c14, 0"); /* Clean+invalidate D cache */
#endif

#endif /* CONFIG_ARM1136 */

#ifdef CONFIG_ARM926EJS
	/* test and clean, page 2-23 of arm926ejs manual */
	asm("0: mrc p15, 0, r15, c7, c10, 3\n\t" "bne 0b\n" : : : "memory");
	/* disable write buffer as well (page 2-22) */
	asm("mcr p15, 0, %0, c7, c10, 4" : : "r" (0));
#endif /* CONFIG_ARM926EJS */
	return;
}
void  flush_cache(unsigned long start, unsigned long size)
	__attribute__((weak, alias("__flush_cache")));

/*
 * Default implementation:
 * do a range flush for the entire range
 */
void	__flush_dcache_all(void)
{
	flush_cache(0, ~0);
}
void	flush_dcache_all(void)
	__attribute__((weak, alias("__flush_dcache_all")));


/*
 * Default implementation of enable_caches()
 * Real implementation should be in platform code
 */
void __enable_caches(void)
{
	puts("WARNING: Caches not enabled\n");
}
void enable_caches(void)
	__attribute__((weak, alias("__enable_caches")));

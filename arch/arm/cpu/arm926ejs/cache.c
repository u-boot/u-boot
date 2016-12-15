/*
 * (C) Copyright 2011
 * Ilya Yanok, EmCraft Systems
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <linux/types.h>
#include <common.h>

#ifndef CONFIG_SYS_DCACHE_OFF
void invalidate_dcache_all(void)
{
	asm volatile("mcr p15, 0, %0, c7, c6, 0\n" : : "r"(0));
}

void flush_dcache_all(void)
{
	asm volatile(
		"0:"
		"mrc p15, 0, r15, c7, c14, 3\n"
		"bne 0b\n"
		"mcr p15, 0, %0, c7, c10, 4\n"
		 : : "r"(0) : "memory"
	);
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	if (!check_cache_range(start, stop))
		return;

	while (start < stop) {
		asm volatile("mcr p15, 0, %0, c7, c6, 1\n" : : "r"(start));
		start += CONFIG_SYS_CACHELINE_SIZE;
	}
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	if (!check_cache_range(start, stop))
		return;

	while (start < stop) {
		asm volatile("mcr p15, 0, %0, c7, c14, 1\n" : : "r"(start));
		start += CONFIG_SYS_CACHELINE_SIZE;
	}

	asm volatile("mcr p15, 0, %0, c7, c10, 4\n" : : "r"(0));
}
#else /* #ifndef CONFIG_SYS_DCACHE_OFF */
void invalidate_dcache_all(void)
{
}

void flush_dcache_all(void)
{
}
#endif /* #ifndef CONFIG_SYS_DCACHE_OFF */

/*
 * Stub implementations for l2 cache operations
 */

__weak void l2_cache_disable(void) {}

#if defined CONFIG_SYS_THUMB_BUILD
__weak void invalidate_l2_cache(void) {}
#endif

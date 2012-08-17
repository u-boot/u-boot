/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* for now: just dummy functions to satisfy the linker */

#include <common.h>

void  __flush_cache(unsigned long start, unsigned long size)
{
#if defined(CONFIG_OMAP2420) || defined(CONFIG_ARM1136)
	void arm1136_cache_flush(void);

	arm1136_cache_flush();
#endif
#ifdef CONFIG_ARM926EJS
	/* test and clean, page 2-23 of arm926ejs manual */
	asm("0: mrc p15, 0, r15, c7, c10, 3\n\t" "bne 0b\n" : : : "memory");
	/* disable write buffer as well (page 2-22) */
	asm("mcr p15, 0, %0, c7, c10, 4" : : "r" (0));
#endif
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

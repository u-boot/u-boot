/*
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include <asm/arch/cacheflush.h>

void dcache_clean_range(volatile void *start, size_t size)
{
	unsigned long v, begin, end, linesz;

	linesz = CONFIG_SYS_DCACHE_LINESZ;

	/* You asked for it, you got it */
	begin = (unsigned long)start & ~(linesz - 1);
	end = ((unsigned long)start + size + linesz - 1) & ~(linesz - 1);

	for (v = begin; v < end; v += linesz)
		dcache_clean_line((void *)v);

	sync_write_buffer();
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	unsigned long v, linesz;

	linesz = CONFIG_SYS_DCACHE_LINESZ;

	/* You asked for it, you got it */
	start = start & ~(linesz - 1);
	stop = (stop + linesz - 1) & ~(linesz - 1);

	for (v = start; v < stop; v += linesz)
		dcache_invalidate_line((void *)v);
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	unsigned long v, linesz;

	linesz = CONFIG_SYS_DCACHE_LINESZ;

	/* You asked for it, you got it */
	start = start & ~(linesz - 1);
	stop = (stop + linesz - 1) & ~(linesz - 1);

	for (v = start; v < stop; v += linesz)
		dcache_flush_line((void *)v);

	sync_write_buffer();
}

void icache_invalidate_range(volatile void *start, size_t size)
{
	unsigned long v, begin, end, linesz;

	linesz = CONFIG_SYS_ICACHE_LINESZ;

	/* You asked for it, you got it */
	begin = (unsigned long)start & ~(linesz - 1);
	end = ((unsigned long)start + size + linesz - 1) & ~(linesz - 1);

	for (v = begin; v < end; v += linesz)
		icache_invalidate_line((void *)v);
}

/*
 * This is called after loading something into memory.  We need to
 * make sure that everything that was loaded is actually written to
 * RAM, and that the icache will look for it. Cleaning the dcache and
 * invalidating the icache will do the trick.
 */
void  flush_cache (unsigned long start_addr, unsigned long size)
{
	dcache_clean_range((void *)start_addr, size);
	icache_invalidate_range((void *)start_addr, size);
}

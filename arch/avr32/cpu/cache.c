/*
 * Copyright (C) 2004-2006 Atmel Corporation
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

void dcache_invalidate_range(volatile void *start, size_t size)
{
	unsigned long v, begin, end, linesz;

	linesz = CONFIG_SYS_DCACHE_LINESZ;

	/* You asked for it, you got it */
	begin = (unsigned long)start & ~(linesz - 1);
	end = ((unsigned long)start + size + linesz - 1) & ~(linesz - 1);

	for (v = begin; v < end; v += linesz)
		dcache_invalidate_line((void *)v);
}

void dcache_flush_range(volatile void *start, size_t size)
{
	unsigned long v, begin, end, linesz;

	linesz = CONFIG_SYS_DCACHE_LINESZ;

	/* You asked for it, you got it */
	begin = (unsigned long)start & ~(linesz - 1);
	end = ((unsigned long)start + size + linesz - 1) & ~(linesz - 1);

	for (v = begin; v < end; v += linesz)
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

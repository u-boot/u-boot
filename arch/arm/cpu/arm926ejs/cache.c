/*
 * (C) Copyright 2011
 * Ilya Yanok, EmCraft Systems
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
 * Foundation, Inc.
 */
#include <linux/types.h>
#include <common.h>

#ifndef CONFIG_SYS_DCACHE_OFF
static inline void dcache_noop(void)
{
	if (dcache_status()) {
		puts("WARNING: cache operations are not implemented!\n"
		     "WARNING: disabling D-Cache now, you can re-enable it"
		     "later with 'dcache on' command\n");
		dcache_disable();
	}
}

void invalidate_dcache_all(void)
{
	dcache_noop();
}

void flush_dcache_all(void)
{
	dcache_noop();
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	dcache_noop();
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	dcache_noop();
}
#else /* #ifndef CONFIG_SYS_DCACHE_OFF */
void invalidate_dcache_all(void)
{
}

void flush_dcache_all(void)
{
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
}

void  flush_cache(unsigned long start, unsigned long size)
{
}
#endif /* #ifndef CONFIG_SYS_DCACHE_OFF */

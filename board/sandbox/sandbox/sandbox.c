/*
 * Copyright (c) 2011 The Chromium OS Authors.
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

#include <os.h>

/*
 * Pointer to initial global data area
 *
 * Here we initialize it.
 */
gd_t *gd;

void flush_cache(unsigned long start, unsigned long size)
{
}

ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

unsigned long long get_ticks(void)
{
	return get_timer(0);
}

ulong get_timer(ulong base)
{
	return (os_get_nsec() / 1000000) - base;
}

int timer_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_DRAM_SIZE;
	return 0;
}

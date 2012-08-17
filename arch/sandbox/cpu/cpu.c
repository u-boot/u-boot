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

DECLARE_GLOBAL_DATA_PTR;

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	/* This is considered normal termination for now */
	os_exit(0);
	return 0;
}

/* delay x useconds */
void __udelay(unsigned long usec)
{
	os_usleep(usec);
}

unsigned long timer_get_us(void)
{
	return os_get_nsec() / 1000;
}

int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	return -1;
}

int cleanup_before_linux(void)
{
	return 0;
}

void *map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags)
{
	return (void *)(gd->ram_buf + paddr);
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
}

/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
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

unsigned long __attribute__((no_instrument_function)) timer_get_us(void)
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
	return (void *)(gd->arch.ram_buf + paddr);
}

phys_addr_t map_to_sysmem(void *ptr)
{
	return (u8 *)ptr - gd->arch.ram_buf;
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
}

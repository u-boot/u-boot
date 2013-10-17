/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
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
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

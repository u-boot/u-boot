/*
 * U-boot - cache.c
 *
 * Copyright (c) 2005-2008 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <asm/blackfin.h>

void flush_cache(unsigned long addr, unsigned long size)
{
	/* no need to flush stuff in on chip memory (L1/L2/etc...) */
	if (addr >= 0xE0000000)
		return;

	if (icache_status())
		blackfin_icache_flush_range((void *)addr, (void *)(addr + size));

	if (dcache_status())
		blackfin_dcache_flush_range((void *)addr, (void *)(addr + size));
}

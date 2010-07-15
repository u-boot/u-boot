/*
 * functions for handling bootcount support
 *
 * Copyright (c) 2010 Analog Devices Inc.
 *
 * Licensed under the 2-clause BSD.
 */

/* This version uses one 32bit storage and combines the magic/count */

#include <common.h>

/* We abuse the EVT0 MMR for bootcount storage by default */
#ifndef CONFIG_SYS_BOOTCOUNT_ADDR
# define CONFIG_SYS_BOOTCOUNT_ADDR EVT0
#endif

#define MAGIC_MASK 0xffff0000
#define COUNT_MASK 0x0000ffff

void bootcount_store(ulong cnt)
{
	ulong magic = (BOOTCOUNT_MAGIC & MAGIC_MASK) | (cnt & COUNT_MASK);
	bfin_write32(CONFIG_SYS_BOOTCOUNT_ADDR, magic);
}

ulong bootcount_load(void)
{
	ulong magic = bfin_read32(CONFIG_SYS_BOOTCOUNT_ADDR);
	if ((magic & MAGIC_MASK) == (BOOTCOUNT_MAGIC & MAGIC_MASK))
		return magic & COUNT_MASK;
	else
		return 0;
}

/*
 * (C) Copyright 2007
 * Daniel Hellstrom, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <config.h>
#include <asm/leon.h>

int dram_init(void)
{
	/* Does not set gd->ram_size here */

	return 0;
}

int checkboard(void)
{
	puts("Board: GR-XC3S-1500\n");
	return 0;
}

int misc_init_r(void)
{
	return 0;
}

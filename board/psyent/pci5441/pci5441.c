/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

int board_early_init_f (void)
{
	return 0;
}

int checkboard (void)
{
	puts ("BOARD : Psyent PCI-5441\n");
	return 0;
}

phys_size_t initdram (int board_type)
{
	return (0);
}

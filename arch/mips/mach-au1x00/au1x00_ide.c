// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <ide.h>

/* AU1X00 swaps data in big-endian mode, enforce little-endian function */
void ide_input_swap_data(int dev, ulong *sect_buf, int words)
{
	ide_input_data(dev, sect_buf, words);
}

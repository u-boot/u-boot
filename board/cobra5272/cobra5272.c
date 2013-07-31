/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/immap.h>


int checkboard (void)
{
	puts ("Board: ");
	puts ("senTec COBRA5272 Board\n");
	return 0;
};

phys_size_t initdram (int board_type)
{
	volatile sdramctrl_t *sdp = (sdramctrl_t *) (MMAP_SDRAM);

	sdp->sdram_sdtr = 0xf539;
	sdp->sdram_sdcr = 0x4211;

	/* Dummy write to start SDRAM */
	*((volatile unsigned long *) 0) = 0;

	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
};

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("DRAM test not implemented!\n");

	return (0);
}

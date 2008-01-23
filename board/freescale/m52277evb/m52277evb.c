/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
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
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale M52277 EVB\n");
	return 0;
};

long int initdram(int board_type)
{
	volatile sdramc_t *sdram = (volatile sdramc_t *)(MMAP_SDRAM);
	u32 dramsize, i;

	dramsize = CFG_SDRAM_SIZE * 0x100000;

	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	sdram->sdcs0 = (CFG_SDRAM_BASE | i);

	sdram->sdcfg1 = CFG_SDRAM_CFG1;
	sdram->sdcfg2 = CFG_SDRAM_CFG2;

	/* Issue PALL */
	sdram->sdcr = CFG_SDRAM_CTRL | 2;

	/* Issue LEMR */
	/*sdram->sdmr = CFG_SDRAM_EMOD; */
	sdram->sdmr = CFG_SDRAM_MODE;

	udelay(1000);

	/* Issue PALL */
	sdram->sdcr = CFG_SDRAM_CTRL | 2;

	/* Perform two refresh cycles */
	sdram->sdcr = CFG_SDRAM_CTRL | 4;
	sdram->sdcr = CFG_SDRAM_CTRL | 4;

	sdram->sdcr = (CFG_SDRAM_CTRL & ~0x80000000) | 0x10000c00;

	udelay(100);

	return (dramsize);
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}

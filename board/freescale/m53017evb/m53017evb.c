/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
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

#include <config.h>
#include <common.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale M53017EVB\n");
	return 0;
};

phys_size_t initdram(int board_type)
{
	volatile sdram_t *sdram = (volatile sdram_t *)(MMAP_SDRAM);
	u32 dramsize, i;

	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000;

	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	sdram->cs0 = (CONFIG_SYS_SDRAM_BASE | i);
#ifdef CONFIG_SYS_SDRAM_BASE1
	sdram->cs1 = (CONFIG_SYS_SDRAM_BASE | i);
#endif
	sdram->cfg1 = CONFIG_SYS_SDRAM_CFG1;
	sdram->cfg2 = CONFIG_SYS_SDRAM_CFG2;

	udelay(500);

	/* Issue PALL */
	sdram->ctrl = (CONFIG_SYS_SDRAM_CTRL | 2);
	asm("nop");

	/* Perform two refresh cycles */
	sdram->ctrl = CONFIG_SYS_SDRAM_CTRL | 4;
	sdram->ctrl = CONFIG_SYS_SDRAM_CTRL | 4;
	asm("nop");

	/* Issue LEMR */
	sdram->mode = CONFIG_SYS_SDRAM_MODE;
	asm("nop");
	sdram->mode = CONFIG_SYS_SDRAM_EMOD;
	asm("nop");

	sdram->ctrl = (CONFIG_SYS_SDRAM_CTRL | 2);
	asm("nop");

	sdram->ctrl = (CONFIG_SYS_SDRAM_CTRL & ~0x80000000) | 0x10000F00;
	asm("nop");

	udelay(100);

	return dramsize;
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}

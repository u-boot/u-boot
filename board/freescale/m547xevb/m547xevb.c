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

#include <config.h>
#include <common.h>
#include <pci.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale FireEngine 5475 EVB\n");
	return 0;
};

long int initdram(int board_type)
{
	volatile siu_t *siu = (siu_t *) (MMAP_SIU);
	volatile sdram_t *sdram = (volatile sdram_t *)(MMAP_SDRAM);
	u32 dramsize, i;

	siu->drv = CFG_SDRAM_DRVSTRENGTH;

	dramsize = CFG_DRAMSZ * 0x100000;
	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;
	siu->cs0cfg = (CFG_SDRAM_BASE | i);

#ifdef CFG_DRAMSZ1
	temp = CFG_DRAMSZ1 * 0x100000;
	for (i = 0x13; i < 0x20; i++) {
		if (temp == (1 << i))
			break;
	}
	i--;
	dramsize += temp;
	siu->cs1cfg = ((CFG_SDRAM_BASE + temp) | i);
#endif

	sdram->cfg1 = CFG_SDRAM_CFG1;
	sdram->cfg2 = CFG_SDRAM_CFG2;

	/* Issue PALL */
	sdram->ctrl = CFG_SDRAM_CTRL | 2;

	/* Issue LEMR */
	sdram->mode = CFG_SDRAM_EMOD;
	sdram->mode = (CFG_SDRAM_MODE | 0x04000000);

	udelay(500);

	/* Issue PALL */
	sdram->ctrl = (CFG_SDRAM_CTRL | 2);

	/* Perform two refresh cycles */
	sdram->ctrl = CFG_SDRAM_CTRL | 4;
	sdram->ctrl = CFG_SDRAM_CTRL | 4;

	sdram->mode = CFG_SDRAM_MODE;

	sdram->ctrl = (CFG_SDRAM_CTRL & ~0x80000000) | 0x10000F00;

	udelay(100);

	return dramsize;
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}

#if defined(CONFIG_PCI)
/*
 * Initialize PCI devices, report devices found.
 */
static struct pci_controller hose;
extern void pci_mcf547x_8x_init(struct pci_controller *hose);

void pci_init_board(void)
{
	pci_mcf547x_8x_init(&hose);
}
#endif				/* CONFIG_PCI */

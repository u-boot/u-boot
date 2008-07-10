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
#include <pci.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale M54455 EVB\n");
	return 0;
};

phys_size_t initdram(int board_type)
{
	volatile sdramc_t *sdram = (volatile sdramc_t *)(MMAP_SDRAM);
	volatile gpio_t *gpio = (volatile gpio_t *)(MMAP_GPIO);
	u32 dramsize, i;

	dramsize = CFG_SDRAM_SIZE * 0x100000 >> 1;

	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	gpio->mscr_sdram = 0xAA;

	sdram->sdcs0 = (CFG_SDRAM_BASE | i);
	sdram->sdcs1 = (CFG_SDRAM_BASE1 | i);

	sdram->sdcfg1 = CFG_SDRAM_CFG1;
	sdram->sdcfg2 = CFG_SDRAM_CFG2;

	/* Issue PALL */
	sdram->sdcr = CFG_SDRAM_CTRL | 2;

	/* Issue LEMR */
	sdram->sdmr = CFG_SDRAM_EMOD | 0x408;
	sdram->sdmr = CFG_SDRAM_MODE | 0x300;

	udelay(500);

	/* Issue PALL */
	sdram->sdcr = CFG_SDRAM_CTRL | 2;

	/* Perform two refresh cycles */
	sdram->sdcr = CFG_SDRAM_CTRL | 4;
	sdram->sdcr = CFG_SDRAM_CTRL | 4;

	sdram->sdmr = CFG_SDRAM_MODE | 0x200;

	sdram->sdcr = (CFG_SDRAM_CTRL & ~0x80000000) | 0x10000c00;

	udelay(100);

	return (dramsize << 1);
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}

#if defined(CONFIG_CMD_IDE)
#include <ata.h>

int ide_preinit(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	gpio->par_fec |= (gpio->par_fec & GPIO_PAR_FEC_FEC1_MASK) | 0x10;
	gpio->par_feci2c |=
	    (gpio->par_feci2c & 0xF0FF) | (GPIO_PAR_FECI2C_MDC1_ATA_DIOR |
					   GPIO_PAR_FECI2C_MDIO1_ATA_DIOW);
	gpio->par_ata |=
	    (GPIO_PAR_ATA_BUFEN | GPIO_PAR_ATA_CS1 | GPIO_PAR_ATA_CS0 |
	     GPIO_PAR_ATA_DA2 | GPIO_PAR_ATA_DA1 | GPIO_PAR_ATA_DA0
	     | GPIO_PAR_ATA_RESET_RESET | GPIO_PAR_ATA_DMARQ_DMARQ |
	     GPIO_PAR_ATA_IORDY_IORDY);
	gpio->par_pci |=
	    (GPIO_PAR_PCI_GNT3_ATA_DMACK | GPIO_PAR_PCI_REQ3_ATA_INTRQ);

	return (0);
}

void ide_set_reset(int idereset)
{
	volatile atac_t *ata = (atac_t *) MMAP_ATA;
	long period;
	/*  t1,  t2,  t3,  t4,  t5,  t6,  t9, tRD,  tA */
	int piotms[5][9] = {
		{70, 165, 60, 30, 50, 5, 20, 0, 35},	/* PIO 0 */
		{50, 125, 45, 20, 35, 5, 15, 0, 35},	/* PIO 1 */
		{30, 100, 30, 15, 20, 5, 10, 0, 35},	/* PIO 2 */
		{30, 80, 30, 10, 20, 5, 10, 0, 35},	/* PIO 3 */
		{25, 70, 20, 10, 20, 5, 10, 0, 35}
	};			/* PIO 4 */

	if (idereset) {
		ata->cr = 0;	/* control reset */
		udelay(10000);
	} else {
#define CALC_TIMING(t) (t + period - 1) / period
		period = 1000000000 / gd->bus_clk;	/* period in ns */

		/*ata->ton = CALC_TIMING (180); */
		ata->t1 = CALC_TIMING(piotms[2][0]);
		ata->t2w = CALC_TIMING(piotms[2][1]);
		ata->t2r = CALC_TIMING(piotms[2][1]);
		ata->ta = CALC_TIMING(piotms[2][8]);
		ata->trd = CALC_TIMING(piotms[2][7]);
		ata->t4 = CALC_TIMING(piotms[2][3]);
		ata->t9 = CALC_TIMING(piotms[2][6]);

		ata->cr = 0x40;	/* IORDY enable */
		udelay(200000);
		ata->cr |= 0x01;	/* IORDY enable */
	}
}
#endif

#if defined(CONFIG_PCI)
/*
 * Initialize PCI devices, report devices found.
 */
static struct pci_controller hose;
extern void pci_mcf5445x_init(struct pci_controller *hose);

void pci_init_board(void)
{
	pci_mcf5445x_init(&hose);
}
#endif				/* CONFIG_PCI */

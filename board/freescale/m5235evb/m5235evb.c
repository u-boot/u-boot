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
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale M5235 EVB\n");
	return 0;
};

long int initdram(int board_type)
{
	volatile sdram_t *sdram = (volatile sdram_t *)(MMAP_SDRAM);
	volatile gpio_t *gpio = (volatile gpio_t *)(MMAP_GPIO);
	u32 dramsize, i, dramclk;

	/*
	 * When booting from external Flash, the port-size is less than
	 * the port-size of SDRAM.  In this case it is necessary to enable
	 * Data[15:0] on Port Address/Data.
	 */
	gpio->par_ad =
	    GPIO_PAR_AD_ADDR23 | GPIO_PAR_AD_ADDR22 | GPIO_PAR_AD_ADDR21 |
	    GPIO_PAR_AD_DATAL;

	/* Initialize PAR to enable SDRAM signals */
	gpio->par_sdram =
	    GPIO_PAR_SDRAM_SDWE | GPIO_PAR_SDRAM_SCAS | GPIO_PAR_SDRAM_SRAS |
	    GPIO_PAR_SDRAM_SCKE | GPIO_PAR_SDRAM_SDCS(3);

	dramsize = CFG_SDRAM_SIZE * 0x100000;
	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	if (!(sdram->dacr0 & SDRAMC_DARCn_RE)) {
		dramclk = gd->bus_clk / (CFG_HZ * CFG_HZ);

		/* Initialize DRAM Control Register: DCR */
		sdram->dcr = SDRAMC_DCR_RTIM_9CLKS |
		    SDRAMC_DCR_RTIM_6CLKS | SDRAMC_DCR_RC((15 * dramclk) >> 4);

		/* Initialize DACR0 */
		sdram->dacr0 =
		    SDRAMC_DARCn_BA(CFG_SDRAM_BASE) | SDRAMC_DARCn_CASL_C1 |
		    SDRAMC_DARCn_CBM_CMD20 | SDRAMC_DARCn_PS_32;

		/* Initialize DMR0 */
		sdram->dmr0 = ((dramsize - 1) & 0xFFFC0000) | SDRAMC_DMRn_V;

		/* Set IP (bit 3) in DACR */
		sdram->dacr0 |= SDRAMC_DARCn_IP;

		/* Wait 30ns to allow banks to precharge */
		for (i = 0; i < 5; i++) {
			asm("nop");
		}

		/* Write to this block to initiate precharge */
		*(u32 *) (CFG_SDRAM_BASE) = 0xA5A59696;

		/*  Set RE (bit 15) in DACR */
		sdram->dacr0 |= SDRAMC_DARCn_RE;

		/* Wait for at least 8 auto refresh cycles to occur */
		for (i = 0; i < 0x2000; i++) {
			asm("nop");
		}

		/* Finish the configuration by issuing the MRS. */
		sdram->dacr0 |= SDRAMC_DARCn_IMRS;

		/* Write to the SDRAM Mode Register */
		*(u32 *) (CFG_SDRAM_BASE + 0x400) = 0xA5A59696;
	}

	return dramsize;
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}

/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2005-2008 Arthur Shipkowski (art@videon-central.com)
 *
 * Copyright (C) 2012 Freescale Semiconductor, Inc. All Rights Reserved.
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
#include <asm/io.h>

#define PERIOD		13	/* system bus period in ns */
#define SDRAM_TREFI	7800	/* in ns */

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale MCF5275 EVB\n");
	return 0;
};

phys_size_t initdram(int board_type)
{
	sdramctrl_t *sdp = (sdramctrl_t *)(MMAP_SDRAM);
	gpio_t *gpio_reg = (gpio_t *)(MMAP_GPIO);

	/* Enable SDRAM */
	out_be16(&gpio_reg->par_sdram, 0x3FF);

	/* Set up chip select */
	out_be32(&sdp->sdbar0, CONFIG_SYS_SDRAM_BASE);
	out_be32(&sdp->sdbmr0, MCF_SDRAMC_SDMRn_BAM_32M | MCF_SDRAMC_SDMRn_V);

	/* Set up timing */
	out_be32(&sdp->sdcfg1, 0x83711630);
	out_be32(&sdp->sdcfg2, 0x46770000);

	/* Enable clock */
	out_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_MODE_EN | MCF_SDRAMC_SDCR_CKE);

	/* Set precharge */
	setbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_IPALL);

	/* Dummy write to start SDRAM */
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Send LEMR */
	setbits_be32(&sdp->sdmr,
		MCF_SDRAMC_SDMR_BNKAD_LEMR | MCF_SDRAMC_SDMR_AD(0x0) |
		MCF_SDRAMC_SDMR_CMD);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Send LMR */
	out_be32(&sdp->sdmr, 0x058d0000);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Stop sending commands */
	clrbits_be32(&sdp->sdmr, MCF_SDRAMC_SDMR_CMD);

	/* Set precharge */
	setbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_IPALL);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Stop manual precharge, send 2 IREF */
	clrbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_IPALL);
	setbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_IREF);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;


	out_be32(&sdp->sdmr, 0x018d0000);
	*((volatile unsigned long *)CONFIG_SYS_SDRAM_BASE) = 0xa5a59696;

	/* Stop sending commands */
	clrbits_be32(&sdp->sdmr, MCF_SDRAMC_SDMR_CMD);
	clrbits_be32(&sdp->sdcr, MCF_SDRAMC_SDCR_MODE_EN);

	/* Turn on auto refresh, lock SDMR */
	out_be32(&sdp->sdcr,
		MCF_SDRAMC_SDCR_CKE
		| MCF_SDRAMC_SDCR_REF
		| MCF_SDRAMC_SDCR_MUX(1)
		/* 1 added to round up */
		| MCF_SDRAMC_SDCR_RCNT((SDRAM_TREFI/(PERIOD*64)) - 1 + 1)
		| MCF_SDRAMC_SDCR_DQS_OE(0x3));

	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}

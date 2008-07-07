/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006. All rights reserved.
 *
 * (C) Copyright 2008
 * Andre Schwarz, Matrix Vision GmbH, andre.schwarz@matrix-vision.de
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
 * MERCHANTABILITY or FITNESS for A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <asm/mpc8349_pci.h>
#include <pci.h>
#include <spi.h>
#include <asm/mmu.h>
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif

#include "mvblm7.h"

int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *)CFG_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	msize = CFG_DDR_SIZE;
	for (ddr_size = msize << 20, ddr_size_log2 = 0;
	     (ddr_size > 1);
	     ddr_size = ddr_size >> 1, ddr_size_log2++) {
		if (ddr_size & 1)
			return -1;
	}
	im->sysconf.ddrlaw[0].bar = ((CFG_DDR_SDRAM_BASE>>12) & 0xfffff);
	im->sysconf.ddrlaw[0].ar = LAWAR_EN | ((ddr_size_log2 - 1) &
		LAWAR_SIZE);

	im->ddr.csbnds[0].csbnds = CFG_DDR_CS0_BNDS;
	im->ddr.cs_config[0] = CFG_DDR_CS0_CONFIG;
	im->ddr.timing_cfg_0 = CFG_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CFG_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CFG_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CFG_DDR_TIMING_3;
	im->ddr.sdram_cfg = CFG_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CFG_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CFG_DDR_MODE;
	im->ddr.sdram_interval = CFG_DDR_INTERVAL;
	im->ddr.sdram_clk_cntl = CFG_DDR_CLK_CNTL;

	udelay(300);

	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	return CFG_DDR_SIZE;
}

phys_size_t initdram(int board_type)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	im->sysconf.ddrlaw[0].bar = CFG_DDR_BASE & LAWBAR_BAR;
	msize = fixed_sdram();

	/* return total bus RAM size(bytes) */
	return msize * 1024 * 1024;
}

int checkboard(void)
{
	puts("Board: Matrix Vision mvBlueLYNX-M7\n");

	return 0;
}

u8 *dhcp_vendorex_prep(u8 *e)
{
	char *ptr;

	/* DHCP vendor-class-identifier = 60 */
	ptr = getenv("dhcp_vendor-class-identifier");
	if (ptr) {
		*e++ = 60;
		*e++ = strlen(ptr);
		while (*ptr)
			*e++ = *ptr++;
	}
	/* DHCP_CLIENT_IDENTIFIER = 61 */
	ptr = getenv("dhcp_client_id");
	if (ptr) {
		*e++ = 61;
		*e++ = strlen(ptr);
		while (*ptr)
			*e++ = *ptr++;
	}

	return e;
}

u8 *dhcp_vendorex_proc(u8 *popt)
{
	return NULL;
}

#ifdef CONFIG_HARD_SPI
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	volatile gpio83xx_t *iopd = &((immap_t *)CFG_IMMR)->gpio[0];

	iopd->dat &= ~MVBLM7_MMC_CS;
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	volatile gpio83xx_t *iopd = &((immap_t *)CFG_IMMR)->gpio[0];

	iopd->dat |= ~MVBLM7_MMC_CS;
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
}

#endif

/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006.
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

#include "../common/mv_common.h"
#include "mvblm7.h"

int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;
	char *s = getenv("ddr_size");

	msize = CONFIG_SYS_DDR_SIZE;
	if (s) {
		u32 env_ddr_size = simple_strtoul(s, NULL, 10);
		if (env_ddr_size == 512)
			msize = 512;
	}

	for (ddr_size = msize << 20, ddr_size_log2 = 0;
	     (ddr_size > 1);
	     ddr_size = ddr_size >> 1, ddr_size_log2++) {
		if (ddr_size & 1)
			return -1;
	}
	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_DDR_SDRAM_BASE & 0xfffff000;
	im->sysconf.ddrlaw[0].ar = LAWAR_EN | ((ddr_size_log2 - 1) &
		LAWAR_SIZE);

	im->ddr.csbnds[0].csbnds = CONFIG_SYS_DDR_CS0_BNDS;
	im->ddr.cs_config[0] = CONFIG_SYS_DDR_CS0_CONFIG;
	im->ddr.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	im->ddr.sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CONFIG_SYS_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CONFIG_SYS_DDR_MODE;
	im->ddr.sdram_mode2 = CONFIG_SYS_DDR_MODE2;
	im->ddr.sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	im->ddr.sdram_clk_cntl = CONFIG_SYS_DDR_SDRAM_CLK_CNTL;

	asm("sync;isync");
	udelay(600);

	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	asm("sync;isync");
	udelay(500);

	return msize;
}

phys_size_t initdram(int board_type)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_DDR_BASE & LAWBAR_BAR;
	msize = fixed_sdram();

	/* return total bus RAM size(bytes) */
	return msize * 1024 * 1024;
}

int misc_init_r(void)
{
	char *s = getenv("reset_env");

	if (s) {
		mv_reset_environment();
	}

	return 0;
}

int checkboard(void)
{
	puts("Board: Matrix Vision mvBlueLYNX-M7\n");

	return 0;
}

#ifdef CONFIG_HARD_SPI
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	volatile gpio83xx_t *iopd = &((immap_t *)CONFIG_SYS_IMMR)->gpio[0];

	iopd->dat &= ~MVBLM7_MMC_CS;
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	volatile gpio83xx_t *iopd = &((immap_t *)CONFIG_SYS_IMMR)->gpio[0];

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

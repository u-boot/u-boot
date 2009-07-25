/*
 * vme8349.c -- esd VME8349 board support
 *
 * Copyright (c) 2008-2009 esd gmbh.
 *
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Reinhard Arlt <reinhard.arlt@esd-electronics.com>
 * Based on board/mpc8349emds/mpc8349emds.c (and previous 834x releases.)
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
 *
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <asm/mpc8349_pci.h>
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif
#include <asm/io.h>
#include <asm/mmu.h>

void ddr_enable_ecc(unsigned int dram_size);

int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	msize = CONFIG_SYS_DDR_SIZE;
	for (ddr_size = msize << 20, ddr_size_log2 = 0;
	     (ddr_size > 1);
	     ddr_size = ddr_size>>1, ddr_size_log2++) {
		if (ddr_size & 1)
			return -1;
	}

	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_DDR_SDRAM_BASE & 0xfffff000;
	im->sysconf.ddrlaw[0].ar  = LAWAR_EN | ((ddr_size_log2 - 1) &
						LAWAR_SIZE);

#if (CONFIG_SYS_DDR_SIZE == 512)
	im->ddr.csbnds[0].csbnds = 0x0000001f;
#else
#warning Currently any DDR size other than 512MiB is not supported
#endif
	im->ddr.cs_config[0]     = CONFIG_SYS_DDR_CONFIG | 0x00330000;

	/* currently we use only one CS, so disable the other banks */
	im->ddr.csbnds[1].csbnds = 0x00000000;
	im->ddr.csbnds[2].csbnds = 0x00000000;
	im->ddr.csbnds[3].csbnds = 0x00000000;
	im->ddr.cs_config[1] = 0;
	im->ddr.cs_config[2] = 0;
	im->ddr.cs_config[3] = 0;

	im->ddr.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;

	im->ddr.sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CONFIG_SYS_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CONFIG_SYS_DDR_MODE;
	im->ddr.sdram_mode2 = CONFIG_SYS_DDR_MODE2;

	im->ddr.sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	sync();
	udelay(200);

	/* enable DDR controller */
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	return msize;
}

phys_size_t initdram(int board_type)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_DDR_BASE & LAWBAR_BAR;

	msize = fixed_sdram();

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif

	/* Now check memory size (after ECC is initialized) */
	msize = get_ram_size(0, msize);

	/* return total bus SDRAM size(bytes)  -- DDR */
	return msize * 1024 * 1024;
}

int checkboard(void)
{
	puts("Board: esd VME8349\n");

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
}
#endif

/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006-2007
 *
 * Authors: Nick.Spence@freescale.com
 *          Wilson.Lo@freescale.com
 *          scottwood@freescale.com
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
#include <mpc83xx.h>
#include <spd_sdram.h>

#include <asm/bitops.h>
#include <asm/io.h>

#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CFG_8313ERDB_BROKEN_PMC
static void resume_from_sleep(void)
{
	u32 magic = *(u32 *)0;

	typedef void (*func_t)(void);
	func_t resume = *(func_t *)4;

	if (magic == 0xf5153ae5)
		resume();

	gd->flags &= ~GD_FLG_SILENT;
	puts("\nResume from sleep failed: bad magic word\n");
}
#endif

/* Fixed sdram init -- doesn't use serial presence detect.
 *
 * This is useful for faster booting in configs where the RAM is unlikely
 * to be changed, or for things like NAND booting where space is tight.
 */
static long fixed_sdram(void)
{
	volatile immap_t *im = (volatile immap_t *)CFG_IMMR;
	u32 msize = CFG_DDR_SIZE * 1024 * 1024;
	u32 msize_log2 = __ilog2(msize);

	im->sysconf.ddrlaw[0].bar = CFG_DDR_SDRAM_BASE >> 12;
	im->sysconf.ddrlaw[0].ar = LBLAWAR_EN | (msize_log2 - 1);
	im->sysconf.ddrcdr = CFG_DDRCDR_VALUE;

	/*
	 * Erratum DDR3 requires a 50ms delay after clearing DDRCDR[DDR_cfg],
	 * or the DDR2 controller may fail to initialize correctly.
	 */
	udelay(50000);

	im->ddr.csbnds[0].csbnds = (msize - 1) >> 24;
	im->ddr.cs_config[0] = CFG_DDR_CONFIG;

	/* Currently we use only one CS, so disable the other bank. */
	im->ddr.cs_config[1] = 0;

	im->ddr.sdram_clk_cntl = CFG_DDR_CLK_CNTL;
	im->ddr.timing_cfg_3 = CFG_DDR_TIMING_3;
	im->ddr.timing_cfg_1 = CFG_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CFG_DDR_TIMING_2;
	im->ddr.timing_cfg_0 = CFG_DDR_TIMING_0;

#ifndef CFG_8313ERDB_BROKEN_PMC
	if (im->pmc.pmccr1 & PMCCR1_POWER_OFF)
		im->ddr.sdram_cfg = CFG_SDRAM_CFG | SDRAM_CFG_BI;
	else
#endif
		im->ddr.sdram_cfg = CFG_SDRAM_CFG;

	im->ddr.sdram_cfg2 = CFG_SDRAM_CFG2;
	im->ddr.sdram_mode = CFG_DDR_MODE;
	im->ddr.sdram_mode2 = CFG_DDR_MODE_2;

	im->ddr.sdram_interval = CFG_DDR_INTERVAL;
	sync();

	/* enable DDR controller */
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	return msize;
}

long int initdram(int board_type)
{
	volatile immap_t *im = (volatile immap_t *)CFG_IMMR;
	volatile lbus83xx_t *lbc = &im->lbus;
	u32 msize;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	msize = fixed_sdram();

	/* Local Bus setup lbcr and mrtpr */
	lbc->lbcr = CFG_LBC_LBCR;
	lbc->mrtpr = CFG_LBC_MRTPR;
	sync();

#ifndef CFG_8313ERDB_BROKEN_PMC
	if (im->pmc.pmccr1 & PMCCR1_POWER_OFF)
		resume_from_sleep();
#endif

	/* return total bus SDRAM size(bytes)  -- DDR */
	return msize;
}

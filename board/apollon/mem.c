/*
 * (C) Copyright 2005-2007
 * Samsung Electronics,
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Derived from omap2420
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
#include <asm/arch/omap2420.h>
#include <asm/io.h>
#include <asm/arch/bits.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/clocks.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>

#include "mem.h"

/************************************************************
 * sdelay() - simple spin loop.  Will be constant time as
 *  its generally used in 12MHz bypass conditions only.  This
 *  is necessary until timers are accessible.
 *
 *  not inline to increase chances its in cache when called
*************************************************************/
void sdelay(unsigned long loops)
{
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

/*****************************************************************************
 * prcm_init() - inits clocks for PRCM as defined in clocks.h
 * (config II default). Called from SRAM, or Flash (using temp SRAM stack).
 *****************************************************************************/
void prcm_init(void)
{
}

/**************************************************************************
 * make_cs1_contiguous() - for es2 and above remap cs1 behind cs0 to allow
 *  command line mem=xyz use all memory with out discontigious support
 *  compiled in.  Could do it at the ATAG, but there really is two banks...
 * Called as part of 2nd phase DDR init.
 **************************************************************************/
void make_cs1_contiguous(void)
{
	u32 size, a_add_low, a_add_high;

	size = get_sdr_cs_size(SDRC_CS0_OSET);
	size /= SZ_32M;		/* find size to offset CS1 */
	a_add_high = (size & 3) << 8;	/* set up low field */
	a_add_low = (size & 0x3C) >> 2;	/* set up high field */
	__raw_writel((a_add_high | a_add_low), SDRC_CS_CFG);
}

/********************************************************
 *  mem_ok() - test used to see if timings are correct
 *             for a part. Helps in gussing which part
 *             we are currently using.
 *******************************************************/
u32 mem_ok(void)
{
	u32 val1, val2;
	u32 pattern = 0x12345678;

	__raw_writel(0x0, OMAP2420_SDRC_CS0 + 0x400);	/* clear pos A */
	__raw_writel(pattern, OMAP2420_SDRC_CS0);	/* pattern to pos B */
	__raw_writel(0x0, OMAP2420_SDRC_CS0 + 4);	/* remove pattern off the bus */
	val1 = __raw_readl(OMAP2420_SDRC_CS0 + 0x400);	/* get pos A value */
	val2 = __raw_readl(OMAP2420_SDRC_CS0);	/* get val2 */

	if ((val1 != 0) || (val2 != pattern))	/* see if pos A value changed */
		return (0);
	else
		return (1);
}

/********************************************************
 *  sdrc_init() - init the sdrc chip selects CS0 and CS1
 *  - early init routines, called from flash or
 *  SRAM.
 *******************************************************/
void sdrc_init(void)
{
#define EARLY_INIT 1
	/* only init up first bank here */
	do_sdrc_init(SDRC_CS0_OSET, EARLY_INIT);
}

/*************************************************************************
 * do_sdrc_init(): initialize the SDRAM for use.
 *  -called from low level code with stack only.
 *  -code sets up SDRAM timing and muxing for 2422 or 2420.
 *  -optimal settings can be placed here, or redone after i2c
 *      inspection of board info
 *
 *  This is a bit ugly, but should handle all memory moduels
 *   used with the APOLLON. The first time though this code from s_init()
 *   we configure the first chip select.  Later on we come back and
 *   will configure the 2nd chip select if it exists.
 *
 **************************************************************************/
void do_sdrc_init(u32 offset, u32 early)
{
}

/*****************************************************
 * gpmc_init(): init gpmc bus
 * Init GPMC for x16, MuxMode (SDRAM in x32).
 * This code can only be executed from SRAM or SDRAM.
 *****************************************************/
void gpmc_init(void)
{
	u32 mux = 0, mtype, mwidth, rev, tval;

	rev = get_cpu_rev();
	if (rev == CPU_2420_2422_ES1)
		tval = 1;
	else
		tval = 0;	/* disable bit switched meaning */

	/* global settings */
	__raw_writel(0x10, GPMC_SYSCONFIG);	/* smart idle */
	__raw_writel(0x0, GPMC_IRQENABLE);	/* isr's sources masked */
	__raw_writel(tval, GPMC_TIMEOUT_CONTROL);	/* timeout disable */
#ifdef CFG_NAND_BOOT
	__raw_writel(0x001, GPMC_CONFIG);	/* set nWP, disable limited addr */
#else
	__raw_writel(0x111, GPMC_CONFIG);	/* set nWP, disable limited addr */
#endif

	/* discover bus connection from sysboot */
	if (is_gpmc_muxed() == GPMC_MUXED)
		mux = BIT9;
	mtype = get_gpmc0_type();
	mwidth = get_gpmc0_width();

	/* setup cs0 */
	__raw_writel(0x0, GPMC_CONFIG7_0);	/* disable current map */
	sdelay(1000);

#ifdef CFG_NOR_BOOT
	__raw_writel(APOLLON_24XX_GPMC_CONFIG1_3, GPMC_CONFIG1_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG2_3, GPMC_CONFIG2_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG3_3, GPMC_CONFIG3_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG4_3, GPMC_CONFIG4_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG5_3, GPMC_CONFIG5_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG6_3, GPMC_CONFIG6_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG7_3, GPMC_CONFIG7_0); #else
	__raw_writel(APOLLON_24XX_GPMC_CONFIG1_0 | mux | mtype | mwidth,
		     GPMC_CONFIG1_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG2_0, GPMC_CONFIG2_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG3_0, GPMC_CONFIG3_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG4_0, GPMC_CONFIG4_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG5_0, GPMC_CONFIG5_0);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG6_0, GPMC_CONFIG6_0);
	/* enable new mapping */
	__raw_writel(APOLLON_24XX_GPMC_CONFIG7_0, GPMC_CONFIG7_0);
#endif
	sdelay(2000);

	/* setup cs1 */
	__raw_writel(0, GPMC_CONFIG7_1);	/* disable any mapping */
	sdelay(1000);

	__raw_writel(APOLLON_24XX_GPMC_CONFIG1_1, GPMC_CONFIG1_1);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG2_1, GPMC_CONFIG2_1);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG3_1, GPMC_CONFIG3_1);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG4_1, GPMC_CONFIG4_1);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG5_1, GPMC_CONFIG5_1);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG6_1, GPMC_CONFIG6_1);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG7_1, GPMC_CONFIG7_1);
	sdelay(2000);

	/* setup cs2 */
	__raw_writel(APOLLON_24XX_GPMC_CONFIG1_0 | mux | mtype | mwidth,
		     GPMC_CONFIG1_2);
	/* It's same as cs 0 */
	__raw_writel(APOLLON_24XX_GPMC_CONFIG2_0, GPMC_CONFIG2_2);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG3_0, GPMC_CONFIG3_2);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG4_0, GPMC_CONFIG4_2);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG5_0, GPMC_CONFIG5_2);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG6_0, GPMC_CONFIG6_2);
#ifdef CFG_NOR_BOOT
	__raw_writel(APOLLON_24XX_GPMC_CONFIG7_0, GPMC_CONFIG7_2);
#else
	__raw_writel(APOLLON_24XX_GPMC_CONFIG7_2, GPMC_CONFIG7_2);
#endif

#ifndef CFG_NOR_BOOT
	/* setup cs3 */
	__raw_writel(0, GPMC_CONFIG7_3);	/* disable any mapping */
	sdelay(1000);

	__raw_writel(APOLLON_24XX_GPMC_CONFIG1_3, GPMC_CONFIG1_3);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG2_3, GPMC_CONFIG2_3);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG3_3, GPMC_CONFIG3_3);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG4_3, GPMC_CONFIG4_3);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG5_3, GPMC_CONFIG5_3);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG6_3, GPMC_CONFIG6_3);
	__raw_writel(APOLLON_24XX_GPMC_CONFIG7_3, GPMC_CONFIG7_3);
#endif

#ifndef ASYNC_NOR
	__raw_writew(0xaa, (APOLLON_CS3_BASE + 0xaaa));
	__raw_writew(0x55, (APOLLON_CS3_BASE + 0x554));
	__raw_writew(0xc0, (APOLLON_CS3_BASE | SYNC_NOR_VALUE));
#endif
	sdelay(2000);
}

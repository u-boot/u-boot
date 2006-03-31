/*
 * Copyright Freescale Semiconductor, Inc.
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
 * Change log:
 * 20050101: Eran Liberty (liberty@freescale.com)
 *           Initial file creating (porting from 85XX & 8260)
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <asm/mpc8349_pci.h>
#include <i2c.h>
#include <spd.h>
#include <miiphy.h>
#if defined(CONFIG_PCI)
#include <pci.h>
#endif
#if defined(CONFIG_SPD_EEPROM)
#include <spd_sdram.h>
#endif
int fixed_sdram(void);
void sdram_init(void);

int board_early_init_f (void)
{
	volatile u8* bcsr = (volatile u8*)CFG_BCSR;

	/* Enable flash write */
	bcsr[1] &= ~0x01;

	return 0;
}


#define ns2clk(ns) (ns / (1000000000 / CONFIG_8349_CLKIN) + 1)

long int initdram (int board_type)
{
	volatile immap_t *im = (immap_t *)CFG_IMMRBAR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CFG_DDR_BASE & LAWBAR_BAR;
#if defined(CONFIG_SPD_EEPROM)
	msize = spd_sdram();
#else
	msize = fixed_sdram();
#endif
	/*
	 * Initialize SDRAM if it is on local bus.
	 */
	sdram_init();
	puts("   DDR RAM: ");
	/* return total bus SDRAM size(bytes)  -- DDR */
	return (msize * 1024 * 1024);
}


#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *)CFG_IMMRBAR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	msize = CFG_DDR_SIZE;
	for (ddr_size = msize << 20, ddr_size_log2 = 0;
	     (ddr_size > 1);
	     ddr_size = ddr_size>>1, ddr_size_log2++) {
		if (ddr_size & 1) {
			return -1;
		}
	}
	im->sysconf.ddrlaw[0].ar = LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);
#if (CFG_DDR_SIZE != 256)
#warning Currenly any ddr size other than 256 is not supported
#endif

	im->ddr.csbnds[0].csbnds = 0x00100017;
	im->ddr.csbnds[1].csbnds = 0x0018001f;
	im->ddr.csbnds[2].csbnds = 0x00000007;
	im->ddr.csbnds[3].csbnds = 0x0008000f;
	im->ddr.cs_config[0] = CFG_DDR_CONFIG;
	im->ddr.cs_config[1] = CFG_DDR_CONFIG;
	im->ddr.cs_config[2] = CFG_DDR_CONFIG;
	im->ddr.cs_config[3] = CFG_DDR_CONFIG;
	im->ddr.timing_cfg_1 =
		3 << TIMING_CFG1_PRETOACT_SHIFT |
		7 << TIMING_CFG1_ACTTOPRE_SHIFT |
		3 << TIMING_CFG1_ACTTORW_SHIFT  |
		4 << TIMING_CFG1_CASLAT_SHIFT   |
		3 << TIMING_CFG1_REFREC_SHIFT   |
		3 << TIMING_CFG1_WRREC_SHIFT    |
		2 << TIMING_CFG1_ACTTOACT_SHIFT |
		1 << TIMING_CFG1_WRTORD_SHIFT;
	im->ddr.timing_cfg_2 = 2 << TIMING_CFG2_WR_DATA_DELAY_SHIFT;
	im->ddr.sdram_cfg =
		SDRAM_CFG_SREN
#if defined(CONFIG_DDR_2T_TIMING)
		| SDRAM_CFG_2T_EN
#endif
		| 2 << SDRAM_CFG_SDRAM_TYPE_SHIFT;
	im->ddr.sdram_mode =
		0x2000 << SDRAM_MODE_ESD_SHIFT |
		0x0162 << SDRAM_MODE_SD_SHIFT;

	im->ddr.sdram_interval = 0x045B << SDRAM_INTERVAL_REFINT_SHIFT |
		0x0100 << SDRAM_INTERVAL_BSTOPRE_SHIFT;
	udelay(200);

	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	return msize;
}
#endif/*!CFG_SPD_EEPROM*/


int checkboard (void)
{
	puts("Board: Freescale MPC8349ADS\n");
	return 0;
}

/*
 * if MPC8349ADS is soldered with SDRAM
 */
#if defined(CFG_BR2_PRELIM)  \
	&& defined(CFG_OR2_PRELIM) \
	&& defined(CFG_LBLAWBAR2_PRELIM) \
	&& defined(CFG_LBLAWAR2_PRELIM)
/*
 * Initialize SDRAM memory on the Local Bus.
 */

void
sdram_init(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMRBAR;
	volatile lbus8349_t *lbc= &immap->lbus;
	uint *sdram_addr = (uint *)CFG_LBC_SDRAM_BASE;

	puts("\n   SDRAM on Local Bus: ");
	print_size (CFG_LBC_SDRAM_SIZE * 1024 * 1024, "\n");

	/*
	 * Setup SDRAM Base and Option Registers, already done in cpu_init.c
	 */

	/*setup mtrpt, lsrt and lbcr for LB bus*/
	lbc->lbcr = CFG_LBC_LBCR;
	lbc->mrtpr = CFG_LBC_MRTPR;
	lbc->lsrt = CFG_LBC_LSRT;
	asm("sync");

	/*
	 * Configure the SDRAM controller Machine Mode Register.
	 */
	lbc->lsdmr = CFG_LBC_LSDMR_5; /* 0x40636733; normal operation*/

	lbc->lsdmr = CFG_LBC_LSDMR_1; /*0x68636733;precharge all the banks*/
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	lbc->lsdmr = CFG_LBC_LSDMR_2;/*0x48636733;auto refresh*/
	asm("sync");
	/*1 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*2 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*3 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*4 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*5 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*6 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*7 times*/
	*sdram_addr = 0xff;
	udelay(100);
	/*8 times*/
	*sdram_addr = 0xff;
	udelay(100);

	/* 0x58636733;mode register write operation */
	lbc->lsdmr = CFG_LBC_LSDMR_4;
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	lbc->lsdmr = CFG_LBC_LSDMR_5; /*0x40636733;normal operation*/
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);
}
#else
void
sdram_init(void)
{
	put("SDRAM on Local Bus is NOT available!\n");
}
#endif

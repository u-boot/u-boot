/*
 * sbc8349.c -- WindRiver SBC8349 board support.
 * Copyright (c) 2006-2007 Wind River Systems, Inc.
 *
 * Paul Gortmaker <paul.gortmaker@windriver.com>
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
#include <i2c.h>
#include <spd.h>
#include <miiphy.h>
#if defined(CONFIG_SPD_EEPROM)
#include <spd_sdram.h>
#endif
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif

int fixed_sdram(void);
void sdram_init(void);

#if defined(CONFIG_DDR_ECC) && defined(CONFIG_MPC83XX)
void ddr_enable_ecc(unsigned int dram_size);
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f (void)
{
	return 0;
}
#endif

#define ns2clk(ns) (ns / (1000000000 / CONFIG_8349_CLKIN) + 1)

long int initdram (int board_type)
{
	volatile immap_t *im = (immap_t *)CFG_IMMR;
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

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif
	/* return total bus SDRAM size(bytes)  -- DDR */
	return (msize * 1024 * 1024);
}

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *)CFG_IMMR;
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
	im->sysconf.ddrlaw[0].bar = ((CFG_DDR_SDRAM_BASE>>12) & 0xfffff);
	im->sysconf.ddrlaw[0].ar = LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);

#if (CFG_DDR_SIZE != 256)
#warning Currently any ddr size other than 256 is not supported
#endif
	im->ddr.csbnds[2].csbnds = 0x0000000f;
	im->ddr.cs_config[2] = CFG_DDR_CONFIG;

	/* currently we use only one CS, so disable the other banks */
	im->ddr.cs_config[0] = 0;
	im->ddr.cs_config[1] = 0;
	im->ddr.cs_config[3] = 0;

	im->ddr.timing_cfg_1 = CFG_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CFG_DDR_TIMING_2;

	im->ddr.sdram_cfg =
		SDRAM_CFG_SREN
#if defined(CONFIG_DDR_2T_TIMING)
		| SDRAM_CFG_2T_EN
#endif
		| SDRAM_CFG_SDRAM_TYPE_DDR1;
#if defined (CONFIG_DDR_32BIT)
	/* for 32-bit mode burst length is 8 */
	im->ddr.sdram_cfg |= (SDRAM_CFG_32_BE | SDRAM_CFG_8_BE);
#endif
	im->ddr.sdram_mode = CFG_DDR_MODE;

	im->ddr.sdram_interval = CFG_DDR_INTERVAL;
	udelay(200);

	/* enable DDR controller */
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;
	return msize;
}
#endif/*!CFG_SPD_EEPROM*/


int checkboard (void)
{
	puts("Board: Wind River SBC834x\n");
	return 0;
}

/*
 * if board is fitted with SDRAM
 */
#if defined(CFG_BR2_PRELIM)  \
	&& defined(CFG_OR2_PRELIM) \
	&& defined(CFG_LBLAWBAR2_PRELIM) \
	&& defined(CFG_LBLAWAR2_PRELIM)
/*
 * Initialize SDRAM memory on the Local Bus.
 */

void sdram_init(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile lbus83xx_t *lbc= &immap->lbus;
	uint *sdram_addr = (uint *)CFG_LBC_SDRAM_BASE;

	puts("\n   SDRAM on Local Bus: ");
	print_size (CFG_LBC_SDRAM_SIZE * 1024 * 1024, "\n");

	/*
	 * Setup SDRAM Base and Option Registers, already done in cpu_init.c
	 */

	/* setup mtrpt, lsrt and lbcr for LB bus */
	lbc->lbcr = CFG_LBC_LBCR;
	lbc->mrtpr = CFG_LBC_MRTPR;
	lbc->lsrt = CFG_LBC_LSRT;
	asm("sync");

	/*
	 * Configure the SDRAM controller Machine Mode Register.
	 */
	lbc->lsdmr = CFG_LBC_LSDMR_5; /* 0x40636733; normal operation */

	lbc->lsdmr = CFG_LBC_LSDMR_1; /* 0x68636733; precharge all the banks */
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	lbc->lsdmr = CFG_LBC_LSDMR_2; /* 0x48636733; auto refresh */
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

	/* 0x58636733; mode register write operation */
	lbc->lsdmr = CFG_LBC_LSDMR_4;
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	lbc->lsdmr = CFG_LBC_LSDMR_5; /* 0x40636733; normal operation */
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);
}
#else
void sdram_init(void)
{
	puts("   SDRAM on Local Bus: Disabled in config\n");
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

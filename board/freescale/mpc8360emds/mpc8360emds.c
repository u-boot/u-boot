/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 * Dave Liu <daveliu@freescale.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <i2c.h>
#include <spd.h>
#include <miiphy.h>
#if defined(CONFIG_PCI)
#include <pci.h>
#endif
#if defined(CONFIG_SPD_EEPROM)
#include <spd_sdram.h>
#else
#include <asm/mmu.h>
#endif
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif
#if defined(CONFIG_PQ_MDS_PIB)
#include "../common/pq-mds-pib.h"
#endif

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* GETH1 */
	{0,  3, 1, 0, 1}, /* TxD0 */
	{0,  4, 1, 0, 1}, /* TxD1 */
	{0,  5, 1, 0, 1}, /* TxD2 */
	{0,  6, 1, 0, 1}, /* TxD3 */
	{1,  6, 1, 0, 3}, /* TxD4 */
	{1,  7, 1, 0, 1}, /* TxD5 */
	{1,  9, 1, 0, 2}, /* TxD6 */
	{1, 10, 1, 0, 2}, /* TxD7 */
	{0,  9, 2, 0, 1}, /* RxD0 */
	{0, 10, 2, 0, 1}, /* RxD1 */
	{0, 11, 2, 0, 1}, /* RxD2 */
	{0, 12, 2, 0, 1}, /* RxD3 */
	{0, 13, 2, 0, 1}, /* RxD4 */
	{1,  1, 2, 0, 2}, /* RxD5 */
	{1,  0, 2, 0, 2}, /* RxD6 */
	{1,  4, 2, 0, 2}, /* RxD7 */
	{0,  7, 1, 0, 1}, /* TX_EN */
	{0,  8, 1, 0, 1}, /* TX_ER */
	{0, 15, 2, 0, 1}, /* RX_DV */
	{0, 16, 2, 0, 1}, /* RX_ER */
	{0,  0, 2, 0, 1}, /* RX_CLK */
	{2,  9, 1, 0, 3}, /* GTX_CLK - CLK10 */
	{2,  8, 2, 0, 1}, /* GTX125 - CLK9 */
	/* GETH2 */
	{0, 17, 1, 0, 1}, /* TxD0 */
	{0, 18, 1, 0, 1}, /* TxD1 */
	{0, 19, 1, 0, 1}, /* TxD2 */
	{0, 20, 1, 0, 1}, /* TxD3 */
	{1,  2, 1, 0, 1}, /* TxD4 */
	{1,  3, 1, 0, 2}, /* TxD5 */
	{1,  5, 1, 0, 3}, /* TxD6 */
	{1,  8, 1, 0, 3}, /* TxD7 */
	{0, 23, 2, 0, 1}, /* RxD0 */
	{0, 24, 2, 0, 1}, /* RxD1 */
	{0, 25, 2, 0, 1}, /* RxD2 */
	{0, 26, 2, 0, 1}, /* RxD3 */
	{0, 27, 2, 0, 1}, /* RxD4 */
	{1, 12, 2, 0, 2}, /* RxD5 */
	{1, 13, 2, 0, 3}, /* RxD6 */
	{1, 11, 2, 0, 2}, /* RxD7 */
	{0, 21, 1, 0, 1}, /* TX_EN */
	{0, 22, 1, 0, 1}, /* TX_ER */
	{0, 29, 2, 0, 1}, /* RX_DV */
	{0, 30, 2, 0, 1}, /* RX_ER */
	{0, 31, 2, 0, 1}, /* RX_CLK */
	{2,  2, 1, 0, 2}, /* GTX_CLK = CLK10 */
	{2,  3, 2, 0, 1}, /* GTX125 - CLK4 */

	{0,  1, 3, 0, 2}, /* MDIO */
	{0,  2, 1, 0, 1}, /* MDC */

	{5,  0, 1, 0, 2}, /* UART2_SOUT */
	{5,  1, 2, 0, 3}, /* UART2_CTS */
	{5,  2, 1, 0, 1}, /* UART2_RTS */
	{5,  3, 2, 0, 2}, /* UART2_SIN */

	{0,  0, 0, 0, QE_IOP_TAB_END}, /* END of table */
};

int board_early_init_f(void)
{

	u8 *bcsr = (u8 *)CFG_BCSR;
	const immap_t *immr = (immap_t *)CFG_IMMR;

	/* Enable flash write */
	bcsr[0xa] &= ~0x04;

	/* Disable G1TXCLK, G2TXCLK h/w buffers (rev.2 h/w bug workaround) */
	if (immr->sysconf.spridr == SPR_8360_REV20 ||
	    immr->sysconf.spridr == SPR_8360E_REV20 ||
	    immr->sysconf.spridr == SPR_8360_REV21 ||
	    immr->sysconf.spridr == SPR_8360E_REV21)
		bcsr[0xe] = 0x30;

	/* Enable second UART */
	bcsr[0x9] &= ~0x01;

	return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_PQ_MDS_PIB
	pib_init();
#endif
	return 0;
}

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRC)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif
int fixed_sdram(void);
void sdram_init(void);

long int initdram(int board_type)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CFG_DDR_BASE & LAWBAR_BAR;
#if defined(CONFIG_SPD_EEPROM)
	msize = spd_sdram();
#else
	msize = fixed_sdram();
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRC)
	/*
	 * Initialize DDR ECC byte
	 */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif
	/*
	 * Initialize SDRAM if it is on local bus.
	 */
	sdram_init();

	/* return total bus SDRAM size(bytes)  -- DDR */
	return (msize * 1024 * 1024);
}

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	msize = CFG_DDR_SIZE;
	for (ddr_size = msize << 20, ddr_size_log2 = 0;
	     (ddr_size > 1); ddr_size = ddr_size >> 1, ddr_size_log2++) {
		if (ddr_size & 1) {
			return -1;
		}
	}
	im->sysconf.ddrlaw[0].ar =
	    LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);
#if (CFG_DDR_SIZE != 256)
#warning Currenly any ddr size other than 256 is not supported
#endif
#ifdef CONFIG_DDR_II
	im->ddr.csbnds[0].csbnds = CFG_DDR_CS0_BNDS;
	im->ddr.cs_config[0] = CFG_DDR_CS0_CONFIG;
	im->ddr.timing_cfg_0 = CFG_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CFG_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CFG_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CFG_DDR_TIMING_3;
	im->ddr.sdram_cfg = CFG_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CFG_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CFG_DDR_MODE;
	im->ddr.sdram_mode2 = CFG_DDR_MODE2;
	im->ddr.sdram_interval = CFG_DDR_INTERVAL;
	im->ddr.sdram_clk_cntl = CFG_DDR_CLK_CNTL;
#else
	im->ddr.csbnds[0].csbnds = 0x00000007;
	im->ddr.csbnds[1].csbnds = 0x0008000f;

	im->ddr.cs_config[0] = CFG_DDR_CONFIG;
	im->ddr.cs_config[1] = CFG_DDR_CONFIG;

	im->ddr.timing_cfg_1 = CFG_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CFG_DDR_TIMING_2;
	im->ddr.sdram_cfg = CFG_DDR_CONTROL;

	im->ddr.sdram_mode = CFG_DDR_MODE;
	im->ddr.sdram_interval = CFG_DDR_INTERVAL;
#endif
	udelay(200);
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	return msize;
}
#endif				/*!CFG_SPD_EEPROM */

int checkboard(void)
{
	puts("Board: Freescale MPC8360EMDS\n");
	return 0;
}

/*
 * if MPC8360EMDS is soldered with SDRAM
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
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile lbus83xx_t *lbc = &immap->lbus;
	uint *sdram_addr = (uint *) CFG_LBC_SDRAM_BASE;

	/*
	 * Setup SDRAM Base and Option Registers, already done in cpu_init.c
	 */
	/*setup mtrpt, lsrt and lbcr for LB bus */
	lbc->lbcr = CFG_LBC_LBCR;
	lbc->mrtpr = CFG_LBC_MRTPR;
	lbc->lsrt = CFG_LBC_LSRT;
	asm("sync");

	/*
	 * Configure the SDRAM controller Machine Mode Register.
	 */
	lbc->lsdmr = CFG_LBC_LSDMR_5;	/* Normal Operation */
	lbc->lsdmr = CFG_LBC_LSDMR_1;	/* Precharge All Banks */
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	/*
	 * We need do 8 times auto refresh operation.
	 */
	lbc->lsdmr = CFG_LBC_LSDMR_2;
	asm("sync");
	*sdram_addr = 0xff;	/* 1 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 2 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 3 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 4 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 5 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 6 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 7 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 8 times */
	udelay(100);

	/* Mode register write operation */
	lbc->lsdmr = CFG_LBC_LSDMR_4;
	asm("sync");
	*(sdram_addr + 0xcc) = 0xff;
	udelay(100);

	/* Normal operation */
	lbc->lsdmr = CFG_LBC_LSDMR_5 | 0x40000000;
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);
}
#else
void sdram_init(void)
{
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	const immap_t *immr = (immap_t *)CFG_IMMR;

	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
	/*
	 * mpc8360ea pb mds errata 2: RGMII timing
	 * if on mpc8360ea rev. 2.1,
	 * change both ucc phy-connection-types from rgmii-id to rgmii-rxid
	 */
	if (immr->sysconf.spridr == SPR_8360_REV21 ||
	    immr->sysconf.spridr == SPR_8360E_REV21) {
		int nodeoffset;
		const char *prop;
		const char *path;

		nodeoffset = fdt_path_offset(fdt, "/aliases");
		if (nodeoffset >= 0) {
#if defined(CONFIG_HAS_ETH0)
			/* fixup UCC 1 if using rgmii-id mode */
			path = fdt_getprop(blob, nodeoffset, "ethernet0", NULL);
			if (path) {
				prop = fdt_getprop(blob, nodeoffset,
							"phy-connection-type", 0);
				if (prop && (strcmp(prop, "rgmii-id") == 0))
					fdt_setprop(blob, nodeoffset, "phy-connection-type",
						    "rgmii-rxid", sizeof("rgmii-rxid"));
			}
#endif
#if defined(CONFIG_HAS_ETH1)
			/* fixup UCC 2 if using rgmii-id mode */
			path = fdt_getprop(blob, nodeoffset, "ethernet1", NULL);
			if (path) {
				prop = fdt_getprop(blob, nodeoffset,
							"phy-connection-type", 0);
				if (prop && (strcmp(prop, "rgmii-id") == 0))
					fdt_setprop(blob, nodeoffset, "phy-connection-type",
						    "rgmii-rxid", sizeof("rgmii-rxid"));
			}
#endif
		}
	}
}
#endif

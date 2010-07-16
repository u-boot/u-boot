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
#include <miiphy.h>
#if defined(CONFIG_PCI)
#include <pci.h>
#endif
#include <spd_sdram.h>
#include <asm/mmu.h>
#include <asm/io.h>
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif
#include <hwconfig.h>
#include <fdt_support.h>
#if defined(CONFIG_PQ_MDS_PIB)
#include "../common/pq-mds-pib.h"
#endif
#include "../../../drivers/qe/uec.h"

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

/* Handle "mpc8360ea rev.2.1 erratum 2: RGMII Timing"? */
static int board_handle_erratum2(void)
{
	const immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	return REVID_MAJOR(immr->sysconf.spridr) == 2 &&
	       REVID_MINOR(immr->sysconf.spridr) == 1;
}

int board_early_init_f(void)
{
	const immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	u8 *bcsr = (u8 *)CONFIG_SYS_BCSR;

	/* Enable flash write */
	bcsr[0xa] &= ~0x04;

	/* Disable G1TXCLK, G2TXCLK h/w buffers (rev.2.x h/w bug workaround) */
	if (REVID_MAJOR(immr->sysconf.spridr) == 2)
		bcsr[0xe] = 0x30;

	/* Enable second UART */
	bcsr[0x9] &= ~0x01;

	if (board_handle_erratum2()) {
		void *immap = (immap_t *)(CONFIG_SYS_IMMR + 0x14a8);

		/*
		 * IMMR + 0x14A8[4:5] = 11 (clk delay for UCC 2)
		 * IMMR + 0x14A8[18:19] = 11 (clk delay for UCC 1)
		 */
		setbits_be32(immap, 0x0c003000);

		/*
		 * IMMR + 0x14AC[20:27] = 10101010
		 * (data delay for both UCC's)
		 */
		clrsetbits_be32(immap + 4, 0xff0, 0xaa0);
	}
	return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_PQ_MDS_PIB
	pib_init();
#endif
	return 0;
}

#ifdef CONFIG_UEC_ETH
static uec_info_t uec_info[] = {
#ifdef CONFIG_UEC_ETH1
	STD_UEC_INFO(1),
#endif
#ifdef CONFIG_UEC_ETH2
	STD_UEC_INFO(2),
#endif
};

int board_eth_init(bd_t *bd)
{
	if (board_handle_erratum2()) {
		int i;

		for (i = 0; i < ARRAY_SIZE(uec_info); i++)
			uec_info[i].enet_interface_type = RGMII_RXID;
			uec_info[i].speed = 1000;
	}
	return uec_eth_init(bd, uec_info, ARRAY_SIZE(uec_info));
}
#endif /* CONFIG_UEC_ETH */

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif
int fixed_sdram(void);
static int sdram_init(unsigned int base);

phys_size_t initdram(int board_type)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msize = 0;
	u32 lbc_sdram_size;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_DDR_BASE & LAWBAR_BAR;
#if defined(CONFIG_SPD_EEPROM)
	msize = spd_sdram();
#else
	msize = fixed_sdram();
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize DDR ECC byte
	 */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif
	/*
	 * Initialize SDRAM if it is on local bus.
	 */
	lbc_sdram_size = sdram_init(msize * 1024 * 1024);
	if (!msize)
		msize = lbc_sdram_size;

	/* return total bus SDRAM size(bytes)  -- DDR */
	return (msize * 1024 * 1024);
}

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	msize = CONFIG_SYS_DDR_SIZE;
	for (ddr_size = msize << 20, ddr_size_log2 = 0;
	     (ddr_size > 1); ddr_size = ddr_size >> 1, ddr_size_log2++) {
		if (ddr_size & 1) {
			return -1;
		}
	}
	im->sysconf.ddrlaw[0].ar =
	    LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);
#if (CONFIG_SYS_DDR_SIZE != 256)
#warning Currenly any ddr size other than 256 is not supported
#endif
#ifdef CONFIG_DDR_II
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
	im->ddr.sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CNTL;
#else
	im->ddr.csbnds[0].csbnds = 0x00000007;
	im->ddr.csbnds[1].csbnds = 0x0008000f;

	im->ddr.cs_config[0] = CONFIG_SYS_DDR_CONFIG;
	im->ddr.cs_config[1] = CONFIG_SYS_DDR_CONFIG;

	im->ddr.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	im->ddr.sdram_cfg = CONFIG_SYS_DDR_CONTROL;

	im->ddr.sdram_mode = CONFIG_SYS_DDR_MODE;
	im->ddr.sdram_interval = CONFIG_SYS_DDR_INTERVAL;
#endif
	udelay(200);
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	return msize;
}
#endif				/*!CONFIG_SYS_SPD_EEPROM */

int checkboard(void)
{
	puts("Board: Freescale MPC8360EMDS\n");
	return 0;
}

/*
 * if MPC8360EMDS is soldered with SDRAM
 */
#ifdef CONFIG_SYS_LB_SDRAM
/*
 * Initialize SDRAM memory on the Local Bus.
 */

static int sdram_init(unsigned int base)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	fsl_lbc_t *lbc = LBC_BASE_ADDR;
	const int sdram_size = CONFIG_SYS_LBC_SDRAM_SIZE * 1024 * 1024;
	int rem = base % sdram_size;
	uint *sdram_addr;

	/* window base address should be aligned to the window size */
	if (rem)
		base = base - rem + sdram_size;

	sdram_addr = (uint *)base;
	/*
	 * Setup SDRAM Base and Option Registers
	 */
	set_lbc_br(2, base | CONFIG_SYS_BR2);
	set_lbc_or(2, CONFIG_SYS_OR2);
	immap->sysconf.lblaw[2].bar = base;
	immap->sysconf.lblaw[2].ar = CONFIG_SYS_LBLAWAR2;

	/*setup mtrpt, lsrt and lbcr for LB bus */
	lbc->lbcr = CONFIG_SYS_LBC_LBCR;
	lbc->mrtpr = CONFIG_SYS_LBC_MRTPR;
	lbc->lsrt = CONFIG_SYS_LBC_LSRT;
	asm("sync");

	/*
	 * Configure the SDRAM controller Machine Mode Register.
	 */
	lbc->lsdmr = CONFIG_SYS_LBC_LSDMR_5;	/* Normal Operation */
	lbc->lsdmr = CONFIG_SYS_LBC_LSDMR_1;	/* Precharge All Banks */
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	/*
	 * We need do 8 times auto refresh operation.
	 */
	lbc->lsdmr = CONFIG_SYS_LBC_LSDMR_2;
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
	lbc->lsdmr = CONFIG_SYS_LBC_LSDMR_4;
	asm("sync");
	*(sdram_addr + 0xcc) = 0xff;
	udelay(100);

	/* Normal operation */
	lbc->lsdmr = CONFIG_SYS_LBC_LSDMR_5 | 0x40000000;
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	/*
	 * In non-aligned case we don't [normally] use that memory because
	 * there is a hole.
	 */
	if (rem)
		return 0;
	return CONFIG_SYS_LBC_SDRAM_SIZE;
}
#else
static int sdram_init(unsigned int base) { return 0; }
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
static void ft_board_fixup_qe_usb(void *blob, bd_t *bd)
{
	if (!hwconfig_subarg_cmp("qe_usb", "mode", "peripheral"))
		return;

	do_fixup_by_compat(blob, "fsl,mpc8323-qe-usb", "mode",
			   "peripheral", sizeof("peripheral"), 1);
}

void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
	ft_board_fixup_qe_usb(blob, bd);
	/*
	 * mpc8360ea pb mds errata 2: RGMII timing
	 * if on mpc8360ea rev. 2.1,
	 * change both ucc phy-connection-types from rgmii-id to rgmii-rxid
	 */
	if (board_handle_erratum2()) {
		int nodeoffset;
		const char *prop;
		int path;

		nodeoffset = fdt_path_offset(blob, "/aliases");
		if (nodeoffset >= 0) {
#if defined(CONFIG_HAS_ETH0)
			/* fixup UCC 1 if using rgmii-id mode */
			prop = fdt_getprop(blob, nodeoffset, "ethernet0", NULL);
			if (prop) {
				path = fdt_path_offset(blob, prop);
				prop = fdt_getprop(blob, path,
				                   "phy-connection-type", 0);
				if (prop && (strcmp(prop, "rgmii-id") == 0))
					fdt_setprop(blob, path,
					            "phy-connection-type",
					            "rgmii-rxid",
					            sizeof("rgmii-rxid"));
			}
#endif
#if defined(CONFIG_HAS_ETH1)
			/* fixup UCC 2 if using rgmii-id mode */
			prop = fdt_getprop(blob, nodeoffset, "ethernet1", NULL);
			if (prop) {
				path = fdt_path_offset(blob, prop);
				prop = fdt_getprop(blob, path,
				                   "phy-connection-type", 0);
				if (prop && (strcmp(prop, "rgmii-id") == 0))
					fdt_setprop(blob, path,
					            "phy-connection-type",
					            "rgmii-rxid",
					            sizeof("rgmii-rxid"));
			}
#endif
		}
	}
}
#endif

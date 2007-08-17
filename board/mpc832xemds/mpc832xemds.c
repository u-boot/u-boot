/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *
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
#include <command.h>
#if defined(CONFIG_PCI)
#include <pci.h>
#endif
#if defined(CONFIG_SPD_EEPROM)
#include <spd_sdram.h>
#else
#include <asm/mmu.h>
#endif
#if defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
#elif defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif
#if defined(CONFIG_PQ_MDS_PIB)
#include "../freescale/common/pq-mds-pib.h"
#endif

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* ETH3 */
	{1,  0, 1, 0, 1}, /* TxD0 */
	{1,  1, 1, 0, 1}, /* TxD1 */
	{1,  2, 1, 0, 1}, /* TxD2 */
	{1,  3, 1, 0, 1}, /* TxD3 */
	{1,  9, 1, 0, 1}, /* TxER */
	{1, 12, 1, 0, 1}, /* TxEN */
	{3, 24, 2, 0, 1}, /* TxCLK->CLK10 */

	{1,  4, 2, 0, 1}, /* RxD0 */
	{1,  5, 2, 0, 1}, /* RxD1 */
	{1,  6, 2, 0, 1}, /* RxD2 */
	{1,  7, 2, 0, 1}, /* RxD3 */
	{1,  8, 2, 0, 1}, /* RxER */
	{1, 10, 2, 0, 1}, /* RxDV */
	{0, 13, 2, 0, 1}, /* RxCLK->CLK9 */
	{1, 11, 2, 0, 1}, /* COL */
	{1, 13, 2, 0, 1}, /* CRS */

	/* ETH4 */
	{1, 18, 1, 0, 1}, /* TxD0 */
	{1, 19, 1, 0, 1}, /* TxD1 */
	{1, 20, 1, 0, 1}, /* TxD2 */
	{1, 21, 1, 0, 1}, /* TxD3 */
	{1, 27, 1, 0, 1}, /* TxER */
	{1, 30, 1, 0, 1}, /* TxEN */
	{3,  6, 2, 0, 1}, /* TxCLK->CLK8 */

	{1, 22, 2, 0, 1}, /* RxD0 */
	{1, 23, 2, 0, 1}, /* RxD1 */
	{1, 24, 2, 0, 1}, /* RxD2 */
	{1, 25, 2, 0, 1}, /* RxD3 */
	{1, 26, 1, 0, 1}, /* RxER */
	{1, 28, 2, 0, 1}, /* Rx_DV */
	{3, 31, 2, 0, 1}, /* RxCLK->CLK7 */
	{1, 29, 2, 0, 1}, /* COL */
	{1, 31, 2, 0, 1}, /* CRS */

	{3,  4, 3, 0, 2}, /* MDIO */
	{3,  5, 1, 0, 2}, /* MDC */

	{0,  0, 0, 0, QE_IOP_TAB_END}, /* END of table */
};

int board_early_init_f(void)
{
	volatile u8 *bcsr = (volatile u8 *)CFG_BCSR;

	/* Enable flash write */
	bcsr[9] &= ~0x08;

	return 0;
}

int board_early_init_r(void)
{
#ifdef CONFIG_PQ_MDS_PIB
	pib_init();
#endif
	return 0;
}

int fixed_sdram(void);

long int initdram(int board_type)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CFG_DDR_BASE & LAWBAR_BAR;

	msize = fixed_sdram();

	/* return total bus SDRAM size(bytes)  -- DDR */
	return (msize * 1024 * 1024);
}

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
#if (CFG_DDR_SIZE != 128)
#warning Currenly any ddr size other than 128 is not supported
#endif
	im->ddr.sdram_clk_cntl = CFG_DDR_CLK_CNTL;
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
	__asm__ __volatile__ ("sync");
	udelay(200);

	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;
	__asm__ __volatile__ ("sync");
	return msize;
}

int checkboard(void)
{
	puts("Board: Freescale MPC832XEMDS\n");
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
#if defined(CONFIG_OF_FLAT_TREE)
	u32 *p;
	int len;

	p = ft_get_prop(blob, "/memory/reg", &len);
	if (p != NULL) {
		*p++ = cpu_to_be32(bd->bi_memstart);
		*p = cpu_to_be32(bd->bi_memsize);
	}
#endif
	ft_cpu_setup(blob, bd);
#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
}
#endif

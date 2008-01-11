/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *                    Dave Liu <daveliu@freescale.com>
 *
 * Copyright (C) 2007 Logic Product Development, Inc.
 *                    Peter Barada <peterb@logicpd.com>
 *
 * Copyright (C) 2007 MontaVista Software, Inc.
 *                    Anton Vorontsov <avorontsov@ru.mvista.com>
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
#include <asm/io.h>
#include <asm/mmu.h>
#include <pci.h>
#include <libfdt.h>

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* MDIO */
	{0,  1, 3, 0, 2}, /* MDIO */
	{0,  2, 1, 0, 1}, /* MDC */

	/* UCC1 - UEC (Gigabit) */
	{0,  3, 1, 0, 1}, /* TxD0 */
	{0,  4, 1, 0, 1}, /* TxD1 */
	{0,  5, 1, 0, 1}, /* TxD2 */
	{0,  6, 1, 0, 1}, /* TxD3 */
	{0,  9, 2, 0, 1}, /* RxD0 */
	{0, 10, 2, 0, 1}, /* RxD1 */
	{0, 11, 2, 0, 1}, /* RxD2 */
	{0, 12, 2, 0, 1}, /* RxD3 */
	{0,  7, 1, 0, 1}, /* TX_EN */
	{0,  8, 1, 0, 1}, /* TX_ER */
	{0, 15, 2, 0, 1}, /* RX_DV */
	{0,  0, 2, 0, 1}, /* RX_CLK */
	{2,  9, 1, 0, 3}, /* GTX_CLK - CLK10 */
	{2,  8, 2, 0, 1}, /* GTX125 - CLK9 */

	/* UCC2 - UEC (Gigabit) */
	{0, 17, 1, 0, 1}, /* TxD0 */
	{0, 18, 1, 0, 1}, /* TxD1 */
	{0, 19, 1, 0, 1}, /* TxD2 */
	{0, 20, 1, 0, 1}, /* TxD3 */
	{0, 23, 2, 0, 1}, /* RxD0 */
	{0, 24, 2, 0, 1}, /* RxD1 */
	{0, 25, 2, 0, 1}, /* RxD2 */
	{0, 26, 2, 0, 1}, /* RxD3 */
	{0, 21, 1, 0, 1}, /* TX_EN */
	{0, 22, 1, 0, 1}, /* TX_ER */
	{0, 29, 2, 0, 1}, /* RX_DV */
	{0, 31, 2, 0, 1}, /* RX_CLK */
	{2,  2, 1, 0, 2}, /* GTX_CLK - CLK10 */
	{2,  3, 2, 0, 1}, /* GTX125 - CLK4 */

	/* UCC7 - UEC */
	{4,  0, 1, 0, 1}, /* TxD0 */
	{4,  1, 1, 0, 1}, /* TxD1 */
	{4,  2, 1, 0, 1}, /* TxD2 */
	{4,  3, 1, 0, 1}, /* TxD3 */
	{4,  6, 2, 0, 1}, /* RxD0 */
	{4,  7, 2, 0, 1}, /* RxD1 */
	{4,  8, 2, 0, 1}, /* RxD2 */
	{4,  9, 2, 0, 1}, /* RxD3 */
	{4,  4, 1, 0, 1}, /* TX_EN */
	{4,  5, 1, 0, 1}, /* TX_ER */
	{4, 12, 2, 0, 1}, /* RX_DV */
	{4, 13, 2, 0, 1}, /* RX_ER */
	{4, 10, 2, 0, 1}, /* COL */
	{4, 11, 2, 0, 1}, /* CRS */
	{2, 18, 2, 0, 1}, /* TX_CLK - CLK19 */
	{2, 19, 2, 0, 1}, /* RX_CLK - CLK20 */

	/* UCC4 - UEC */
	{1, 14, 1, 0, 1}, /* TxD0 */
	{1, 15, 1, 0, 1}, /* TxD1 */
	{1, 16, 1, 0, 1}, /* TxD2 */
	{1, 17, 1, 0, 1}, /* TxD3 */
	{1, 20, 2, 0, 1}, /* RxD0 */
	{1, 21, 2, 0, 1}, /* RxD1 */
	{1, 22, 2, 0, 1}, /* RxD2 */
	{1, 23, 2, 0, 1}, /* RxD3 */
	{1, 18, 1, 0, 1}, /* TX_EN */
	{1, 19, 1, 0, 2}, /* TX_ER */
	{1, 26, 2, 0, 1}, /* RX_DV */
	{1, 27, 2, 0, 1}, /* RX_ER */
	{1, 24, 2, 0, 1}, /* COL */
	{1, 25, 2, 0, 1}, /* CRS */
	{2,  6, 2, 0, 1}, /* TX_CLK - CLK7 */
	{2,  7, 2, 0, 1}, /* RX_CLK - CLK8 */

	/* PCI1 */
	{5,  4, 2, 0, 3}, /* PCI_M66EN */
	{5,  5, 1, 0, 3}, /* PCI_INTA */
	{5,  6, 1, 0, 3}, /* PCI_RSTO */
	{5,  7, 3, 0, 3}, /* PCI_C_BE0 */
	{5,  8, 3, 0, 3}, /* PCI_C_BE1 */
	{5,  9, 3, 0, 3}, /* PCI_C_BE2 */
	{5, 10, 3, 0, 3}, /* PCI_C_BE3 */
	{5, 11, 3, 0, 3}, /* PCI_PAR */
	{5, 12, 3, 0, 3}, /* PCI_FRAME */
	{5, 13, 3, 0, 3}, /* PCI_TRDY */
	{5, 14, 3, 0, 3}, /* PCI_IRDY */
	{5, 15, 3, 0, 3}, /* PCI_STOP */
	{5, 16, 3, 0, 3}, /* PCI_DEVSEL */
	{5, 17, 0, 0, 0}, /* PCI_IDSEL */
	{5, 18, 3, 0, 3}, /* PCI_SERR */
	{5, 19, 3, 0, 3}, /* PCI_PERR */
	{5, 20, 3, 0, 3}, /* PCI_REQ0 */
	{5, 21, 2, 0, 3}, /* PCI_REQ1 */
	{5, 22, 2, 0, 3}, /* PCI_GNT2 */
	{5, 23, 3, 0, 3}, /* PCI_GNT0 */
	{5, 24, 1, 0, 3}, /* PCI_GNT1 */
	{5, 25, 1, 0, 3}, /* PCI_GNT2 */
	{5, 26, 0, 0, 0}, /* PCI_CLK0 */
	{5, 27, 0, 0, 0}, /* PCI_CLK1 */
	{5, 28, 0, 0, 0}, /* PCI_CLK2 */
	{5, 29, 0, 0, 3}, /* PCI_SYNC_OUT */
	{6,  0, 3, 0, 3}, /* PCI_AD0 */
	{6,  1, 3, 0, 3}, /* PCI_AD1 */
	{6,  2, 3, 0, 3}, /* PCI_AD2 */
	{6,  3, 3, 0, 3}, /* PCI_AD3 */
	{6,  4, 3, 0, 3}, /* PCI_AD4 */
	{6,  5, 3, 0, 3}, /* PCI_AD5 */
	{6,  6, 3, 0, 3}, /* PCI_AD6 */
	{6,  7, 3, 0, 3}, /* PCI_AD7 */
	{6,  8, 3, 0, 3}, /* PCI_AD8 */
	{6,  9, 3, 0, 3}, /* PCI_AD9 */
	{6, 10, 3, 0, 3}, /* PCI_AD10 */
	{6, 11, 3, 0, 3}, /* PCI_AD11 */
	{6, 12, 3, 0, 3}, /* PCI_AD12 */
	{6, 13, 3, 0, 3}, /* PCI_AD13 */
	{6, 14, 3, 0, 3}, /* PCI_AD14 */
	{6, 15, 3, 0, 3}, /* PCI_AD15 */
	{6, 16, 3, 0, 3}, /* PCI_AD16 */
	{6, 17, 3, 0, 3}, /* PCI_AD17 */
	{6, 18, 3, 0, 3}, /* PCI_AD18 */
	{6, 19, 3, 0, 3}, /* PCI_AD19 */
	{6, 20, 3, 0, 3}, /* PCI_AD20 */
	{6, 21, 3, 0, 3}, /* PCI_AD21 */
	{6, 22, 3, 0, 3}, /* PCI_AD22 */
	{6, 23, 3, 0, 3}, /* PCI_AD23 */
	{6, 24, 3, 0, 3}, /* PCI_AD24 */
	{6, 25, 3, 0, 3}, /* PCI_AD25 */
	{6, 26, 3, 0, 3}, /* PCI_AD26 */
	{6, 27, 3, 0, 3}, /* PCI_AD27 */
	{6, 28, 3, 0, 3}, /* PCI_AD28 */
	{6, 29, 3, 0, 3}, /* PCI_AD29 */
	{6, 30, 3, 0, 3}, /* PCI_AD30 */
	{6, 31, 3, 0, 3}, /* PCI_AD31 */

	/* NAND */
	{4, 18, 2, 0, 0}, /* NAND_RYnBY */

	/* DUART - UART2 */
	{5,  0, 1, 0, 2}, /* UART2_SOUT */
	{5,  2, 1, 0, 1}, /* UART2_RTS */
	{5,  3, 2, 0, 2}, /* UART2_SIN */
	{5,  1, 2, 0, 3}, /* UART2_CTS */

	/* UCC5 - UART3 */
	{3,  0, 1, 0, 1}, /* UART3_TX */
	{3,  4, 1, 0, 1}, /* UART3_RTS */
	{3,  6, 2, 0, 1}, /* UART3_RX */
	{3, 12, 2, 0, 0}, /* UART3_CTS */
	{3, 13, 2, 0, 0}, /* UCC5_CD */

	/* UCC6 - UART4 */
	{3, 14, 1, 0, 1}, /* UART4_TX */
	{3, 18, 1, 0, 1}, /* UART4_RTS */
	{3, 20, 2, 0, 1}, /* UART4_RX */
	{3, 26, 2, 0, 0}, /* UART4_CTS */
	{3, 27, 2, 0, 0}, /* UCC6_CD */

	/* Fujitsu MB86277 (MINT) graphics controller */
	{0, 30, 1, 0, 0}, /* nSRESET_GRAPHICS */
	{1,  5, 1, 0, 0}, /* nXRST_GRAPHICS */
	{1,  7, 1, 0, 0}, /* LVDS_BKLT_CTR */
	{2, 16, 1, 0, 0}, /* LVDS_BKLT_EN */

	/* END of table */
	{0,  0, 0, 0, QE_IOP_TAB_END},
};

int board_early_init_f(void)
{
	return 0;
}

int board_early_init_r(void)
{
	void *reg = (void *)(CFG_IMMR + 0x14a8);
	u32 val;

	/*
	 * Because of errata in the UCCs, we have to write to the reserved
	 * registers to slow the clocks down.
	 */
	val = in_be32(reg);
	/* UCC1 */
	val |= 0x00003000;
	/* UCC2 */
	val |= 0x0c000000;
	out_be32(reg, val);

	return 0;
}

int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *)CFG_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	msize = CFG_DDR_SIZE;
	for (ddr_size = msize << 20, ddr_size_log2 = 0;
	     (ddr_size > 1); ddr_size = ddr_size >> 1, ddr_size_log2++) {
		if (ddr_size & 1)
			return -1;
	}

	im->sysconf.ddrlaw[0].ar =
	    LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);

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
	udelay(200);
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	return msize;
}

long int initdram(int board_type)
{
#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRC)
	extern void ddr_enable_ecc(unsigned int dram_size);
#endif
	volatile immap_t *im = (immap_t *)CFG_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CFG_DDR_BASE & LAWBAR_BAR;
	msize = fixed_sdram();

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRC)
	/*
	 * Initialize DDR ECC byte
	 */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif

	/* return total bus SDRAM size(bytes)  -- DDR */
	return (msize * 1024 * 1024);
}

int checkboard(void)
{
	puts("Board: Freescale/Logic MPC8360ERDK\n");
	return 0;
}

static struct pci_region pci_regions[] = {
	{
		.bus_start = CFG_PCI1_MEM_BASE,
		.phys_start = CFG_PCI1_MEM_PHYS,
		.size = CFG_PCI1_MEM_SIZE,
		.flags = PCI_REGION_MEM | PCI_REGION_PREFETCH,
	},
	{
		.bus_start = CFG_PCI1_MMIO_BASE,
		.phys_start = CFG_PCI1_MMIO_PHYS,
		.size = CFG_PCI1_MMIO_SIZE,
		.flags = PCI_REGION_MEM,
	},
	{
		.bus_start = CFG_PCI1_IO_BASE,
		.phys_start = CFG_PCI1_IO_PHYS,
		.size = CFG_PCI1_IO_SIZE,
		.flags = PCI_REGION_IO,
	},
};

void pci_init_board(void)
{
	volatile immap_t *immr = (volatile immap_t *)CFG_IMMR;
	volatile clk83xx_t *clk = (volatile clk83xx_t *)&immr->clk;
	volatile law83xx_t *pci_law = immr->sysconf.pcilaw;
	struct pci_region *reg[] = { pci_regions, };

#if defined(PCI_33M)
	clk->occr = OCCR_PCICOE0 | OCCR_PCICOE1 | OCCR_PCICOE2 |
		    OCCR_PCICD0 | OCCR_PCICD1 | OCCR_PCICD2 | OCCR_PCICR;
	printf("PCI clock is 33MHz\n");
#else
	clk->occr = OCCR_PCICOE0 | OCCR_PCICOE1 | OCCR_PCICOE2;
	printf("PCI clock is 66MHz\n");
#endif

	udelay(2000);

	/* Configure PCI Local Access Windows */
	pci_law[0].bar = CFG_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LBLAWAR_EN | LBLAWAR_512MB;

	pci_law[1].bar = CFG_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LBLAWAR_EN | LBLAWAR_1MB;

	mpc83xx_pci_init(1, reg, 0);
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	ft_pci_setup(blob, bd);
}
#endif

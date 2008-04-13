/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 * Dave Liu <daveliu@freescale.com>
 *
 * CREDITS: Kim Phillips contribute to LIBFDT code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <spd_sdram.h>
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif
#if defined(CONFIG_PQ_MDS_PIB)
#include "../common/pq-mds-pib.h"
#endif

int board_early_init_f(void)
{
	u8 *bcsr = (u8 *)CFG_BCSR;

	/* Enable flash write */
	bcsr[0x9] &= ~0x04;
	/* Clear all of the interrupt of BCSR */
	bcsr[0xe] = 0xff;

#ifdef CONFIG_FSL_SERDES
	immap_t *immr = (immap_t *)CFG_IMMR;
	u32 spridr = in_be32(&immr->sysconf.spridr);

	/* we check only part num, and don't look for CPU revisions */
	switch (PARTID_NO_E(spridr)) {
	case SPR_8377:
		fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_SATA,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		fsl_setup_serdes(CONFIG_FSL_SERDES2, FSL_SERDES_PROTO_PEX,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		break;
	case SPR_8378:
		fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_PEX,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		break;
	case SPR_8379:
		fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_SATA,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		fsl_setup_serdes(CONFIG_FSL_SERDES2, FSL_SERDES_PROTO_SATA,
				 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
		break;
	default:
		printf("serdes not configured: unknown CPU part number: "
		       "%04x\n", spridr >> 16);
		break;
	}
#endif /* CONFIG_FSL_SERDES */
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

long int initdram(int board_type)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

#if defined(CONFIG_SPD_EEPROM)
	msize = spd_sdram();
#else
	msize = fixed_sdram();
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRC)
	/* Initialize DDR ECC byte */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif

	/* return total bus DDR size(bytes) */
	return (msize * 1024 * 1024);
}

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = CFG_DDR_SIZE * 1024 * 1024;
	u32 msize_log2 = __ilog2(msize);

	im->sysconf.ddrlaw[0].bar = CFG_DDR_SDRAM_BASE >> 12;
	im->sysconf.ddrlaw[0].ar = LBLAWAR_EN | (msize_log2 - 1);

#if (CFG_DDR_SIZE != 512)
#warning Currenly any ddr size other than 512 is not supported
#endif
	im->sysconf.ddrcdr = CFG_DDRCDR_VALUE;
	udelay(50000);

	im->ddr.sdram_clk_cntl = CFG_DDR_SDRAM_CLK_CNTL;
	udelay(1000);

	im->ddr.csbnds[0].csbnds = CFG_DDR_CS0_BNDS;
	im->ddr.cs_config[0] = CFG_DDR_CS0_CONFIG;
	udelay(1000);

	im->ddr.timing_cfg_0 = CFG_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CFG_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CFG_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CFG_DDR_TIMING_3;
	im->ddr.sdram_cfg = CFG_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CFG_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CFG_DDR_MODE;
	im->ddr.sdram_mode2 = CFG_DDR_MODE2;
	im->ddr.sdram_interval = CFG_DDR_INTERVAL;
	__asm__ __volatile__("sync");
	udelay(1000);

	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;
	udelay(2000);
	return CFG_DDR_SIZE;
}
#endif /*!CFG_SPD_EEPROM */

int checkboard(void)
{
	puts("Board: Freescale MPC837xEMDS\n");
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
#endif /* CONFIG_OF_BOARD_SETUP */

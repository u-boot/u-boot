// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 */

#include <config.h>
#include <clock_legacy.h>
#include <fdt_support.h>
#include <init.h>
#include <net.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/arch/clock.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/ls102xa_stream_id.h>
#include <asm/global_data.h>
#include <linux/delay.h>

#include <asm/arch/ls102xa_devdis.h>
#include <asm/arch/ls102xa_soc.h>
#include <asm/sections.h>
#include <fsl_csu.h>
#include <fsl_immap.h>
#include <netdev.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <spl.h>

#include <fsl_validate.h>
#include "../common/sleep.h"

DECLARE_GLOBAL_DATA_PTR;

#define DDR_SIZE		0x40000000

int checkboard(void)
{
	puts("Board: LS1021AIOT\n");

#ifndef CONFIG_QSPI_BOOT
	struct ccsr_gur *dcfg = (struct ccsr_gur *)CFG_SYS_FSL_GUTS_ADDR;
	u32 cpldrev;

	cpldrev = in_be32(&dcfg->gpporcr1);

	printf("CPLD:  V%d.%d\n", ((cpldrev >> 28) & 0xf), ((cpldrev >> 24) &
		0xf));
#endif
	return 0;
}

void ddrmc_init(void)
{
	struct ccsr_ddr *ddr = (struct ccsr_ddr *)CFG_SYS_FSL_DDR_ADDR;
	u32 temp_sdram_cfg, tmp;

	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG);

	out_be32(&ddr->cs0_bnds, DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, DDR_CS0_CONFIG);

	out_be32(&ddr->timing_cfg_0, DDR_TIMING_CFG_0);
	out_be32(&ddr->timing_cfg_1, DDR_TIMING_CFG_1);
	out_be32(&ddr->timing_cfg_2, DDR_TIMING_CFG_2);
	out_be32(&ddr->timing_cfg_3, DDR_TIMING_CFG_3);
	out_be32(&ddr->timing_cfg_4, DDR_TIMING_CFG_4);
	out_be32(&ddr->timing_cfg_5, DDR_TIMING_CFG_5);

	out_be32(&ddr->sdram_cfg_2, DDR_SDRAM_CFG_2);
	out_be32(&ddr->ddr_cdr2, DDR_DDR_CDR2);

	out_be32(&ddr->sdram_mode, DDR_SDRAM_MODE);
	out_be32(&ddr->sdram_mode_2, DDR_SDRAM_MODE_2);

	out_be32(&ddr->sdram_interval, DDR_SDRAM_INTERVAL);

	out_be32(&ddr->ddr_wrlvl_cntl, DDR_DDR_WRLVL_CNTL);

	out_be32(&ddr->ddr_wrlvl_cntl_2, DDR_DDR_WRLVL_CNTL_2);
	out_be32(&ddr->ddr_wrlvl_cntl_3, DDR_DDR_WRLVL_CNTL_3);

	out_be32(&ddr->ddr_cdr1, DDR_DDR_CDR1);

	out_be32(&ddr->sdram_clk_cntl, DDR_SDRAM_CLK_CNTL);
	out_be32(&ddr->ddr_zq_cntl, DDR_DDR_ZQ_CNTL);

	out_be32(&ddr->cs0_config_2, DDR_CS0_CONFIG_2);

	/* DDR erratum A-009942 */
	tmp = in_be32(&ddr->debug[28]);
	out_be32(&ddr->debug[28], tmp | 0x0070006f);

	udelay(500);

	temp_sdram_cfg = (DDR_SDRAM_CFG_MEM_EN & ~SDRAM_CFG_BI);

	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG | temp_sdram_cfg);
}

int dram_init(void)
{
#if (!defined(CONFIG_SPL) || defined(CONFIG_XPL_BUILD))
	ddrmc_init();
#endif

	erratum_a008850_post();

	gd->ram_size = DDR_SIZE;
	return 0;
}

int board_early_init_f(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CFG_SYS_FSL_SCFG_ADDR;

#ifdef CONFIG_TSEC_ENET
	/* clear BD & FR bits for BE BD's and frame data */
	clrbits_be32(&scfg->etsecdmamcr, SCFG_ETSECDMAMCR_LE_BD_FR);
	out_be32(&scfg->etsecmcr, SCFG_ETSECCMCR_GE2_CLK125);

#endif

	arch_soc_init();

	return 0;
}

#ifdef CONFIG_XPL_BUILD
void board_init_f(ulong dummy)
{
	/* Clear the BSS */
	memset(__bss_start, 0, __bss_end - __bss_start);

	get_clocks();

	preloader_console_init();

	dram_init();

	/* Allow OCRAM access permission as R/W */

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

	board_init_r(NULL, 0);
}
#endif

int board_init(void)
{
#ifndef CONFIG_SYS_FSL_NO_SERDES
	fsl_serdes_init();
#endif

	ls102xa_smmu_stream_id_init();

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	return 0;
}
#endif

#if defined(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
#ifdef CONFIG_FSL_DEVICE_DISABLE
	device_disable(devdis_tbl, ARRAY_SIZE(devdis_tbl));

#endif
	return 0;
}
#endif

int ft_board_setup(void *blob, struct bd_info *bd)
{
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif

	return 0;
}

void flash_write16(u16 val, void *addr)
{
	u16 shftval = (((val >> 8) & 0xff) | ((val << 8) & 0xff00));

	__raw_writew(shftval, addr);
}

u16 flash_read16(void *addr)
{
	u16 val = __raw_readw(addr);

	return (((val) >> 8) & 0x00ff) | (((val) << 8) & 0xff00);
}

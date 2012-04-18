/*
 * Copyright 2010-2012 Freescale Semiconductor, Inc.
 *
 * Authors:  Roy Zang <tie-fei.zang@freescale.com>
 *           Chunhe Lan <b25806@freescale.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/io.h>
#include <asm/cache.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_portals.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <netdev.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/fsl_dtsec.h>

#include "bcsr.h"

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	fsl_lbc_t *lbc = LBC_BASE_ADDR;

	/* Set ABSWP to implement conversion of addresses in the LBC */
	setbits_be32(&lbc->lbcr, CONFIG_SYS_LBC_LBCR);

	return 0;
}

int checkboard(void)
{
	u8 *bcsr = (u8 *)BCSR_ACCESS_REG_ADDR;

	printf("Board: P1023 RDS\n");

	clrbits_8(&bcsr[15], BCSR15_I2C_BUS0_SEG_CLR);
	setbits_8(&bcsr[15], BCSR15_I2C_BUS0_SEG0);

	return 0;
}

/* Fixed sdram init -- doesn't use serial presence detect. */
phys_size_t fixed_sdram(void)
{
#ifndef CONFIG_SYS_RAMBOOT
	ccsr_ddr_t *ddr = (ccsr_ddr_t *)CONFIG_SYS_MPC85xx_DDR_ADDR;

	set_next_law(0, LAW_SIZE_2G, LAW_TRGT_IF_DDR_1);

	out_be32(&ddr->cs0_bnds, CONFIG_SYS_DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, CONFIG_SYS_DDR_CS0_CONFIG);
	out_be32(&ddr->cs1_bnds, CONFIG_SYS_DDR_CS1_BNDS);
	out_be32(&ddr->cs1_config, CONFIG_SYS_DDR_CS1_CONFIG);
	out_be32(&ddr->timing_cfg_3, CONFIG_SYS_DDR_TIMING_3);
	out_be32(&ddr->timing_cfg_0, CONFIG_SYS_DDR_TIMING_0);
	out_be32(&ddr->timing_cfg_1, CONFIG_SYS_DDR_TIMING_1);
	out_be32(&ddr->timing_cfg_2, CONFIG_SYS_DDR_TIMING_2);
	out_be32(&ddr->sdram_cfg_2, CONFIG_SYS_DDR_CONTROL2);
	out_be32(&ddr->sdram_mode, CONFIG_SYS_DDR_MODE_1);
	out_be32(&ddr->sdram_mode_2, CONFIG_SYS_DDR_MODE_2);
	out_be32(&ddr->sdram_interval, CONFIG_SYS_DDR_INTERVAL);
	out_be32(&ddr->sdram_data_init, CONFIG_SYS_DDR_DATA_INIT);
	out_be32(&ddr->sdram_clk_cntl, CONFIG_SYS_DDR_CLK_CTRL);
	out_be32(&ddr->timing_cfg_4, CONFIG_SYS_DDR_TIMING_4);
	out_be32(&ddr->timing_cfg_5, CONFIG_SYS_DDR_TIMING_5);
	out_be32(&ddr->ddr_zq_cntl, CONFIG_SYS_DDR_ZQ_CNTL);
	out_be32(&ddr->ddr_wrlvl_cntl, CONFIG_SYS_DDR_WRLVL_CNTL);
	out_be32(&ddr->ddr_cdr1, CONFIG_SYS_DDR_CDR_1);
	out_be32(&ddr->ddr_cdr2, CONFIG_SYS_DDR_CDR_2);
	out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_CONTROL);
#endif
	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024ul;
}

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}
#endif

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_BCSR_BASE;
	const u8 flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + BCSR region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* invalidate existing TLB entry for flash + bcsr */
	disable_tlb(flash_esel);

	set_tlb(1, flashbase, CONFIG_SYS_BCSR_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_256M, 1);

	setup_portals();

	return 0;
}

unsigned long get_board_sys_clk(ulong dummy)
{
	return gd->bus_clk;
}

unsigned long get_board_ddr_clk(ulong dummy)
{
	return gd->mem_clk;
}

int board_eth_init(bd_t *bis)
{
	u8 *bcsr = (u8 *)BCSR_ACCESS_REG_ADDR;
	ccsr_gur_t *gur = (ccsr_gur_t *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	struct fsl_pq_mdio_info dtsec_mdio_info;

	/*
	 * Need to set dTSEC 1 pin multiplexing to TSEC. The default setting
	 * is not correct.
	 */
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_TSEC1_1);

	dtsec_mdio_info.regs =
		(struct tsec_mii_mng *)CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR;
	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the 1G MDIO bus */
	fsl_pq_mdio_init(bis, &dtsec_mdio_info);

	fm_info_set_phy_address(FM1_DTSEC1, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR);
	fm_info_set_phy_address(FM1_DTSEC2, CONFIG_SYS_FM1_DTSEC2_PHY_ADDR);

	fm_info_set_mdio(FM1_DTSEC1,
		miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));
	fm_info_set_mdio(FM1_DTSEC2,
		miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME));

	/* Make SERDES connected to SGMII by cleaing bcsr19[7] */
	if (fm_info_get_enet_if(FM1_DTSEC1) == PHY_INTERFACE_MODE_SGMII)
		clrbits_8(&bcsr[19], BCSR19_SGMII_SEL_L);

#ifdef CONFIG_FMAN_ENET
	cpu_eth_init(bis);
#endif

	return pci_eth_init(bis);
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	/* By default NOR is on, and NAND is disabled */
#ifdef CONFIG_NAND_U_BOOT
	do_fixup_by_path_string(blob, "nor_flash", "status", "disabled");
	do_fixup_by_path_string(blob, "nand_flash", "status", "okay");
#endif
#ifdef CONFIG_HAS_FSL_DR_USB
	fdt_fixup_dr_usb(blob, bd);
#endif

	fdt_fixup_fman_ethernet(blob);
}
#endif

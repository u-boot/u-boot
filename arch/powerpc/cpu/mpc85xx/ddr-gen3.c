/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/fsl_ddr_sdram.h>

#if (CONFIG_CHIP_SELECTS_PER_CTRL > 4)
#error Invalid setting for CONFIG_CHIP_SELECTS_PER_CTRL
#endif

void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
			     unsigned int ctrl_num)
{
	unsigned int i;
	volatile ccsr_ddr_t *ddr;
	u32 temp_sdram_cfg;

	switch (ctrl_num) {
	case 0:
		ddr = (void *)CONFIG_SYS_MPC85xx_DDR_ADDR;
		break;
	case 1:
		ddr = (void *)CONFIG_SYS_MPC85xx_DDR2_ADDR;
		break;
	default:
		printf("%s unexpected ctrl_num = %u\n", __FUNCTION__, ctrl_num);
		return;
	}

	out_be32(&ddr->eor, regs->ddr_eor);

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (i == 0) {
			out_be32(&ddr->cs0_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs0_config, regs->cs[i].config);
			out_be32(&ddr->cs0_config_2, regs->cs[i].config_2);

		} else if (i == 1) {
			out_be32(&ddr->cs1_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs1_config, regs->cs[i].config);
			out_be32(&ddr->cs1_config_2, regs->cs[i].config_2);

		} else if (i == 2) {
			out_be32(&ddr->cs2_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs2_config, regs->cs[i].config);
			out_be32(&ddr->cs2_config_2, regs->cs[i].config_2);

		} else if (i == 3) {
			out_be32(&ddr->cs3_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs3_config, regs->cs[i].config);
			out_be32(&ddr->cs3_config_2, regs->cs[i].config_2);
		}
	}

	out_be32(&ddr->timing_cfg_3, regs->timing_cfg_3);
	out_be32(&ddr->timing_cfg_0, regs->timing_cfg_0);
	out_be32(&ddr->timing_cfg_1, regs->timing_cfg_1);
	out_be32(&ddr->timing_cfg_2, regs->timing_cfg_2);
	out_be32(&ddr->sdram_cfg_2, regs->ddr_sdram_cfg_2);
	out_be32(&ddr->sdram_mode, regs->ddr_sdram_mode);
	out_be32(&ddr->sdram_mode_2, regs->ddr_sdram_mode_2);
	out_be32(&ddr->sdram_md_cntl, regs->ddr_sdram_md_cntl);
	out_be32(&ddr->sdram_interval, regs->ddr_sdram_interval);
	out_be32(&ddr->sdram_data_init, regs->ddr_data_init);
	out_be32(&ddr->sdram_clk_cntl, regs->ddr_sdram_clk_cntl);
	out_be32(&ddr->init_addr, regs->ddr_init_addr);
	out_be32(&ddr->init_ext_addr, regs->ddr_init_ext_addr);

	out_be32(&ddr->timing_cfg_4, regs->timing_cfg_4);
	out_be32(&ddr->timing_cfg_5, regs->timing_cfg_5);
	out_be32(&ddr->ddr_zq_cntl, regs->ddr_zq_cntl);
	out_be32(&ddr->ddr_wrlvl_cntl, regs->ddr_wrlvl_cntl);
	out_be32(&ddr->ddr_sr_cntr, regs->ddr_sr_cntr);
	out_be32(&ddr->ddr_sdram_rcw_1, regs->ddr_sdram_rcw_1);
	out_be32(&ddr->ddr_sdram_rcw_2, regs->ddr_sdram_rcw_2);

	/* Set, but do not enable the memory */
	temp_sdram_cfg = regs->ddr_sdram_cfg;
	temp_sdram_cfg &= ~(SDRAM_CFG_MEM_EN);
	out_be32(&ddr->sdram_cfg, temp_sdram_cfg);
	/*
	 * For 8572 DDR1 erratum - DDR controller may enter illegal state
	 * when operatiing in 32-bit bus mode with 4-beat bursts,
	 * This erratum does not affect DDR3 mode, only for DDR2 mode.
	 */
#ifdef CONFIG_MPC8572
	if ((((in_be32(&ddr->sdram_cfg) >> 24) & 0x7) == SDRAM_TYPE_DDR2)
	    && in_be32(&ddr->sdram_cfg) & 0x80000) {
		/* set DEBUG_1[31] */
		u32 temp = in_be32(&ddr->debug_1);
		out_be32(&ddr->debug_1, temp | 1);
	}
#endif

	/*
	 * 500 painful micro-seconds must elapse between
	 * the DDR clock setup and the DDR config enable.
	 * DDR2 need 200 us, and DDR3 need 500 us from spec,
	 * we choose the max, that is 500 us for all of case.
	 */
	udelay(500);
	asm volatile("sync;isync");

	/* Let the controller go */
	temp_sdram_cfg = in_be32(&ddr->sdram_cfg);
	out_be32(&ddr->sdram_cfg, temp_sdram_cfg | SDRAM_CFG_MEM_EN);

	/* Poll DDR_SDRAM_CFG_2[D_INIT] bit until auto-data init is done.  */
	while (in_be32(&ddr->sdram_cfg_2) & 0x10) {
		udelay(10000);		/* throttle polling rate */
	}
}

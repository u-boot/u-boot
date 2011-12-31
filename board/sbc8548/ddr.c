/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>

#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	/*
	 * Factors to consider for clock adjust:
	 *	- number of chips on bus
	 *	- position of slot
	 *	- DDR1 vs. DDR2?
	 *	- ???
	 *
	 * This needs to be determined on a board-by-board basis.
	 *	0110	3/4 cycle late
	 *	0111	7/8 cycle late
	 */
	popts->clk_adjust = 7;

	/*
	 * Factors to consider for CPO:
	 *	- frequency
	 *	- ddr1 vs. ddr2
	 */
	popts->cpo_override = 10;

	/*
	 * Factors to consider for write data delay:
	 *	- number of DIMMs
	 *
	 * 1 = 1/4 clock delay
	 * 2 = 1/2 clock delay
	 * 3 = 3/4 clock delay
	 * 4 = 1   clock delay
	 * 5 = 5/4 clock delay
	 * 6 = 3/2 clock delay
	 */
	popts->write_data_delay = 3;

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
}

#if !defined(CONFIG_SPD_EEPROM)
/*
 *  fixed_sdram init -- doesn't use serial presence detect.
 *  Assumes 256MB DDR2 SDRAM SODIMM, without ECC, running at DDR400 speed.
 */
phys_size_t fixed_sdram(void)
{
	volatile ccsr_ddr_t *ddr = (void *)(CONFIG_SYS_MPC85xx_DDR_ADDR);

	out_be32(&ddr->cs0_bnds,	0x0000007f);
	out_be32(&ddr->cs1_bnds,	0x008000ff);
	out_be32(&ddr->cs2_bnds,	0x00000000);
	out_be32(&ddr->cs3_bnds,	0x00000000);

	out_be32(&ddr->cs0_config,	0x80010101);
	out_be32(&ddr->cs1_config,	0x80010101);
	out_be32(&ddr->cs2_config,	0x00000000);
	out_be32(&ddr->cs3_config,	0x00000000);

	out_be32(&ddr->timing_cfg_3,	0x00000000);
	out_be32(&ddr->timing_cfg_0,	0x00220802);
	out_be32(&ddr->timing_cfg_1,	0x38377322);
	out_be32(&ddr->timing_cfg_2,	0x0fa044C7);

	out_be32(&ddr->sdram_cfg,	0x4300C000);
	out_be32(&ddr->sdram_cfg_2,	0x24401000);

	out_be32(&ddr->sdram_mode,	0x23C00542);
	out_be32(&ddr->sdram_mode_2,	0x00000000);

	out_be32(&ddr->sdram_interval,	0x05080100);
	out_be32(&ddr->sdram_md_cntl,	0x00000000);
	out_be32(&ddr->sdram_data_init,	0x00000000);
	out_be32(&ddr->sdram_clk_cntl,	0x03800000);
	asm("sync;isync;msync");
	udelay(500);

	#ifdef CONFIG_DDR_ECC
	  /* Enable ECC checking */
	  out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_CONTROL | 0x20000000);
	#else
	  out_be32(&ddr->sdram_cfg, CONFIG_SYS_DDR_CONTROL);
	#endif

	return CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
}
#endif

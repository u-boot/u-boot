/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 * Authors: Srikanth Srinivasan <srikanth.srinivasan@freescale.com>
 *          Timur Tabi <timur@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <common.h>

#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>

typedef struct {
	u32 datarate_mhz_low;
	u32 datarate_mhz_high;
	u32 n_ranks;
	u32 clk_adjust;		/* Range: 0-8 */
	u32 cpo;		/* Range: 2-31 */
	u32 write_data_delay;	/* Range: 0-6 */
	u32 force_2T;
} board_specific_parameters_t;

static const board_specific_parameters_t bsp[] = {
/*
 *        lo|  hi|  num|  clk| cpo|wrdata|2T
 *       mhz| mhz|ranks|adjst|    | delay|
 */
	{  0, 333,    1,    5,  31,     3, 0},
	{334, 400,    1,    5,  31,     3, 0},
	{401, 549,    1,    5,  31,     3, 0},
	{550, 680,    1,    5,  31,     5, 0},
	{681, 850,    1,    5,  31,     5, 0},
	{  0, 333,    2,    5,  31,     3, 0},
	{334, 400,    2,    5,  31,     3, 0},
	{401, 549,    2,    5,  31,     3, 0},
	{550, 680,    2,    5,  31,     5, 0},
	{681, 850,    2,    5,  31,     5, 0},
};

void fsl_ddr_board_options(memctl_options_t *popts, dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
	unsigned long ddr_freq;
	unsigned int i;

	/* set odt_rd_cfg and odt_wr_cfg. */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		popts->cs_local_opts[i].odt_rd_cfg = 0;
		popts->cs_local_opts[i].odt_wr_cfg = 1;
	}

	/*
	 * Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	for (i = 0; i < ARRAY_SIZE(bsp); i++) {
		if (ddr_freq >= bsp[i].datarate_mhz_low &&
		    ddr_freq <= bsp[i].datarate_mhz_high &&
		    pdimm->n_ranks == bsp[i].n_ranks) {
			popts->clk_adjust = bsp[i].clk_adjust;
			popts->cpo_override = bsp[i].cpo;
			popts->write_data_delay = bsp[i].write_data_delay;
			popts->twoT_en = bsp[i].force_2T;
			break;
		}
	}

	popts->half_strength_driver_enable = 1;

	/* Per AN4039, enable ZQ calibration. */
	popts->zq_en = 1;

	/*
	 * For wake-up on ARP, we need auto self refresh enabled
	 */
	popts->auto_self_refresh_en = 1;
	popts->sr_it = 0xb;
}

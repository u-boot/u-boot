/*
 * Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>

#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>

struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 clk_adjust;
	u32 cpo;
	u32 write_data_delay;
	u32 force_2T;
};


/*
 * This table contains all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 *
 * ranges for parameters:
 *  wr_data_delay = 0-6
 *  clk adjust = 0-8
 *  cpo 2-0x1E (30)
 */
static const struct board_specific_parameters dimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| cpo|wrdata|2T
	 * ranks| mhz|adjst|    | delay|
	 */
#ifdef CONFIG_FSL_DDR2
	{2,  549,    4,   0x1f,    2,  0},
	{2,  680,    4,   0x1f,    3,  0},
	{2,  850,    4,   0x1f,    4,  0},
	{1,  549,    4,   0x1f,    2,  0},
	{1,  680,    4,   0x1f,    3,  0},
	{1,  850,    4,   0x1f,    4,  0},
#else
	{2,  850,    6,   0x1f,    4,  0},
	{1,  850,    4,   0x1f,    4,  0},
#endif
	{}
};

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;
	int i;

	if (ctrl_num) {
		printf("Wrong parameter for controller number %d", ctrl_num);
		return;
	}
	if (!pdimm->n_ranks)
		return;

	/*
	 * set odt_rd_cfg and odt_wr_cfg. If the there is only one dimm in
	 * that controller, set odt_wr_cfg to 4 for CS0, and 0 to CS1. If
	 * there are two dimms in the controller, set odt_rd_cfg to 3 and
	 * odt_wr_cfg to 3 for the even CS, 0 for the odd CS.
	 */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		popts->cs_local_opts[i].odt_rd_cfg = 0;
		popts->cs_local_opts[i].odt_wr_cfg = 1;
	}

	pbsp = dimm0;

	/* Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm->n_ranks) {
			if (ddr_freq <= pbsp->datarate_mhz_high) {
				popts->clk_adjust = pbsp->clk_adjust;
				popts->cpo_override = pbsp->cpo;
				popts->write_data_delay =
					pbsp->write_data_delay;
				popts->twoT_en = pbsp->force_2T;
				goto found;
			}
			pbsp_highest = pbsp;
		}
		pbsp++;
	}

	if (pbsp_highest) {
		printf("Error: board specific timing not found "
			"for data rate %lu MT/s!\n"
			"Trying to use the highest speed (%u) parameters\n",
			ddr_freq, pbsp_highest->datarate_mhz_high);
		popts->clk_adjust = pbsp_highest->clk_adjust;
		popts->cpo_override = pbsp_highest->cpo;
		popts->write_data_delay = pbsp_highest->write_data_delay;
		popts->twoT_en = pbsp_highest->force_2T;
	} else {
		panic("DIMM is not supported by this board");
	}

found:
	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
	popts->wrlvl_en = 1;
	/* Write leveling override */
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xa;
	popts->wrlvl_start = 0x8;
	/* Rtt and Rtt_WR override */
	popts->rtt_override = 1;
	popts->rtt_override_value = DDR3_RTT_120_OHM;
	popts->rtt_wr_override_value = 0; /* Rtt_WR= dynamic ODT off */
}

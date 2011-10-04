/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
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

#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>

struct board_specific_parameters {
	u32 datarate_mhz_low;
	u32 datarate_mhz_high;
	u32 n_ranks;
	u32 clk_adjust;
	u32 cpo;
	u32 write_data_delay;
	u32 force_2T;
};

const struct board_specific_parameters board_specific_parameters_udimm[][20] = {
	{
	/*
	 *	memory controller 0
	 *	  lo|  hi|  num|  clk| cpo|wrdata|2T
	 *	 mhz| mhz|ranks|adjst|    | delay|
	 */
		{  0, 300,    2,    4,   4,    2,  0},
		{301, 365,    2,    4,   6,    2,  0},
		{366, 450,    2,    4,   7,    2,  0},
		{451, 850,    2,    4,  31,    2,  0},
		{  0, 300,    1,    4,   4,    2,  0},
		{301, 365,    1,    4,   6,    2,  0},
		{366, 450,    1,    4,   7,    2,  0},
		{451, 850,    1,    4,  31,    2,  0}
	}
};

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp;
	u32 num_params;
	u32 i, dimm_num;
	ulong ddr_freq;

	if (ctrl_num != 0)	/* we have only one controller */
		return;
	for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
		if (pdimm[i].n_ranks)
			break;
	}
	if (i >= CONFIG_DIMM_SLOTS_PER_CTLR)	/* no DIMM */
		return;

	dimm_num = i;
	pbsp = &(board_specific_parameters_udimm[ctrl_num][0]);
	num_params = sizeof(board_specific_parameters_udimm[ctrl_num]) /
			sizeof(board_specific_parameters_udimm[0][0]);

	/* Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	for (i = 0; i < num_params; i++) {
		if (ddr_freq >= pbsp->datarate_mhz_low &&
		    ddr_freq <= pbsp->datarate_mhz_high &&
		    pdimm[dimm_num].n_ranks == pbsp->n_ranks) {
			popts->clk_adjust = pbsp->clk_adjust;
			popts->cpo_override = pbsp->cpo;
			popts->write_data_delay = pbsp->write_data_delay;
			popts->twoT_en = pbsp->force_2T;
			break;
		}
		pbsp++;
	}

	if (i == num_params) {
		printf("Warning: board specific timing not found "
			"for data rate %lu MT/s!\n", ddr_freq);
	}

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
	popts->DQS_config = 0;	/* only true DQS signal is used on board */
}

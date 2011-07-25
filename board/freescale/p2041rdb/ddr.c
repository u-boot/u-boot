/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <i2c.h>
#include <hwconfig.h>
#include <asm/mmu.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>
#include <asm/fsl_law.h>

typedef struct {
	u32 datarate_mhz_low;
	u32 datarate_mhz_high;
	u32 n_ranks;
	u32 clk_adjust;
	u32 wrlvl_start;
	u32 cpo;
	u32 write_data_delay;
	u32 force_2T;
} board_specific_parameters_t;

/*
 * ranges for parameters:
 *  wr_data_delay = 0-6
 *  clk adjust = 0-8
 *  cpo 2-0x1E (30)
 */
const board_specific_parameters_t board_specific_parameters[] = {
	/*
	 * memory controller 0
	 *  lo|  hi|  num|  clk| wrlvl | cpo  |wrdata|2T
	 * mhz| mhz|ranks|adjst| start | delay|
	 */
	{  1017, 1116,    2,    4,     6,   0xff,    2,  0},
};

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	const board_specific_parameters_t *pbsp =
				&board_specific_parameters[0];
	u32 num_params = ARRAY_SIZE(board_specific_parameters);
	u32 i;
	ulong ddr_freq;

	/*
	 * Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	for (i = 0; i < num_params; i++) {
		if (ddr_freq >= pbsp->datarate_mhz_low &&
			ddr_freq <= pbsp->datarate_mhz_high &&
			pdimm[0].n_ranks == pbsp->n_ranks) {
			popts->cpo_override = pbsp->cpo;
			popts->write_data_delay = pbsp->write_data_delay;
			popts->clk_adjust = pbsp->clk_adjust;
			popts->wrlvl_start = pbsp->wrlvl_start;
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
	/* Write leveling override */
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;

	/* Rtt and Rtt_WR override */
	popts->rtt_override = 0;

	/* Enable ZQ calibration */
	popts->zq_en = 1;

	/* DHC_EN =1, ODT = 60 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN;
}

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size = 0;

	puts("Initializing....");

	if (fsl_use_spd()) {
		puts("using SPD\n");
		dram_size = fsl_ddr_sdram();
	} else {
		puts("no SPD and fixed parameters\n");
		return dram_size;
	}

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

	debug("    DDR: ");
	return dram_size;
}

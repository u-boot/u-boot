/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or later as published by the Free Software Foundation.
 */

#include <common.h>
#include <i2c.h>
#include <hwconfig.h>
#include <asm/mmu.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>
#include <asm/fsl_law.h>

DECLARE_GLOBAL_DATA_PTR;

struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 clk_adjust;
	u32 wrlvl_start;
	u32 wrlvl_ctl_2;
	u32 wrlvl_ctl_3;
	u32 cpo;
	u32 write_data_delay;
	u32 force_2T;
};

/*
 * This table contains all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */
static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz|adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
	{2,  1350,    5,     7, 0x0809090b, 0x0c0c0d09,   0xff,    2,  0},
	{2,  1666,    5,     8, 0x080a0a0c, 0x0c0d0e0a,   0xff,    2,  0},
	{2,  2140,    5,     8, 0x090a0b0c, 0x0e0f100b,   0xff,    2,  0},
	{1,  1350,    5,     8, 0x0809090b, 0x0c0c0d0a,   0xff,    2,  0},
	{1,  1700,    5,     8, 0x080a0a0c, 0x0c0d0e0a,   0xff,    2,  0},
	{1,  1900,    4,     8, 0x080a0a0c, 0x0e0e0f0a,   0xff,    2,  0},
	{1,  2140,    4,     8, 0x090a0b0c, 0x0e0f100b,   0xff,    2,  0},
	{}
};

/*
 * The three slots have slightly different timing. The center values are good
 * for all slots. We use identical speed tables for them. In future use, if
 * DIMMs require separated tables, make more entries as needed.
 */
static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

static const struct board_specific_parameters rdimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz|adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
	{4,  1350,    5,     9, 0x08070605, 0x07080805,   0xff,    2,  0},
	{4,  1666,    5,     8, 0x08070605, 0x07080805,   0xff,    2,  0},
	{4,  2140,    5,     8, 0x08070605, 0x07081805,   0xff,    2,  0},
	{2,  1350,    5,     7, 0x0809090b, 0x0c0c0d09,   0xff,    2,  0},
	{2,  1666,    5,     8, 0x080a0a0c, 0x0c0d0e0a,   0xff,    2,  0},
	{2,  2140,    5,     8, 0x090a0b0c, 0x0e0f100b,   0xff,    2,  0},
	{1,  1350,    5,     8, 0x0809090b, 0x0c0c0d0a,   0xff,    2,  0},
	{1,  1700,    5,     8, 0x080a0a0c, 0x0c0d0e0a,   0xff,    2,  0},
	{1,  1900,    4,     8, 0x080a0a0c, 0x0e0e0f0a,   0xff,    2,  0},
	{1,  2140,    4,     8, 0x090a0b0c, 0x0e0f100b,   0xff,    2,  0},
	{}
};

/*
 * The three slots have slightly different timing. See comments above.
 */
static const struct board_specific_parameters *rdimms[] = {
	rdimm0,
};

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;

	if (ctrl_num > 2) {
		printf("Not supported controller number %d\n", ctrl_num);
		return;
	}
	if (!pdimm->n_ranks)
		return;

	/*
	 * we use identical timing for all slots. If needed, change the code
	 * to  pbsp = rdimms[ctrl_num] or pbsp = udimms[ctrl_num];
	 */
	if (popts->registered_dimm_en)
		pbsp = rdimms[0];
	else
		pbsp = udimms[0];


	/* Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm->n_ranks) {
			if (ddr_freq <= pbsp->datarate_mhz_high) {
				popts->cpo_override = pbsp->cpo;
				popts->write_data_delay =
					pbsp->write_data_delay;
				popts->clk_adjust = pbsp->clk_adjust;
				popts->wrlvl_start = pbsp->wrlvl_start;
				popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
				popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
				popts->twoT_en = pbsp->force_2T;
				goto found;
			}
			pbsp_highest = pbsp;
		}
		pbsp++;
	}

	if (pbsp_highest) {
		printf("Error: board specific timing not found "
			"for data rate %lu MT/s\n"
			"Trying to use the highest speed (%u) parameters\n",
			ddr_freq, pbsp_highest->datarate_mhz_high);
		popts->cpo_override = pbsp_highest->cpo;
		popts->write_data_delay = pbsp_highest->write_data_delay;
		popts->clk_adjust = pbsp_highest->clk_adjust;
		popts->wrlvl_start = pbsp_highest->wrlvl_start;
		popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
		popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
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
	/*
	 * Write leveling override
	 */
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;

	/*
	 * Rtt and Rtt_WR override
	 */
	popts->rtt_override = 0;

	/* Enable ZQ calibration */
	popts->zq_en = 1;

	/* DHC_EN =1, ODT = 75 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_75ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_75ohm);
}

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size;

	puts("Initializing....using SPD\n");

	dram_size = fsl_ddr_sdram();

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

	puts("    DDR: ");
	return dram_size;
}

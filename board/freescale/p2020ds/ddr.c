/*
 * Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <i2c.h>

#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>

static void get_spd(generic_spd_eeprom_t *spd, unsigned char i2c_address)
{
	i2c_read(i2c_address, 0, 1, (uchar *)spd, sizeof(ddr3_spd_eeprom_t));
}

unsigned int fsl_ddr_get_mem_data_rate(void)
{
	return get_ddr_freq(0);
}

void fsl_ddr_get_spd(generic_spd_eeprom_t *ctrl_dimms_spd,
		      unsigned int ctrl_num)
{
	unsigned int i;
	unsigned int i2c_address = 0;

	for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
		if (ctrl_num == 0 && i == 0)
			i2c_address = SPD_EEPROM_ADDRESS1;
		get_spd(&(ctrl_dimms_spd[i]), i2c_address);
	}
}

typedef struct {
	u32 datarate_mhz_low;
	u32 datarate_mhz_high;
	u32 n_ranks;
	u32 clk_adjust;
	u32 cpo;
	u32 write_data_delay;
	u32 force_2T;
} board_specific_parameters_t;

/* ranges for parameters:
 *  wr_data_delay = 0-6
 *  clk adjust = 0-8
 *  cpo 2-0x1E (30)
 */

const board_specific_parameters_t board_specific_parameters[][20] = {
	{
	/* 	memory controller 0 			*/
	/*	  lo|  hi|  num|  clk| cpo|wrdata|2T	*/
	/*	 mhz| mhz|ranks|adjst|    | delay|	*/
#ifdef CONFIG_FSL_DDR2
		{  0, 333,    2,    4,   0x1f,    2,  0},
		{334, 400,    2,    4,   0x1f,    2,  0},
		{401, 549,    2,    4,   0x1f,    2,  0},
		{550, 680,    2,    4,   0x1f,    3,  0},
		{681, 850,    2,    4,   0x1f,    4,  0},
		{  0, 333,    1,    4,   0x1f,    2,  0},
		{334, 400,    1,    4,   0x1f,    2,  0},
		{401, 549,    1,    4,   0x1f,    2,  0},
		{550, 680,    1,    4,   0x1f,    3,  0},
		{681, 850,    1,    4,   0x1f,    4,  0}
#else
		{  0, 850,    2,    4,   0x1f,    4,  0},
		{  0, 850,    1,    4,   0x1f,    4,  0}
#endif
	},
};

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	const board_specific_parameters_t *pbsp =
				&(board_specific_parameters[ctrl_num][0]);
	u32 num_params = sizeof(board_specific_parameters[ctrl_num]) /
				sizeof(board_specific_parameters[0][0]);
	u32 i;
	ulong ddr_freq;

	/* set odt_rd_cfg and odt_wr_cfg. If the there is only one dimm in
	 * that controller, set odt_wr_cfg to 4 for CS0, and 0 to CS1. If
	 * there are two dimms in the controller, set odt_rd_cfg to 3 and
	 * odt_wr_cfg to 3 for the even CS, 0 for the odd CS.
	 */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
			popts->cs_local_opts[i].odt_rd_cfg = 0;
			popts->cs_local_opts[i].odt_wr_cfg = 1;
	}

	/* Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	for (i = 0; i < num_params; i++) {
		if (ddr_freq >= pbsp->datarate_mhz_low &&
		    ddr_freq <= pbsp->datarate_mhz_high &&
		    pdimm->n_ranks == pbsp->n_ranks) {
			popts->clk_adjust = pbsp->clk_adjust;
			popts->cpo_override = pbsp->cpo;
			popts->write_data_delay = pbsp->write_data_delay;
			popts->twoT_en = pbsp->force_2T;
		}
		pbsp++;
	}

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 0;
	popts->wrlvl_en = 1;
	/* Write leveling override */
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xa;
	popts->wrlvl_start = 0x7;
	/* Rtt and Rtt_WR override */
	popts->rtt_override = 1;
	popts->rtt_override_value = DDR3_RTT_120_OHM;
	popts->rtt_wr_override_value = 0; /* Rtt_WR= dynamic ODT off */
}

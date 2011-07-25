/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
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

DECLARE_GLOBAL_DATA_PTR;


/*
 * Fixed sdram init -- doesn't use serial presence detect.
 */
extern fixed_ddr_parm_t fixed_ddr_parm_0[];
#if (CONFIG_NUM_DDR_CONTROLLERS == 2)
extern fixed_ddr_parm_t fixed_ddr_parm_1[];
#endif

phys_size_t fixed_sdram(void)
{
	int i;
	char buf[32];
	fsl_ddr_cfg_regs_t ddr_cfg_regs;
	phys_size_t ddr_size;
	unsigned int lawbar1_target_id;
	ulong ddr_freq, ddr_freq_mhz;

	ddr_freq = get_ddr_freq(0);
	ddr_freq_mhz = ddr_freq / 1000000;

	printf("Configuring DDR for %s MT/s data rate\n",
				strmhz(buf, ddr_freq));

	for (i = 0; fixed_ddr_parm_0[i].max_freq > 0; i++) {
		if ((ddr_freq_mhz > fixed_ddr_parm_0[i].min_freq) &&
		   (ddr_freq_mhz <= fixed_ddr_parm_0[i].max_freq)) {
			memcpy(&ddr_cfg_regs,
				fixed_ddr_parm_0[i].ddr_settings,
				sizeof(ddr_cfg_regs));
			break;
		}
	}

	if (fixed_ddr_parm_0[i].max_freq == 0)
		panic("Unsupported DDR data rate %s MT/s data rate\n",
			strmhz(buf, ddr_freq));

	ddr_size = (phys_size_t) CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
	ddr_cfg_regs.ddr_cdr1 = DDR_CDR1_DHC_EN;
	fsl_ddr_set_memctl_regs(&ddr_cfg_regs, 0);

#if (CONFIG_NUM_DDR_CONTROLLERS == 2)
	memcpy(&ddr_cfg_regs,
		fixed_ddr_parm_1[i].ddr_settings,
		sizeof(ddr_cfg_regs));
	ddr_cfg_regs.ddr_cdr1 = DDR_CDR1_DHC_EN;
	fsl_ddr_set_memctl_regs(&ddr_cfg_regs, 1);
#endif

	/*
	 * setup laws for DDR. If not interleaving, presuming half memory on
	 * DDR1 and the other half on DDR2
	 */
	if (fixed_ddr_parm_0[i].ddr_settings->cs[0].config & 0x20000000) {
		if (set_ddr_laws(CONFIG_SYS_DDR_SDRAM_BASE,
				 ddr_size,
				 LAW_TRGT_IF_DDR_INTRLV) < 0) {
			printf("ERROR setting Local Access Windows for DDR\n");
			return 0;
		}
	} else {
#if (CONFIG_NUM_DDR_CONTROLLERS == 2)
		/* We require both controllers have identical DIMMs */
		lawbar1_target_id = LAW_TRGT_IF_DDR_1;
		if (set_ddr_laws(CONFIG_SYS_DDR_SDRAM_BASE,
				 ddr_size / 2,
				 lawbar1_target_id) < 0) {
			printf("ERROR setting Local Access Windows for DDR\n");
			return 0;
		}
		lawbar1_target_id = LAW_TRGT_IF_DDR_2;
		if (set_ddr_laws(CONFIG_SYS_DDR_SDRAM_BASE + ddr_size / 2,
				 ddr_size / 2,
				 lawbar1_target_id) < 0) {
			printf("ERROR setting Local Access Windows for DDR\n");
			return 0;
		}
#else
		lawbar1_target_id = LAW_TRGT_IF_DDR_1;
		if (set_ddr_laws(CONFIG_SYS_DDR_SDRAM_BASE,
				 ddr_size,
				 lawbar1_target_id) < 0) {
			printf("ERROR setting Local Access Windows for DDR\n");
			return 0;
		}
#endif
	}
	return ddr_size;
}

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

/* ranges for parameters:
 *  wr_data_delay = 0-6
 *  clk adjust = 0-8
 *  cpo 2-0x1E (30)
 */


/* XXX: these values need to be checked for all interleaving modes.  */
/* XXX: No reliable dual-rank 800 MHz setting has been found.  It may
 *      seem reliable, but errors will appear when memory intensive
 *      program is run. */
/* XXX: Single rank at 800 MHz is OK.  */
const board_specific_parameters_t board_specific_parameters[][30] = {
	{
	/*
	 * memory controller 0
	 *  lo|  hi|  num|  clk| wrlvl | cpo  |wrdata|2T
	 * mhz| mhz|ranks|adjst| start | delay|
	 */
		{  0, 850,    4,    4,     6,   0xff,    2,  0},
		{851, 950,    4,    5,     7,   0xff,    2,  0},
		{951, 1050,   4,    5,     8,   0xff,    2,  0},
		{1051, 1250,  4,    5,    10,   0xff,    2,  0},
		{1251, 1350,  4,    5,    11,   0xff,    2,  0},
		{  0, 850,    2,    5,     6,   0xff,    2,  0},
		{851, 950,    2,    5,     7,   0xff,    2,  0},
		{951, 1050,   2,    5,     7,   0xff,    2,  0},
		{1051, 1250,  2,    4,     6,   0xff,    2,  0},
		{1251, 1350,  2,    5,     7,   0xff,    2,  0},
	},

	{
	/*
	 * memory controller 1
	 *  lo|  hi|  num|  clk| wrlvl | cpo  |wrdata|2T
	 * mhz| mhz|ranks|adjst| start | delay|
	 */
		{  0, 850,    4,    4,     6,   0xff,    2,  0},
		{851, 950,    4,    5,     7,   0xff,    2,  0},
		{951, 1050,   4,    5,     8,   0xff,    2,  0},
		{1051, 1250,  4,    5,    10,   0xff,    2,  0},
		{1251, 1350,  4,    5,    11,   0xff,    2,  0},
		{  0, 850,    2,    5,     6,   0xff,    2,  0},
		{851, 950,    2,    5,     7,   0xff,    2,  0},
		{951, 1050,   2,    5,     7,   0xff,    2,  0},
		{1051, 1250,  2,    4,     6,   0xff,    2,  0},
		{1251, 1350,  2,    5,     7,   0xff,    2,  0},
	}
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

	/* Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
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

	/* DHC_EN =1, ODT = 60 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN;

	/* override SPD values. rcw_2 should vary at differnt speed */
	if (pdimm[0].registered_dimm == 1) {
		popts->rcw_override = 1;
		popts->rcw_1 = 0x000a5a00;
		if (ddr_freq <= 800)
			popts->rcw_2 = 0x00000000;
		else if (ddr_freq <= 1066)
			popts->rcw_2 = 0x00100000;
		else if (ddr_freq <= 1333)
			popts->rcw_2 = 0x00200000;
		else
			popts->rcw_2 = 0x00300000;
	}
}

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size;

	puts("Initializing....");

	if (fsl_use_spd()) {
		puts("using SPD\n");
		dram_size = fsl_ddr_sdram();
	} else {
		puts("using fixed parameters\n");
		dram_size = fixed_sdram();
	}

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

	debug("    DDR: ");
	return dram_size;
}

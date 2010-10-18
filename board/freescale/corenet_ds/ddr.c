/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc.
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

extern void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
				   unsigned int ctrl_num);


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
	sys_info_t sysinfo;
	char buf[32];
	fsl_ddr_cfg_regs_t ddr_cfg_regs;
	phys_size_t ddr_size;
	unsigned int lawbar1_target_id;

	get_sys_info(&sysinfo);
	printf("Configuring DDR for %s MT/s data rate\n",
				strmhz(buf, sysinfo.freqDDRBus));

	for (i = 0; fixed_ddr_parm_0[i].max_freq > 0; i++) {
		if ((sysinfo.freqDDRBus > fixed_ddr_parm_0[i].min_freq) &&
		   (sysinfo.freqDDRBus <= fixed_ddr_parm_0[i].max_freq)) {
			memcpy(&ddr_cfg_regs,
				fixed_ddr_parm_0[i].ddr_settings,
				sizeof(ddr_cfg_regs));
			break;
		}
	}

	if (fixed_ddr_parm_0[i].max_freq == 0)
		panic("Unsupported DDR data rate %s MT/s data rate\n",
			strmhz(buf, sysinfo.freqDDRBus));

	ddr_size = (phys_size_t) CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;
	fsl_ddr_set_memctl_regs(&ddr_cfg_regs, 0);

#if (CONFIG_NUM_DDR_CONTROLLERS == 2)
	memcpy(&ddr_cfg_regs,
		fixed_ddr_parm_1[i].ddr_settings,
		sizeof(ddr_cfg_regs));
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

static void get_spd(ddr3_spd_eeprom_t *spd, unsigned char i2c_address)
{
	int ret;

	ret = i2c_read(i2c_address, 0, 1, (uchar *)spd, sizeof(ddr3_spd_eeprom_t));
	if (ret) {
		debug("DDR: failed to read SPD from address %u\n", i2c_address);
		memset(spd, 0, sizeof(ddr3_spd_eeprom_t));
	}
}

unsigned int fsl_ddr_get_mem_data_rate(void)
{
	return get_ddr_freq(0);
}

void fsl_ddr_get_spd(ddr3_spd_eeprom_t *ctrl_dimms_spd,
		      unsigned int ctrl_num)
{
	unsigned int i;
	unsigned int i2c_address = 0;

	for (i = 0; i < CONFIG_DIMM_SLOTS_PER_CTLR; i++) {
		if (ctrl_num == 0 && i == 0)
			i2c_address = SPD_EEPROM_ADDRESS1;
		else if (ctrl_num == 1 && i == 0)
			i2c_address = SPD_EEPROM_ADDRESS2;

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


/* XXX: these values need to be checked for all interleaving modes.  */
/* XXX: No reliable dual-rank 800 MHz setting has been found.  It may
 *      seem reliable, but errors will appear when memory intensive
 *      program is run. */
/* XXX: Single rank at 800 MHz is OK.  */
const board_specific_parameters_t board_specific_parameters[][30] = {
	{
	/* 	memory controller 0 			*/
	/*	  lo|  hi|  num|  clk| cpo|wrdata|2T	*/
	/*	 mhz| mhz|ranks|adjst|    | delay|	*/
		{  0, 333,    4,    6,   7,    3,  0},
		{334, 400,    4,    6,   9,    3,  0},
		{401, 549,    4,    6,  11,    3,  0},
		{550, 680,    4,    1,  10,    5,  0},
		{681, 850,    4,    1,  12,    5,  0},
		{851, 1050,   4,    1,  12,    5,  0},
		{1051, 1250,  4,    1,  15,    4,  0},
		{1251, 1350,  4,    1,  15,    4,  0},
		{  0, 333,    2,    6,   7,    3,  0},
		{334, 400,    2,    6,   9,    3,  0},
		{401, 549,    2,    6,  11,    3,  0},
		{550, 680,    2,    1,  10,    5,  0},
		{681, 850,    2,    1,  12,    5,  0},
		{851, 1050,   2,    1,  12,    5,  0},
		{1051, 1250,  2,    1,  15,    4,  0},
		{1251, 1350,  2,    1,  15,    4,  0},
		{  0, 333,    1,    6,   7,    3,  0},
		{334, 400,    1,    6,   9,    3,  0},
		{401, 549,    1,    6,  11,    3,  0},
		{550, 680,    1,    1,  10,    5,  0},
		{681, 850,    1,    1,  12,    5,  0}
	},

	{
	/*	memory controller 1			*/
	/*	  lo|  hi|  num|  clk| cpo|wrdata|2T	*/
	/*	 mhz| mhz|ranks|adjst|    | delay|	*/
		{  0, 333,    4,    6,   7,    3,  0},
		{334, 400,    4,    6,   9,    3,  0},
		{401, 549,    4,    6,  11,    3,  0},
		{550, 680,    4,    1,  10,    5,  0},
		{681, 850,    4,    1,  12,    5,  0},
		{851, 1050,   4,    1,  12,    5,  0},
		{1051, 1250,  4,    1,  15,    4,  0},
		{1251, 1350,  4,    1,  15,    4,  0},
		{  0, 333,    2,     6,  7,    3,  0},
		{334, 400,    2,     6,  9,    3,  0},
		{401, 549,    2,     6, 11,    3,  0},
		{550, 680,    2,     1, 11,    6,  0},
		{681, 850,    2,     1, 13,    6,  0},
		{851, 1050,   2,     1, 13,    6,  0},
		{1051, 1250,  2,     1, 15,    4,  0},
		{1251, 1350,  2,     1, 15,    4,  0},
		{  0, 333,    1,     6,  7,    3,  0},
		{334, 400,    1,     6,  9,    3,  0},
		{401, 549,    1,     6, 11,    3,  0},
		{550, 680,    1,     1, 11,    6,  0},
		{681, 850,    1,     1, 13,    6,  0}
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

	/* set odt_rd_cfg and odt_wr_cfg. If the there is only one dimm in
	 * that controller, set odt_wr_cfg to 4 for CS0, and 0 to CS1. If
	 * there are two dimms in the controller, set odt_rd_cfg to 3 and
	 * odt_wr_cfg to 3 for the even CS, 0 for the odd CS.
	 */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (i&1) {	/* odd CS */
			popts->cs_local_opts[i].odt_rd_cfg = 0;
			popts->cs_local_opts[i].odt_wr_cfg = 1;
		} else {	/* even CS */
			if (CONFIG_DIMM_SLOTS_PER_CTLR == 1) {
				popts->cs_local_opts[i].odt_rd_cfg = 0;
				popts->cs_local_opts[i].odt_wr_cfg = 1;
			} else if (CONFIG_DIMM_SLOTS_PER_CTLR == 2) {
			popts->cs_local_opts[i].odt_rd_cfg = 3;
			popts->cs_local_opts[i].odt_wr_cfg = 3;
			}
		}
	}

	/* Get clk_adjust, cpo, write_data_delay,2T, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	for (i = 0; i < num_params; i++) {
		if (ddr_freq >= pbsp->datarate_mhz_low &&
		    ddr_freq <= pbsp->datarate_mhz_high &&
		    pdimm->n_ranks == pbsp->n_ranks) {
			popts->cpo_override = 0xff; /* force auto CPO calibration */
			popts->write_data_delay = 2;
			popts->clk_adjust = 5; /* Force value to be 5/8 clock cycle */
			popts->twoT_en = pbsp->force_2T;
		}
		pbsp++;
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
	popts->wrlvl_sample = 0xa;
	popts->wrlvl_start = 0x7;
	/*
	 * Rtt and Rtt_WR override
	 */
	popts->rtt_override = 1;
	popts->rtt_override_value = DDR3_RTT_120_OHM;
	popts->rtt_wr_override_value = 0; /* Rtt_WR= dynamic ODT off */

	/* Enable ZQ calibration */
	popts->zq_en = 1;
}

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size;
	int use_spd = 0;

	puts("Initializing....");

#ifdef CONFIG_DDR_SPD
	/* if hwconfig is not enabled, or "sdram" is not defined, use spd */
	if (hwconfig_sub("fsl_ddr", "sdram")) {
		if (hwconfig_subarg_cmp("fsl_ddr", "sdram", "spd"))
			use_spd = 1;
		else if (hwconfig_subarg_cmp("fsl_ddr", "sdram", "fixed"))
			use_spd = 0;
		else
			use_spd = 1;
	} else
		use_spd = 1;
#endif

	if (use_spd) {
		puts("using SPD\n");
		dram_size = fsl_ddr_sdram();
	} else {
		puts("using fixed parameters\n");
		dram_size = fixed_sdram();
	}

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

	puts("    DDR: ");
	return dram_size;
}

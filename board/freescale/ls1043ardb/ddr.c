/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include "ddr.h"
#ifdef CONFIG_FSL_DEEP_SLEEP
#include <fsl_sleep.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

void fsl_ddr_board_options(memctl_options_t *popts,
			   dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;

	if (ctrl_num > 1) {
		printf("Not supported controller number %d\n", ctrl_num);
		return;
	}
	if (!pdimm->n_ranks)
		return;

	pbsp = udimms[0];

	/* Get clk_adjust, wrlvl_start, wrlvl_ctl, according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm->n_ranks) {
			if (ddr_freq <= pbsp->datarate_mhz_high) {
				popts->clk_adjust = pbsp->clk_adjust;
				popts->wrlvl_start = pbsp->wrlvl_start;
				popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
				popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
				popts->cpo_override = pbsp->cpo_override;
				popts->write_data_delay =
					pbsp->write_data_delay;
				goto found;
			}
			pbsp_highest = pbsp;
		}
		pbsp++;
	}

	if (pbsp_highest) {
		printf("Error: board specific timing not found for %lu MT/s\n",
		       ddr_freq);
		printf("Trying to use the highest speed (%u) parameters\n",
		       pbsp_highest->datarate_mhz_high);
		popts->clk_adjust = pbsp_highest->clk_adjust;
		popts->wrlvl_start = pbsp_highest->wrlvl_start;
		popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
		popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
	} else {
		panic("DIMM is not supported by this board");
	}
found:
	debug("Found timing match: n_ranks %d, data rate %d, rank_gb %d\n",
	      pbsp->n_ranks, pbsp->datarate_mhz_high, pbsp->rank_gb);

	/* force DDR bus width to 32 bits */
	popts->data_bus_width = 1;
	popts->otf_burst_chop_en = 0;
	popts->burst_length = DDR_BL8;

	/*
	 * Factors to consider for half-strength driver enable:
	 *	- number of DIMMs installed
	 */
	popts->half_strength_driver_enable = 1;
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

	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_80ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_80ohm) |
			  DDR_CDR2_VREF_OVRD(70);	/* Vref = 70% */
}

/* DDR model number: MT40A512M8HX-093E */
#ifdef CONFIG_SYS_DDR_RAW_TIMING
dimm_params_t ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 2147483648u,
	.capacity = 2147483648u,
	.primary_sdram_width = 32,
	.ec_sdram_width = 0,
	.registered_dimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 15,
	.n_col_addr = 10,
	.bank_addr_bits = 0,
	.bank_group_bits = 2,
	.edc_config = 0,
	.burst_lengths_bitmask = 0x0c,

	.tckmin_x_ps = 938,
	.tckmax_ps = 1500,
	.caslat_x = 0x000DFA00,
	.taa_ps = 13500,
	.trcd_ps = 13500,
	.trp_ps = 13500,
	.tras_ps = 33000,
	.trc_ps = 46500,
	.trfc1_ps = 260000,
	.trfc2_ps = 160000,
	.trfc4_ps = 110000,
	.tfaw_ps = 21000,
	.trrds_ps = 3700,
	.trrdl_ps = 5300,
	.tccdl_ps = 5355,
	.refresh_rate_ps = 7800000,
	.dq_mapping[0] = 0x0,
	.dq_mapping[1] = 0x0,
	.dq_mapping[2] = 0x0,
	.dq_mapping[3] = 0x0,
	.dq_mapping[4] = 0x0,
	.dq_mapping[5] = 0x0,
	.dq_mapping[6] = 0x0,
	.dq_mapping[7] = 0x0,
	.dq_mapping[8] = 0x0,
	.dq_mapping[9] = 0x0,
	.dq_mapping[10] = 0x0,
	.dq_mapping[11] = 0x0,
	.dq_mapping[12] = 0x0,
	.dq_mapping[13] = 0x0,
	.dq_mapping[14] = 0x0,
	.dq_mapping[15] = 0x0,
	.dq_mapping[16] = 0x0,
	.dq_mapping[17] = 0x0,
	.dq_mapping_ors = 0,
};

int fsl_ddr_get_dimm_params(dimm_params_t *pdimm,
			    unsigned int controller_number,
			    unsigned int dimm_number)
{
	static const char dimm_model[] = "Fixed DDR on board";

	if (((controller_number == 0) && (dimm_number == 0)) ||
	    ((controller_number == 1) && (dimm_number == 0))) {
		memcpy(pdimm, &ddr_raw_timing, sizeof(dimm_params_t));
		memset(pdimm->mpart, 0, sizeof(pdimm->mpart));
		memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);
	}

	return 0;
}
#endif

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size;

#if defined(CONFIG_SPL_BUILD) || !defined(CONFIG_SPL)
	puts("Initializing DDR....\n");
	dram_size = fsl_ddr_sdram();
#else
	dram_size =  fsl_ddr_sdram_size();
#endif
#ifdef CONFIG_FSL_DEEP_SLEEP
	fsl_dp_ddr_restore();
#endif

	return dram_size;
}

void dram_init_banksize(void)
{
	/*
	 * gd->secure_ram tracks the location of secure memory.
	 * It was set as if the memory starts from 0.
	 * The address needs to add the offset of its bank.
	 */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	if (gd->ram_size > CONFIG_SYS_DDR_BLOCK1_SIZE) {
		gd->bd->bi_dram[0].size = CONFIG_SYS_DDR_BLOCK1_SIZE;
		gd->bd->bi_dram[1].start = CONFIG_SYS_DDR_BLOCK2_BASE;
		gd->bd->bi_dram[1].size = gd->ram_size -
					  CONFIG_SYS_DDR_BLOCK1_SIZE;
#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
		gd->secure_ram = gd->bd->bi_dram[1].start +
				 gd->secure_ram -
				 CONFIG_SYS_DDR_BLOCK1_SIZE;
		gd->secure_ram |= MEM_RESERVE_SECURE_MAINTAINED;
#endif
	} else {
		gd->bd->bi_dram[0].size = gd->ram_size;
#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
		gd->secure_ram = gd->bd->bi_dram[0].start + gd->secure_ram;
		gd->secure_ram |= MEM_RESERVE_SECURE_MAINTAINED;
#endif
	}
}

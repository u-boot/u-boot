// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <linux/delay.h>
#include "ddr.h"

DECLARE_GLOBAL_DATA_PTR;

void fsl_ddr_board_options(memctl_options_t *popts,
			   dimm_params_t *pdimm,
			   unsigned int ctrl_num)
{
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;

	if (ctrl_num > 3) {
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

#ifdef CONFIG_SYS_FSL_DDR4
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_80ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_80ohm) |
			  DDR_CDR2_VREF_OVRD(70);	/* Vref = 70% */
#else
	popts->cswl_override = DDR_CSWL_CS0;

	/* optimize cpo for erratum A-009942 */
	popts->cpo_sample = 0x58;

	/* DHC_EN =1, ODT = 75 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_75ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_75ohm);
#endif
}

#ifdef CONFIG_SYS_DDR_RAW_TIMING
dimm_params_t ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 1073741824u,
	.capacity = 1073741824u,
	.primary_sdram_width = 32,
	.ec_sdram_width = 0,
	.registered_dimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 15,
	.n_col_addr = 10,
	.n_banks_per_sdram_device = 8,
	.edc_config = 0,
	.burst_lengths_bitmask = 0x0c,

	.tckmin_x_ps = 1071,
	.caslat_x = 0xfe << 4,	/* 5,6,7,8 */
	.taa_ps = 13125,
	.twr_ps = 15000,
	.trcd_ps = 13125,
	.trrd_ps = 7500,
	.trp_ps = 13125,
	.tras_ps = 37500,
	.trc_ps = 50625,
	.trfc_ps = 160000,
	.twtr_ps = 7500,
	.trtp_ps = 7500,
	.refresh_rate_ps = 7800000,
	.tfaw_ps = 37500,
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

#if defined(CONFIG_DEEP_SLEEP)
void board_mem_sleep_setup(void)
{
	void __iomem *qixis_base = (void *)QIXIS_BASE;

	/* does not provide HW signals for power management */
	clrbits_8(qixis_base + 0x21, 0x2);
	udelay(1);
}
#endif

int fsl_initdram(void)
{
	phys_size_t dram_size;

#if defined(CONFIG_XPL_BUILD) || !defined(CONFIG_SPL)
	puts("Initializing DDR....using SPD\n");
	dram_size = fsl_ddr_sdram();
#else
	dram_size =  fsl_ddr_sdram_size();
#endif

#if defined(CONFIG_DEEP_SLEEP) && !defined(CONFIG_XPL_BUILD)
	fsl_dp_resume();
#endif

	erratum_a008850_post();

	gd->ram_size = dram_size;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

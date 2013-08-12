/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/fsl_law.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>

/*
 * Micron MT41J128M16HA-15E
 * */
dimm_params_t ddr_raw_timing = {
	.n_ranks = 1,
	.rank_density = 536870912u,
	.capacity = 536870912u,
	.primary_sdram_width = 32,
	.ec_sdram_width = 8,
	.registered_dimm = 0,
	.mirrored_dimm = 0,
	.n_row_addr = 14,
	.n_col_addr = 10,
	.n_banks_per_sdram_device = 8,
	.edc_config = 2,
	.burst_lengths_bitmask = 0x0c,

	.tCKmin_X_ps = 1650,
	.caslat_X = 0x7e << 4,	/* 5,6,7,8,9,10 */
	.tAA_ps = 14050,
	.tWR_ps = 15000,
	.tRCD_ps = 13500,
	.tRRD_ps = 75000,
	.tRP_ps = 13500,
	.tRAS_ps = 40000,
	.tRC_ps = 49500,
	.tRFC_ps = 160000,
	.tWTR_ps = 75000,
	.tRTP_ps = 75000,
	.refresh_rate_ps = 7800000,
	.tFAW_ps = 30000,
};

int fsl_ddr_get_dimm_params(dimm_params_t *pdimm,
		unsigned int controller_number,
		unsigned int dimm_number)
{
	const char dimm_model[] = "Fixed DDR on board";

	if ((controller_number == 0) && (dimm_number == 0)) {
		memcpy(pdimm, &ddr_raw_timing, sizeof(dimm_params_t));
		memset(pdimm->mpart, 0, sizeof(pdimm->mpart));
		memcpy(pdimm->mpart, dimm_model, sizeof(dimm_model) - 1);
	}

	return 0;
}

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	int i;
	popts->clk_adjust = 2;
	popts->cpo_override = 0x1f;
	popts->write_data_delay = 4;
	popts->half_strength_driver_enable = 1;
	popts->bstopre = 0x3cf;
	popts->quad_rank_present = 1;
	popts->rtt_override = 1;
	popts->rtt_override_value = 1;
	popts->dynamic_power = 1;
	/* Write leveling override */
	popts->wrlvl_en = 1;
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;
	popts->wrlvl_start = 0x4;
	popts->trwt_override = 1;
	popts->trwt = 0;

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		popts->cs_local_opts[i].odt_rd_cfg = FSL_DDR_ODT_NEVER;
		popts->cs_local_opts[i].odt_wr_cfg = FSL_DDR_ODT_CS;
	}
}

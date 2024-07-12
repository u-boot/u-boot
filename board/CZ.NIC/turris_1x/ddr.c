// SPDX-License-Identifier: GPL-2.0+
// (C) 2022 Pali Rohár <pali@kernel.org>

#include <config.h>
#include <linux/types.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

void fsl_ddr_board_options(memctl_options_t *popts, dimm_params_t *pdimm, unsigned int ctrl_num)
{
	int i;

	popts->clk_adjust = 6;
	popts->cpo_override = 0x1f;
	popts->write_data_delay = 2;
	popts->half_strength_driver_enable = 1;
	popts->wrlvl_en = 1;
	popts->wrlvl_override = 1;
	popts->wrlvl_sample = 0xf;
	popts->wrlvl_start = 0x8;
	popts->trwt_override = 1;
	popts->trwt = 0;

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		popts->cs_local_opts[i].odt_rd_cfg = FSL_DDR_ODT_NEVER;
		popts->cs_local_opts[i].odt_wr_cfg = FSL_DDR_ODT_CS;
	}
}

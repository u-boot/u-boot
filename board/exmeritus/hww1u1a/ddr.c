/*
 * Copyright 2009-2010 eXMeritus, A Boeing Company
 * Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>

#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	/*
	 * We only support one DIMM, so according to the P2020 docs we should
	 * set the options as follows:
	 */
	popts->cs_local_opts[0].odt_rd_cfg = 0;
	popts->cs_local_opts[0].odt_wr_cfg = 4;
	popts->cs_local_opts[1].odt_rd_cfg = 0;
	popts->cs_local_opts[1].odt_wr_cfg = 0;
	popts->half_strength_driver_enable = 0;

	/* Manually configured for our static clock rate */
	popts->clk_adjust = 4;
	popts->cpo_override = 4;
	popts->write_data_delay = 2;
	popts->twot_en = 0;
}

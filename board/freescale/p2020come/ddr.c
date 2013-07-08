/*
 * Copyright 2009, 2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_ddr_dimm_params.h>

void fsl_ddr_board_options(memctl_options_t *popts,
				dimm_params_t *pdimm,
				unsigned int ctrl_num)
{
	if (ctrl_num) {
		printf("Wrong parameter for controller number %d", ctrl_num);
		return;
	}

	if (!pdimm->n_ranks)
		return;

	/*
	 * Set DDR_SDRAM_CLK_CNTL = 0x02800000
	 *
	 * Clock is launched 5/8 applied cycle after address/command
	 */
	popts->clk_adjust = 5;
}

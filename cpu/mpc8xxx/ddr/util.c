/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <asm/fsl_law.h>

#include "ddr.h"

unsigned int fsl_ddr_get_mem_data_rate(void);

/*
 * Round mclk_ps to nearest 10 ps in memory controller code.
 *
 * If an imprecise data rate is too high due to rounding error
 * propagation, compute a suitably rounded mclk_ps to compute
 * a working memory controller configuration.
 */
unsigned int get_memory_clk_period_ps(void)
{
	unsigned int mclk_ps;

	mclk_ps = 2000000000000ULL / fsl_ddr_get_mem_data_rate();
	/* round to nearest 10 ps */
	return 10 * ((mclk_ps + 5) / 10);
}

/* Convert picoseconds into DRAM clock cycles (rounding up if needed). */
unsigned int picos_to_mclk(unsigned int picos)
{
	const unsigned long long ULL_2e12 = 2000000000000ULL;
	const unsigned long long ULL_8Fs = 0xFFFFFFFFULL;
	unsigned long long clks;
	unsigned long long clks_temp;

	if (!picos)
		return 0;

	clks = fsl_ddr_get_mem_data_rate() * (unsigned long long) picos;
	clks_temp = clks;
	clks = clks / ULL_2e12;
	if (clks_temp % ULL_2e12) {
		clks++;
	}

	if (clks > ULL_8Fs) {
		clks = ULL_8Fs;
	}

	return (unsigned int) clks;
}

unsigned int mclk_to_picos(unsigned int mclk)
{
	return get_memory_clk_period_ps() * mclk;
}

void
__fsl_ddr_set_lawbar(const common_timing_params_t *memctl_common_params,
			   unsigned int memctl_interleaved,
			   unsigned int ctrl_num)
{
	/*
	 * If no DIMMs on this controller, do not proceed any further.
	 */
	if (!memctl_common_params->ndimms_present) {
		return;
	}

	if (ctrl_num == 0) {
		/*
		 * Set up LAW for DDR controller 1 space.
		 */
		unsigned int lawbar1_target_id = memctl_interleaved
			? LAW_TRGT_IF_DDR_INTRLV : LAW_TRGT_IF_DDR_1;

		if (set_ddr_laws(memctl_common_params->base_address,
				memctl_common_params->total_mem,
				lawbar1_target_id) < 0) {
			printf("ERROR\n");
			return ;
		}
	} else if (ctrl_num == 1) {
		if (set_ddr_laws(memctl_common_params->base_address,
				memctl_common_params->total_mem,
				LAW_TRGT_IF_DDR_2) < 0) {
			printf("ERROR\n");
			return ;
		}
	} else {
		printf("unexpected controller number %u in %s\n",
			ctrl_num, __FUNCTION__);
	}
}

__attribute__((weak, alias("__fsl_ddr_set_lawbar"))) void
fsl_ddr_set_lawbar(const common_timing_params_t *memctl_common_params,
			 unsigned int memctl_interleaved,
			 unsigned int ctrl_num);

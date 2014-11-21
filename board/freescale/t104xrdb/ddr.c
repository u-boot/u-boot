/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <hwconfig.h>
#include <asm/mmu.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <asm/fsl_law.h>
#include <asm/mpc85xx_gpio.h>
#include "ddr.h"

DECLARE_GLOBAL_DATA_PTR;

int fsl_ddr_get_dimm_params(dimm_params_t *pdimm,
		unsigned int controller_number,
		unsigned int dimm_number)
{
	const char dimm_model[] = "RAW timing DDR";

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
	const struct board_specific_parameters *pbsp, *pbsp_highest = NULL;
	ulong ddr_freq;

	if (ctrl_num > 1) {
		printf("Not supported controller number %d\n", ctrl_num);
		return;
	}
	if (!pdimm->n_ranks)
		return;

	pbsp = udimms[0];

	/* Get clk_adjust according to the board ddr
	 * freqency and n_banks specified in board_specific_parameters table.
	 */
	ddr_freq = get_ddr_freq(0) / 1000000;
	while (pbsp->datarate_mhz_high) {
		if (pbsp->n_ranks == pdimm->n_ranks &&
		    (pdimm->rank_density >> 30) >= pbsp->rank_gb) {
			if (ddr_freq <= pbsp->datarate_mhz_high) {
				popts->clk_adjust = pbsp->clk_adjust;
				popts->wrlvl_start = pbsp->wrlvl_start;
				popts->wrlvl_ctl_2 = pbsp->wrlvl_ctl_2;
				popts->wrlvl_ctl_3 = pbsp->wrlvl_ctl_3;
				goto found;
			}
			pbsp_highest = pbsp;
		}
		pbsp++;
	}

	if (pbsp_highest) {
		printf("Error: board specific timing not found\n");
		printf("for data rate %lu MT/s\n", ddr_freq);
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
	debug("Found timing match: n_ranks %d, data rate %d, rank_gb %d\n"
		"\tclk_adjust %d, wrlvl_start %d, wrlvl_ctrl_2 0x%x, "
		"wrlvl_ctrl_3 0x%x\n",
		pbsp->n_ranks, pbsp->datarate_mhz_high, pbsp->rank_gb,
		pbsp->clk_adjust, pbsp->wrlvl_start, pbsp->wrlvl_ctl_2,
		pbsp->wrlvl_ctl_3);

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
	 * rtt and rtt_wr override
	 */
	popts->rtt_override = 0;

	/* Enable ZQ calibration */
	popts->zq_en = 1;

	/* DHC_EN =1, ODT = 75 Ohm */
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_75ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_75ohm);
}

#if defined(CONFIG_DEEP_SLEEP)
void board_mem_sleep_setup(void)
{
	void __iomem *cpld_base = (void *)CONFIG_SYS_CPLD_BASE;

	/* does not provide HW signals for power management */
	clrbits_8(cpld_base + 0x17, 0x40);
	/* Disable MCKE isolation */
	gpio_set_value(2, 0);
	udelay(1);
}
#endif

phys_size_t initdram(int board_type)
{
	phys_size_t dram_size;

#if defined(CONFIG_SPL_BUILD) || !defined(CONFIG_RAMBOOT_PBL)
	puts("Initializing....using SPD\n");

	dram_size = fsl_ddr_sdram();

	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

#else
	dram_size =  fsl_ddr_sdram_size();
#endif

#if defined(CONFIG_DEEP_SLEEP) && !defined(CONFIG_SPL_BUILD)
	fsl_dp_resume();
#endif

	return dram_size;
}

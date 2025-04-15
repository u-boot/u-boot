// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 * Copyright 2021 NXP
 */

#include <config.h>
#include <i2c.h>
#include <hwconfig.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/mmu.h>
#include <fsl_ddr_sdram.h>
#include <fsl_ddr_dimm_params.h>
#include <asm/fsl_law.h>
#include <asm/mpc85xx_gpio.h>
#include <linux/delay.h>
#include "ddr.h"

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
#ifdef CONFIG_SYS_FSL_DDR4
	popts->half_strength_driver_enable = 1;
	/* optimize cpo for erratum A-009942 */
	popts->cpo_sample = 0x59;
#else
	popts->cpo_sample = 0x54;
	popts->half_strength_driver_enable = 0;
#endif
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
#ifdef CONFIG_SYS_FSL_DDR4
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_120OHM);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_120OHM) |
		DDR_CDR2_VREF_OVRD(70);       /* Vref = 70% */
#else
	popts->ddr_cdr1 = DDR_CDR1_DHC_EN | DDR_CDR1_ODT(DDR_CDR_ODT_75ohm);
	popts->ddr_cdr2 = DDR_CDR2_ODT(DDR_CDR_ODT_75ohm);
#endif
}

#if defined(CONFIG_DEEP_SLEEP)
void board_mem_sleep_setup(void)
{
	void __iomem *cpld_base = (void *)CFG_SYS_CPLD_BASE;

	/* does not provide HW signals for power management */
	clrbits_8(cpld_base + 0x17, 0x40);
	/* Disable MCKE isolation */
	gpio_set_value(2, 0);
	udelay(1);
}
#endif

int dram_init(void)
{
	phys_size_t dram_size;

#if defined(CONFIG_XPL_BUILD) || !defined(CONFIG_RAMBOOT_PBL)
	puts("Initializing....using SPD\n");
	dram_size = fsl_ddr_sdram();
#else
	dram_size =  fsl_ddr_sdram_size();
#endif
	dram_size = setup_ddr_tlbs(dram_size / 0x100000);
	dram_size *= 0x100000;

#if defined(CONFIG_DEEP_SLEEP) && !defined(CONFIG_XPL_BUILD)
	fsl_dp_resume();
#endif

	gd->ram_size = dram_size;

	return 0;
}

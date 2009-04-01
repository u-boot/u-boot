/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <asm/fsl_ddr_sdram.h>

#include "ddr.h"

/* Board-specific functions defined in each board's ddr.c */
extern void fsl_ddr_board_options(memctl_options_t *popts,
		dimm_params_t *pdimm,
		unsigned int ctrl_num);

unsigned int populate_memctl_options(int all_DIMMs_registered,
			memctl_options_t *popts,
			dimm_params_t *pdimm,
			unsigned int ctrl_num)
{
	unsigned int i;
#if (CONFIG_NUM_DDR_CONTROLLERS > 1)
	const char *p;
#endif

	/* Chip select options. */

	/* Pick chip-select local options. */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		/* If not DDR2, odt_rd_cfg and odt_wr_cfg need to be 0. */

		/* only for single CS? */
		popts->cs_local_opts[i].odt_rd_cfg = 0;

		popts->cs_local_opts[i].odt_wr_cfg = 1;
		popts->cs_local_opts[i].auto_precharge = 0;
	}

	/* Pick interleaving mode. */

	/*
	 * 0 = no interleaving
	 * 1 = interleaving between 2 controllers
	 */
	popts->memctl_interleaving = 0;

	/*
	 * 0 = cacheline
	 * 1 = page
	 * 2 = (logical) bank
	 * 3 = superbank (only if CS interleaving is enabled)
	 */
	popts->memctl_interleaving_mode = 0;

	/*
	 * 0: cacheline: bit 30 of the 36-bit physical addr selects the memctl
	 * 1: page:      bit to the left of the column bits selects the memctl
	 * 2: bank:      bit to the left of the bank bits selects the memctl
	 * 3: superbank: bit to the left of the chip select selects the memctl
	 *
	 * NOTE: ba_intlv (rank interleaving) is independent of memory
	 * controller interleaving; it is only within a memory controller.
	 * Must use superbank interleaving if rank interleaving is used and
	 * memory controller interleaving is enabled.
	 */

	/*
	 * 0 = no
	 * 0x40 = CS0,CS1
	 * 0x20 = CS2,CS3
	 * 0x60 = CS0,CS1 + CS2,CS3
	 * 0x04 = CS0,CS1,CS2,CS3
	 */
	popts->ba_intlv_ctl = 0;

	/* Memory Organization Parameters */
	popts->registered_dimm_en = all_DIMMs_registered;

	/* Operational Mode Paramters */

	/* Pick ECC modes */
#ifdef CONFIG_DDR_ECC
	popts->ECC_mode = 1;		  /* 0 = disabled, 1 = enabled */
#else
	popts->ECC_mode = 0;		  /* 0 = disabled, 1 = enabled */
#endif
	popts->ECC_init_using_memctl = 1; /* 0 = use DMA, 1 = use memctl */

	/*
	 * Choose DQS config
	 * 0 for DDR1
	 * 1 for DDR2
	 */
#if defined(CONFIG_FSL_DDR1)
	popts->DQS_config = 0;
#elif defined(CONFIG_FSL_DDR2) || defined(CONFIG_FSL_DDR3)
	popts->DQS_config = 1;
#endif

	/* Choose self-refresh during sleep. */
	popts->self_refresh_in_sleep = 1;

	/* Choose dynamic power management mode. */
	popts->dynamic_power = 0;

	/* 0 = 64-bit, 1 = 32-bit, 2 = 16-bit */
	popts->data_bus_width = 0;

	/* Choose burst length. */
#if defined(CONFIG_FSL_DDR3)
	popts->OTF_burst_chop_en = 1;	/* on-the-fly burst chop */
	popts->burst_length = DDR_OTF;	/* on-the-fly BC4 and BL8 */
#else
	popts->burst_length = DDR_BL4;	/* has to be 4 for DDR2 */
#endif

	/* Choose ddr controller address mirror mode */
#if defined(CONFIG_FSL_DDR3)
	popts->mirrored_dimm = pdimm[0].mirrored_dimm;
#endif

	/* Global Timing Parameters. */
	debug("mclk_ps = %u ps\n", get_memory_clk_period_ps());

	/* Pick a caslat override. */
	popts->cas_latency_override = 0;
	popts->cas_latency_override_value = 3;
	if (popts->cas_latency_override) {
		debug("using caslat override value = %u\n",
		       popts->cas_latency_override_value);
	}

	/* Decide whether to use the computed derated latency */
	popts->use_derated_caslat = 0;

	/* Choose an additive latency. */
	popts->additive_latency_override = 0;
	popts->additive_latency_override_value = 3;
	if (popts->additive_latency_override) {
		debug("using additive latency override value = %u\n",
		       popts->additive_latency_override_value);
	}

	/*
	 * 2T_EN setting
	 *
	 * Factors to consider for 2T_EN:
	 *	- number of DIMMs installed
	 *	- number of components, number of active ranks
	 *	- how much time you want to spend playing around
	 */
	popts->twoT_en = 0;
	popts->threeT_en = 0;

	/*
	 * BSTTOPRE precharge interval
	 *
	 * Set this to 0 for global auto precharge
	 *
	 * FIXME: Should this be configured in picoseconds?
	 * Why it should be in ps:  better understanding of this
	 * relative to actual DRAM timing parameters such as tRAS.
	 * e.g. tRAS(min) = 40 ns
	 */
	popts->bstopre = 0x100;

	/* Minimum CKE pulse width -- tCKE(MIN) */
	popts->tCKE_clock_pulse_width_ps
		= mclk_to_picos(FSL_DDR_MIN_TCKE_PULSE_WIDTH_DDR);

	/*
	 * Window for four activates -- tFAW
	 *
	 * FIXME: UM: applies only to DDR2/DDR3 with eight logical banks only
	 * FIXME: varies depending upon number of column addresses or data
	 * FIXME: width, was considering looking at pdimm->primary_sdram_width
	 */
#if defined(CONFIG_FSL_DDR1)
	popts->tFAW_window_four_activates_ps = mclk_to_picos(1);

#elif defined(CONFIG_FSL_DDR2)
	/*
	 * x4/x8;  some datasheets have 35000
	 * x16 wide columns only?  Use 50000?
	 */
	popts->tFAW_window_four_activates_ps = 37500;

#elif defined(CONFIG_FSL_DDR3)
	popts->tFAW_window_four_activates_ps = pdimm[0].tFAW_ps;
#endif
	popts->zq_en = 0;
	popts->wrlvl_en = 0;
#if defined(CONFIG_FSL_DDR3)
	/*
	 * due to ddr3 dimm is fly-by topology
	 * we suggest to enable write leveling to
	 * meet the tQDSS under different loading.
	 */
	popts->wrlvl_en = 1;
#endif

	/*
	 * Check interleaving configuration from environment.
	 * Please refer to doc/README.fsl-ddr for the detail.
	 *
	 * If memory controller interleaving is enabled, then the data
	 * bus widths must be programmed identically for the 2 memory
	 * controllers.
	 *
	 * XXX: Attempt to set both controllers to the same chip select
	 * interleaving mode. It will do a best effort to get the
	 * requested ranks interleaved together such that the result
	 * should be a subset of the requested configuration.
	 */
#if (CONFIG_NUM_DDR_CONTROLLERS > 1)
	if ((p = getenv("memctl_intlv_ctl")) != NULL) {
		if (pdimm[0].n_ranks == 0) {
			printf("There is no rank on CS0. Because only rank on "
				"CS0 and ranks chip-select interleaved with CS0"
				" are controller interleaved, force non memory "
				"controller interleaving\n");
			popts->memctl_interleaving = 0;
		} else {
			popts->memctl_interleaving = 1;
			if (strcmp(p, "cacheline") == 0)
				popts->memctl_interleaving_mode =
					FSL_DDR_CACHE_LINE_INTERLEAVING;
			else if (strcmp(p, "page") == 0)
				popts->memctl_interleaving_mode =
					FSL_DDR_PAGE_INTERLEAVING;
			else if (strcmp(p, "bank") == 0)
				popts->memctl_interleaving_mode =
					FSL_DDR_BANK_INTERLEAVING;
			else if (strcmp(p, "superbank") == 0)
				popts->memctl_interleaving_mode =
					FSL_DDR_SUPERBANK_INTERLEAVING;
			else
				popts->memctl_interleaving_mode =
						simple_strtoul(p, NULL, 0);
		}
	}

	if( (p = getenv("ba_intlv_ctl")) != NULL) {
		if (strcmp(p, "cs0_cs1") == 0)
			popts->ba_intlv_ctl = FSL_DDR_CS0_CS1;
		else if (strcmp(p, "cs2_cs3") == 0)
			popts->ba_intlv_ctl = FSL_DDR_CS2_CS3;
		else if (strcmp(p, "cs0_cs1_and_cs2_cs3") == 0)
			popts->ba_intlv_ctl = FSL_DDR_CS0_CS1_AND_CS2_CS3;
		else if (strcmp(p, "cs0_cs1_cs2_cs3") == 0)
			popts->ba_intlv_ctl = FSL_DDR_CS0_CS1_CS2_CS3;
		else
			popts->ba_intlv_ctl = simple_strtoul(p, NULL, 0);

		switch (popts->ba_intlv_ctl & FSL_DDR_CS0_CS1_CS2_CS3) {
		case FSL_DDR_CS0_CS1_CS2_CS3:
		case FSL_DDR_CS0_CS1:
			if (pdimm[0].n_ranks != 2) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for "
					"CS0+CS1, force non-interleaving!\n");
			}
			break;
		case FSL_DDR_CS2_CS3:
			if (pdimm[1].n_ranks !=2){
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(CS) for CS2+CS3, "
					"force non-interleaving!\n");
			}
			break;
		case FSL_DDR_CS0_CS1_AND_CS2_CS3:
			if ((pdimm[0].n_ranks != 2)||(pdimm[1].n_ranks != 2)) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(CS) for CS0+CS1 or "
					"CS2+CS3, force non-interleaving!\n");
			}
			break;
		default:
			popts->ba_intlv_ctl = 0;
			break;
		}
	}
#endif

	fsl_ddr_board_options(popts, pdimm, ctrl_num);

	return 0;
}

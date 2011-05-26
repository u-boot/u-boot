/*
 * Copyright 2008, 2010-2011 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <common.h>
#include <hwconfig.h>
#include <asm/fsl_ddr_sdram.h>

#include "ddr.h"

/*
 * Use our own stack based buffer before relocation to allow accessing longer
 * hwconfig strings that might be in the environment before we've relocated.
 * This is pretty fragile on both the use of stack and if the buffer is big
 * enough. However we will get a warning from getenv_f for the later.
 */
#define HWCONFIG_BUFFER_SIZE	128

/* Board-specific functions defined in each board's ddr.c */
extern void fsl_ddr_board_options(memctl_options_t *popts,
		dimm_params_t *pdimm,
		unsigned int ctrl_num);

typedef struct {
	unsigned int odt_rd_cfg;
	unsigned int odt_wr_cfg;
	unsigned int odt_rtt_norm;
	unsigned int odt_rtt_wr;
} dynamic_odt_t;

static const dynamic_odt_t single_Q[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS_AND_OTHER_DIMM,
		DDR3_RTT_20_OHM,
		DDR3_RTT_120_OHM
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,	/* tied high */
		DDR3_RTT_OFF,
		DDR3_RTT_120_OHM
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS_AND_OTHER_DIMM,
		DDR3_RTT_20_OHM,
		DDR3_RTT_120_OHM
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,	/* tied high */
		DDR3_RTT_OFF,
		DDR3_RTT_120_OHM
	}
};

static const dynamic_odt_t single_D[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR3_RTT_OFF,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static const dynamic_odt_t single_S[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
};

static const dynamic_odt_t dual_DD[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR3_RTT_30_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR3_RTT_30_OHM,
		DDR3_RTT_OFF
	}
};

static const dynamic_odt_t dual_DS[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR3_RTT_30_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_20_OHM,
		DDR3_RTT_120_OHM
	},
	{0, 0, 0, 0}
};
static const dynamic_odt_t dual_SD[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_20_OHM,
		DDR3_RTT_120_OHM
	},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_OTHER_DIMM,
		DDR3_RTT_20_OHM,
		DDR3_RTT_OFF
	}
};

static const dynamic_odt_t dual_SS[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_30_OHM,
		DDR3_RTT_120_OHM
	},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_OTHER_DIMM,
		FSL_DDR_ODT_ALL,
		DDR3_RTT_30_OHM,
		DDR3_RTT_120_OHM
	},
	{0, 0, 0, 0}
};

static const dynamic_odt_t dual_D0[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR3_RTT_OFF,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0}
};

static const dynamic_odt_t dual_0D[4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_SAME_DIMM,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_NEVER,
		DDR3_RTT_OFF,
		DDR3_RTT_OFF
	}
};

static const dynamic_odt_t dual_S0[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0}

};

static const dynamic_odt_t dual_0S[4] = {
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_40_OHM,
		DDR3_RTT_OFF
	},
	{0, 0, 0, 0}

};

static const dynamic_odt_t odt_unknown[4] = {
	{	/* cs0 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs1 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs2 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	},
	{	/* cs3 */
		FSL_DDR_ODT_NEVER,
		FSL_DDR_ODT_CS,
		DDR3_RTT_120_OHM,
		DDR3_RTT_OFF
	}
};

unsigned int populate_memctl_options(int all_DIMMs_registered,
			memctl_options_t *popts,
			dimm_params_t *pdimm,
			unsigned int ctrl_num)
{
	unsigned int i;
	char buffer[HWCONFIG_BUFFER_SIZE];
	char *buf = NULL;
	const dynamic_odt_t *pdodt = odt_unknown;

	/*
	 * Extract hwconfig from environment since we have not properly setup
	 * the environment but need it for ddr config params
	 */
	if (getenv_f("hwconfig", buffer, sizeof(buffer)) > 0)
		buf = buffer;

	/* Chip select options. */
	if (CONFIG_DIMM_SLOTS_PER_CTLR == 1) {
		switch (pdimm[0].n_ranks) {
		case 1:
			pdodt = single_S;
			break;
		case 2:
			pdodt = single_D;
			break;
		case 4:
			pdodt = single_Q;
			break;
		}
	} else if (CONFIG_DIMM_SLOTS_PER_CTLR == 2) {
		switch (pdimm[0].n_ranks) {
		case 2:
			switch (pdimm[1].n_ranks) {
			case 2:
				pdodt = dual_DD;
				break;
			case 1:
				pdodt = dual_DS;
				break;
			case 0:
				pdodt = dual_D0;
				break;
			}
			break;
		case 1:
			switch (pdimm[1].n_ranks) {
			case 2:
				pdodt = dual_SD;
				break;
			case 1:
				pdodt = dual_SS;
				break;
			case 0:
				pdodt = dual_S0;
				break;
			}
			break;
		case 0:
			switch (pdimm[1].n_ranks) {
			case 2:
				pdodt = dual_0D;
				break;
			case 1:
				pdodt = dual_0S;
				break;
			}
			break;
		}
	}

	/* Pick chip-select local options. */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
#if defined(CONFIG_FSL_DDR3)
		popts->cs_local_opts[i].odt_rd_cfg = pdodt[i].odt_rd_cfg;
		popts->cs_local_opts[i].odt_wr_cfg = pdodt[i].odt_wr_cfg;
		popts->cs_local_opts[i].odt_rtt_norm = pdodt[i].odt_rtt_norm;
		popts->cs_local_opts[i].odt_rtt_wr = pdodt[i].odt_rtt_wr;
#else
		popts->cs_local_opts[i].odt_rd_cfg = FSL_DDR_ODT_NEVER;
		popts->cs_local_opts[i].odt_wr_cfg = FSL_DDR_ODT_CS;
#endif
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
	popts->ECC_mode = 0;		  /* 0 = disabled, 1 = enabled */
#ifdef CONFIG_DDR_ECC
	if (hwconfig_sub_f("fsl_ddr", "ecc", buf)) {
		if (hwconfig_subarg_cmp_f("fsl_ddr", "ecc", "on", buf))
			popts->ECC_mode = 1;
	} else
		popts->ECC_mode = 1;
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

	/*
	 * check first dimm for primary sdram width
	 * presuming all dimms are similar
	 * 0 = 64-bit, 1 = 32-bit, 2 = 16-bit
	 */
	if (pdimm[0].primary_sdram_width == 64)
		popts->data_bus_width = 0;
	else if (pdimm[0].primary_sdram_width == 32)
		popts->data_bus_width = 1;
	else if (pdimm[0].primary_sdram_width == 16)
		popts->data_bus_width = 2;
	else
		panic("Error: invalid primary sdram width!\n");

	/* Choose burst length. */
#if defined(CONFIG_FSL_DDR3)
#if defined(CONFIG_E500MC)
	popts->OTF_burst_chop_en = 0;	/* on-the-fly burst chop disable */
	popts->burst_length = DDR_BL8;	/* Fixed 8-beat burst len */
#else
	if (popts->data_bus_width == 1) {	/* 32-bit bus */
		popts->OTF_burst_chop_en = 0;
		popts->burst_length = DDR_BL8;
	} else {
		popts->OTF_burst_chop_en = 1;	/* on-the-fly burst chop */
		popts->burst_length = DDR_OTF;	/* on-the-fly BC4 and BL8 */
	}
#endif
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

	/* for RDIMM, address parity enable */
	popts->ap_en = 1;

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
	popts->zq_en = 1;
	popts->wrlvl_override = 0;
#endif

	/*
	 * Check interleaving configuration from environment.
	 * Please refer to doc/README.fsl-ddr for the detail.
	 *
	 * If memory controller interleaving is enabled, then the data
	 * bus widths must be programmed identically for all memory controllers.
	 *
	 * XXX: Attempt to set all controllers to the same chip select
	 * interleaving mode. It will do a best effort to get the
	 * requested ranks interleaved together such that the result
	 * should be a subset of the requested configuration.
	 */
#if (CONFIG_NUM_DDR_CONTROLLERS > 1)
	if (hwconfig_sub_f("fsl_ddr", "ctlr_intlv", buf)) {
		if (pdimm[0].n_ranks == 0) {
			printf("There is no rank on CS0 for controller %d. Because only"
				" rank on CS0 and ranks chip-select interleaved with CS0"
				" are controller interleaved, force non memory "
				"controller interleaving\n", ctrl_num);
			popts->memctl_interleaving = 0;
		} else {
			popts->memctl_interleaving = 1;
			/*
			 * test null first. if CONFIG_HWCONFIG is not defined
			 * hwconfig_arg_cmp returns non-zero
			 */
			if (hwconfig_subarg_cmp_f("fsl_ddr", "ctlr_intlv",
						    "null", buf)) {
				popts->memctl_interleaving = 0;
				debug("memory controller interleaving disabled.\n");
			} else if (hwconfig_subarg_cmp_f("fsl_ddr",
							 "ctlr_intlv",
							 "cacheline", buf))
				popts->memctl_interleaving_mode =
					FSL_DDR_CACHE_LINE_INTERLEAVING;
			else if (hwconfig_subarg_cmp_f("fsl_ddr", "ctlr_intlv",
						       "page", buf))
				popts->memctl_interleaving_mode =
					FSL_DDR_PAGE_INTERLEAVING;
			else if (hwconfig_subarg_cmp_f("fsl_ddr", "ctlr_intlv",
						       "bank", buf))
				popts->memctl_interleaving_mode =
					FSL_DDR_BANK_INTERLEAVING;
			else if (hwconfig_subarg_cmp_f("fsl_ddr", "ctlr_intlv",
						       "superbank", buf))
				popts->memctl_interleaving_mode =
					FSL_DDR_SUPERBANK_INTERLEAVING;
			else {
				popts->memctl_interleaving = 0;
				printf("hwconfig has unrecognized parameter for ctlr_intlv.\n");
			}
		}
	}
#endif
	if ((hwconfig_sub_f("fsl_ddr", "bank_intlv", buf)) &&
		(CONFIG_CHIP_SELECTS_PER_CTRL > 1)) {
		/* test null first. if CONFIG_HWCONFIG is not defined,
		 * hwconfig_subarg_cmp_f returns non-zero */
		if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
					    "null", buf))
			debug("bank interleaving disabled.\n");
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
						 "cs0_cs1", buf))
			popts->ba_intlv_ctl = FSL_DDR_CS0_CS1;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
						 "cs2_cs3", buf))
			popts->ba_intlv_ctl = FSL_DDR_CS2_CS3;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
						 "cs0_cs1_and_cs2_cs3", buf))
			popts->ba_intlv_ctl = FSL_DDR_CS0_CS1_AND_CS2_CS3;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "bank_intlv",
						 "cs0_cs1_cs2_cs3", buf))
			popts->ba_intlv_ctl = FSL_DDR_CS0_CS1_CS2_CS3;
		else
			printf("hwconfig has unrecognized parameter for bank_intlv.\n");
		switch (popts->ba_intlv_ctl & FSL_DDR_CS0_CS1_CS2_CS3) {
		case FSL_DDR_CS0_CS1_CS2_CS3:
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
			if (pdimm[0].n_ranks < 4) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for "
					"CS0+CS1+CS2+CS3 on controller %d, "
					"force non-interleaving!\n", ctrl_num);
			}
#elif (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if ((pdimm[0].n_ranks < 2) && (pdimm[1].n_ranks < 2)) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for "
					"CS0+CS1+CS2+CS3 on controller %d, "
					"force non-interleaving!\n", ctrl_num);
			}
			if (pdimm[0].capacity != pdimm[1].capacity) {
				popts->ba_intlv_ctl = 0;
				printf("Not identical DIMM size for "
					"CS0+CS1+CS2+CS3 on controller %d, "
					"force non-interleaving!\n", ctrl_num);
			}
#endif
			break;
		case FSL_DDR_CS0_CS1:
			if (pdimm[0].n_ranks < 2) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for "
					"CS0+CS1 on controller %d, "
					"force non-interleaving!\n", ctrl_num);
			}
			break;
		case FSL_DDR_CS2_CS3:
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
			if (pdimm[0].n_ranks < 4) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for CS2+CS3 "
					"on controller %d, force non-interleaving!\n", ctrl_num);
			}
#elif (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if (pdimm[1].n_ranks < 2) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(chip-select) for CS2+CS3 "
					"on controller %d, force non-interleaving!\n", ctrl_num);
			}
#endif
			break;
		case FSL_DDR_CS0_CS1_AND_CS2_CS3:
#if (CONFIG_DIMM_SLOTS_PER_CTLR == 1)
			if (pdimm[0].n_ranks < 4) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(CS) for CS0+CS1 and "
					"CS2+CS3 on controller %d, "
					"force non-interleaving!\n", ctrl_num);
			}
#elif (CONFIG_DIMM_SLOTS_PER_CTLR == 2)
			if ((pdimm[0].n_ranks < 2) || (pdimm[1].n_ranks < 2)) {
				popts->ba_intlv_ctl = 0;
				printf("Not enough bank(CS) for CS0+CS1 and "
					"CS2+CS3 on controller %d, "
					"force non-interleaving!\n", ctrl_num);
			}
#endif
			break;
		default:
			popts->ba_intlv_ctl = 0;
			break;
		}
	}

	if (hwconfig_sub_f("fsl_ddr", "addr_hash", buf)) {
		if (hwconfig_subarg_cmp_f("fsl_ddr", "addr_hash", "null", buf))
			popts->addr_hash = 0;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "addr_hash",
					       "true", buf))
			popts->addr_hash = 1;
	}

	if (pdimm[0].n_ranks == 4)
		popts->quad_rank_present = 1;

	fsl_ddr_board_options(popts, pdimm, ctrl_num);

	return 0;
}

void check_interleaving_options(fsl_ddr_info_t *pinfo)
{
	int i, j, check_n_ranks, intlv_fixed = 0;
	unsigned long long check_rank_density;
	/*
	 * Check if all controllers are configured for memory
	 * controller interleaving. Identical dimms are recommended. At least
	 * the size should be checked.
	 */
	j = 0;
	check_n_ranks = pinfo->dimm_params[0][0].n_ranks;
	check_rank_density = pinfo->dimm_params[0][0].rank_density;
	for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++) {
		if ((pinfo->memctl_opts[i].memctl_interleaving) && \
		    (check_rank_density == pinfo->dimm_params[i][0].rank_density) && \
		    (check_n_ranks == pinfo->dimm_params[i][0].n_ranks)) {
			j++;
		}
	}
	if (j != CONFIG_NUM_DDR_CONTROLLERS) {
		for (i = 0; i < CONFIG_NUM_DDR_CONTROLLERS; i++)
			if (pinfo->memctl_opts[i].memctl_interleaving) {
				pinfo->memctl_opts[i].memctl_interleaving = 0;
				intlv_fixed = 1;
			}
		if (intlv_fixed)
			printf("Not all DIMMs are identical in size. "
				"Memory controller interleaving disabled.\n");
	}
}

int fsl_use_spd(void)
{
	int use_spd = 0;

#ifdef CONFIG_DDR_SPD
	char buffer[HWCONFIG_BUFFER_SIZE];
	char *buf = NULL;

	/*
	 * Extract hwconfig from environment since we have not properly setup
	 * the environment but need it for ddr config params
	 */
	if (getenv_f("hwconfig", buffer, sizeof(buffer)) > 0)
		buf = buffer;

	/* if hwconfig is not enabled, or "sdram" is not defined, use spd */
	if (hwconfig_sub_f("fsl_ddr", "sdram", buf)) {
		if (hwconfig_subarg_cmp_f("fsl_ddr", "sdram", "spd", buf))
			use_spd = 1;
		else if (hwconfig_subarg_cmp_f("fsl_ddr", "sdram",
					       "fixed", buf))
			use_spd = 0;
		else
			use_spd = 1;
	} else
		use_spd = 1;
#endif

	return use_spd;
}

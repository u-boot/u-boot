/*
 * Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

/*
 * Generic driver for Freescale DDR/DDR2/DDR3 memory controller.
 * Based on code from spd_sdram.c
 * Author: James Yang [at freescale.com]
 */

#include <common.h>
#include <asm/fsl_ddr_sdram.h>

#include "ddr.h"

extern unsigned int picos_to_mclk(unsigned int picos);
/*
 * Determine Rtt value.
 *
 * This should likely be either board or controller specific.
 *
 * Rtt(nominal) - DDR2:
 *	0 = Rtt disabled
 *	1 = 75 ohm
 *	2 = 150 ohm
 *	3 = 50 ohm
 * Rtt(nominal) - DDR3:
 *	0 = Rtt disabled
 *	1 = 60 ohm
 *	2 = 120 ohm
 *	3 = 40 ohm
 *	4 = 20 ohm
 *	5 = 30 ohm
 *
 * FIXME: Apparently 8641 needs a value of 2
 * FIXME: Old code seys if 667 MHz or higher, use 3 on 8572
 *
 * FIXME: There was some effort down this line earlier:
 *
 *	unsigned int i;
 *	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL/2; i++) {
 *		if (popts->dimmslot[i].num_valid_cs
 *		    && (popts->cs_local_opts[2*i].odt_rd_cfg
 *			|| popts->cs_local_opts[2*i].odt_wr_cfg)) {
 *			rtt = 2;
 *			break;
 *		}
 *	}
 */
static inline int fsl_ddr_get_rtt(void)
{
	int rtt;

#if defined(CONFIG_FSL_DDR1)
	rtt = 0;
#elif defined(CONFIG_FSL_DDR2)
	rtt = 3;
#else
	rtt = 0;
#endif

	return rtt;
}

/*
 * compute the CAS write latency according to DDR3 spec
 * CWL = 5 if tCK >= 2.5ns
 *       6 if 2.5ns > tCK >= 1.875ns
 *       7 if 1.875ns > tCK >= 1.5ns
 *       8 if 1.5ns > tCK >= 1.25ns
 */
static inline unsigned int compute_cas_write_latency(void)
{
	unsigned int cwl;
	const unsigned int mclk_ps = get_memory_clk_period_ps();

	if (mclk_ps >= 2500)
		cwl = 5;
	else if (mclk_ps >= 1875)
		cwl = 6;
	else if (mclk_ps >= 1500)
		cwl = 7;
	else if (mclk_ps >= 1250)
		cwl = 8;
	else
		cwl = 8;
	return cwl;
}

/* Chip Select Configuration (CSn_CONFIG) */
static void set_csn_config(int i, fsl_ddr_cfg_regs_t *ddr,
			       const memctl_options_t *popts,
			       const dimm_params_t *dimm_params)
{
	unsigned int cs_n_en = 0; /* Chip Select enable */
	unsigned int intlv_en = 0; /* Memory controller interleave enable */
	unsigned int intlv_ctl = 0; /* Interleaving control */
	unsigned int ap_n_en = 0; /* Chip select n auto-precharge enable */
	unsigned int odt_rd_cfg = 0; /* ODT for reads configuration */
	unsigned int odt_wr_cfg = 0; /* ODT for writes configuration */
	unsigned int ba_bits_cs_n = 0; /* Num of bank bits for SDRAM on CSn */
	unsigned int row_bits_cs_n = 0; /* Num of row bits for SDRAM on CSn */
	unsigned int col_bits_cs_n = 0; /* Num of ocl bits for SDRAM on CSn */

	/* Compute CS_CONFIG only for existing ranks of each DIMM.  */
	if ((((i&1) == 0)
	    && (dimm_params[i/2].n_ranks == 1))
	    || (dimm_params[i/2].n_ranks == 2)) {
		unsigned int n_banks_per_sdram_device;
		cs_n_en = 1;
		if (i == 0) {
			/* These fields only available in CS0_CONFIG */
			intlv_en = popts->memctl_interleaving;
			intlv_ctl = popts->memctl_interleaving_mode;
		}
		ap_n_en = popts->cs_local_opts[i].auto_precharge;
		odt_rd_cfg = popts->cs_local_opts[i].odt_rd_cfg;
		odt_wr_cfg = popts->cs_local_opts[i].odt_wr_cfg;
		n_banks_per_sdram_device
			= dimm_params[i/2].n_banks_per_sdram_device;
		ba_bits_cs_n = __ilog2(n_banks_per_sdram_device) - 2;
		row_bits_cs_n = dimm_params[i/2].n_row_addr - 12;
		col_bits_cs_n = dimm_params[i/2].n_col_addr - 8;
	}

	ddr->cs[i].config = (0
		| ((cs_n_en & 0x1) << 31)
		| ((intlv_en & 0x3) << 29)
		| ((intlv_ctl & 0xf) << 24)
		| ((ap_n_en & 0x1) << 23)

		/* XXX: some implementation only have 1 bit starting at left */
		| ((odt_rd_cfg & 0x7) << 20)

		/* XXX: Some implementation only have 1 bit starting at left */
		| ((odt_wr_cfg & 0x7) << 16)

		| ((ba_bits_cs_n & 0x3) << 14)
		| ((row_bits_cs_n & 0x7) << 8)
		| ((col_bits_cs_n & 0x7) << 0)
		);
	debug("FSLDDR: cs[%d]_config = 0x%08x\n", i,ddr->cs[i].config);
}

/* Chip Select Configuration 2 (CSn_CONFIG_2) */
/* FIXME: 8572 */
static void set_csn_config_2(int i, fsl_ddr_cfg_regs_t *ddr)
{
	unsigned int pasr_cfg = 0;	/* Partial array self refresh config */

	ddr->cs[i].config_2 = ((pasr_cfg & 7) << 24);
	debug("FSLDDR: cs[%d]_config_2 = 0x%08x\n", i, ddr->cs[i].config_2);
}

/* -3E = 667 CL5, -25 = CL6 800, -25E = CL5 800 */

#if !defined(CONFIG_FSL_DDR1)
/*
 * DDR SDRAM Timing Configuration 0 (TIMING_CFG_0)
 *
 * Avoid writing for DDR I.  The new PQ38 DDR controller
 * dreams up non-zero default values to be backwards compatible.
 */
static void set_timing_cfg_0(fsl_ddr_cfg_regs_t *ddr)
{
	unsigned char trwt_mclk = 0;   /* Read-to-write turnaround */
	unsigned char twrt_mclk = 0;   /* Write-to-read turnaround */
	/* 7.5 ns on -3E; 0 means WL - CL + BL/2 + 1 */
	unsigned char trrt_mclk = 0;   /* Read-to-read turnaround */
	unsigned char twwt_mclk = 0;   /* Write-to-write turnaround */

	/* Active powerdown exit timing (tXARD and tXARDS). */
	unsigned char act_pd_exit_mclk;
	/* Precharge powerdown exit timing (tXP). */
	unsigned char pre_pd_exit_mclk;
	/* Precharge powerdown exit timing (tAXPD). */
	unsigned char taxpd_mclk;
	/* Mode register set cycle time (tMRD). */
	unsigned char tmrd_mclk;

#if defined(CONFIG_FSL_DDR3)
	/*
	 * (tXARD and tXARDS). Empirical?
	 * The DDR3 spec has not tXARD,
	 * we use the tXP instead of it.
	 * tXP=max(3nCK, 7.5ns) for DDR3.
	 * spec has not the tAXPD, we use
	 * tAXPD=8, need design to confirm.
	 */
	int tXP = max((get_memory_clk_period_ps() * 3), 7500); /* unit=ps */
	act_pd_exit_mclk = picos_to_mclk(tXP);
	/* Mode register MR0[A12] is '1' - fast exit */
	pre_pd_exit_mclk = act_pd_exit_mclk;
	taxpd_mclk = 8;
	tmrd_mclk = 4;
	/* set the turnaround time */
	trwt_mclk = 1;
#else /* CONFIG_FSL_DDR2 */
	/*
	 * (tXARD and tXARDS). Empirical?
	 * tXARD = 2 for DDR2
	 * tXP=2
	 * tAXPD=8
	 */
	act_pd_exit_mclk = 2;
	pre_pd_exit_mclk = 2;
	taxpd_mclk = 8;
	tmrd_mclk = 2;
#endif

	ddr->timing_cfg_0 = (0
		| ((trwt_mclk & 0x3) << 30)	/* RWT */
		| ((twrt_mclk & 0x3) << 28)	/* WRT */
		| ((trrt_mclk & 0x3) << 26)	/* RRT */
		| ((twwt_mclk & 0x3) << 24)	/* WWT */
		| ((act_pd_exit_mclk & 0x7) << 20)  /* ACT_PD_EXIT */
		| ((pre_pd_exit_mclk & 0xF) << 16)  /* PRE_PD_EXIT */
		| ((taxpd_mclk & 0xf) << 8)	/* ODT_PD_EXIT */
		| ((tmrd_mclk & 0xf) << 0)	/* MRS_CYC */
		);
	debug("FSLDDR: timing_cfg_0 = 0x%08x\n", ddr->timing_cfg_0);
}
#endif	/* defined(CONFIG_FSL_DDR2) */

/* DDR SDRAM Timing Configuration 3 (TIMING_CFG_3) */
static void set_timing_cfg_3(fsl_ddr_cfg_regs_t *ddr,
			       const common_timing_params_t *common_dimm,
			       unsigned int cas_latency)
{
	/* Extended Activate to precharge interval (tRAS) */
	unsigned int ext_acttopre = 0;
	unsigned int ext_refrec; /* Extended refresh recovery time (tRFC) */
	unsigned int ext_caslat = 0; /* Extended MCAS latency from READ cmd */
	unsigned int cntl_adj = 0; /* Control Adjust */

	/* If the tRAS > 19 MCLK, we use the ext mode */
	if (picos_to_mclk(common_dimm->tRAS_ps) > 0x13)
		ext_acttopre = 1;

	ext_refrec = (picos_to_mclk(common_dimm->tRFC_ps) - 8) >> 4;

	/* If the CAS latency more than 8, use the ext mode */
	if (cas_latency > 8)
		ext_caslat = 1;

	ddr->timing_cfg_3 = (0
		| ((ext_acttopre & 0x1) << 24)
		| ((ext_refrec & 0xF) << 16)
		| ((ext_caslat & 0x1) << 12)
		| ((cntl_adj & 0x7) << 0)
		);
	debug("FSLDDR: timing_cfg_3 = 0x%08x\n", ddr->timing_cfg_3);
}

/* DDR SDRAM Timing Configuration 1 (TIMING_CFG_1) */
static void set_timing_cfg_1(fsl_ddr_cfg_regs_t *ddr,
			       const memctl_options_t *popts,
			       const common_timing_params_t *common_dimm,
			       unsigned int cas_latency)
{
	/* Precharge-to-activate interval (tRP) */
	unsigned char pretoact_mclk;
	/* Activate to precharge interval (tRAS) */
	unsigned char acttopre_mclk;
	/*  Activate to read/write interval (tRCD) */
	unsigned char acttorw_mclk;
	/* CASLAT */
	unsigned char caslat_ctrl;
	/*  Refresh recovery time (tRFC) ; trfc_low */
	unsigned char refrec_ctrl;
	/* Last data to precharge minimum interval (tWR) */
	unsigned char wrrec_mclk;
	/* Activate-to-activate interval (tRRD) */
	unsigned char acttoact_mclk;
	/* Last write data pair to read command issue interval (tWTR) */
	unsigned char wrtord_mclk;

	pretoact_mclk = picos_to_mclk(common_dimm->tRP_ps);
	acttopre_mclk = picos_to_mclk(common_dimm->tRAS_ps);
	acttorw_mclk = picos_to_mclk(common_dimm->tRCD_ps);

	/*
	 * Translate CAS Latency to a DDR controller field value:
	 *
	 *      CAS Lat DDR I   DDR II  Ctrl
	 *      Clocks  SPD Bit SPD Bit Value
	 *      ------- ------- ------- -----
	 *      1.0     0               0001
	 *      1.5     1               0010
	 *      2.0     2       2       0011
	 *      2.5     3               0100
	 *      3.0     4       3       0101
	 *      3.5     5               0110
	 *      4.0             4       0111
	 *      4.5                     1000
	 *      5.0             5       1001
	 */
#if defined(CONFIG_FSL_DDR1)
	caslat_ctrl = (cas_latency + 1) & 0x07;
#elif defined(CONFIG_FSL_DDR2)
	caslat_ctrl = 2 * cas_latency - 1;
#else
	/*
	 * if the CAS latency more than 8 cycle,
	 * we need set extend bit for it at
	 * TIMING_CFG_3[EXT_CASLAT]
	 */
	if (cas_latency > 8)
		cas_latency -= 8;
	caslat_ctrl = 2 * cas_latency - 1;
#endif

	refrec_ctrl = picos_to_mclk(common_dimm->tRFC_ps) - 8;
	wrrec_mclk = picos_to_mclk(common_dimm->tWR_ps);
	if (popts->OTF_burst_chop_en)
		wrrec_mclk += 2;

	acttoact_mclk = picos_to_mclk(common_dimm->tRRD_ps);
	/*
	 * JEDEC has min requirement for tRRD
	 */
#if defined(CONFIG_FSL_DDR3)
	if (acttoact_mclk < 4)
		acttoact_mclk = 4;
#endif
	wrtord_mclk = picos_to_mclk(common_dimm->tWTR_ps);
	/*
	 * JEDEC has some min requirements for tWTR
	 */
#if defined(CONFIG_FSL_DDR2)
	if (wrtord_mclk < 2)
		wrtord_mclk = 2;
#elif defined(CONFIG_FSL_DDR3)
	if (wrtord_mclk < 4)
		wrtord_mclk = 4;
#endif
	if (popts->OTF_burst_chop_en)
		wrtord_mclk += 2;

	ddr->timing_cfg_1 = (0
		| ((pretoact_mclk & 0x0F) << 28)
		| ((acttopre_mclk & 0x0F) << 24)
		| ((acttorw_mclk & 0xF) << 20)
		| ((caslat_ctrl & 0xF) << 16)
		| ((refrec_ctrl & 0xF) << 12)
		| ((wrrec_mclk & 0x0F) << 8)
		| ((acttoact_mclk & 0x07) << 4)
		| ((wrtord_mclk & 0x07) << 0)
		);
	debug("FSLDDR: timing_cfg_1 = 0x%08x\n", ddr->timing_cfg_1);
}

/* DDR SDRAM Timing Configuration 2 (TIMING_CFG_2) */
static void set_timing_cfg_2(fsl_ddr_cfg_regs_t *ddr,
			       const memctl_options_t *popts,
			       const common_timing_params_t *common_dimm,
			       unsigned int cas_latency,
			       unsigned int additive_latency)
{
	/* Additive latency */
	unsigned char add_lat_mclk;
	/* CAS-to-preamble override */
	unsigned short cpo;
	/* Write latency */
	unsigned char wr_lat;
	/*  Read to precharge (tRTP) */
	unsigned char rd_to_pre;
	/* Write command to write data strobe timing adjustment */
	unsigned char wr_data_delay;
	/* Minimum CKE pulse width (tCKE) */
	unsigned char cke_pls;
	/* Window for four activates (tFAW) */
	unsigned short four_act;

	/* FIXME add check that this must be less than acttorw_mclk */
	add_lat_mclk = additive_latency;
	cpo = popts->cpo_override;

#if defined(CONFIG_FSL_DDR1)
	/*
	 * This is a lie.  It should really be 1, but if it is
	 * set to 1, bits overlap into the old controller's
	 * otherwise unused ACSM field.  If we leave it 0, then
	 * the HW will magically treat it as 1 for DDR 1.  Oh Yea.
	 */
	wr_lat = 0;
#elif defined(CONFIG_FSL_DDR2)
	wr_lat = cas_latency - 1;
#else
	wr_lat = compute_cas_write_latency();
#endif

	rd_to_pre = picos_to_mclk(common_dimm->tRTP_ps);
	/*
	 * JEDEC has some min requirements for tRTP
	 */
#if defined(CONFIG_FSL_DDR2)
	if (rd_to_pre  < 2)
		rd_to_pre  = 2;
#elif defined(CONFIG_FSL_DDR3)
	if (rd_to_pre < 4)
		rd_to_pre = 4;
#endif
	if (additive_latency)
		rd_to_pre += additive_latency;
	if (popts->OTF_burst_chop_en)
		rd_to_pre += 2; /* according to UM */

	wr_data_delay = popts->write_data_delay;
	cke_pls = picos_to_mclk(popts->tCKE_clock_pulse_width_ps);
	four_act = picos_to_mclk(popts->tFAW_window_four_activates_ps);

	ddr->timing_cfg_2 = (0
		| ((add_lat_mclk & 0xf) << 28)
		| ((cpo & 0x1f) << 23)
		| ((wr_lat & 0xf) << 19)
		| ((rd_to_pre & RD_TO_PRE_MASK) << RD_TO_PRE_SHIFT)
		| ((wr_data_delay & WR_DATA_DELAY_MASK) << WR_DATA_DELAY_SHIFT)
		| ((cke_pls & 0x7) << 6)
		| ((four_act & 0x3f) << 0)
		);
	debug("FSLDDR: timing_cfg_2 = 0x%08x\n", ddr->timing_cfg_2);
}

/* DDR SDRAM control configuration (DDR_SDRAM_CFG) */
static void set_ddr_sdram_cfg(fsl_ddr_cfg_regs_t *ddr,
			       const memctl_options_t *popts,
			       const common_timing_params_t *common_dimm)
{
	unsigned int mem_en;		/* DDR SDRAM interface logic enable */
	unsigned int sren;		/* Self refresh enable (during sleep) */
	unsigned int ecc_en;		/* ECC enable. */
	unsigned int rd_en;		/* Registered DIMM enable */
	unsigned int sdram_type;	/* Type of SDRAM */
	unsigned int dyn_pwr;		/* Dynamic power management mode */
	unsigned int dbw;		/* DRAM dta bus width */
	unsigned int eight_be = 0;	/* 8-beat burst enable, DDR2 is zero */
	unsigned int ncap = 0;		/* Non-concurrent auto-precharge */
	unsigned int threeT_en;		/* Enable 3T timing */
	unsigned int twoT_en;		/* Enable 2T timing */
	unsigned int ba_intlv_ctl;	/* Bank (CS) interleaving control */
	unsigned int x32_en = 0;	/* x32 enable */
	unsigned int pchb8 = 0;		/* precharge bit 8 enable */
	unsigned int hse;		/* Global half strength override */
	unsigned int mem_halt = 0;	/* memory controller halt */
	unsigned int bi = 0;		/* Bypass initialization */

	mem_en = 1;
	sren = popts->self_refresh_in_sleep;
	if (common_dimm->all_DIMMs_ECC_capable) {
		/* Allow setting of ECC only if all DIMMs are ECC. */
		ecc_en = popts->ECC_mode;
	} else {
		ecc_en = 0;
	}

	rd_en = (common_dimm->all_DIMMs_registered
		 && !common_dimm->all_DIMMs_unbuffered);

	sdram_type = CONFIG_FSL_SDRAM_TYPE;

	dyn_pwr = popts->dynamic_power;
	dbw = popts->data_bus_width;
	/* 8-beat burst enable DDR-III case
	 * we must clear it when use the on-the-fly mode,
	 * must set it when use the 32-bits bus mode.
	 */
	if (sdram_type == SDRAM_TYPE_DDR3) {
		if (popts->burst_length == DDR_BL8)
			eight_be = 1;
		if (popts->burst_length == DDR_OTF)
			eight_be = 0;
		if (dbw == 0x1)
			eight_be = 1;
	}

	threeT_en = popts->threeT_en;
	twoT_en = popts->twoT_en;
	ba_intlv_ctl = popts->ba_intlv_ctl;
	hse = popts->half_strength_driver_enable;

	ddr->ddr_sdram_cfg = (0
			| ((mem_en & 0x1) << 31)
			| ((sren & 0x1) << 30)
			| ((ecc_en & 0x1) << 29)
			| ((rd_en & 0x1) << 28)
			| ((sdram_type & 0x7) << 24)
			| ((dyn_pwr & 0x1) << 21)
			| ((dbw & 0x3) << 19)
			| ((eight_be & 0x1) << 18)
			| ((ncap & 0x1) << 17)
			| ((threeT_en & 0x1) << 16)
			| ((twoT_en & 0x1) << 15)
			| ((ba_intlv_ctl & 0x7F) << 8)
			| ((x32_en & 0x1) << 5)
			| ((pchb8 & 0x1) << 4)
			| ((hse & 0x1) << 3)
			| ((mem_halt & 0x1) << 1)
			| ((bi & 0x1) << 0)
			);
	debug("FSLDDR: ddr_sdram_cfg = 0x%08x\n", ddr->ddr_sdram_cfg);
}

/* DDR SDRAM control configuration 2 (DDR_SDRAM_CFG_2) */
static void set_ddr_sdram_cfg_2(fsl_ddr_cfg_regs_t *ddr,
			       const memctl_options_t *popts)
{
	unsigned int frc_sr = 0;	/* Force self refresh */
	unsigned int sr_ie = 0;		/* Self-refresh interrupt enable */
	unsigned int dll_rst_dis;	/* DLL reset disable */
	unsigned int dqs_cfg;		/* DQS configuration */
	unsigned int odt_cfg;		/* ODT configuration */
	unsigned int num_pr;		/* Number of posted refreshes */
	unsigned int obc_cfg;		/* On-The-Fly Burst Chop Cfg */
	unsigned int ap_en;		/* Address Parity Enable */
	unsigned int d_init;		/* DRAM data initialization */
	unsigned int rcw_en = 0;	/* Register Control Word Enable */
	unsigned int md_en = 0;		/* Mirrored DIMM Enable */

	dll_rst_dis = 1;	/* Make this configurable */
	dqs_cfg = popts->DQS_config;
	if (popts->cs_local_opts[0].odt_rd_cfg
	    || popts->cs_local_opts[0].odt_wr_cfg) {
		/* FIXME */
		odt_cfg = 2;
	} else {
		odt_cfg = 0;
	}

	num_pr = 1;	/* Make this configurable */

	/*
	 * 8572 manual says
	 *     {TIMING_CFG_1[PRETOACT]
	 *      + [DDR_SDRAM_CFG_2[NUM_PR]
	 *        * ({EXT_REFREC || REFREC} + 8 + 2)]}
	 *      << DDR_SDRAM_INTERVAL[REFINT]
	 */
#if defined(CONFIG_FSL_DDR3)
	obc_cfg = popts->OTF_burst_chop_en;
#else
	obc_cfg = 0;
#endif

	ap_en = 0;	/* Make this configurable? */

#if defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/* Use the DDR controller to auto initialize memory. */
	d_init = 1;
	ddr->ddr_data_init = CONFIG_MEM_INIT_VALUE;
	debug("DDR: ddr_data_init = 0x%08x\n", ddr->ddr_data_init);
#else
	/* Memory will be initialized via DMA, or not at all. */
	d_init = 0;
#endif

#if defined(CONFIG_FSL_DDR3)
	md_en = popts->mirrored_dimm;
#endif
	ddr->ddr_sdram_cfg_2 = (0
		| ((frc_sr & 0x1) << 31)
		| ((sr_ie & 0x1) << 30)
		| ((dll_rst_dis & 0x1) << 29)
		| ((dqs_cfg & 0x3) << 26)
		| ((odt_cfg & 0x3) << 21)
		| ((num_pr & 0xf) << 12)
		| ((obc_cfg & 0x1) << 6)
		| ((ap_en & 0x1) << 5)
		| ((d_init & 0x1) << 4)
		| ((rcw_en & 0x1) << 2)
		| ((md_en & 0x1) << 0)
		);
	debug("FSLDDR: ddr_sdram_cfg_2 = 0x%08x\n", ddr->ddr_sdram_cfg_2);
}

/* DDR SDRAM Mode configuration 2 (DDR_SDRAM_MODE_2) */
static void set_ddr_sdram_mode_2(fsl_ddr_cfg_regs_t *ddr,
				const memctl_options_t *popts)
{
	unsigned short esdmode2 = 0;	/* Extended SDRAM mode 2 */
	unsigned short esdmode3 = 0;	/* Extended SDRAM mode 3 */

#if defined(CONFIG_FSL_DDR3)
	unsigned int rtt_wr = 0;	/* Rtt_WR - dynamic ODT off */
	unsigned int srt = 0;	/* self-refresh temerature, normal range */
	unsigned int asr = 0;	/* auto self-refresh disable */
	unsigned int cwl = compute_cas_write_latency() - 5;
	unsigned int pasr = 0;	/* partial array self refresh disable */

	if (popts->rtt_override)
		rtt_wr = popts->rtt_wr_override_value;

	esdmode2 = (0
		| ((rtt_wr & 0x3) << 9)
		| ((srt & 0x1) << 7)
		| ((asr & 0x1) << 6)
		| ((cwl & 0x7) << 3)
		| ((pasr & 0x7) << 0));
#endif
	ddr->ddr_sdram_mode_2 = (0
				 | ((esdmode2 & 0xFFFF) << 16)
				 | ((esdmode3 & 0xFFFF) << 0)
				 );
	debug("FSLDDR: ddr_sdram_mode_2 = 0x%08x\n", ddr->ddr_sdram_mode_2);
}

/* DDR SDRAM Interval Configuration (DDR_SDRAM_INTERVAL) */
static void set_ddr_sdram_interval(fsl_ddr_cfg_regs_t *ddr,
			       const memctl_options_t *popts,
			       const common_timing_params_t *common_dimm)
{
	unsigned int refint;	/* Refresh interval */
	unsigned int bstopre;	/* Precharge interval */

	refint = picos_to_mclk(common_dimm->refresh_rate_ps);

	bstopre = popts->bstopre;

	/* refint field used 0x3FFF in earlier controllers */
	ddr->ddr_sdram_interval = (0
				   | ((refint & 0xFFFF) << 16)
				   | ((bstopre & 0x3FFF) << 0)
				   );
	debug("FSLDDR: ddr_sdram_interval = 0x%08x\n", ddr->ddr_sdram_interval);
}

#if defined(CONFIG_FSL_DDR3)
/* DDR SDRAM Mode configuration set (DDR_SDRAM_MODE) */
static void set_ddr_sdram_mode(fsl_ddr_cfg_regs_t *ddr,
			       const memctl_options_t *popts,
			       const common_timing_params_t *common_dimm,
			       unsigned int cas_latency,
			       unsigned int additive_latency)
{
	unsigned short esdmode;		/* Extended SDRAM mode */
	unsigned short sdmode;		/* SDRAM mode */

	/* Mode Register - MR1 */
	unsigned int qoff = 0;		/* Output buffer enable 0=yes, 1=no */
	unsigned int tdqs_en = 0;	/* TDQS Enable: 0=no, 1=yes */
	unsigned int rtt;
	unsigned int wrlvl_en = 0;	/* Write level enable: 0=no, 1=yes */
	unsigned int al = 0;		/* Posted CAS# additive latency (AL) */
	unsigned int dic = 1;		/* Output driver impedance, 34ohm */
	unsigned int dll_en = 0;	/* DLL Enable  0=Enable (Normal),
						       1=Disable (Test/Debug) */

	/* Mode Register - MR0 */
	unsigned int dll_on;	/* DLL control for precharge PD, 0=off, 1=on */
	unsigned int wr;	/* Write Recovery */
	unsigned int dll_rst;	/* DLL Reset */
	unsigned int mode;	/* Normal=0 or Test=1 */
	unsigned int caslat = 4;/* CAS# latency, default set as 6 cycles */
	/* BT: Burst Type (0=Nibble Sequential, 1=Interleaved) */
	unsigned int bt;
	unsigned int bl;	/* BL: Burst Length */

	unsigned int wr_mclk;

	const unsigned int mclk_ps = get_memory_clk_period_ps();

	rtt = fsl_ddr_get_rtt();
	if (popts->rtt_override)
		rtt = popts->rtt_override_value;

	if (additive_latency == (cas_latency - 1))
		al = 1;
	if (additive_latency == (cas_latency - 2))
		al = 2;

	/*
	 * The esdmode value will also be used for writing
	 * MR1 during write leveling for DDR3, although the
	 * bits specifically related to the write leveling
	 * scheme will be handled automatically by the DDR
	 * controller. so we set the wrlvl_en = 0 here.
	 */
	esdmode = (0
		| ((qoff & 0x1) << 12)
		| ((tdqs_en & 0x1) << 11)
		| ((rtt & 0x4) << 7)   /* rtt field is split */
		| ((wrlvl_en & 0x1) << 7)
		| ((rtt & 0x2) << 5)   /* rtt field is split */
		| ((dic & 0x2) << 4)   /* DIC field is split */
		| ((al & 0x3) << 3)
		| ((rtt & 0x1) << 2)  /* rtt field is split */
		| ((dic & 0x1) << 1)   /* DIC field is split */
		| ((dll_en & 0x1) << 0)
		);

	/*
	 * DLL control for precharge PD
	 * 0=slow exit DLL off (tXPDLL)
	 * 1=fast exit DLL on (tXP)
	 */
	dll_on = 1;
	wr_mclk = (common_dimm->tWR_ps + mclk_ps - 1) / mclk_ps;
	if (wr_mclk >= 12)
		wr = 6;
	else if (wr_mclk >= 9)
		wr = 5;
	else
		wr = wr_mclk - 4;
	dll_rst = 0;	/* dll no reset */
	mode = 0;	/* normal mode */

	/* look up table to get the cas latency bits */
	if (cas_latency >= 5 && cas_latency <= 11) {
		unsigned char cas_latency_table[7] = {
			0x2,	/* 5 clocks */
			0x4,	/* 6 clocks */
			0x6,	/* 7 clocks */
			0x8,	/* 8 clocks */
			0xa,	/* 9 clocks */
			0xc,	/* 10 clocks */
			0xe	/* 11 clocks */
		};
		caslat = cas_latency_table[cas_latency - 5];
	}
	bt = 0;	/* Nibble sequential */

	switch (popts->burst_length) {
	case DDR_BL8:
		bl = 0;
		break;
	case DDR_OTF:
		bl = 1;
		break;
	case DDR_BC4:
		bl = 2;
		break;
	default:
		printf("Error: invalid burst length of %u specified. "
			" Defaulting to on-the-fly BC4 or BL8 beats.\n",
			popts->burst_length);
		bl = 1;
		break;
	}

	sdmode = (0
		  | ((dll_on & 0x1) << 12)
		  | ((wr & 0x7) << 9)
		  | ((dll_rst & 0x1) << 8)
		  | ((mode & 0x1) << 7)
		  | (((caslat >> 1) & 0x7) << 4)
		  | ((bt & 0x1) << 3)
		  | ((bl & 0x3) << 0)
		  );

	ddr->ddr_sdram_mode = (0
			       | ((esdmode & 0xFFFF) << 16)
			       | ((sdmode & 0xFFFF) << 0)
			       );

	debug("FSLDDR: ddr_sdram_mode = 0x%08x\n", ddr->ddr_sdram_mode);
}

#else /* !CONFIG_FSL_DDR3 */

/* DDR SDRAM Mode configuration set (DDR_SDRAM_MODE) */
static void set_ddr_sdram_mode(fsl_ddr_cfg_regs_t *ddr,
			       const memctl_options_t *popts,
			       const common_timing_params_t *common_dimm,
			       unsigned int cas_latency,
			       unsigned int additive_latency)
{
	unsigned short esdmode;		/* Extended SDRAM mode */
	unsigned short sdmode;		/* SDRAM mode */

	/*
	 * FIXME: This ought to be pre-calculated in a
	 * technology-specific routine,
	 * e.g. compute_DDR2_mode_register(), and then the
	 * sdmode and esdmode passed in as part of common_dimm.
	 */

	/* Extended Mode Register */
	unsigned int mrs = 0;		/* Mode Register Set */
	unsigned int outputs = 0;	/* 0=Enabled, 1=Disabled */
	unsigned int rdqs_en = 0;	/* RDQS Enable: 0=no, 1=yes */
	unsigned int dqs_en = 0;	/* DQS# Enable: 0=enable, 1=disable */
	unsigned int ocd = 0;		/* 0x0=OCD not supported,
					   0x7=OCD default state */
	unsigned int rtt;
	unsigned int al;		/* Posted CAS# additive latency (AL) */
	unsigned int ods = 0;		/* Output Drive Strength:
						0 = Full strength (18ohm)
						1 = Reduced strength (4ohm) */
	unsigned int dll_en = 0;	/* DLL Enable  0=Enable (Normal),
						       1=Disable (Test/Debug) */

	/* Mode Register (MR) */
	unsigned int mr;	/* Mode Register Definition */
	unsigned int pd;	/* Power-Down Mode */
	unsigned int wr;	/* Write Recovery */
	unsigned int dll_res;	/* DLL Reset */
	unsigned int mode;	/* Normal=0 or Test=1 */
	unsigned int caslat = 0;/* CAS# latency */
	/* BT: Burst Type (0=Sequential, 1=Interleaved) */
	unsigned int bt;
	unsigned int bl;	/* BL: Burst Length */

#if defined(CONFIG_FSL_DDR2)
	const unsigned int mclk_ps = get_memory_clk_period_ps();
#endif

	rtt = fsl_ddr_get_rtt();

	al = additive_latency;

	esdmode = (0
		| ((mrs & 0x3) << 14)
		| ((outputs & 0x1) << 12)
		| ((rdqs_en & 0x1) << 11)
		| ((dqs_en & 0x1) << 10)
		| ((ocd & 0x7) << 7)
		| ((rtt & 0x2) << 5)   /* rtt field is split */
		| ((al & 0x7) << 3)
		| ((rtt & 0x1) << 2)   /* rtt field is split */
		| ((ods & 0x1) << 1)
		| ((dll_en & 0x1) << 0)
		);

	mr = 0;		 /* FIXME: CHECKME */

	/*
	 * 0 = Fast Exit (Normal)
	 * 1 = Slow Exit (Low Power)
	 */
	pd = 0;

#if defined(CONFIG_FSL_DDR1)
	wr = 0;       /* Historical */
#elif defined(CONFIG_FSL_DDR2)
	wr = (common_dimm->tWR_ps + mclk_ps - 1) / mclk_ps - 1;
#endif
	dll_res = 0;
	mode = 0;

#if defined(CONFIG_FSL_DDR1)
	if (1 <= cas_latency && cas_latency <= 4) {
		unsigned char mode_caslat_table[4] = {
			0x5,	/* 1.5 clocks */
			0x2,	/* 2.0 clocks */
			0x6,	/* 2.5 clocks */
			0x3	/* 3.0 clocks */
		};
		caslat = mode_caslat_table[cas_latency - 1];
	} else {
		printf("Warning: unknown cas_latency %d\n", cas_latency);
	}
#elif defined(CONFIG_FSL_DDR2)
	caslat = cas_latency;
#endif
	bt = 0;

	switch (popts->burst_length) {
	case DDR_BL4:
		bl = 2;
		break;
	case DDR_BL8:
		bl = 3;
		break;
	default:
		printf("Error: invalid burst length of %u specified. "
			" Defaulting to 4 beats.\n",
			popts->burst_length);
		bl = 2;
		break;
	}

	sdmode = (0
		  | ((mr & 0x3) << 14)
		  | ((pd & 0x1) << 12)
		  | ((wr & 0x7) << 9)
		  | ((dll_res & 0x1) << 8)
		  | ((mode & 0x1) << 7)
		  | ((caslat & 0x7) << 4)
		  | ((bt & 0x1) << 3)
		  | ((bl & 0x7) << 0)
		  );

	ddr->ddr_sdram_mode = (0
			       | ((esdmode & 0xFFFF) << 16)
			       | ((sdmode & 0xFFFF) << 0)
			       );
	debug("FSLDDR: ddr_sdram_mode = 0x%08x\n", ddr->ddr_sdram_mode);
}
#endif

/* DDR SDRAM Data Initialization (DDR_DATA_INIT) */
static void set_ddr_data_init(fsl_ddr_cfg_regs_t *ddr)
{
	unsigned int init_value;	/* Initialization value */

	init_value = 0xDEADBEEF;
	ddr->ddr_data_init = init_value;
}

/*
 * DDR SDRAM Clock Control (DDR_SDRAM_CLK_CNTL)
 * The old controller on the 8540/60 doesn't have this register.
 * Hope it's OK to set it (to 0) anyway.
 */
static void set_ddr_sdram_clk_cntl(fsl_ddr_cfg_regs_t *ddr,
					 const memctl_options_t *popts)
{
	unsigned int clk_adjust;	/* Clock adjust */

	clk_adjust = popts->clk_adjust;
	ddr->ddr_sdram_clk_cntl = (clk_adjust & 0xF) << 23;
}

/* DDR Initialization Address (DDR_INIT_ADDR) */
static void set_ddr_init_addr(fsl_ddr_cfg_regs_t *ddr)
{
	unsigned int init_addr = 0;	/* Initialization address */

	ddr->ddr_init_addr = init_addr;
}

/* DDR Initialization Address (DDR_INIT_EXT_ADDR) */
static void set_ddr_init_ext_addr(fsl_ddr_cfg_regs_t *ddr)
{
	unsigned int uia = 0;	/* Use initialization address */
	unsigned int init_ext_addr = 0;	/* Initialization address */

	ddr->ddr_init_ext_addr = (0
				  | ((uia & 0x1) << 31)
				  | (init_ext_addr & 0xF)
				  );
}

/* DDR SDRAM Timing Configuration 4 (TIMING_CFG_4) */
static void set_timing_cfg_4(fsl_ddr_cfg_regs_t *ddr,
				const memctl_options_t *popts)
{
	unsigned int rwt = 0; /* Read-to-write turnaround for same CS */
	unsigned int wrt = 0; /* Write-to-read turnaround for same CS */
	unsigned int rrt = 0; /* Read-to-read turnaround for same CS */
	unsigned int wwt = 0; /* Write-to-write turnaround for same CS */
	unsigned int dll_lock = 0; /* DDR SDRAM DLL Lock Time */

#if defined(CONFIG_FSL_DDR3)
	if (popts->burst_length == DDR_BL8) {
		/* We set BL/2 for fixed BL8 */
		rrt = 0;	/* BL/2 clocks */
		wwt = 0;	/* BL/2 clocks */
	} else {
		/* We need to set BL/2 + 2 to BC4 and OTF */
		rrt = 2;	/* BL/2 + 2 clocks */
		wwt = 2;	/* BL/2 + 2 clocks */
	}
	dll_lock = 1;	/* tDLLK = 512 clocks from spec */
#endif
	ddr->timing_cfg_4 = (0
			     | ((rwt & 0xf) << 28)
			     | ((wrt & 0xf) << 24)
			     | ((rrt & 0xf) << 20)
			     | ((wwt & 0xf) << 16)
			     | (dll_lock & 0x3)
			     );
	debug("FSLDDR: timing_cfg_4 = 0x%08x\n", ddr->timing_cfg_4);
}

/* DDR SDRAM Timing Configuration 5 (TIMING_CFG_5) */
static void set_timing_cfg_5(fsl_ddr_cfg_regs_t *ddr)
{
	unsigned int rodt_on = 0;	/* Read to ODT on */
	unsigned int rodt_off = 0;	/* Read to ODT off */
	unsigned int wodt_on = 0;	/* Write to ODT on */
	unsigned int wodt_off = 0;	/* Write to ODT off */

#if defined(CONFIG_FSL_DDR3)
	rodt_on = 3;	/*  2 clocks */
	rodt_off = 4;	/*  4 clocks */
	wodt_on = 2;	/*  1 clocks */
	wodt_off = 4;	/*  4 clocks */
#endif

	ddr->timing_cfg_5 = (0
			     | ((rodt_on & 0x1f) << 24)
			     | ((rodt_off & 0x7) << 20)
			     | ((wodt_on & 0x1f) << 12)
			     | ((wodt_off & 0x7) << 8)
			     );
	debug("FSLDDR: timing_cfg_5 = 0x%08x\n", ddr->timing_cfg_5);
}

/* DDR ZQ Calibration Control (DDR_ZQ_CNTL) */
static void set_ddr_zq_cntl(fsl_ddr_cfg_regs_t *ddr, unsigned int zq_en)
{
	unsigned int zqinit = 0;/* POR ZQ Calibration Time (tZQinit) */
	/* Normal Operation Full Calibration Time (tZQoper) */
	unsigned int zqoper = 0;
	/* Normal Operation Short Calibration Time (tZQCS) */
	unsigned int zqcs = 0;

	if (zq_en) {
		zqinit = 9;	/* 512 clocks */
		zqoper = 8;	/* 256 clocks */
		zqcs = 6;	/* 64 clocks */
	}

	ddr->ddr_zq_cntl = (0
			    | ((zq_en & 0x1) << 31)
			    | ((zqinit & 0xF) << 24)
			    | ((zqoper & 0xF) << 16)
			    | ((zqcs & 0xF) << 8)
			    );
}

/* DDR Write Leveling Control (DDR_WRLVL_CNTL) */
static void set_ddr_wrlvl_cntl(fsl_ddr_cfg_regs_t *ddr, unsigned int wrlvl_en,
				const memctl_options_t *popts)
{
	/*
	 * First DQS pulse rising edge after margining mode
	 * is programmed (tWL_MRD)
	 */
	unsigned int wrlvl_mrd = 0;
	/* ODT delay after margining mode is programmed (tWL_ODTEN) */
	unsigned int wrlvl_odten = 0;
	/* DQS/DQS_ delay after margining mode is programmed (tWL_DQSEN) */
	unsigned int wrlvl_dqsen = 0;
	/* WRLVL_SMPL: Write leveling sample time */
	unsigned int wrlvl_smpl = 0;
	/* WRLVL_WLR: Write leveling repeition time */
	unsigned int wrlvl_wlr = 0;
	/* WRLVL_START: Write leveling start time */
	unsigned int wrlvl_start = 0;

	/* suggest enable write leveling for DDR3 due to fly-by topology */
	if (wrlvl_en) {
		/* tWL_MRD min = 40 nCK, we set it 64 */
		wrlvl_mrd = 0x6;
		/* tWL_ODTEN 128 */
		wrlvl_odten = 0x7;
		/* tWL_DQSEN min = 25 nCK, we set it 32 */
		wrlvl_dqsen = 0x5;
		/*
		 * Write leveling sample time at least need 6 clocks
		 * higher than tWLO to allow enough time for progagation
		 * delay and sampling the prime data bits.
		 */
		wrlvl_smpl = 0xf;
		/*
		 * Write leveling repetition time
		 * at least tWLO + 6 clocks clocks
		 * we set it 32
		 */
		wrlvl_wlr = 0x5;
		/*
		 * Write leveling start time
		 * The value use for the DQS_ADJUST for the first sample
		 * when write leveling is enabled.
		 */
		wrlvl_start = 0x8;
		/*
		 * Override the write leveling sample and start time
		 * according to specific board
		 */
		if (popts->wrlvl_override) {
			wrlvl_smpl = popts->wrlvl_sample;
			wrlvl_start = popts->wrlvl_start;
		}
	}

	ddr->ddr_wrlvl_cntl = (0
			       | ((wrlvl_en & 0x1) << 31)
			       | ((wrlvl_mrd & 0x7) << 24)
			       | ((wrlvl_odten & 0x7) << 20)
			       | ((wrlvl_dqsen & 0x7) << 16)
			       | ((wrlvl_smpl & 0xf) << 12)
			       | ((wrlvl_wlr & 0x7) << 8)
			       | ((wrlvl_start & 0x1F) << 0)
			       );
}

/* DDR Self Refresh Counter (DDR_SR_CNTR) */
static void set_ddr_sr_cntr(fsl_ddr_cfg_regs_t *ddr, unsigned int sr_it)
{
	/* Self Refresh Idle Threshold */
	ddr->ddr_sr_cntr = (sr_it & 0xF) << 16;
}

/* DDR SDRAM Register Control Word 1 (DDR_SDRAM_RCW_1) */
static void set_ddr_sdram_rcw_1(fsl_ddr_cfg_regs_t *ddr)
{
	unsigned int rcw0 = 0;	/* RCW0: Register Control Word 0 */
	unsigned int rcw1 = 0;	/* RCW1: Register Control Word 1 */
	unsigned int rcw2 = 0;	/* RCW2: Register Control Word 2 */
	unsigned int rcw3 = 0;	/* RCW3: Register Control Word 3 */
	unsigned int rcw4 = 0;	/* RCW4: Register Control Word 4 */
	unsigned int rcw5 = 0;	/* RCW5: Register Control Word 5 */
	unsigned int rcw6 = 0;	/* RCW6: Register Control Word 6 */
	unsigned int rcw7 = 0;	/* RCW7: Register Control Word 7 */

	ddr->ddr_sdram_rcw_1 = (0
				| ((rcw0 & 0xF) << 28)
				| ((rcw1 & 0xF) << 24)
				| ((rcw2 & 0xF) << 20)
				| ((rcw3 & 0xF) << 16)
				| ((rcw4 & 0xF) << 12)
				| ((rcw5 & 0xF) << 8)
				| ((rcw6 & 0xF) << 4)
				| ((rcw7 & 0xF) << 0)
				);
}

/* DDR SDRAM Register Control Word 2 (DDR_SDRAM_RCW_2) */
static void set_ddr_sdram_rcw_2(fsl_ddr_cfg_regs_t *ddr)
{
	unsigned int rcw8 = 0;	/* RCW0: Register Control Word 8 */
	unsigned int rcw9 = 0;	/* RCW1: Register Control Word 9 */
	unsigned int rcw10 = 0;	/* RCW2: Register Control Word 10 */
	unsigned int rcw11 = 0;	/* RCW3: Register Control Word 11 */
	unsigned int rcw12 = 0;	/* RCW4: Register Control Word 12 */
	unsigned int rcw13 = 0;	/* RCW5: Register Control Word 13 */
	unsigned int rcw14 = 0;	/* RCW6: Register Control Word 14 */
	unsigned int rcw15 = 0;	/* RCW7: Register Control Word 15 */

	ddr->ddr_sdram_rcw_2 = (0
				| ((rcw8 & 0xF) << 28)
				| ((rcw9 & 0xF) << 24)
				| ((rcw10 & 0xF) << 20)
				| ((rcw11 & 0xF) << 16)
				| ((rcw12 & 0xF) << 12)
				| ((rcw13 & 0xF) << 8)
				| ((rcw14 & 0xF) << 4)
				| ((rcw15 & 0xF) << 0)
				);
}

unsigned int
check_fsl_memctl_config_regs(const fsl_ddr_cfg_regs_t *ddr)
{
	unsigned int res = 0;

	/*
	 * Check that DDR_SDRAM_CFG[RD_EN] and DDR_SDRAM_CFG[2T_EN] are
	 * not set at the same time.
	 */
	if (ddr->ddr_sdram_cfg & 0x10000000
	    && ddr->ddr_sdram_cfg & 0x00008000) {
		printf("Error: DDR_SDRAM_CFG[RD_EN] and DDR_SDRAM_CFG[2T_EN] "
				" should not be set at the same time.\n");
		res++;
	}

	return res;
}

unsigned int
compute_fsl_memctl_config_regs(const memctl_options_t *popts,
			       fsl_ddr_cfg_regs_t *ddr,
			       const common_timing_params_t *common_dimm,
			       const dimm_params_t *dimm_params,
			       unsigned int dbw_cap_adj)
{
	unsigned int i;
	unsigned int cas_latency;
	unsigned int additive_latency;
	unsigned int sr_it;
	unsigned int zq_en;
	unsigned int wrlvl_en;

	memset(ddr, 0, sizeof(fsl_ddr_cfg_regs_t));

	if (common_dimm == NULL) {
		printf("Error: subset DIMM params struct null pointer\n");
		return 1;
	}

	/*
	 * Process overrides first.
	 *
	 * FIXME: somehow add dereated caslat to this
	 */
	cas_latency = (popts->cas_latency_override)
		? popts->cas_latency_override_value
		: common_dimm->lowest_common_SPD_caslat;

	additive_latency = (popts->additive_latency_override)
		? popts->additive_latency_override_value
		: common_dimm->additive_latency;

	sr_it = (popts->auto_self_refresh_en)
		? popts->sr_it
		: 0;
	/* ZQ calibration */
	zq_en = (popts->zq_en) ? 1 : 0;
	/* write leveling */
	wrlvl_en = (popts->wrlvl_en) ? 1 : 0;

	/* Chip Select Memory Bounds (CSn_BNDS) */
	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		unsigned long long ea = 0, sa = 0;

		if (popts->ba_intlv_ctl && (i > 0) &&
			((popts->ba_intlv_ctl & 0x60) != FSL_DDR_CS2_CS3 )) {
			/* Don't set up boundaries for other CS
			 * other than CS0, if bank interleaving
			 * is enabled and not CS2+CS3 interleaved.
			 * But we need to set the ODT_RD_CFG and
			 * ODT_WR_CFG for CS1_CONFIG here.
			 */
			set_csn_config(i, ddr, popts, dimm_params);
			break;
		}

		if (dimm_params[i/2].n_ranks == 0) {
			debug("Skipping setup of CS%u "
				"because n_ranks on DIMM %u is 0\n", i, i/2);
			continue;
		}
		if (popts->memctl_interleaving && popts->ba_intlv_ctl) {
			/*
			 * This works superbank 2CS
			 * There are 2 memory controllers configured
			 * identically, memory is interleaved between them,
			 * and each controller uses rank interleaving within
			 * itself. Therefore the starting and ending address
			 * on each controller is twice the amount present on
			 * each controller.
			 */
			unsigned long long rank_density
					= dimm_params[0].capacity;
			ea = (2 * (rank_density >> dbw_cap_adj)) - 1;
		}
		else if (!popts->memctl_interleaving && popts->ba_intlv_ctl) {
			/*
			 * If memory interleaving between controllers is NOT
			 * enabled, the starting address for each memory
			 * controller is distinct.  However, because rank
			 * interleaving is enabled, the starting and ending
			 * addresses of the total memory on that memory
			 * controller needs to be programmed into its
			 * respective CS0_BNDS.
			 */
			unsigned long long rank_density
						= dimm_params[i/2].rank_density;
			switch (popts->ba_intlv_ctl & FSL_DDR_CS0_CS1_CS2_CS3) {
			case FSL_DDR_CS0_CS1_CS2_CS3:
				/* CS0+CS1+CS2+CS3 interleaving, only CS0_CNDS
				 * needs to be set.
				 */
				sa = common_dimm->base_address;
				ea = sa + (4 * (rank_density >> dbw_cap_adj))-1;
				break;
			case FSL_DDR_CS0_CS1_AND_CS2_CS3:
				/* CS0+CS1 and CS2+CS3 interleaving, CS0_CNDS
				 * and CS2_CNDS need to be set.
				 */
				if (!(i&1)) {
					sa = dimm_params[i/2].base_address;
					ea = sa + (i * (rank_density >>
						dbw_cap_adj)) - 1;
				}
				break;
			case FSL_DDR_CS0_CS1:
				/* CS0+CS1 interleaving, CS0_CNDS needs
				 * to be set
				 */
				sa = common_dimm->base_address;
				ea = sa + (2 * (rank_density >> dbw_cap_adj))-1;
				break;
			case FSL_DDR_CS2_CS3:
				/* CS2+CS3 interleaving*/
				if (i == 2) {
					sa = dimm_params[i/2].base_address;
					ea = sa + (2 * (rank_density >>
						dbw_cap_adj)) - 1;
				}
				break;
			default:  /* No bank(chip-select) interleaving */
				break;
			}
		}
		else if (popts->memctl_interleaving && !popts->ba_intlv_ctl) {
			/*
			 * Only the rank on CS0 of each memory controller may
			 * be used if memory controller interleaving is used
			 * without rank interleaving within each memory
			 * controller.  However, the ending address programmed
			 * into each CS0 must be the sum of the amount of
			 * memory in the two CS0 ranks.
			 */
			if (i == 0) {
				unsigned long long rank_density
						= dimm_params[0].rank_density;
				ea = (2 * (rank_density >> dbw_cap_adj)) - 1;
			}

		}
		else if (!popts->memctl_interleaving && !popts->ba_intlv_ctl) {
			/*
			 * No rank interleaving and no memory controller
			 * interleaving.
			 */
			unsigned long long rank_density
						= dimm_params[i/2].rank_density;
			sa = dimm_params[i/2].base_address;
			ea = sa + (rank_density >> dbw_cap_adj) - 1;
			if (i&1) {
				if ((dimm_params[i/2].n_ranks == 1)) {
					/* Odd chip select, single-rank dimm */
					sa = 0;
					ea = 0;
				} else {
					/* Odd chip select, dual-rank DIMM */
					sa += rank_density >> dbw_cap_adj;
					ea += rank_density >> dbw_cap_adj;
				}
			}
		}

		sa >>= 24;
		ea >>= 24;

		ddr->cs[i].bnds = (0
			| ((sa & 0xFFF) << 16)	/* starting address MSB */
			| ((ea & 0xFFF) << 0)	/* ending address MSB */
			);

		debug("FSLDDR: cs[%d]_bnds = 0x%08x\n", i, ddr->cs[i].bnds);
		set_csn_config(i, ddr, popts, dimm_params);
		set_csn_config_2(i, ddr);
	}

#if !defined(CONFIG_FSL_DDR1)
	set_timing_cfg_0(ddr);
#endif

	set_timing_cfg_3(ddr, common_dimm, cas_latency);
	set_timing_cfg_1(ddr, popts, common_dimm, cas_latency);
	set_timing_cfg_2(ddr, popts, common_dimm,
				cas_latency, additive_latency);

	set_ddr_sdram_cfg(ddr, popts, common_dimm);

	set_ddr_sdram_cfg_2(ddr, popts);
	set_ddr_sdram_mode(ddr, popts, common_dimm,
				cas_latency, additive_latency);
	set_ddr_sdram_mode_2(ddr, popts);
	set_ddr_sdram_interval(ddr, popts, common_dimm);
	set_ddr_data_init(ddr);
	set_ddr_sdram_clk_cntl(ddr, popts);
	set_ddr_init_addr(ddr);
	set_ddr_init_ext_addr(ddr);
	set_timing_cfg_4(ddr, popts);
	set_timing_cfg_5(ddr);

	set_ddr_zq_cntl(ddr, zq_en);
	set_ddr_wrlvl_cntl(ddr, wrlvl_en, popts);

	set_ddr_sr_cntr(ddr, sr_it);

	set_ddr_sdram_rcw_1(ddr);
	set_ddr_sdram_rcw_2(ddr);

	return check_fsl_memctl_config_regs(ddr);
}

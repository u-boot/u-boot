/*
 * Copyright 2004, 2007-2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2003 Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <ppc_asm.tmpl>
#include <linux/compiler.h>
#include <asm/processor.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* --------------------------------------------------------------- */

void get_sys_info(sys_info_t *sys_info)
{
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#ifdef CONFIG_FSL_IFC
	struct fsl_ifc *ifc_regs = (void *)CONFIG_SYS_IFC_ADDR;
	u32 ccr;
#endif
#ifdef CONFIG_FSL_CORENET
	volatile ccsr_clk_t *clk = (void *)(CONFIG_SYS_FSL_CORENET_CLK_ADDR);
	unsigned int cpu;

	const u8 core_cplx_PLL[16] = {
		[ 0] = 0,	/* CC1 PPL / 1 */
		[ 1] = 0,	/* CC1 PPL / 2 */
		[ 2] = 0,	/* CC1 PPL / 4 */
		[ 4] = 1,	/* CC2 PPL / 1 */
		[ 5] = 1,	/* CC2 PPL / 2 */
		[ 6] = 1,	/* CC2 PPL / 4 */
		[ 8] = 2,	/* CC3 PPL / 1 */
		[ 9] = 2,	/* CC3 PPL / 2 */
		[10] = 2,	/* CC3 PPL / 4 */
		[12] = 3,	/* CC4 PPL / 1 */
		[13] = 3,	/* CC4 PPL / 2 */
		[14] = 3,	/* CC4 PPL / 4 */
	};

	const u8 core_cplx_pll_div[16] = {
		[ 0] = 1,	/* CC1 PPL / 1 */
		[ 1] = 2,	/* CC1 PPL / 2 */
		[ 2] = 4,	/* CC1 PPL / 4 */
		[ 4] = 1,	/* CC2 PPL / 1 */
		[ 5] = 2,	/* CC2 PPL / 2 */
		[ 6] = 4,	/* CC2 PPL / 4 */
		[ 8] = 1,	/* CC3 PPL / 1 */
		[ 9] = 2,	/* CC3 PPL / 2 */
		[10] = 4,	/* CC3 PPL / 4 */
		[12] = 1,	/* CC4 PPL / 1 */
		[13] = 2,	/* CC4 PPL / 2 */
		[14] = 4,	/* CC4 PPL / 4 */
	};
	uint i, freq_cc_pll[6], rcw_tmp;
	uint ratio[6];
	unsigned long sysclk = CONFIG_SYS_CLK_FREQ;
	uint mem_pll_rat;

	sys_info->freq_systembus = sysclk;
#ifdef CONFIG_DDR_CLK_FREQ
	sys_info->freq_ddrbus = CONFIG_DDR_CLK_FREQ;
#else
	sys_info->freq_ddrbus = sysclk;
#endif

	sys_info->freq_systembus *= (in_be32(&gur->rcwsr[0]) >> 25) & 0x1f;
	mem_pll_rat = (in_be32(&gur->rcwsr[0]) >>
			FSL_CORENET_RCWSR0_MEM_PLL_RAT_SHIFT)
			& FSL_CORENET_RCWSR0_MEM_PLL_RAT_MASK;
	if (mem_pll_rat > 2)
		sys_info->freq_ddrbus *= mem_pll_rat;
	else
		sys_info->freq_ddrbus = sys_info->freq_systembus * mem_pll_rat;

	ratio[0] = (in_be32(&clk->pllc1gsr) >> 1) & 0x3f;
	ratio[1] = (in_be32(&clk->pllc2gsr) >> 1) & 0x3f;
	ratio[2] = (in_be32(&clk->pllc3gsr) >> 1) & 0x3f;
	ratio[3] = (in_be32(&clk->pllc4gsr) >> 1) & 0x3f;
	ratio[4] = (in_be32(&clk->pllc5gsr) >> 1) & 0x3f;
	ratio[5] = (in_be32(&clk->pllc6gsr) >> 1) & 0x3f;
	for (i = 0; i < 6; i++) {
		if (ratio[i] > 4)
			freq_cc_pll[i] = sysclk * ratio[i];
		else
			freq_cc_pll[i] = sys_info->freq_systembus * ratio[i];
	}
#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
	/*
	 * Each cluster has up to 4 cores, sharing the same PLL selection.
	 * The cluster assignment is fixed per SoC. PLL1, PLL2, PLL3 are
	 * cluster group A, feeding cores on cluster 1 and cluster 2.
	 * PLL4, PLL5, PLL6 are cluster group B, feeding cores on cluster 3
	 * and cluster 4 if existing.
	 */
	for_each_cpu(i, cpu, cpu_numcores(), cpu_mask()) {
		int cluster = fsl_qoriq_core_to_cluster(cpu);
		u32 c_pll_sel = (in_be32(&clk->clkcsr[cluster].clkcncsr) >> 27)
				& 0xf;
		u32 cplx_pll = core_cplx_PLL[c_pll_sel];
		if (cplx_pll > 3)
			printf("Unsupported architecture configuration"
				" in function %s\n", __func__);
		cplx_pll += (cluster / 2) * 3;
		sys_info->freq_processor[cpu] =
			 freq_cc_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}
#ifdef CONFIG_PPC_B4860
#define FM1_CLK_SEL	0xe0000000
#define FM1_CLK_SHIFT	29
#else
#define PME_CLK_SEL	0xe0000000
#define PME_CLK_SHIFT	29
#define FM1_CLK_SEL	0x1c000000
#define FM1_CLK_SHIFT	26
#endif
	rcw_tmp = in_be32(&gur->rcwsr[7]);

#ifdef CONFIG_SYS_DPAA_PME
	switch ((rcw_tmp & PME_CLK_SEL) >> PME_CLK_SHIFT) {
	case 1:
		sys_info->freq_pme = freq_cc_pll[0];
		break;
	case 2:
		sys_info->freq_pme = freq_cc_pll[0] / 2;
		break;
	case 3:
		sys_info->freq_pme = freq_cc_pll[0] / 3;
		break;
	case 4:
		sys_info->freq_pme = freq_cc_pll[0] / 4;
		break;
	case 6:
		sys_info->freq_pme = freq_cc_pll[1] / 2;
		break;
	case 7:
		sys_info->freq_pme = freq_cc_pll[1] / 3;
		break;
	default:
		printf("Error: Unknown PME clock select!\n");
	case 0:
		sys_info->freq_pme = sys_info->freq_systembus / 2;
		break;

	}
#endif

#ifdef CONFIG_SYS_DPAA_QBMAN
	sys_info->freq_qman = sys_info->freq_systembus / 2;
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
	switch ((rcw_tmp & FM1_CLK_SEL) >> FM1_CLK_SHIFT) {
	case 1:
		sys_info->freq_fman[0] = freq_cc_pll[3];
		break;
	case 2:
		sys_info->freq_fman[0] = freq_cc_pll[3] / 2;
		break;
	case 3:
		sys_info->freq_fman[0] = freq_cc_pll[3] / 3;
		break;
	case 4:
		sys_info->freq_fman[0] = freq_cc_pll[3] / 4;
		break;
	case 5:
		sys_info->freq_fman[0] = sys_info->freq_systembus;
		break;
	case 6:
		sys_info->freq_fman[0] = freq_cc_pll[4] / 2;
		break;
	case 7:
		sys_info->freq_fman[0] = freq_cc_pll[4] / 3;
		break;
	default:
		printf("Error: Unknown FMan1 clock select!\n");
	case 0:
		sys_info->freq_fman[0] = sys_info->freq_systembus / 2;
		break;
	}
#if (CONFIG_SYS_NUM_FMAN) == 2
#define FM2_CLK_SEL	0x00000038
#define FM2_CLK_SHIFT	3
	rcw_tmp = in_be32(&gur->rcwsr[15]);
	switch ((rcw_tmp & FM2_CLK_SEL) >> FM2_CLK_SHIFT) {
	case 1:
		sys_info->freq_fman[1] = freq_cc_pll[4];
		break;
	case 2:
		sys_info->freq_fman[1] = freq_cc_pll[4] / 2;
		break;
	case 3:
		sys_info->freq_fman[1] = freq_cc_pll[4] / 3;
		break;
	case 4:
		sys_info->freq_fman[1] = freq_cc_pll[4] / 4;
		break;
	case 6:
		sys_info->freq_fman[1] = freq_cc_pll[3] / 2;
		break;
	case 7:
		sys_info->freq_fman[1] = freq_cc_pll[3] / 3;
		break;
	default:
		printf("Error: Unknown FMan2 clock select!\n");
	case 0:
		sys_info->freq_fman[1] = sys_info->freq_systembus / 2;
		break;
	}
#endif	/* CONFIG_SYS_NUM_FMAN == 2 */
#endif	/* CONFIG_SYS_DPAA_FMAN */

#else /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

	for_each_cpu(i, cpu, cpu_numcores(), cpu_mask()) {
		u32 c_pll_sel = (in_be32(&clk->clkcsr[cpu].clkcncsr) >> 27)
				& 0xf;
		u32 cplx_pll = core_cplx_PLL[c_pll_sel];

		sys_info->freq_processor[cpu] =
			 freq_cc_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}
#define PME_CLK_SEL	0x80000000
#define FM1_CLK_SEL	0x40000000
#define FM2_CLK_SEL	0x20000000
#define HWA_ASYNC_DIV	0x04000000
#if (CONFIG_SYS_FSL_NUM_CC_PLLS == 2)
#define HWA_CC_PLL	1
#elif (CONFIG_SYS_FSL_NUM_CC_PLLS == 3)
#define HWA_CC_PLL	2
#elif (CONFIG_SYS_FSL_NUM_CC_PLLS == 4)
#define HWA_CC_PLL	2
#else
#error CONFIG_SYS_FSL_NUM_CC_PLLS not set or unknown case
#endif
	rcw_tmp = in_be32(&gur->rcwsr[7]);

#ifdef CONFIG_SYS_DPAA_PME
	if (rcw_tmp & PME_CLK_SEL) {
		if (rcw_tmp & HWA_ASYNC_DIV)
			sys_info->freq_pme = freq_cc_pll[HWA_CC_PLL] / 4;
		else
			sys_info->freq_pme = freq_cc_pll[HWA_CC_PLL] / 2;
	} else {
		sys_info->freq_pme = sys_info->freq_systembus / 2;
	}
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
	if (rcw_tmp & FM1_CLK_SEL) {
		if (rcw_tmp & HWA_ASYNC_DIV)
			sys_info->freq_fman[0] = freq_cc_pll[HWA_CC_PLL] / 4;
		else
			sys_info->freq_fman[0] = freq_cc_pll[HWA_CC_PLL] / 2;
	} else {
		sys_info->freq_fman[0] = sys_info->freq_systembus / 2;
	}
#if (CONFIG_SYS_NUM_FMAN) == 2
	if (rcw_tmp & FM2_CLK_SEL) {
		if (rcw_tmp & HWA_ASYNC_DIV)
			sys_info->freq_fman[1] = freq_cc_pll[HWA_CC_PLL] / 4;
		else
			sys_info->freq_fman[1] = freq_cc_pll[HWA_CC_PLL] / 2;
	} else {
		sys_info->freq_fman[1] = sys_info->freq_systembus / 2;
	}
#endif
#endif

#ifdef CONFIG_SYS_DPAA_QBMAN
	sys_info->freq_qman = sys_info->freq_systembus / 2;
#endif

#endif /* CONFIG_SYS_FSL_QORIQ_CHASSIS2 */

#else /* CONFIG_FSL_CORENET */
	uint plat_ratio, e500_ratio, half_freq_systembus;
	int i;
#ifdef CONFIG_QE
	__maybe_unused u32 qe_ratio;
#endif

	plat_ratio = (gur->porpllsr) & 0x0000003e;
	plat_ratio >>= 1;
	sys_info->freq_systembus = plat_ratio * CONFIG_SYS_CLK_FREQ;

	/* Divide before multiply to avoid integer
	 * overflow for processor speeds above 2GHz */
	half_freq_systembus = sys_info->freq_systembus/2;
	for (i = 0; i < cpu_numcores(); i++) {
		e500_ratio = ((gur->porpllsr) >> (i * 8 + 16)) & 0x3f;
		sys_info->freq_processor[i] = e500_ratio * half_freq_systembus;
	}

	/* Note: freq_ddrbus is the MCLK frequency, not the data rate. */
	sys_info->freq_ddrbus = sys_info->freq_systembus;

#ifdef CONFIG_DDR_CLK_FREQ
	{
		u32 ddr_ratio = ((gur->porpllsr) & MPC85xx_PORPLLSR_DDR_RATIO)
			>> MPC85xx_PORPLLSR_DDR_RATIO_SHIFT;
		if (ddr_ratio != 0x7)
			sys_info->freq_ddrbus = ddr_ratio * CONFIG_DDR_CLK_FREQ;
	}
#endif

#ifdef CONFIG_QE
#if defined(CONFIG_P1012) || defined(CONFIG_P1021) || defined(CONFIG_P1025)
	sys_info->freq_qe =  sys_info->freq_systembus;
#else
	qe_ratio = ((gur->porpllsr) & MPC85xx_PORPLLSR_QE_RATIO)
			>> MPC85xx_PORPLLSR_QE_RATIO_SHIFT;
	sys_info->freq_qe = qe_ratio * CONFIG_SYS_CLK_FREQ;
#endif
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
		sys_info->freq_fman[0] = sys_info->freq_systembus;
#endif

#endif /* CONFIG_FSL_CORENET */

#if defined(CONFIG_FSL_LBC)
	uint lcrr_div;
#if defined(CONFIG_SYS_LBC_LCRR)
	/* We will program LCRR to this value later */
	lcrr_div = CONFIG_SYS_LBC_LCRR & LCRR_CLKDIV;
#else
	lcrr_div = in_be32(&(LBC_BASE_ADDR)->lcrr) & LCRR_CLKDIV;
#endif
	if (lcrr_div == 2 || lcrr_div == 4 || lcrr_div == 8) {
#if defined(CONFIG_FSL_CORENET)
		/* If this is corenet based SoC, bit-representation
		 * for four times the clock divider values.
		 */
		lcrr_div *= 4;
#elif !defined(CONFIG_MPC8540) && !defined(CONFIG_MPC8541) && \
    !defined(CONFIG_MPC8555) && !defined(CONFIG_MPC8560)
		/*
		 * Yes, the entire PQ38 family use the same
		 * bit-representation for twice the clock divider values.
		 */
		lcrr_div *= 2;
#endif
		sys_info->freq_localbus = sys_info->freq_systembus / lcrr_div;
	} else {
		/* In case anyone cares what the unknown value is */
		sys_info->freq_localbus = lcrr_div;
	}
#endif

#if defined(CONFIG_FSL_IFC)
	ccr = in_be32(&ifc_regs->ifc_ccr);
	ccr = ((ccr & IFC_CCR_CLK_DIV_MASK) >> IFC_CCR_CLK_DIV_SHIFT) + 1;

	sys_info->freq_localbus = sys_info->freq_systembus / ccr;
#endif
}


int get_clocks (void)
{
	sys_info_t sys_info;
#ifdef CONFIG_MPC8544
	volatile ccsr_gur_t *gur = (void *) CONFIG_SYS_MPC85xx_GUTS_ADDR;
#endif
#if defined(CONFIG_CPM2)
	volatile ccsr_cpm_t *cpm = (ccsr_cpm_t *)CONFIG_SYS_MPC85xx_CPM_ADDR;
	uint sccr, dfbrg;

	/* set VCO = 4 * BRG */
	cpm->im_cpm_intctl.sccr &= 0xfffffffc;
	sccr = cpm->im_cpm_intctl.sccr;
	dfbrg = (sccr & SCCR_DFBRG_MSK) >> SCCR_DFBRG_SHIFT;
#endif
	get_sys_info (&sys_info);
	gd->cpu_clk = sys_info.freq_processor[0];
	gd->bus_clk = sys_info.freq_systembus;
	gd->mem_clk = sys_info.freq_ddrbus;
	gd->arch.lbc_clk = sys_info.freq_localbus;

#ifdef CONFIG_QE
	gd->arch.qe_clk = sys_info.freq_qe;
	gd->arch.brg_clk = gd->arch.qe_clk / 2;
#endif
	/*
	 * The base clock for I2C depends on the actual SOC.  Unfortunately,
	 * there is no pattern that can be used to determine the frequency, so
	 * the only choice is to look up the actual SOC number and use the value
	 * for that SOC. This information is taken from application note
	 * AN2919.
	 */
#if defined(CONFIG_MPC8540) || defined(CONFIG_MPC8541) || \
	defined(CONFIG_MPC8560) || defined(CONFIG_MPC8555) || \
	defined(CONFIG_P1022)
	gd->arch.i2c1_clk = sys_info.freq_systembus;
#elif defined(CONFIG_MPC8544)
	/*
	 * On the 8544, the I2C clock is the same as the SEC clock.  This can be
	 * either CCB/2 or CCB/3, depending on the value of cfg_sec_freq. See
	 * 4.4.3.3 of the 8544 RM.  Note that this might actually work for all
	 * 85xx, but only the 8544 has cfg_sec_freq, so it's unknown if the
	 * PORDEVSR2_SEC_CFG bit is 0 on all 85xx boards that are not an 8544.
	 */
	if (gur->pordevsr2 & MPC85xx_PORDEVSR2_SEC_CFG)
		gd->arch.i2c1_clk = sys_info.freq_systembus / 3;
	else
		gd->arch.i2c1_clk = sys_info.freq_systembus / 2;
#else
	/* Most 85xx SOCs use CCB/2, so this is the default behavior. */
	gd->arch.i2c1_clk = sys_info.freq_systembus / 2;
#endif
	gd->arch.i2c2_clk = gd->arch.i2c1_clk;

#if defined(CONFIG_FSL_ESDHC)
#if defined(CONFIG_MPC8569) || defined(CONFIG_P1010) ||\
       defined(CONFIG_P1014)
	gd->arch.sdhc_clk = gd->bus_clk;
#else
	gd->arch.sdhc_clk = gd->bus_clk / 2;
#endif
#endif /* defined(CONFIG_FSL_ESDHC) */

#if defined(CONFIG_CPM2)
	gd->arch.vco_out = 2*sys_info.freq_systembus;
	gd->arch.cpm_clk = gd->arch.vco_out / 2;
	gd->arch.scc_clk = gd->arch.vco_out / 4;
	gd->arch.brg_clk = gd->arch.vco_out / (1 << (2 * (dfbrg + 1)));
#endif

	if(gd->cpu_clk != 0) return (0);
	else return (1);
}


/********************************************
 * get_bus_freq
 * return system bus freq in Hz
 *********************************************/
ulong get_bus_freq (ulong dummy)
{
	return gd->bus_clk;
}

/********************************************
 * get_ddr_freq
 * return ddr bus freq in Hz
 *********************************************/
ulong get_ddr_freq (ulong dummy)
{
	return gd->mem_clk;
}

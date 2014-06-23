/*
 * Copyright 2014, Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Derived from arch/power/cpu/mpc85xx/speed.c
 */

#include <common.h>
#include <linux/compiler.h>
#include <fsl_ifc.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/arch-fsl-lsch3/immap_lsch3.h>
#include <asm/arch/clock.h>
#include "cpu.h"

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_FSL_NUM_CC_PLLS
#define CONFIG_SYS_FSL_NUM_CC_PLLS	6
#endif


void get_sys_info(struct sys_info *sys_info)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
#ifdef CONFIG_FSL_IFC
	struct fsl_ifc *ifc_regs = (void *)CONFIG_SYS_IFC_ADDR;
	u32 ccr;
#endif
	struct ccsr_clk_cluster_group __iomem *clk_grp[2] = {
		(void *)(CONFIG_SYS_FSL_CH3_CLK_GRPA_ADDR),
		(void *)(CONFIG_SYS_FSL_CH3_CLK_GRPB_ADDR)
	};
	struct ccsr_clk_ctrl __iomem *clk_ctrl =
		(void *)(CONFIG_SYS_FSL_CH3_CLK_CTRL_ADDR);
	unsigned int cpu;
	const u8 core_cplx_pll[16] = {
		[0] = 0,	/* CC1 PPL / 1 */
		[1] = 0,	/* CC1 PPL / 2 */
		[2] = 0,	/* CC1 PPL / 4 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 1,	/* CC2 PPL / 2 */
		[6] = 1,	/* CC2 PPL / 4 */
		[8] = 2,	/* CC3 PPL / 1 */
		[9] = 2,	/* CC3 PPL / 2 */
		[10] = 2,	/* CC3 PPL / 4 */
		[12] = 3,	/* CC4 PPL / 1 */
		[13] = 3,	/* CC4 PPL / 2 */
		[14] = 3,	/* CC4 PPL / 4 */
	};

	const u8 core_cplx_pll_div[16] = {
		[0] = 1,	/* CC1 PPL / 1 */
		[1] = 2,	/* CC1 PPL / 2 */
		[2] = 4,	/* CC1 PPL / 4 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 2,	/* CC2 PPL / 2 */
		[6] = 4,	/* CC2 PPL / 4 */
		[8] = 1,	/* CC3 PPL / 1 */
		[9] = 2,	/* CC3 PPL / 2 */
		[10] = 4,	/* CC3 PPL / 4 */
		[12] = 1,	/* CC4 PPL / 1 */
		[13] = 2,	/* CC4 PPL / 2 */
		[14] = 4,	/* CC4 PPL / 4 */
	};

	uint i, cluster;
	uint freq_c_pll[CONFIG_SYS_FSL_NUM_CC_PLLS];
	uint ratio[CONFIG_SYS_FSL_NUM_CC_PLLS];
	unsigned long sysclk = CONFIG_SYS_CLK_FREQ;
	int cc_group[12] = CONFIG_SYS_FSL_CLUSTER_CLOCKS;
	u32 c_pll_sel, cplx_pll;
	void *offset;

	sys_info->freq_systembus = sysclk;
#ifdef CONFIG_DDR_CLK_FREQ
	sys_info->freq_ddrbus = CONFIG_DDR_CLK_FREQ;
#else
	sys_info->freq_ddrbus = sysclk;
#endif

	sys_info->freq_systembus *= (in_le32(&gur->rcwsr[0]) >>
			FSL_CHASSIS3_RCWSR0_SYS_PLL_RAT_SHIFT) &
			FSL_CHASSIS3_RCWSR0_SYS_PLL_RAT_MASK;
	sys_info->freq_ddrbus *= (in_le32(&gur->rcwsr[0]) >>
			FSL_CHASSIS3_RCWSR0_MEM_PLL_RAT_SHIFT) &
			FSL_CHASSIS3_RCWSR0_MEM_PLL_RAT_MASK;

	for (i = 0; i < CONFIG_SYS_FSL_NUM_CC_PLLS; i++) {
		/*
		 * fixme: prefer to combine the following into one line, but
		 * cannot pass compiling without warning about in_le32.
		 */
		offset = (void *)((size_t)clk_grp[i/3] +
			 offsetof(struct ccsr_clk_cluster_group,
				  pllngsr[i%3].gsr));
		ratio[i] = (in_le32(offset) >> 1) & 0x3f;
		if (ratio[i] > 4)
			freq_c_pll[i] = sysclk * ratio[i];
		else
			freq_c_pll[i] = sys_info->freq_systembus * ratio[i];
	}

	for_each_cpu(i, cpu, cpu_numcores(), cpu_mask()) {
		cluster = fsl_qoriq_core_to_cluster(cpu);
		c_pll_sel = (in_le32(&clk_ctrl->clkcncsr[cluster].csr) >> 27)
			    & 0xf;
		cplx_pll = core_cplx_pll[c_pll_sel];
		cplx_pll += cc_group[cluster] - 1;
		sys_info->freq_processor[cpu] =
			freq_c_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}

#if defined(CONFIG_FSL_IFC)
	ccr = in_le32(&ifc_regs->ifc_ccr);
	ccr = ((ccr & IFC_CCR_CLK_DIV_MASK) >> IFC_CCR_CLK_DIV_SHIFT) + 1;

	sys_info->freq_localbus = sys_info->freq_systembus / ccr;
#endif
}


int get_clocks(void)
{
	struct sys_info sys_info;
	get_sys_info(&sys_info);
	gd->cpu_clk = sys_info.freq_processor[0];
	gd->bus_clk = sys_info.freq_systembus;
	gd->mem_clk = sys_info.freq_ddrbus;

#if defined(CONFIG_FSL_ESDHC)
	gd->arch.sdhc_clk = gd->bus_clk / 2;
#endif /* defined(CONFIG_FSL_ESDHC) */

	if (gd->cpu_clk != 0)
		return 0;
	else
		return 1;
}

/********************************************
 * get_bus_freq
 * return system bus freq in Hz
 *********************************************/
ulong get_bus_freq(ulong dummy)
{
	if (!gd->bus_clk)
		get_clocks();

	return gd->bus_clk;
}

/********************************************
 * get_ddr_freq
 * return ddr bus freq in Hz
 *********************************************/
ulong get_ddr_freq(ulong dummy)
{
	if (!gd->mem_clk)
		get_clocks();

	return gd->mem_clk;
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_I2C_CLK:
		return get_bus_freq(0) / 2;
	default:
		printf("Unsupported clock\n");
	}
	return 0;
}

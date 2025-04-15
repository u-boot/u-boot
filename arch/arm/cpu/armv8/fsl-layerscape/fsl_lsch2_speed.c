// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 * Copyright 2019 NXP.
 */

#include <config.h>
#include <clock_legacy.h>
#include <cpu_func.h>
#include <asm/global_data.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/arch/clock.h>
#include <asm/arch/soc.h>
#include <fsl_ifc.h>
#include "cpu.h"

DECLARE_GLOBAL_DATA_PTR;

void get_sys_info(struct sys_info *sys_info)
{
	struct ccsr_gur __iomem *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
/* rcw_tmp is needed to get FMAN clock, or to get cluster group A
 * mux 2 clock for LS1043A/LS1046A.
 */
	__maybe_unused u32 rcw_tmp;
	struct ccsr_clk *clk = (void *)(CFG_SYS_FSL_CLK_ADDR);
	unsigned int cpu;
	const u8 core_cplx_pll[8] = {
		[0] = 0,	/* CC1 PPL / 1 */
		[1] = 0,	/* CC1 PPL / 2 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 1,	/* CC2 PPL / 2 */
	};

	const u8 core_cplx_pll_div[8] = {
		[0] = 1,	/* CC1 PPL / 1 */
		[1] = 2,	/* CC1 PPL / 2 */
		[4] = 1,	/* CC2 PPL / 1 */
		[5] = 2,	/* CC2 PPL / 2 */
	};

	uint i, cluster;
	uint freq_c_pll[CONFIG_SYS_FSL_NUM_CC_PLLS];
	uint ratio[CONFIG_SYS_FSL_NUM_CC_PLLS];
	unsigned long sysclk = get_board_sys_clk();
	unsigned long cluster_clk;

	sys_info->freq_systembus = sysclk;
#ifdef CONFIG_CLUSTER_CLK_FREQ
	cluster_clk = CONFIG_CLUSTER_CLK_FREQ;
#else
	cluster_clk = get_board_sys_clk();
#endif

#if defined(CONFIG_DYNAMIC_DDR_CLK_FREQ) || defined(CONFIG_STATIC_DDR_CLK_FREQ)
	sys_info->freq_ddrbus = get_board_ddr_clk();
#else
	sys_info->freq_ddrbus = sysclk;
#endif

	/* The freq_systembus is used to record frequency of platform PLL */
	sys_info->freq_systembus *= (gur_in32(&gur->rcwsr[0]) >>
			FSL_CHASSIS2_RCWSR0_SYS_PLL_RAT_SHIFT) &
			FSL_CHASSIS2_RCWSR0_SYS_PLL_RAT_MASK;

#ifdef CONFIG_ARCH_LS1012A
	sys_info->freq_ddrbus = 2 * sys_info->freq_systembus;
#else
	sys_info->freq_ddrbus *= (gur_in32(&gur->rcwsr[0]) >>
			FSL_CHASSIS2_RCWSR0_MEM_PLL_RAT_SHIFT) &
			FSL_CHASSIS2_RCWSR0_MEM_PLL_RAT_MASK;
#endif

	for (i = 0; i < CONFIG_SYS_FSL_NUM_CC_PLLS; i++) {
		ratio[i] = (in_be32(&clk->pllcgsr[i].pllcngsr) >> 1) & 0xff;
		if (ratio[i] > 4)
			freq_c_pll[i] = cluster_clk * ratio[i];
		else
			freq_c_pll[i] = sys_info->freq_systembus * ratio[i];
	}

	for_each_cpu(i, cpu, cpu_numcores(), cpu_mask()) {
		cluster = fsl_qoriq_core_to_cluster(cpu);
		u32 c_pll_sel = (in_be32(&clk->clkcsr[cluster].clkcncsr) >> 27)
				& 0xf;
		u32 cplx_pll = core_cplx_pll[c_pll_sel];

		sys_info->freq_processor[cpu] =
			freq_c_pll[cplx_pll] / core_cplx_pll_div[c_pll_sel];
	}

#define HWA_CGA_M1_CLK_SEL	0xe0000000
#define HWA_CGA_M1_CLK_SHIFT	29
#if defined(CONFIG_SYS_DPAA_FMAN) && !defined(CONFIG_XPL_BUILD)
	rcw_tmp = in_be32(&gur->rcwsr[7]);
	switch ((rcw_tmp & HWA_CGA_M1_CLK_SEL) >> HWA_CGA_M1_CLK_SHIFT) {
	case 2:
		sys_info->freq_fman[0] = freq_c_pll[0] / 2;
		break;
	case 3:
		sys_info->freq_fman[0] = freq_c_pll[0] / 3;
		break;
	case 4:
		sys_info->freq_fman[0] = freq_c_pll[0] / 4;
		break;
	case 5:
		sys_info->freq_fman[0] = sys_info->freq_systembus;
		break;
	case 6:
		sys_info->freq_fman[0] = freq_c_pll[1] / 2;
		break;
	case 7:
		sys_info->freq_fman[0] = freq_c_pll[1] / 3;
		break;
	default:
		printf("Error: Unknown FMan1 clock select!\n");
		break;
	}
#endif

#define HWA_CGA_M2_CLK_SEL	0x00000007
#define HWA_CGA_M2_CLK_SHIFT	0
#if defined(CONFIG_ARCH_LS1046A) || defined(CONFIG_ARCH_LS1043A)
	rcw_tmp = in_be32(&gur->rcwsr[15]);
	switch ((rcw_tmp & HWA_CGA_M2_CLK_SEL) >> HWA_CGA_M2_CLK_SHIFT) {
	case 1:
		sys_info->freq_cga_m2 = freq_c_pll[1];
		break;
#if defined(CONFIG_ARCH_LS1046A)
	case 2:
		sys_info->freq_cga_m2 = freq_c_pll[1] / 2;
		break;
#endif
	case 3:
		sys_info->freq_cga_m2 = freq_c_pll[1] / 3;
		break;
#if defined(CONFIG_ARCH_LS1046A)
	case 6:
		sys_info->freq_cga_m2 = freq_c_pll[0] / 2;
		break;
#endif
	default:
		printf("Error: Unknown cluster group A mux 2 clock select!\n");
		break;
	}
#endif

#if defined(CONFIG_FSL_IFC)
	sys_info->freq_localbus = sys_info->freq_systembus /
						CONFIG_SYS_FSL_IFC_CLK_DIV;
#endif
#ifdef CONFIG_SYS_DPAA_QBMAN
	sys_info->freq_qman = (sys_info->freq_systembus /
				CONFIG_SYS_FSL_PCLK_DIV) /
				CONFIG_SYS_FSL_QMAN_CLK_DIV;
#endif
}

#ifdef CONFIG_SYS_DPAA_QBMAN
unsigned long get_qman_freq(void)
{
	struct sys_info sys_info;

	get_sys_info(&sys_info);

	return sys_info.freq_qman;
}
#endif

int get_clocks(void)
{
	struct sys_info sys_info;
#ifdef CONFIG_FSL_ESDHC
	u32 clock = 0;
#endif
	get_sys_info(&sys_info);
	gd->cpu_clk = sys_info.freq_processor[0];
	gd->bus_clk = sys_info.freq_systembus / CONFIG_SYS_FSL_PCLK_DIV;
	gd->mem_clk = sys_info.freq_ddrbus;
#ifdef CONFIG_FSL_ESDHC
#if defined(CONFIG_ARCH_LS1012A)
	clock = sys_info.freq_systembus;
#elif defined(CONFIG_ARCH_LS1043A) || defined(CONFIG_ARCH_LS1046A)
	clock = sys_info.freq_cga_m2;
#endif
	gd->arch.sdhc_per_clk = clock / CONFIG_SYS_FSL_SDHC_CLK_DIV;
	gd->arch.sdhc_clk = gd->bus_clk / CONFIG_SYS_FSL_SDHC_CLK_DIV;
#endif
	if (gd->cpu_clk != 0)
		return 0;
	else
		return 1;
}

/********************************************
 * get_bus_freq
 * return platform clock in Hz
 *********************************************/
ulong get_bus_freq(ulong dummy)
{
	if (!gd->bus_clk)
		get_clocks();

	return gd->bus_clk;
}

ulong get_ddr_freq(ulong dummy)
{
	if (!gd->mem_clk)
		get_clocks();

	return gd->mem_clk;
}

int get_serial_clock(void)
{
	return get_bus_freq(0) / CONFIG_SYS_FSL_DUART_CLK_DIV;
}

int get_i2c_freq(ulong dummy)
{
	return get_bus_freq(0) / CONFIG_SYS_FSL_I2C_CLK_DIV;
}

int get_dspi_freq(ulong dummy)
{
	return get_bus_freq(0) / CONFIG_SYS_FSL_DSPI_CLK_DIV;
}

#ifdef CONFIG_FSL_LPUART
int get_uart_freq(ulong dummy)
{
	return get_bus_freq(0) / CONFIG_SYS_FSL_LPUART_CLK_DIV;
}
#endif

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_I2C_CLK:
		return get_i2c_freq(0);
	case MXC_DSPI_CLK:
		return get_dspi_freq(0);
#ifdef CONFIG_FSL_LPUART
	case MXC_UART_CLK:
		return get_uart_freq(0);
#endif
	default:
		printf("Unsupported clock\n");
	}
	return 0;
}

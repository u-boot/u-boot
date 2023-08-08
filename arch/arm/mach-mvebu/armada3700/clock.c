/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0+
 * https://spdx.org/licenses
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <mach/clock.h>

#define NB_CLOCK_REGS_BASE	(MVEBU_REGISTER(0x13000))
#define NB_PLL_BASE		(NB_CLOCK_REGS_BASE + 0x200)
#define NB_TBG_CTRL0		(NB_PLL_BASE + 0x4)
 #define NB_TBG_CTRL0_TBG_A_FBDIV_OFFSET		2
 #define NB_TBG_CTRL0_TBG_A_FBDIV_MASK			0x1FFUL
 #define NB_TBG_CTRL0_TBG_B_FBDIV_OFFSET		18
 #define NB_TBG_CTRL0_TBG_B_FBDIV_MASK			0x1FFUL

#define NB_TBG_CTRL1		(NB_PLL_BASE + 0x8)
 #define NB_TBG_CTRL1_TBG_B_VCODIV_SEL_SE_OFFSET	16
 #define NB_TBG_CTRL1_TBG_B_VCODIV_SEL_SE_MASK		0x1FFUL
 #define NB_TBG_CTRL1_TBG_A_VCODIV_SEL_SE_MASK		0x1FFUL

#define NB_TBG_CTRL7		(NB_PLL_BASE + 0x20)
 #define NB_TBG_CTRL7_TBG_B_REFDIV_OFFSET		16
 #define NB_TBG_CTRL7_TBG_B_REFDIV_MASK			0x1FFUL
 #define NB_TBG_CTRL7_TBG_A_REFDIV_MASK			0x1FFUL

#define NB_TBG_CTRL8		(NB_PLL_BASE + 0x30)
 #define NB_TBG_CTRL8_TBG_A_VCODIV_SEL_DIFF_OFFSET	1
 #define NB_TBG_CTRL8_TBG_A_VCODIV_SEL_DIFF_MASK	0x1FFUL
 #define NB_TBG_CTRL8_TBG_B_VCODIV_SEL_DIFF_OFFSET	17
 #define NB_TBG_CTRL8_TBG_B_VCODIV_SEL_DIFF_MASK	0x1FFUL

#define NB_CLOCK_TBG_SELECT_REG	NB_CLOCK_REGS_BASE
 #define NB_CLOCK_TBG_SEL_A53_CPU_PCLK_OFFSET		22
 #define NB_CLOCK_TBG_SEL_A53_CPU_PCLK_MASK		0x3

/* north bridge clock divider select registers */
#define NB_CLOCK_DIV_SEL0_REG	(NB_CLOCK_REGS_BASE + 0x4)
 #define NB_CLOCK_DIV_SEL0_A53_CPU_CLK_PRSCL_OFFSET	28
 #define NB_CLOCK_DIV_SEL0_A53_CPU_CLK_PRSCL_MASK	0x7

/* north bridge clock source register */
#define NB_CLOCK_SELECT_REG	(NB_CLOCK_REGS_BASE + 0x10)
 #define NB_CLOCK_SEL_DDR_PHY_CLK_SEL_OFFSET		10
 #define NB_CLOCK_SEL_DDR_PHY_CLK_SEL_MASK		0x1
 #define NB_CLOCK_SEL_A53_CPU_CLK_OFFSET		15
 #define NB_CLOCK_SEL_A53_CPU_CLK_MASK			0x1

#define TBG_A_REFDIV_GET(reg_val)	((reg_val >> 0) &\
					NB_TBG_CTRL7_TBG_A_REFDIV_MASK)
#define TBG_B_REFDIV_GET(reg_val)	((reg_val >>\
					NB_TBG_CTRL7_TBG_B_REFDIV_OFFSET) &\
					NB_TBG_CTRL7_TBG_B_REFDIV_MASK)
#define	TBG_A_FBDIV_GET(reg_val)	((reg_val >>\
					NB_TBG_CTRL0_TBG_A_FBDIV_OFFSET) &\
					NB_TBG_CTRL0_TBG_A_FBDIV_MASK)
#define TBG_B_FBDIV_GET(reg_val)	((reg_val >>\
					NB_TBG_CTRL0_TBG_B_FBDIV_OFFSET) &\
					NB_TBG_CTRL0_TBG_B_FBDIV_MASK)
#define TBG_A_VCODIV_SEL_SE_GET(reg_val)	((reg_val >> 0) &\
					NB_TBG_CTRL1_TBG_A_VCODIV_SEL_SE_MASK)
#define TBG_B_VCODIV_SEL_SE_GET(reg_val)	((reg_val >>\
				NB_TBG_CTRL1_TBG_B_VCODIV_SEL_SE_OFFSET) &\
				NB_TBG_CTRL1_TBG_B_VCODIV_SEL_SE_MASK)
#define TBG_A_VCODIV_SEL_DIFF_GET(reg_val)	((reg_val >>\
				NB_TBG_CTRL8_TBG_A_VCODIV_SEL_DIFF_OFFSET) &\
				NB_TBG_CTRL8_TBG_A_VCODIV_SEL_DIFF_MASK)
#define TBG_B_VCODIV_SEL_DIFF_GET(reg_val)	((reg_val >>\
				NB_TBG_CTRL8_TBG_B_VCODIV_SEL_DIFF_OFFSET) &\
				NB_TBG_CTRL8_TBG_B_VCODIV_SEL_DIFF_MASK)
#define A53_CPU_CLK_SEL_GET(reg_val)	((reg_val >>\
					NB_CLOCK_SEL_A53_CPU_CLK_OFFSET) &\
					NB_CLOCK_SEL_A53_CPU_CLK_MASK)
#define A53_CPU_PCLK_SEL_GET(reg_val)	((reg_val >>\
					NB_CLOCK_TBG_SEL_A53_CPU_PCLK_OFFSET) &\
					NB_CLOCK_TBG_SEL_A53_CPU_PCLK_MASK)
#define A53_CPU_CLK_PRSCL_GET(reg_val)	((reg_val >>\
				NB_CLOCK_DIV_SEL0_A53_CPU_CLK_PRSCL_OFFSET) &\
				NB_CLOCK_DIV_SEL0_A53_CPU_CLK_PRSCL_MASK)
#define DDR_PHY_CLK_SEL_GET(reg_val)	((reg_val >>\
					NB_CLOCK_SEL_DDR_PHY_CLK_SEL_OFFSET) &\
					NB_CLOCK_SEL_DDR_PHY_CLK_SEL_MASK)

#define TCLK		200
#define L2_CLK		800
#define TIMER_CLK	800

enum a3700_clock_line {
	TBG_A_P = 0,
	TBG_B_P = 1,
	TBG_A_S = 2,
	TBG_B_S = 3
};

/* Clock source selection */
enum a3700_clk_select {
	CLK_SEL_OSC = 0,
	CLK_SEL_TBG,
};

/* TBG divider */
enum a3700_tbg_divider {
	TBG_DIVIDER_1 = 1,
	TBG_DIVIDER_2,
	TBG_DIVIDER_3,
	TBG_DIVIDER_4,
	TBG_DIVIDER_5,
	TBG_DIVIDER_6,
	TBG_DIVIDER_NUM
};

static u32 get_tbg_clk(enum a3700_clock_line tbg_typ)
{
	u32 tbg_M, tbg_N, vco_div;
	u32 ref, reg_val;

	/* get ref clock */
	ref = get_ref_clk();

	/* get M, N */
	reg_val = readl(NB_TBG_CTRL7);
	tbg_M = ((tbg_typ == TBG_A_S) || (tbg_typ == TBG_A_P)) ?
		TBG_A_REFDIV_GET(reg_val) : TBG_B_REFDIV_GET(reg_val);
	tbg_M = (tbg_M == 0) ? 1 : tbg_M;

	reg_val = readl(NB_TBG_CTRL0);
	tbg_N = ((tbg_typ == TBG_A_S) || (tbg_typ == TBG_A_P)) ?
		TBG_A_FBDIV_GET(reg_val) : TBG_B_FBDIV_GET(reg_val);

	if ((tbg_typ == TBG_A_S) || (tbg_typ == TBG_B_S)) {
		/* get SE VCODIV */
		reg_val = readl(NB_TBG_CTRL1);
		reg_val = (tbg_typ == TBG_A_S) ?
			  TBG_A_VCODIV_SEL_SE_GET(reg_val) :
			  TBG_B_VCODIV_SEL_SE_GET(reg_val);
	} else {
		/* get DIFF VCODIV */
		reg_val = readl(NB_TBG_CTRL8);
		reg_val = (tbg_typ == TBG_A_P) ?
			  TBG_A_VCODIV_SEL_DIFF_GET(reg_val) :
			  TBG_B_VCODIV_SEL_DIFF_GET(reg_val);
	}
	if (reg_val > 7)
		return 0; /*invalid*/

	vco_div = 0x1 << reg_val;

	return ((tbg_N * ref) << 2) / (tbg_M * vco_div);
}

u32 soc_cpu_clk_get(void)
{
	u32 tbg, cpu_prscl;
	enum a3700_clock_line tbg_typ;

	/* 1. check cpu clock select */
	if (!A53_CPU_CLK_SEL_GET(readl(NB_CLOCK_SELECT_REG)))
		return 0; /* CPU clock is using XTAL output*/

	/* 2. get TBG select */
	tbg_typ = A53_CPU_PCLK_SEL_GET(readl(NB_CLOCK_TBG_SELECT_REG));

	/* 3. get TBG clock */
	tbg = get_tbg_clk(tbg_typ);
	if (tbg == 0)
		return 0;

	/* 4. get CPU clk divider */
	cpu_prscl = A53_CPU_CLK_PRSCL_GET(readl(NB_CLOCK_DIV_SEL0_REG));
	if (cpu_prscl == 7)
		return 0; /* divider value error */

	return tbg / cpu_prscl;
}

u32 soc_ddr_clk_get(void)
{
	u32 tbg;

	/* 1. check DDR clock select */
	if (!DDR_PHY_CLK_SEL_GET(readl(NB_CLOCK_SELECT_REG)))
		return 0; /* DDR clock is using XTAL output*/

	/* 2. get TBG_A clock */
	tbg = get_tbg_clk(TBG_A_S);
	if (tbg == 0)
		return 0;

	return tbg >> 1;
}

/******************************************************************************
 * Name: get_cpu_clk_src_div
 *
 * Description: Get CPU clock source selection and prescaling divider
 *
 * Input:	None
 * Output:	cpu_clk_sel: CPU clock source selection
 *		cpu_clk_prscl: CPU clock prescaling divider
 * Return:	Non-zero if failed to get the CPU clock selection and prescaling
 *******************************************************************************
 */
int get_cpu_clk_src_div(u32 *cpu_clk_sel, u32 *cpu_clk_prscl)
{
	/* 1. check cpu clock select */
	if (!A53_CPU_CLK_SEL_GET(readl(NB_CLOCK_SELECT_REG)))
		return -1; /* CPU clock is using XTAL output*/

	/* 2. get TBG select */
	*cpu_clk_sel = A53_CPU_PCLK_SEL_GET(readl(NB_CLOCK_TBG_SELECT_REG));

	/* 3. get CPU clk divider */
	*cpu_clk_prscl = A53_CPU_CLK_PRSCL_GET(readl(NB_CLOCK_DIV_SEL0_REG));

	return 0;
}

u32 soc_tclk_get(void)
{
	return TCLK;
}

u32 soc_l2_clk_get(void)
{
	return L2_CLK;
}

u32 soc_timer_clk_get(void)
{
	return TIMER_CLK;
}

void soc_print_clock_info(void)
{
	printf("       CPU     %d [MHz]\n", soc_cpu_clk_get());
	printf("       L2      %d [MHz]\n", soc_l2_clk_get());
	printf("       TClock  %d [MHz]\n", soc_tclk_get());
	printf("       DDR     %d [MHz]\n", soc_ddr_clk_get());
}

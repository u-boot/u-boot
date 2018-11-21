/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 */
#ifndef _ASM_ARCH_CRU_RV1108_H
#define _ASM_ARCH_CRU_RV1108_H

#include <common.h>

#define OSC_HZ		(24 * 1000 * 1000)

#define APLL_HZ		(600 * 1000000)
#define GPLL_HZ		(594 * 1000000)

struct rv1108_clk_priv {
	struct rv1108_cru *cru;
	ulong rate;
};

struct rv1108_cru {
	struct rv1108_pll {
		unsigned int con0;
		unsigned int con1;
		unsigned int con2;
		unsigned int con3;
		unsigned int con4;
		unsigned int con5;
		unsigned int reserved[2];
	} pll[3];
	unsigned int clksel_con[46];
	unsigned int reserved1[2];
	unsigned int clkgate_con[20];
	unsigned int reserved2[4];
	unsigned int softrst_con[13];
	unsigned int reserved3[3];
	unsigned int glb_srst_fst_val;
	unsigned int glb_srst_snd_val;
	unsigned int glb_cnt_th;
	unsigned int misc_con;
	unsigned int glb_rst_con;
	unsigned int glb_rst_st;
	unsigned int sdmmc_con[2];
	unsigned int sdio_con[2];
	unsigned int emmc_con[2];
};
check_member(rv1108_cru, emmc_con[1], 0x01ec);

struct pll_div {
	u32 refdiv;
	u32 fbdiv;
	u32 postdiv1;
	u32 postdiv2;
	u32 frac;
};

enum {
	/* PLL CON0 */
	FBDIV_MASK		= 0xfff,
	FBDIV_SHIFT		= 0,

	/* PLL CON1 */
	POSTDIV2_SHIFT          = 12,
	POSTDIV2_MASK		= 7 << POSTDIV2_SHIFT,
	POSTDIV1_SHIFT          = 8,
	POSTDIV1_MASK		= 7 << POSTDIV1_SHIFT,
	REFDIV_MASK		= 0x3f,
	REFDIV_SHIFT		= 0,

	/* PLL CON2 */
	LOCK_STA_SHIFT          = 31,
	LOCK_STA_MASK		= 1 << LOCK_STA_SHIFT,
	FRACDIV_MASK		= 0xffffff,
	FRACDIV_SHIFT		= 0,

	/* PLL CON3 */
	WORK_MODE_SHIFT         = 8,
	WORK_MODE_MASK		= 1 << WORK_MODE_SHIFT,
	WORK_MODE_SLOW		= 0,
	WORK_MODE_NORMAL	= 1,
	DSMPD_SHIFT             = 3,
	DSMPD_MASK		= 1 << DSMPD_SHIFT,

	/* CLKSEL0_CON */
	CORE_PLL_SEL_SHIFT	= 8,
	CORE_PLL_SEL_MASK	= 3 << CORE_PLL_SEL_SHIFT,
	CORE_PLL_SEL_APLL	= 0,
	CORE_PLL_SEL_GPLL	= 1,
	CORE_PLL_SEL_DPLL	= 2,
	CORE_CLK_DIV_SHIFT	= 0,
	CORE_CLK_DIV_MASK	= 0x1f << CORE_CLK_DIV_SHIFT,

	/* CLKSEL_CON22 */
	CLK_SARADC_DIV_CON_SHIFT= 0,
	CLK_SARADC_DIV_CON_MASK	= GENMASK(9, 0),
	CLK_SARADC_DIV_CON_WIDTH= 10,

	/* CLKSEL24_CON */
	MAC_PLL_SEL_SHIFT	= 12,
	MAC_PLL_SEL_MASK	= 1 << MAC_PLL_SEL_SHIFT,
	MAC_PLL_SEL_APLL	= 0,
	MAC_PLL_SEL_GPLL	= 1,
	RMII_EXTCLK_SEL_SHIFT   = 8,
	RMII_EXTCLK_SEL_MASK	= 1 << RMII_EXTCLK_SEL_SHIFT,
	MAC_CLK_DIV_MASK	= 0x1f,
	MAC_CLK_DIV_SHIFT	= 0,

	/* CLKSEL27_CON */
	SFC_PLL_SEL_SHIFT	= 7,
	SFC_PLL_SEL_MASK	= 1 << SFC_PLL_SEL_SHIFT,
	SFC_PLL_SEL_DPLL	= 0,
	SFC_PLL_SEL_GPLL	= 1,
	SFC_CLK_DIV_SHIFT	= 0,
	SFC_CLK_DIV_MASK	= 0x3f << SFC_CLK_DIV_SHIFT,
};
#endif

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#ifndef __ASM_ARCH_CRU_RK3328_H_
#define __ASM_ARCH_CRU_RK3328_H_

struct rk3328_clk_priv {
	struct rk3328_cru *cru;
	ulong rate;
};

struct rk3328_cru {
	u32 apll_con[5];
	u32 reserved1[3];
	u32 dpll_con[5];
	u32 reserved2[3];
	u32 cpll_con[5];
	u32 reserved3[3];
	u32 gpll_con[5];
	u32 reserved4[3];
	u32 mode_con;
	u32 misc;
	u32 reserved5[2];
	u32 glb_cnt_th;
	u32 glb_rst_st;
	u32 glb_srst_snd_value;
	u32 glb_srst_fst_value;
	u32 npll_con[5];
	u32 reserved6[(0x100 - 0xb4) / 4];
	u32 clksel_con[53];
	u32 reserved7[(0x200 - 0x1d4) / 4];
	u32 clkgate_con[29];
	u32 reserved8[3];
	u32 ssgtbl[32];
	u32 softrst_con[12];
	u32 reserved9[(0x380 - 0x330) / 4];
	u32 sdmmc_con[2];
	u32 sdio_con[2];
	u32 emmc_con[2];
	u32 sdmmc_ext_con[2];
};
check_member(rk3328_cru, sdmmc_ext_con[1], 0x39c);
#define MHz		1000000
#define KHz		1000
#define OSC_HZ		(24 * MHz)
#define APLL_HZ		(600 * MHz)
#define GPLL_HZ		(576 * MHz)
#define CPLL_HZ		(594 * MHz)

#define CLK_CORE_HZ	(600 * MHz)
#define ACLKM_CORE_HZ	(300 * MHz)
#define PCLK_DBG_HZ	(300 * MHz)

#define PERIHP_ACLK_HZ	(144000 * KHz)
#define PERIHP_HCLK_HZ	(72000 * KHz)
#define PERIHP_PCLK_HZ	(72000 * KHz)

#define PWM_CLOCK_HZ    (74 * MHz)

enum apll_frequencies {
	APLL_816_MHZ,
	APLL_600_MHZ,

	/* CRU_CLK_SEL37_CON */
	ACLK_VIO_PLL_SEL_CPLL		= 0,
	ACLK_VIO_PLL_SEL_GPLL		= 1,
	ACLK_VIO_PLL_SEL_HDMIPHY	= 2,
	ACLK_VIO_PLL_SEL_USB480M	= 3,
	ACLK_VIO_PLL_SEL_SHIFT		= 6,
	ACLK_VIO_PLL_SEL_MASK		= 3 << ACLK_VIO_PLL_SEL_SHIFT,
	ACLK_VIO_DIV_CON_SHIFT		= 0,
	ACLK_VIO_DIV_CON_MASK		= 0x1f << ACLK_VIO_DIV_CON_SHIFT,
	HCLK_VIO_DIV_CON_SHIFT		= 8,
	HCLK_VIO_DIV_CON_MASK		= 0x1f << HCLK_VIO_DIV_CON_SHIFT,

	/* CRU_CLK_SEL39_CON */
	ACLK_VOP_PLL_SEL_CPLL		= 0,
	ACLK_VOP_PLL_SEL_GPLL		= 1,
	ACLK_VOP_PLL_SEL_HDMIPHY	= 2,
	ACLK_VOP_PLL_SEL_USB480M	= 3,
	ACLK_VOP_PLL_SEL_SHIFT		= 6,
	ACLK_VOP_PLL_SEL_MASK		= 3 << ACLK_VOP_PLL_SEL_SHIFT,
	ACLK_VOP_DIV_CON_SHIFT		= 0,
	ACLK_VOP_DIV_CON_MASK		= 0x1f << ACLK_VOP_DIV_CON_SHIFT,

	/* CRU_CLK_SEL40_CON */
	DCLK_LCDC_PLL_SEL_GPLL		= 0,
	DCLK_LCDC_PLL_SEL_CPLL		= 1,
	DCLK_LCDC_PLL_SEL_SHIFT		= 0,
	DCLK_LCDC_PLL_SEL_MASK		= 1 << DCLK_LCDC_PLL_SEL_SHIFT,
	DCLK_LCDC_SEL_HDMIPHY		= 0,
	DCLK_LCDC_SEL_PLL		= 1,
	DCLK_LCDC_SEL_SHIFT		= 1,
	DCLK_LCDC_SEL_MASK		= 1 << DCLK_LCDC_SEL_SHIFT,
	DCLK_LCDC_DIV_CON_SHIFT		= 8,
	DCLK_LCDC_DIV_CON_MASK		= 0xFf << DCLK_LCDC_DIV_CON_SHIFT,
};

void rk3328_configure_cpu(struct rk3328_cru *cru,
			  enum apll_frequencies apll_freq);

#endif	/* __ASM_ARCH_CRU_RK3328_H_ */

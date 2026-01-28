/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2024 Rockchip Electronics Co. Ltd.
 * Author: Elaine Zhang <zhangqing@rock-chips.com>
 */

#ifndef _ASM_ARCH_CRU_RV1103B_H
#define _ASM_ARCH_CRU_RV1103B_H

#define MHz		1000000
#define KHz		1000
#define OSC_HZ		(24 * MHz)
#define RC_OSC_HZ	(125 * MHz)

#define GPLL_HZ		(1188 * MHz)

/* RV1103B pll id */
enum rv1103b_pll_id {
	GPLL,
	PLL_COUNT,
};

struct rv1103b_clk_info {
	unsigned long id;
	char *name;
	bool is_cru;
};

struct rv1103b_clk_priv {
	struct rv1103b_cru *cru;
	struct rv1103b_grf *grf;
	ulong gpll_hz;
	ulong armclk_hz;
	ulong armclk_enter_hz;
	ulong armclk_init_hz;
	bool sync_kernel;
	bool set_armclk_rate;
};

struct rv1103b_grf_clk_priv {
	struct rv1103b_grf *grf;
};

struct rv1103b_pll {
	unsigned int con0;
	unsigned int con1;
	unsigned int con2;
	unsigned int con3;
	unsigned int con4;
	unsigned int reserved0[3];
};

struct rv1103_clk_priv {
	struct rv1103b_cru *cru;
	ulong rate;
};

struct rv1103b_cru {
	unsigned int reserved0[192];
	unsigned int peri_clksel_con[4];
	unsigned int reserved1[316];
	unsigned int peri_clkgate_con[12];
	unsigned int reserved2[116];
	unsigned int peri_softrst_con[12];
	unsigned int reserved3[15924];
	unsigned int vepu_clksel_con[3];
	unsigned int reserved4[317];
	unsigned int vepu_clkgate_con[1];
	unsigned int reserved5[127];
	unsigned int vepu_softrst_con[1];
	unsigned int reserved6[15935];
	unsigned int npu_clksel_con[3];
	unsigned int reserved7[317];
	unsigned int npu_clkgate_con[1];
	unsigned int reserved8[127];
	unsigned int npu_softrst_con[1];
	unsigned int reserved9[15935];
	unsigned int vi_clksel_con[1];
	unsigned int reserved10[319];
	unsigned int vi_clkgate_con[3];
	unsigned int reserved11[125];
	unsigned int vi_softrst_con[3];
	unsigned int reserved12[15933];
	unsigned int core_clksel_con[3];
	unsigned int reserved13[16381];
	unsigned int ddr_clksel_con[1];
	unsigned int reserved14[16207];
	struct rv1103b_pll pll[2];
	unsigned int reserved15[128];
	unsigned int mode;
	unsigned int reserved16[31];
	unsigned int clksel_con[42];
	unsigned int reserved17[278];
	unsigned int clkgate_con[7];
	unsigned int reserved18[121];
	unsigned int softrst_con[1];
	unsigned int reserved19[127];
	unsigned int glb_cnt_th;
	unsigned int glb_rst_st;
	unsigned int glb_srst_fst;
	unsigned int glb_srst_snd;
	unsigned int glb_rst_con;
	unsigned int reserved20[15803];
	unsigned int pmu_clksel_con[3];
	unsigned int reserved21[317];
	unsigned int pmu_clkgate_con[3];
	unsigned int reserved22[125];
	unsigned int pmu_softrst_con[3];
	unsigned int reserved23[15933];
	unsigned int pmu1_clksel_con[1];
	unsigned int reserved24[319];
	unsigned int pmu1_clkgate_con[2];
	unsigned int reserved25[126];
	unsigned int pmu1_softrst_con[2];
};

check_member(rv1103b_cru, pmu1_softrst_con[1], 0x80a04);

struct pll_rate_table {
	unsigned long rate;
	unsigned int fbdiv;
	unsigned int postdiv1;
	unsigned int refdiv;
	unsigned int postdiv2;
	unsigned int dsmpd;
	unsigned int frac;
};

#define RV1103B_TOPCRU_BASE		0x60000
#define RV1103B_PERICRU_BASE		0x0
#define RV1103B_VICRU_BASE		0x30000
#define RV1103B_NPUCRU_BASE		0x20000
#define RV1103B_CORECRU_BASE		0x40000
#define RV1103B_VEPUCRU_BASE		0x10000
#define RV1103B_DDRCRU_BASE		0x50000
#define RV1103B_SUBDDRCRU_BASE		0x58000
#define RV1103B_PMUCRU_BASE		0x70000
#define RV1103B_PMU1CRU_BASE		0x80000

#define RV1103B_CRU_BASE		0x20000000

#define RV1103B_PLL_CON(x)		((x) * 0x4 + RV1103B_TOPCRU_BASE)
#define RV1103B_MODE_CON		(0x280 + RV1103B_TOPCRU_BASE)
#define RV1103B_CLKSEL_CON(x)		((x) * 0x4 + 0x300 + RV1103B_TOPCRU_BASE)
#define RV1103B_SUBDDRMODE_CON		(0x280 + RV1103B_SUBDDRCRU_BASE)

enum {
	/* CORECRU_CLK_SEL0_CON */
	CLK_CORE_SRC_SEL_SHIFT		= 1,
	CLK_CORE_SRC_SEL_MASK		= 0x1 << CLK_CORE_SRC_SEL_SHIFT,
	CLK_CORE_SRC_SEL_GPLL		= 0,
	CLK_CORE_SRC_SEL_PVTPLL,

	/* CRU_PERI_CLK_SEL0_CON */
	CLK_TSADC_TSEN_DIV_SHIFT	= 10,
	CLK_TSADC_TSEN_DIV_MASK		= 0x1f << CLK_TSADC_TSEN_DIV_SHIFT,
	CLK_TSADC_DIV_SHIFT		= 4,
	CLK_TSADC_DIV_MASK		= 0x1f << CLK_TSADC_DIV_SHIFT,
	PCLK_PERI_DIV_SHIFT		= 0,
	PCLK_PERI_DIV_MASK		= 0x3 << PCLK_PERI_DIV_SHIFT,

	/* CRU_PERI_CLK_SEL1_CON */
	CLK_SARADC_DIV_SHIFT		= 0,
	CLK_SARADC_DIV_MASK		= 0x7 << CLK_SARADC_DIV_SHIFT,

	/* CRU_CLK_SEL5_CON */
	CLK_UART2_SRC_DIV_SHIFT		= 10,
	CLK_UART2_SRC_DIV_MASK		= 0x1f << CLK_UART2_SRC_DIV_SHIFT,
	CLK_UART1_SRC_DIV_SHIFT		= 5,
	CLK_UART1_SRC_DIV_MASK		= 0x1f << CLK_UART1_SRC_DIV_SHIFT,
	CLK_UART0_SRC_DIV_SHIFT		= 0,
	CLK_UART0_SRC_DIV_MASK		= 0x1f << CLK_UART0_SRC_DIV_SHIFT,

	/* CRU_CLK_SEL10_CON */
	CLK_UART_FRAC_NUMERATOR_SHIFT	= 16,
	CLK_UART_FRAC_NUMERATOR_MASK	= 0xffff << 16,
	CLK_UART_FRAC_DENOMINATOR_SHIFT	= 0,
	CLK_UART_FRAC_DENOMINATOR_MASK	= 0xffff,

	/* CRU_CLK_SEL31_CON */
	CLK_EMMC_SEL_SHIFT		= 15,
	CLK_EMMC_SEL_MASK		= 0x1 << CLK_EMMC_SEL_SHIFT,
	ACLK_PERI_SEL_SHIFT		= 10,
	ACLK_PERI_SEL_MASK		= 0x3 << ACLK_PERI_SEL_SHIFT,
	ACLK_PERI_SEL_600M		= 0,
	ACLK_PERI_SEL_480M,
	ACLK_PERI_SEL_400M,
	LSCLK_PERI_SEL_SHIFT		= 9,
	LSCLK_PERI_SEL_MASK		= 0x1 << LSCLK_PERI_SEL_SHIFT,
	LSCLK_PERI_SEL_300M		= 0,
	LSCLK_PERI_SEL_200M,
	CLK_EMMC_DIV_SHIFT		= 0,
	CLK_EMMC_DIV_MASK		= 0xff << CLK_EMMC_DIV_SHIFT,

	/* CRU_CLK_SEL32_CON */
	CLK_SDMMC_SEL_SHIFT		= 15,
	CLK_SDMMC_SEL_MASK		= 0x1 << CLK_SDMMC_SEL_SHIFT,
	CLK_MMC_SEL_GPLL		= 0,
	CLK_MMC_SEL_OSC,
	CLK_UART2_SEL_SHIFT		= 12,
	CLK_UART2_SEL_MASK		= 3 << CLK_UART2_SEL_SHIFT,
	CLK_UART1_SEL_SHIFT		= 10,
	CLK_UART1_SEL_MASK		= 3 << CLK_UART1_SEL_SHIFT,
	CLK_UART0_SEL_SHIFT		= 8,
	CLK_UART0_SEL_MASK		= 3 << CLK_UART0_SEL_SHIFT,
	CLK_UART_SEL_SRC		= 0,
	CLK_UART_SEL_FRAC,
	CLK_UART_SEL_OSC,
	CLK_SDMMC_DIV_SHIFT		= 0,
	CLK_SDMMC_DIV_MASK		= 0xff << CLK_SDMMC_DIV_SHIFT,

	/* CRU_CLK_SEL33_CON */
	CLK_SFC_SEL_SHIFT		= 15,
	CLK_SFC_SEL_MASK		= 0x1 << CLK_SFC_SEL_SHIFT,
	CLK_SFC_DIV_SHIFT		= 0,
	CLK_SFC_DIV_MASK		= 0xff << CLK_SFC_DIV_SHIFT,

	/* CRU_CLK_SEL34_CON */
	CLK_PWM2_SEL_SHIFT		= 14,
	CLK_PWM2_SEL_MASK		= 1 << CLK_PWM2_SEL_SHIFT,
	CLK_PWM1_SEL_SHIFT		= 13,
	CLK_PWM1_SEL_MASK		= 1 << CLK_PWM1_SEL_SHIFT,
	CLK_PWM0_SEL_SHIFT		= 12,
	CLK_PWM0_SEL_MASK		= 1 << CLK_PWM0_SEL_SHIFT,
	CLK_PWM_SEL_100M		= 0,
	CLK_PWM_SEL_24M,
	CLK_SPI0_SEL_SHIFT		= 2,
	CLK_SPI0_SEL_MASK		= 3 << CLK_SPI0_SEL_SHIFT,
	CLK_SPI0_SEL_200M		= 0,
	CLK_SPI0_SEL_100M,
	CLK_SPI0_SEL_50M,
	CLK_SPI0_SEL_24M,
	CLK_I2C1_SEL_SHIFT		= 1,
	CLK_I2C1_SEL_MASK		= 0x1 << CLK_I2C1_SEL_SHIFT,
	CLK_I2C0_SEL_SHIFT		= 0,
	CLK_I2C0_SEL_MASK		= 0x1 << CLK_I2C0_SEL_SHIFT,
	CLK_I2C_SEL_100M		= 0,
	CLK_I2C_SEL_24M,

	/* CRU_CLK_SEL35_CON */
	CLK_PKA_CRYPTO_SEL_SHIFT	= 4,
	CLK_PKA_CRYPTO_SEL_MASK		= 0x3 << CLK_PKA_CRYPTO_SEL_SHIFT,
	CLK_CORE_CRYPTO_SEL_SHIFT	= 2,
	CLK_CORE_CRYPTO_SEL_MASK	= 0x3 << CLK_CORE_CRYPTO_SEL_SHIFT,
	CLK_CORE_CRYPTO_SEL_300M	= 0,
	CLK_CORE_CRYPTO_SEL_200M,
	CLK_CORE_CRYPTO_SEL_100M,
	DCLK_DECOM_SEL_SHIFT		= 0,
	DCLK_DECOM_SEL_MASK		= 0x3 << DCLK_DECOM_SEL_SHIFT,
	DCLK_DECOM_SEL_480M		= 0,
	DCLK_DECOM_SEL_400M,
	DCLK_DECOM_SEL_300M,

	/* CRU_CLK_SEL37_CON */
	CLK_CORE_GPLL_DIV_SHIFT		= 13,
	CLK_CORE_GPLL_DIV_MASK		= 0x7 << CLK_CORE_GPLL_DIV_SHIFT,
	CLK_CORE_GPLL_SEL_SHIFT		= 12,
	CLK_CORE_GPLL_SEL_MASK		= 0x1 << CLK_CORE_GPLL_SEL_SHIFT,
	CLK_CORE_GPLL_SEL_GPLL		= 0,
	CLK_CORE_GPLL_SEL_OSC,

	/* CRU_PMU_CLK_SEL2_CON */
	LSCLK_PMU_SEL_SHIFT		= 4,
	LSCLK_PMU_SEL_MASK		= 0x1 << LSCLK_PMU_SEL_SHIFT,
	LSCLK_PMU_SEL_24M		= 0,
	LSCLK_PMU_SEL_RC_OSC,
	LSCLK_PMU_DIV_SHIFT		= 0,
	LSCLK_PMU_DIV_MASK		= 0x3 << LSCLK_PMU_DIV_SHIFT,

};
#endif

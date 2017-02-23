/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3328.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rk3328-cru.h>

DECLARE_GLOBAL_DATA_PTR;

struct pll_div {
	u32 refdiv;
	u32 fbdiv;
	u32 postdiv1;
	u32 postdiv2;
	u32 frac;
};

#define RATE_TO_DIV(input_rate, output_rate) \
	((input_rate) / (output_rate) - 1);
#define DIV_TO_RATE(input_rate, div)    ((input_rate) / ((div) + 1))

#define PLL_DIVISORS(hz, _refdiv, _postdiv1, _postdiv2) {\
	.refdiv = _refdiv,\
	.fbdiv = (u32)((u64)hz * _refdiv * _postdiv1 * _postdiv2 / OSC_HZ),\
	.postdiv1 = _postdiv1, .postdiv2 = _postdiv2};

static const struct pll_div gpll_init_cfg = PLL_DIVISORS(GPLL_HZ, 1, 4, 1);
static const struct pll_div cpll_init_cfg = PLL_DIVISORS(CPLL_HZ, 2, 2, 1);

static const struct pll_div apll_816_cfg = PLL_DIVISORS(816 * MHz, 1, 2, 1);
static const struct pll_div apll_600_cfg = PLL_DIVISORS(600 * MHz, 1, 3, 1);

static const struct pll_div *apll_cfgs[] = {
	[APLL_816_MHZ] = &apll_816_cfg,
	[APLL_600_MHZ] = &apll_600_cfg,
};

enum {
	/* PLL_CON0 */
	PLL_POSTDIV1_SHIFT		= 12,
	PLL_POSTDIV1_MASK		= 0x7 << PLL_POSTDIV1_SHIFT,
	PLL_FBDIV_SHIFT			= 0,
	PLL_FBDIV_MASK			= 0xfff,

	/* PLL_CON1 */
	PLL_DSMPD_SHIFT			= 12,
	PLL_DSMPD_MASK			= 1 << PLL_DSMPD_SHIFT,
	PLL_INTEGER_MODE		= 1,
	PLL_LOCK_STATUS_SHIFT		= 10,
	PLL_LOCK_STATUS_MASK		= 1 << PLL_LOCK_STATUS_SHIFT,
	PLL_POSTDIV2_SHIFT		= 6,
	PLL_POSTDIV2_MASK		= 0x7 << PLL_POSTDIV2_SHIFT,
	PLL_REFDIV_SHIFT		= 0,
	PLL_REFDIV_MASK			= 0x3f,

	/* PLL_CON2 */
	PLL_FRACDIV_SHIFT		= 0,
	PLL_FRACDIV_MASK		= 0xffffff,

	/* MODE_CON */
	APLL_MODE_SHIFT			= 0,
	NPLL_MODE_SHIFT			= 1,
	DPLL_MODE_SHIFT			= 4,
	CPLL_MODE_SHIFT			= 8,
	GPLL_MODE_SHIFT			= 12,
	PLL_MODE_SLOW			= 0,
	PLL_MODE_NORM,

	/* CLKSEL_CON0 */
	CLK_CORE_PLL_SEL_APLL		= 0,
	CLK_CORE_PLL_SEL_GPLL,
	CLK_CORE_PLL_SEL_DPLL,
	CLK_CORE_PLL_SEL_NPLL,
	CLK_CORE_PLL_SEL_SHIFT		= 6,
	CLK_CORE_PLL_SEL_MASK		= 3 << CLK_CORE_PLL_SEL_SHIFT,
	CLK_CORE_DIV_SHIFT		= 0,
	CLK_CORE_DIV_MASK		= 0x1f,

	/* CLKSEL_CON1 */
	ACLKM_CORE_DIV_SHIFT		= 4,
	ACLKM_CORE_DIV_MASK		= 0x7 << ACLKM_CORE_DIV_SHIFT,
	PCLK_DBG_DIV_SHIFT		= 0,
	PCLK_DBG_DIV_MASK		= 0xF << PCLK_DBG_DIV_SHIFT,

	/* CLKSEL_CON28 */
	ACLK_PERIHP_PLL_SEL_CPLL	= 0,
	ACLK_PERIHP_PLL_SEL_GPLL,
	ACLK_PERIHP_PLL_SEL_HDMIPHY,
	ACLK_PERIHP_PLL_SEL_SHIFT	= 6,
	ACLK_PERIHP_PLL_SEL_MASK	= 3 << ACLK_PERIHP_PLL_SEL_SHIFT,
	ACLK_PERIHP_DIV_CON_SHIFT	= 0,
	ACLK_PERIHP_DIV_CON_MASK	= 0x1f,

	/* CLKSEL_CON29 */
	PCLK_PERIHP_DIV_CON_SHIFT	= 4,
	PCLK_PERIHP_DIV_CON_MASK	= 0x7 << PCLK_PERIHP_DIV_CON_SHIFT,
	HCLK_PERIHP_DIV_CON_SHIFT	= 0,
	HCLK_PERIHP_DIV_CON_MASK	= 3 << HCLK_PERIHP_DIV_CON_SHIFT,

	/* CLKSEL_CON22 */
	CLK_TSADC_DIV_CON_SHIFT		= 0,
	CLK_TSADC_DIV_CON_MASK		= 0x3ff,

	/* CLKSEL_CON23 */
	CLK_SARADC_DIV_CON_SHIFT	= 0,
	CLK_SARADC_DIV_CON_MASK		= 0x3ff << CLK_SARADC_DIV_CON_SHIFT,

	/* CLKSEL_CON24 */
	CLK_PWM_PLL_SEL_CPLL		= 0,
	CLK_PWM_PLL_SEL_GPLL,
	CLK_PWM_PLL_SEL_SHIFT		= 15,
	CLK_PWM_PLL_SEL_MASK		= 1 << CLK_PWM_PLL_SEL_SHIFT,
	CLK_PWM_DIV_CON_SHIFT		= 8,
	CLK_PWM_DIV_CON_MASK		= 0x7f << CLK_PWM_DIV_CON_SHIFT,

	CLK_SPI_PLL_SEL_CPLL		= 0,
	CLK_SPI_PLL_SEL_GPLL,
	CLK_SPI_PLL_SEL_SHIFT		= 7,
	CLK_SPI_PLL_SEL_MASK		= 1 << CLK_SPI_PLL_SEL_SHIFT,
	CLK_SPI_DIV_CON_SHIFT		= 0,
	CLK_SPI_DIV_CON_MASK		= 0x7f << CLK_SPI_DIV_CON_SHIFT,

	/* CLKSEL_CON30 */
	CLK_SDMMC_PLL_SEL_CPLL		= 0,
	CLK_SDMMC_PLL_SEL_GPLL,
	CLK_SDMMC_PLL_SEL_24M,
	CLK_SDMMC_PLL_SEL_USBPHY,
	CLK_SDMMC_PLL_SHIFT		= 8,
	CLK_SDMMC_PLL_MASK		= 0x3 << CLK_SDMMC_PLL_SHIFT,
	CLK_SDMMC_DIV_CON_SHIFT          = 0,
	CLK_SDMMC_DIV_CON_MASK           = 0xff << CLK_SDMMC_DIV_CON_SHIFT,

	/* CLKSEL_CON32 */
	CLK_EMMC_PLL_SEL_CPLL		= 0,
	CLK_EMMC_PLL_SEL_GPLL,
	CLK_EMMC_PLL_SEL_24M,
	CLK_EMMC_PLL_SEL_USBPHY,
	CLK_EMMC_PLL_SHIFT		= 8,
	CLK_EMMC_PLL_MASK		= 0x3 << CLK_EMMC_PLL_SHIFT,
	CLK_EMMC_DIV_CON_SHIFT          = 0,
	CLK_EMMC_DIV_CON_MASK           = 0xff << CLK_EMMC_DIV_CON_SHIFT,

	/* CLKSEL_CON34 */
	CLK_I2C_PLL_SEL_CPLL		= 0,
	CLK_I2C_PLL_SEL_GPLL,
	CLK_I2C_DIV_CON_MASK		= 0x7f,
	CLK_I2C_PLL_SEL_MASK		= 1,
	CLK_I2C1_PLL_SEL_SHIFT		= 15,
	CLK_I2C1_DIV_CON_SHIFT		= 8,
	CLK_I2C0_PLL_SEL_SHIFT		= 7,
	CLK_I2C0_DIV_CON_SHIFT		= 0,

	/* CLKSEL_CON35 */
	CLK_I2C3_PLL_SEL_SHIFT		= 15,
	CLK_I2C3_DIV_CON_SHIFT		= 8,
	CLK_I2C2_PLL_SEL_SHIFT		= 7,
	CLK_I2C2_DIV_CON_SHIFT		= 0,
};

#define VCO_MAX_KHZ	(3200 * (MHz / KHz))
#define VCO_MIN_KHZ	(800 * (MHz / KHz))
#define OUTPUT_MAX_KHZ	(3200 * (MHz / KHz))
#define OUTPUT_MIN_KHZ	(16 * (MHz / KHz))

/*
 *  the div restructions of pll in integer mode, these are defined in
 *  * CRU_*PLL_CON0 or PMUCRU_*PLL_CON0
 */
#define PLL_DIV_MIN	16
#define PLL_DIV_MAX	3200

/*
 * How to calculate the PLL(from TRM V0.3 Part 1 Page 63):
 * Formulas also embedded within the Fractional PLL Verilog model:
 * If DSMPD = 1 (DSM is disabled, "integer mode")
 * FOUTVCO = FREF / REFDIV * FBDIV
 * FOUTPOSTDIV = FOUTVCO / POSTDIV1 / POSTDIV2
 * Where:
 * FOUTVCO = Fractional PLL non-divided output frequency
 * FOUTPOSTDIV = Fractional PLL divided output frequency
 *               (output of second post divider)
 * FREF = Fractional PLL input reference frequency, (the OSC_HZ 24MHz input)
 * REFDIV = Fractional PLL input reference clock divider
 * FBDIV = Integer value programmed into feedback divide
 *
 */
static void rkclk_set_pll(struct rk3328_cru *cru, enum rk_clk_id clk_id,
			const struct pll_div *div)
{
	u32 *pll_con;
	u32 mode_shift, mode_mask;

	pll_con = NULL;
	mode_shift = 0;
	switch (clk_id) {
	case CLK_ARM:
		pll_con = cru->apll_con;
		mode_shift = APLL_MODE_SHIFT;
		break;
	case CLK_DDR:
		pll_con = cru->dpll_con;
		mode_shift = DPLL_MODE_SHIFT;
		break;
	case CLK_CODEC:
		pll_con = cru->cpll_con;
		mode_shift = CPLL_MODE_SHIFT;
		break;
	case CLK_GENERAL:
		pll_con = cru->gpll_con;
		mode_shift = GPLL_MODE_SHIFT;
		break;
	case CLK_NEW:
		pll_con = cru->npll_con;
		mode_shift = NPLL_MODE_SHIFT;
		break;
	default:
		break;
	}
	mode_mask = 1 << mode_shift;

	/* All 8 PLLs have same VCO and output frequency range restrictions. */
	u32 vco_khz = OSC_HZ / 1000 * div->fbdiv / div->refdiv;
	u32 output_khz = vco_khz / div->postdiv1 / div->postdiv2;

	debug("PLL at %p: fbdiv=%d, refdiv=%d, postdiv1=%d, \
	      postdiv2=%d, vco=%u khz, output=%u khz\n",
	      pll_con, div->fbdiv, div->refdiv, div->postdiv1,
	      div->postdiv2, vco_khz, output_khz);
	assert(vco_khz >= VCO_MIN_KHZ && vco_khz <= VCO_MAX_KHZ &&
	       output_khz >= OUTPUT_MIN_KHZ && output_khz <= OUTPUT_MAX_KHZ &&
	       div->fbdiv >= PLL_DIV_MIN && div->fbdiv <= PLL_DIV_MAX);

	/*
	 * When power on or changing PLL setting,
	 * we must force PLL into slow mode to ensure output stable clock.
	 */
	rk_clrsetreg(&cru->mode_con, mode_mask, PLL_MODE_SLOW << mode_shift);

	/* use integer mode */
	rk_clrsetreg(&pll_con[1], PLL_DSMPD_MASK,
		     PLL_INTEGER_MODE << PLL_DSMPD_SHIFT);

	rk_clrsetreg(&pll_con[0],
		     PLL_FBDIV_MASK | PLL_POSTDIV1_MASK,
		     (div->fbdiv << PLL_FBDIV_SHIFT) |
		     (div->postdiv1 << PLL_POSTDIV1_SHIFT));
	rk_clrsetreg(&pll_con[1],
		     PLL_POSTDIV2_MASK | PLL_REFDIV_MASK,
		     (div->postdiv2 << PLL_POSTDIV2_SHIFT) |
		     (div->refdiv << PLL_REFDIV_SHIFT));

	/* waiting for pll lock */
	while (!(readl(&pll_con[1]) & (1 << PLL_LOCK_STATUS_SHIFT)))
		udelay(1);

	/* pll enter normal mode */
	rk_clrsetreg(&cru->mode_con, mode_mask, PLL_MODE_NORM << mode_shift);
}

static void rkclk_init(struct rk3328_cru *cru)
{
	u32 aclk_div;
	u32 hclk_div;
	u32 pclk_div;

	/* configure gpll cpll */
	rkclk_set_pll(cru, CLK_GENERAL, &gpll_init_cfg);
	rkclk_set_pll(cru, CLK_CODEC, &cpll_init_cfg);

	/* configure perihp aclk, hclk, pclk */
	aclk_div = GPLL_HZ / PERIHP_ACLK_HZ - 1;
	hclk_div = PERIHP_ACLK_HZ / PERIHP_HCLK_HZ - 1;
	pclk_div = PERIHP_ACLK_HZ / PERIHP_PCLK_HZ - 1;

	rk_clrsetreg(&cru->clksel_con[28],
		     ACLK_PERIHP_PLL_SEL_MASK | ACLK_PERIHP_DIV_CON_MASK,
		     ACLK_PERIHP_PLL_SEL_GPLL << ACLK_PERIHP_PLL_SEL_SHIFT |
		     aclk_div << ACLK_PERIHP_DIV_CON_SHIFT);
	rk_clrsetreg(&cru->clksel_con[29],
		     PCLK_PERIHP_DIV_CON_MASK | HCLK_PERIHP_DIV_CON_MASK,
		     pclk_div << PCLK_PERIHP_DIV_CON_SHIFT |
		     hclk_div << HCLK_PERIHP_DIV_CON_SHIFT);
}

void rk3328_configure_cpu(struct rk3328_cru *cru,
			  enum apll_frequencies apll_freq)
{
	u32 clk_core_div;
	u32 aclkm_div;
	u32 pclk_dbg_div;

	rkclk_set_pll(cru, CLK_ARM, apll_cfgs[apll_freq]);

	clk_core_div = APLL_HZ / CLK_CORE_HZ - 1;
	aclkm_div = APLL_HZ / ACLKM_CORE_HZ / (clk_core_div + 1) - 1;
	pclk_dbg_div = APLL_HZ / PCLK_DBG_HZ / (clk_core_div + 1) - 1;

	rk_clrsetreg(&cru->clksel_con[0],
		     CLK_CORE_PLL_SEL_MASK | CLK_CORE_DIV_MASK,
		     CLK_CORE_PLL_SEL_APLL << CLK_CORE_PLL_SEL_SHIFT |
		     clk_core_div << CLK_CORE_DIV_SHIFT);

	rk_clrsetreg(&cru->clksel_con[1],
		     PCLK_DBG_DIV_MASK | ACLKM_CORE_DIV_MASK,
		     pclk_dbg_div << PCLK_DBG_DIV_SHIFT |
		     aclkm_div << ACLKM_CORE_DIV_SHIFT);
}


static ulong rk3328_i2c_get_clk(struct rk3328_cru *cru, ulong clk_id)
{
	u32 div, con;

	switch (clk_id) {
	case SCLK_I2C0:
		con = readl(&cru->clksel_con[34]);
		div = con >> CLK_I2C0_DIV_CON_SHIFT & CLK_I2C_DIV_CON_MASK;
		break;
	case SCLK_I2C1:
		con = readl(&cru->clksel_con[34]);
		div = con >> CLK_I2C1_DIV_CON_SHIFT & CLK_I2C_DIV_CON_MASK;
		break;
	case SCLK_I2C2:
		con = readl(&cru->clksel_con[35]);
		div = con >> CLK_I2C2_DIV_CON_SHIFT & CLK_I2C_DIV_CON_MASK;
		break;
	case SCLK_I2C3:
		con = readl(&cru->clksel_con[35]);
		div = con >> CLK_I2C3_DIV_CON_SHIFT & CLK_I2C_DIV_CON_MASK;
		break;
	default:
		printf("do not support this i2c bus\n");
		return -EINVAL;
	}

	return DIV_TO_RATE(GPLL_HZ, div);
}

static ulong rk3328_i2c_set_clk(struct rk3328_cru *cru, ulong clk_id, uint hz)
{
	int src_clk_div;

	src_clk_div = GPLL_HZ / hz;
	assert(src_clk_div - 1 < 127);

	switch (clk_id) {
	case SCLK_I2C0:
		rk_clrsetreg(&cru->clksel_con[34],
			     CLK_I2C_DIV_CON_MASK << CLK_I2C0_DIV_CON_SHIFT |
			     CLK_I2C_PLL_SEL_MASK << CLK_I2C0_PLL_SEL_SHIFT,
			     (src_clk_div - 1) << CLK_I2C0_DIV_CON_SHIFT |
			     CLK_I2C_PLL_SEL_GPLL << CLK_I2C0_PLL_SEL_SHIFT);
		break;
	case SCLK_I2C1:
		rk_clrsetreg(&cru->clksel_con[34],
			     CLK_I2C_DIV_CON_MASK << CLK_I2C1_DIV_CON_SHIFT |
			     CLK_I2C_PLL_SEL_MASK << CLK_I2C1_PLL_SEL_SHIFT,
			     (src_clk_div - 1) << CLK_I2C1_DIV_CON_SHIFT |
			     CLK_I2C_PLL_SEL_GPLL << CLK_I2C1_PLL_SEL_SHIFT);
		break;
	case SCLK_I2C2:
		rk_clrsetreg(&cru->clksel_con[35],
			     CLK_I2C_DIV_CON_MASK << CLK_I2C2_DIV_CON_SHIFT |
			     CLK_I2C_PLL_SEL_MASK << CLK_I2C2_PLL_SEL_SHIFT,
			     (src_clk_div - 1) << CLK_I2C2_DIV_CON_SHIFT |
			     CLK_I2C_PLL_SEL_GPLL << CLK_I2C2_PLL_SEL_SHIFT);
		break;
	case SCLK_I2C3:
		rk_clrsetreg(&cru->clksel_con[35],
			     CLK_I2C_DIV_CON_MASK << CLK_I2C3_DIV_CON_SHIFT |
			     CLK_I2C_PLL_SEL_MASK << CLK_I2C3_PLL_SEL_SHIFT,
			     (src_clk_div - 1) << CLK_I2C3_DIV_CON_SHIFT |
			     CLK_I2C_PLL_SEL_GPLL << CLK_I2C3_PLL_SEL_SHIFT);
		break;
	default:
		printf("do not support this i2c bus\n");
		return -EINVAL;
	}

	return DIV_TO_RATE(GPLL_HZ, src_clk_div);
}

static ulong rk3328_mmc_get_clk(struct rk3328_cru *cru, uint clk_id)
{
	u32 div, con, con_id;

	switch (clk_id) {
	case HCLK_SDMMC:
		con_id = 30;
		break;
	case HCLK_EMMC:
		con_id = 32;
		break;
	default:
		return -EINVAL;
	}
	con = readl(&cru->clksel_con[con_id]);
	div = (con & CLK_EMMC_DIV_CON_MASK) >> CLK_EMMC_DIV_CON_SHIFT;

	if ((con & CLK_EMMC_PLL_MASK) >> CLK_EMMC_PLL_SHIFT
	    == CLK_EMMC_PLL_SEL_24M)
		return DIV_TO_RATE(OSC_HZ, div);
	else
		return DIV_TO_RATE(GPLL_HZ, div);
}

static ulong rk3328_mmc_set_clk(struct rk3328_cru *cru,
				ulong clk_id, ulong set_rate)
{
	int src_clk_div;
	u32 con_id;

	switch (clk_id) {
	case HCLK_SDMMC:
		con_id = 30;
		break;
	case HCLK_EMMC:
		con_id = 32;
		break;
	default:
		return -EINVAL;
	}
	/* Select clk_sdmmc/emmc source from GPLL by default */
	src_clk_div = GPLL_HZ / set_rate;

	if (src_clk_div > 127) {
		/* use 24MHz source for 400KHz clock */
		src_clk_div = OSC_HZ / set_rate;
		rk_clrsetreg(&cru->clksel_con[con_id],
			     CLK_EMMC_PLL_MASK | CLK_EMMC_DIV_CON_MASK,
			     CLK_EMMC_PLL_SEL_24M << CLK_EMMC_PLL_SHIFT |
			     (src_clk_div - 1) << CLK_EMMC_DIV_CON_SHIFT);
	} else {
		rk_clrsetreg(&cru->clksel_con[con_id],
			     CLK_EMMC_PLL_MASK | CLK_EMMC_DIV_CON_MASK,
			     CLK_EMMC_PLL_SEL_GPLL << CLK_EMMC_PLL_SHIFT |
			     (src_clk_div - 1) << CLK_EMMC_DIV_CON_SHIFT);
	}

	return rk3328_mmc_get_clk(cru, clk_id);
}

static ulong rk3328_pwm_get_clk(struct rk3328_cru *cru)
{
	u32 div, con;

	con = readl(&cru->clksel_con[24]);
	div = (con & CLK_PWM_DIV_CON_MASK) >> CLK_PWM_DIV_CON_SHIFT;

	return DIV_TO_RATE(GPLL_HZ, div);
}

static ulong rk3328_pwm_set_clk(struct rk3328_cru *cru, uint hz)
{
	u32 div = GPLL_HZ / hz;

	rk_clrsetreg(&cru->clksel_con[24],
		     CLK_PWM_PLL_SEL_MASK | CLK_PWM_DIV_CON_MASK,
		     CLK_PWM_PLL_SEL_GPLL << CLK_PWM_PLL_SEL_SHIFT |
		     (div - 1) << CLK_PWM_DIV_CON_SHIFT);

	return DIV_TO_RATE(GPLL_HZ, div);
}

static ulong rk3328_clk_get_rate(struct clk *clk)
{
	struct rk3328_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	switch (clk->id) {
	case 0 ... 29:
		return 0;
	case HCLK_SDMMC:
	case HCLK_EMMC:
		rate = rk3328_mmc_get_clk(priv->cru, clk->id);
		break;
	case SCLK_I2C0:
	case SCLK_I2C1:
	case SCLK_I2C2:
	case SCLK_I2C3:
		rate = rk3328_i2c_get_clk(priv->cru, clk->id);
		break;
	case SCLK_PWM:
		rate = rk3328_pwm_get_clk(priv->cru);
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong rk3328_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3328_clk_priv *priv = dev_get_priv(clk->dev);
	ulong ret = 0;

	switch (clk->id) {
	case 0 ... 29:
		return 0;
	case HCLK_SDMMC:
	case HCLK_EMMC:
		ret = rk3328_mmc_set_clk(priv->cru, clk->id, rate);
		break;
	case SCLK_I2C0:
	case SCLK_I2C1:
	case SCLK_I2C2:
	case SCLK_I2C3:
		ret = rk3328_i2c_set_clk(priv->cru, clk->id, rate);
		break;
	case SCLK_PWM:
		ret = rk3328_pwm_set_clk(priv->cru, rate);
		break;
	default:
		return -ENOENT;
	}

	return ret;
}

static struct clk_ops rk3328_clk_ops = {
	.get_rate = rk3328_clk_get_rate,
	.set_rate = rk3328_clk_set_rate,
};

static int rk3328_clk_probe(struct udevice *dev)
{
	struct rk3328_clk_priv *priv = dev_get_priv(dev);

	rkclk_init(priv->cru);

	return 0;
}

static int rk3328_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct rk3328_clk_priv *priv = dev_get_priv(dev);

	priv->cru = (struct rk3328_cru *)dev_get_addr(dev);

	return 0;
}

static int rk3328_clk_bind(struct udevice *dev)
{
	int ret;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(gd->dm_root, "rk3328_sysreset", "reset", &dev);
	if (ret)
		printf("Warning: No RK3328 reset driver: ret=%d\n", ret);

	return ret;
}

static const struct udevice_id rk3328_clk_ids[] = {
	{ .compatible = "rockchip,rk3328-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3328_cru) = {
	.name		= "rockchip_rk3328_cru",
	.id		= UCLASS_CLK,
	.of_match	= rk3328_clk_ids,
	.priv_auto_alloc_size = sizeof(struct rk3328_clk_priv),
	.ofdata_to_platdata = rk3328_clk_ofdata_to_platdata,
	.ops		= &rk3328_clk_ops,
	.bind		= rk3328_clk_bind,
	.probe		= rk3328_clk_probe,
};

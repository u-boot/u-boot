// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd
 * Author: Elaine Zhang <zhangqing@rock-chips.com>
 */

#include <bitfield.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rv1103-cru.h>

#include <clk.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rv1103.h>
#include <asm/arch-rockchip/hardware.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/delay.h>
#include <linux/stringify.h>

DECLARE_GLOBAL_DATA_PTR;

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

#ifndef BITS_WITH_WMASK
#define BITS_WITH_WMASK(bits, msk, shift) \
	((bits) << (shift)) | ((msk) << ((shift) + 16))
#endif

static struct rockchip_pll_rate_table rv1103b_pll_rates[] = {
	/* _mhz, _refdiv, _fbdiv, _postdiv1, _postdiv2, _dsmpd, _frac */
	RK3036_PLL_RATE(1188000000, 1, 99, 2, 1, 1, 0),
	RK3036_PLL_RATE(594000000, 1, 99, 4, 1, 1, 0),
	{ /* sentinel */ },
};

static struct rockchip_pll_clock rv1103b_pll_clks[] = {
	[GPLL] = PLL(pll_rk3328, PLL_GPLL, RV1103B_PLL_CON(24),
		     RV1103B_MODE_CON, 0, 10, 0, rv1103b_pll_rates),
};

static ulong rv1103b_peri_get_clk(struct rv1103b_clk_priv *priv, ulong clk_id)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 con, sel, div, rate, prate;

	switch (clk_id) {
	case ACLK_PERI_SRC:
		con = readl(&cru->clksel_con[31]);
		sel = (con & ACLK_PERI_SEL_MASK) >> ACLK_PERI_SEL_SHIFT;
		if (sel == ACLK_PERI_SEL_600M)
			rate = 600 * MHz;
		else if (sel == ACLK_PERI_SEL_480M)
			rate = 480 * MHz;
		else
			rate = 400 * MHz;
		break;
	case LSCLK_PERI_SRC:
		con = readl(&cru->clksel_con[31]);
		sel = (con & LSCLK_PERI_SEL_MASK) >> LSCLK_PERI_SEL_SHIFT;
		if (sel == LSCLK_PERI_SEL_300M)
			rate = 300 * MHz;
		else
			rate = 200 * MHz;
		break;
	case PCLK_PERI_ROOT:
		con = readl(&cru->peri_clksel_con[0]);
		div = (con & PCLK_PERI_DIV_MASK) >> PCLK_PERI_DIV_SHIFT;
		rate = DIV_TO_RATE(rv1103b_peri_get_clk(priv, LSCLK_PERI_SRC),
				   div);
		break;
	case PCLK_TOP_ROOT:
		rate = DIV_TO_RATE(priv->gpll_hz, 11);
		break;
	case LSCLK_PMU_ROOT:
	case PCLK_PMU:
		con = readl(&cru->pmu_clksel_con[2]);
		sel = (con & LSCLK_PMU_SEL_MASK) >> LSCLK_PMU_SEL_SHIFT;
		div = (con & LSCLK_PMU_DIV_MASK) >> LSCLK_PMU_DIV_SHIFT;
		if (sel == LSCLK_PMU_SEL_24M)
			prate = OSC_HZ;
		else
			prate = RC_OSC_HZ;
		rate = DIV_TO_RATE(prate, div);
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong rv1103b_peri_set_clk(struct rv1103b_clk_priv *priv,
				  ulong clk_id, ulong rate)
{
	struct rv1103b_cru *cru = priv->cru;
	int src_clk, div;

	switch (clk_id) {
	case ACLK_PERI_SRC:
		if (rate >= 594 * MHz)
			src_clk = ACLK_PERI_SEL_600M;
		else if (rate >= 480 * MHz)
			src_clk = ACLK_PERI_SEL_480M;
		else
			src_clk = ACLK_PERI_SEL_400M;
		rk_clrsetreg(&cru->clksel_con[31],
			     ACLK_PERI_SEL_MASK,
			     src_clk << ACLK_PERI_SEL_SHIFT);
		break;
	case LSCLK_PERI_SRC:
		if (rate >= 297 * MHz)
			src_clk = LSCLK_PERI_SEL_300M;
		else
			src_clk = LSCLK_PERI_SEL_200M;
		rk_clrsetreg(&cru->clksel_con[31],
			     LSCLK_PERI_SEL_MASK,
			     src_clk << LSCLK_PERI_SEL_SHIFT);
		break;
	case PCLK_PERI_ROOT:
		div = DIV_ROUND_UP(rv1103b_peri_get_clk(priv, LSCLK_PERI_SRC),
				   rate);
		rk_clrsetreg(&cru->peri_clksel_con[0],
			     PCLK_PERI_DIV_MASK,
			     (div - 1) << PCLK_PERI_DIV_SHIFT);
		break;
	case PCLK_TOP_ROOT:
		break;
	case LSCLK_PMU_ROOT:
	case PCLK_PMU:
		if (!(OSC_HZ % rate)) {
			src_clk = LSCLK_PMU_SEL_24M;
			div = DIV_ROUND_UP(OSC_HZ, rate);
		} else {
			src_clk = LSCLK_PMU_SEL_RC_OSC;
			div = DIV_ROUND_UP(RC_OSC_HZ, rate);
		}
		rk_clrsetreg(&cru->pmu_clksel_con[2],
			     LSCLK_PMU_SEL_MASK | LSCLK_PMU_DIV_MASK,
			     (src_clk << LSCLK_PMU_SEL_SHIFT) |
			     ((div - 1) << LSCLK_PMU_DIV_SHIFT));
		break;
	default:
		printf("do not support this permid freq\n");
		return -EINVAL;
	}

	return rv1103b_peri_get_clk(priv, clk_id);
}

static ulong rv1103b_i2c_get_clk(struct rv1103b_clk_priv *priv, ulong clk_id)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 sel, con;
	ulong rate;

	switch (clk_id) {
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C_PERI:
		con = readl(&cru->clksel_con[34]);
		sel = (con & CLK_I2C1_SEL_MASK) >> CLK_I2C1_SEL_SHIFT;
		break;
	case CLK_I2C0:
	case CLK_I2C_PMU:
		con = readl(&cru->clksel_con[34]);
		sel = (con & CLK_I2C0_SEL_MASK) >> CLK_I2C0_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == CLK_I2C_SEL_100M)
		rate = 100 * MHz;
	else
		rate = OSC_HZ;

	return rate;
}

static ulong rv1103b_crypto_get_clk(struct rv1103b_clk_priv *priv, ulong clk_id)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 sel, con, rate;

	switch (clk_id) {
	case ACLK_CRYPTO:
	case HCLK_CRYPTO:
	case HCLK_RK_RNG_NS:
	case HCLK_RK_RNG_S:
		return rv1103b_peri_get_clk(priv, LSCLK_PERI_SRC);
	case CLK_CORE_CRYPTO:
		con = readl(&cru->clksel_con[35]);
		sel = (con & CLK_CORE_CRYPTO_SEL_MASK) >>
		      CLK_CORE_CRYPTO_SEL_SHIFT;
		break;
	case CLK_PKA_CRYPTO:
		con = readl(&cru->clksel_con[35]);
		sel = (con & CLK_PKA_CRYPTO_SEL_MASK) >>
		      CLK_PKA_CRYPTO_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}
	if (sel == CLK_CORE_CRYPTO_SEL_300M)
		rate = 300 * MHz;
	else if (sel == CLK_CORE_CRYPTO_SEL_200M)
		rate = 200 * MHz;
	else
		rate = 100 * MHz;

	return rate;
}

static ulong rv1103b_crypto_set_clk(struct rv1103b_clk_priv *priv,
				    ulong clk_id, ulong rate)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 sel;

	if (rate >= 297 * MHz)
		sel = CLK_CORE_CRYPTO_SEL_300M;
	else if (rate >= 198 * MHz)
		sel = CLK_CORE_CRYPTO_SEL_200M;
	else
		sel = CLK_CORE_CRYPTO_SEL_100M;

	switch (clk_id) {
	case ACLK_CRYPTO:
	case HCLK_CRYPTO:
	case HCLK_RK_RNG_NS:
	case HCLK_RK_RNG_S:
		rv1103b_peri_set_clk(priv, LSCLK_PERI_SRC, rate);
	case CLK_CORE_CRYPTO:
		rk_clrsetreg(&cru->clksel_con[35],
			     CLK_CORE_CRYPTO_SEL_MASK,
			     (sel << CLK_CORE_CRYPTO_SEL_SHIFT));
		break;
	case CLK_PKA_CRYPTO:
		rk_clrsetreg(&cru->clksel_con[35],
			     CLK_PKA_CRYPTO_SEL_MASK,
			     (sel << CLK_PKA_CRYPTO_SEL_SHIFT));
		break;
	default:
		return -ENOENT;
	}
	return rv1103b_crypto_get_clk(priv, clk_id);
}

static ulong rv1103b_mmc_get_clk(struct rv1103b_clk_priv *priv, ulong clk_id)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 div, sel, con, prate;

	switch (clk_id) {
	case CCLK_SDMMC1:
	case HCLK_SDMMC1:
		con = readl(&cru->clksel_con[36]);
		sel = (con & CLK_SDMMC_SEL_MASK) >>
		      CLK_SDMMC_SEL_SHIFT;
		div = (con & CLK_SDMMC_DIV_MASK) >>
		      CLK_SDMMC_DIV_SHIFT;
		if (sel == CLK_MMC_SEL_GPLL)
			prate = priv->gpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case CCLK_SDMMC0:
	case HCLK_SDMMC0:
		con = readl(&cru->clksel_con[32]);
		sel = (con & CLK_SDMMC_SEL_MASK) >>
		      CLK_SDMMC_SEL_SHIFT;
		div = (con & CLK_SDMMC_DIV_MASK) >>
		      CLK_SDMMC_DIV_SHIFT;
		if (sel == CLK_MMC_SEL_GPLL)
			prate = priv->gpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case CCLK_EMMC:
	case HCLK_EMMC:
		con = readl(&cru->clksel_con[31]);
		sel = (con & CLK_EMMC_SEL_MASK) >>
		      CLK_EMMC_SEL_SHIFT;
		div = (con & CLK_EMMC_DIV_MASK) >>
		      CLK_EMMC_DIV_SHIFT;
		if (sel == CLK_MMC_SEL_GPLL)
			prate = priv->gpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case SCLK_SFC_2X:
	case HCLK_SFC:
		con = readl(&cru->clksel_con[33]);
		sel = (con & CLK_SFC_SEL_MASK) >>
		      CLK_SFC_SEL_SHIFT;
		div = (con & CLK_SFC_DIV_MASK) >>
		      CLK_SFC_DIV_SHIFT;
		if (sel == CLK_MMC_SEL_GPLL)
			prate = priv->gpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	default:
		return -ENOENT;
	}
}

static ulong rv1103b_mmc_set_clk(struct rv1103b_clk_priv *priv,
				 ulong clk_id, ulong rate)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 sel, src_clk_div;
	ulong prate = 0;

	if ((OSC_HZ % rate) == 0) {
		sel = CLK_MMC_SEL_OSC;
		prate = OSC_HZ;
	} else {
		sel = CLK_MMC_SEL_GPLL;
		prate = priv->gpll_hz;
	}
	src_clk_div = DIV_ROUND_UP(prate, rate);

	switch (clk_id) {
	case CCLK_SDMMC1:
	case HCLK_SDMMC1:
		src_clk_div = DIV_ROUND_UP(prate, rate);
		rk_clrsetreg(&cru->clksel_con[36],
			     CLK_SDMMC_SEL_MASK |
			     CLK_SDMMC_DIV_MASK,
			     (sel << CLK_SDMMC_SEL_SHIFT) |
			     ((src_clk_div - 1) <<
			      CLK_SDMMC_DIV_SHIFT));
		break;
	case CCLK_SDMMC0:
	case HCLK_SDMMC0:
		src_clk_div = DIV_ROUND_UP(prate, rate);
		rk_clrsetreg(&cru->clksel_con[32],
			     CLK_SDMMC_SEL_MASK |
			     CLK_SDMMC_DIV_MASK,
			     (sel << CLK_SDMMC_SEL_SHIFT) |
			     ((src_clk_div - 1) <<
			      CLK_SDMMC_DIV_SHIFT));
		break;
	case CCLK_EMMC:
	case HCLK_EMMC:
		src_clk_div = DIV_ROUND_UP(prate, rate);
		rk_clrsetreg(&cru->clksel_con[31],
			     CLK_EMMC_SEL_MASK |
			     CLK_EMMC_DIV_MASK,
			     (sel << CLK_EMMC_SEL_SHIFT) |
			     ((src_clk_div - 1) <<
			      CLK_EMMC_DIV_SHIFT));
		break;
	case SCLK_SFC_2X:
	case HCLK_SFC:
		src_clk_div = DIV_ROUND_UP(prate, rate);
		rk_clrsetreg(&cru->clksel_con[33],
			     CLK_SFC_SEL_MASK |
			     CLK_SFC_DIV_MASK,
			     (sel << CLK_SFC_SEL_SHIFT) |
			     ((src_clk_div - 1) <<
			      CLK_SFC_DIV_SHIFT));
		break;
	default:
		return -ENOENT;
	}
	return rv1103b_mmc_get_clk(priv, clk_id);
}

static ulong rv1103b_i2c_set_clk(struct rv1103b_clk_priv *priv, ulong clk_id,
				 ulong rate)
{
	struct rv1103b_cru *cru = priv->cru;
	int src_clk;

	if (rate == OSC_HZ)
		src_clk = CLK_I2C_SEL_24M;
	else
		src_clk = CLK_I2C_SEL_100M;

	switch (clk_id) {
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C_PERI:
		rk_clrsetreg(&cru->clksel_con[34], CLK_I2C1_SEL_MASK,
			     src_clk << CLK_I2C1_SEL_SHIFT);
		break;
	case CLK_I2C0:
	case CLK_I2C_PMU:
		rk_clrsetreg(&cru->clksel_con[34], CLK_I2C0_SEL_MASK,
			     src_clk << CLK_I2C0_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}
	return rv1103b_i2c_get_clk(priv, clk_id);
}

static ulong rv1103b_spi_get_clk(struct rv1103b_clk_priv *priv, ulong clk_id)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 sel, con, rate;

	switch (clk_id) {
	case CLK_SPI0:
		con = readl(&cru->clksel_con[34]);
		sel = (con & CLK_SPI0_SEL_MASK) >> CLK_SPI0_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}
	if (sel == CLK_SPI0_SEL_200M)
		rate = 200 * MHz;
	else if (sel == CLK_SPI0_SEL_100M)
		rate = 100 * MHz;
	else if (sel == CLK_SPI0_SEL_50M)
		rate = 50 * MHz;
	else
		rate = OSC_HZ;

	return rate;
}

static ulong rv1103b_spi_set_clk(struct rv1103b_clk_priv *priv,
				 ulong clk_id, ulong rate)
{
	struct rv1103b_cru *cru = priv->cru;
	int src_clk;

	if (rate >= 198 * MHz)
		src_clk = CLK_SPI0_SEL_200M;
	else if (rate >= 99 * MHz)
		src_clk = CLK_SPI0_SEL_100M;
	else if (rate >= 48 * MHz)
		src_clk = CLK_SPI0_SEL_50M;
	else
		src_clk = CLK_SPI0_SEL_24M;

	switch (clk_id) {
	case CLK_SPI0:
		rk_clrsetreg(&cru->clksel_con[34], CLK_SPI0_SEL_MASK,
			     src_clk << CLK_SPI0_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rv1103b_spi_get_clk(priv, clk_id);
}

static ulong rv1103b_pwm_get_clk(struct rv1103b_clk_priv *priv, ulong clk_id)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 sel, con;

	switch (clk_id) {
	case CLK_PWM0:
	case CLK_PWM0_SRC:
		con = readl(&cru->clksel_con[34]);
		sel = (con & CLK_PWM0_SEL_MASK) >> CLK_PWM0_SEL_SHIFT;
		break;
	case CLK_PWM1:
		con = readl(&cru->clksel_con[34]);
		sel = (con & CLK_PWM1_SEL_MASK) >> CLK_PWM1_SEL_SHIFT;
		break;
	case CLK_PWM2:
		con = readl(&cru->clksel_con[34]);
		sel = (con & CLK_PWM2_SEL_MASK) >> CLK_PWM2_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	switch (sel) {
	case CLK_PWM_SEL_100M:
		return 100 * MHz;
	case CLK_PWM_SEL_24M:
		return OSC_HZ;
	default:
		return -ENOENT;
	}
}

static ulong rv1103b_pwm_set_clk(struct rv1103b_clk_priv *priv,
				 ulong clk_id, ulong rate)
{
	struct rv1103b_cru *cru = priv->cru;
	int src_clk;

	if (rate >= 99 * MHz)
		src_clk = CLK_PWM_SEL_100M;
	else
		src_clk = CLK_PWM_SEL_24M;

	switch (clk_id) {
	case CLK_PWM0:
	case CLK_PWM0_SRC:
		rk_clrsetreg(&cru->clksel_con[34],
			     CLK_PWM0_SEL_MASK,
			     src_clk << CLK_PWM0_SEL_SHIFT);
		break;
	case CLK_PWM1:
		rk_clrsetreg(&cru->clksel_con[34],
			     CLK_PWM1_SEL_MASK,
			     src_clk << CLK_PWM1_SEL_SHIFT);
		break;
	case CLK_PWM2:
		rk_clrsetreg(&cru->clksel_con[34],
			     CLK_PWM2_SEL_MASK,
			     src_clk << CLK_PWM2_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rv1103b_pwm_get_clk(priv, clk_id);
}

static ulong rv1103b_adc_get_clk(struct rv1103b_clk_priv *priv, ulong clk_id)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 div, con;

	switch (clk_id) {
	case CLK_SARADC:
		con = readl(&cru->peri_clksel_con[1]);
		div = (con & CLK_SARADC_DIV_MASK) >>
		      CLK_SARADC_DIV_SHIFT;
		return DIV_TO_RATE(OSC_HZ, div);
	case CLK_TSADC_TSEN:
		con = readl(&cru->peri_clksel_con[0]);
		div = (con & CLK_TSADC_TSEN_DIV_MASK) >>
		      CLK_TSADC_TSEN_DIV_SHIFT;
		return DIV_TO_RATE(OSC_HZ, div);
	case CLK_TSADC:
		con = readl(&cru->peri_clksel_con[0]);
		div = (con & CLK_TSADC_DIV_MASK) >> CLK_TSADC_DIV_SHIFT;
		return DIV_TO_RATE(OSC_HZ, div);
	default:
		return -ENOENT;
	}
}

static ulong rv1103b_adc_set_clk(struct rv1103b_clk_priv *priv,
				 ulong clk_id, ulong rate)
{
	struct rv1103b_cru *cru = priv->cru;
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(OSC_HZ, rate);

	switch (clk_id) {
	case CLK_SARADC:
		assert(src_clk_div - 1 <= 7);
		rk_clrsetreg(&cru->peri_clksel_con[1],
			     CLK_SARADC_DIV_MASK,
			     (src_clk_div - 1) <<
			     CLK_SARADC_DIV_SHIFT);
		break;
	case CLK_TSADC_TSEN:
		assert(src_clk_div - 1 <= 32);
		rk_clrsetreg(&cru->peri_clksel_con[0],
			     CLK_TSADC_TSEN_DIV_MASK,
			     (src_clk_div - 1) <<
			     CLK_TSADC_TSEN_DIV_SHIFT);
		break;
	case CLK_TSADC:
		assert(src_clk_div - 1 <= 32);
		rk_clrsetreg(&cru->peri_clksel_con[0],
			     CLK_TSADC_DIV_MASK,
			     (src_clk_div - 1) <<
			     CLK_TSADC_DIV_SHIFT);
		break;
	default:
		return -ENOENT;
	}
	return rv1103b_adc_get_clk(priv, clk_id);
}

/*
 *
 * rational_best_approximation(31415, 10000,
 *		(1 << 8) - 1, (1 << 5) - 1, &n, &d);
 *
 * you may look at given_numerator as a fixed point number,
 * with the fractional part size described in given_denominator.
 *
 * for theoretical background, see:
 * http://en.wikipedia.org/wiki/Continued_fraction
 */
static void rational_best_approximation(unsigned long given_numerator,
					unsigned long given_denominator,
					unsigned long max_numerator,
					unsigned long max_denominator,
					unsigned long *best_numerator,
					unsigned long *best_denominator)
{
	unsigned long n, d, n0, d0, n1, d1;

	n = given_numerator;
	d = given_denominator;
	n0 = 0;
	d1 = 0;
	n1 = 1;
	d0 = 1;
	for (;;) {
		unsigned long t, a;

		if (n1 > max_numerator || d1 > max_denominator) {
			n1 = n0;
			d1 = d0;
			break;
		}
		if (d == 0)
			break;
		t = d;
		a = n / d;
		d = n % d;
		n = t;
		t = n0 + a * n1;
		n0 = n1;
		n1 = t;
		t = d0 + a * d1;
		d0 = d1;
		d1 = t;
	}
	*best_numerator = n1;
	*best_denominator = d1;
}

static ulong rv1103b_uart_get_rate(struct rv1103b_clk_priv *priv, ulong clk_id)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 reg, con, fracdiv, div, src, p_rate;
	unsigned long m, n;

	switch (clk_id) {
	case SCLK_UART0:
		reg = 10;
		con = readl(&cru->clksel_con[32]);
		src = (con & CLK_UART0_SEL_MASK) >> CLK_UART0_SEL_SHIFT;
		con = readl(&cru->clksel_con[5]);
		div = (con & CLK_UART0_SRC_DIV_MASK) >> CLK_UART0_SRC_DIV_SHIFT;
		break;
	case SCLK_UART1:
		reg = 11;
		con = readl(&cru->clksel_con[32]);
		src = (con & CLK_UART1_SEL_MASK) >> CLK_UART1_SEL_SHIFT;
		con = readl(&cru->clksel_con[5]);
		div = (con & CLK_UART1_SRC_DIV_MASK) >> CLK_UART1_SRC_DIV_SHIFT;
		break;
	case SCLK_UART2:
		reg = 12;
		con = readl(&cru->clksel_con[32]);
		src = (con & CLK_UART2_SEL_MASK) >> CLK_UART2_SEL_SHIFT;
		con = readl(&cru->clksel_con[5]);
		div = (con & CLK_UART2_SRC_DIV_MASK) >> CLK_UART2_SRC_DIV_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	p_rate = priv->gpll_hz;
	if (src == CLK_UART_SEL_SRC) {
		return DIV_TO_RATE(p_rate, div);
	} else if (src == CLK_UART_SEL_FRAC) {
		fracdiv = readl(&cru->clksel_con[reg]);
		n = fracdiv & CLK_UART_FRAC_NUMERATOR_MASK;
		n >>= CLK_UART_FRAC_NUMERATOR_SHIFT;
		m = fracdiv & CLK_UART_FRAC_DENOMINATOR_MASK;
		m >>= CLK_UART_FRAC_DENOMINATOR_SHIFT;
		return DIV_TO_RATE(p_rate, div) * n / m;
	} else {
		return OSC_HZ;
	}
}

static ulong rv1103b_uart_set_rate(struct rv1103b_clk_priv *priv,
				   ulong clk_id, ulong rate)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 reg, uart_src, div;
	unsigned long m = 0, n = 0, val;

	if (priv->gpll_hz % rate == 0) {
		uart_src = CLK_UART_SEL_SRC;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	} else if (rate == OSC_HZ) {
		uart_src = CLK_UART_SEL_OSC;
		div = 2;
	} else {
		uart_src = CLK_UART_SEL_FRAC;
		div = 2;
		rational_best_approximation(rate, priv->gpll_hz / div,
					    GENMASK(16 - 1, 0),
					    GENMASK(16 - 1, 0),
					    &m, &n);
	}

	switch (clk_id) {
	case SCLK_UART0:
		reg = 10;
		rk_clrsetreg(&cru->clksel_con[5],
			     CLK_UART0_SRC_DIV_MASK,
			     div << CLK_UART0_SRC_DIV_SHIFT);
		rk_clrsetreg(&cru->clksel_con[32],
			     CLK_UART0_SEL_MASK,
			     uart_src << CLK_UART0_SEL_SHIFT);
		break;
	case SCLK_UART1:
		reg = 11;
		rk_clrsetreg(&cru->clksel_con[5],
			     CLK_UART1_SRC_DIV_MASK,
			     div << CLK_UART1_SRC_DIV_SHIFT);
		rk_clrsetreg(&cru->clksel_con[32],
			     CLK_UART1_SEL_MASK,
			     uart_src << CLK_UART1_SEL_SHIFT);
		break;
	case SCLK_UART2:
		reg = 12;
		rk_clrsetreg(&cru->clksel_con[5],
			     CLK_UART2_SRC_DIV_MASK,
			     div << CLK_UART2_SRC_DIV_SHIFT);
		rk_clrsetreg(&cru->clksel_con[32],
			     CLK_UART2_SEL_MASK,
			     uart_src << CLK_UART2_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}
	if (m && n) {
		val = m << CLK_UART_FRAC_NUMERATOR_SHIFT | n;
		writel(val, &cru->clksel_con[reg]);
	}

	return rv1103b_uart_get_rate(priv, clk_id);
}

static ulong rv1103b_decom_get_clk(struct rv1103b_clk_priv *priv)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 sel, con, prate;

	con = readl(&cru->clksel_con[35]);
	sel = (con & DCLK_DECOM_SEL_MASK) >>
	      DCLK_DECOM_SEL_SHIFT;
	if (sel == DCLK_DECOM_SEL_480M)
		prate = 480 * MHz;
	else if (sel == DCLK_DECOM_SEL_400M)
		prate = 400 * MHz;
	else
		prate = 300 * MHz;
	return prate;
}

static ulong rv1103b_decom_set_clk(struct rv1103b_clk_priv *priv, ulong rate)
{
	struct rv1103b_cru *cru = priv->cru;
	u32 sel;

	if (rate >= 480 * MHz)
		sel = DCLK_DECOM_SEL_480M;
	else if (rate >= 396 * MHz)
		sel = DCLK_DECOM_SEL_400M;
	else
		sel = DCLK_DECOM_SEL_300M;
	rk_clrsetreg(&cru->clksel_con[35], DCLK_DECOM_SEL_MASK,
		     (sel << DCLK_DECOM_SEL_SHIFT));

	return rv1103b_decom_get_clk(priv);
}

static ulong rv1103b_clk_get_rate(struct clk *clk)
{
	struct rv1103b_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	if (!priv->gpll_hz) {
		printf("%s gpll=%lu\n", __func__, priv->gpll_hz);
		return -ENOENT;
	}

	switch (clk->id) {
	case PLL_GPLL:
		rate = rockchip_pll_get_rate(&rv1103b_pll_clks[GPLL], priv->cru,
					     GPLL);
		break;
	case ACLK_PERI_SRC:
	case LSCLK_PERI_SRC:
	case PCLK_PERI_ROOT:
	case PCLK_TOP_ROOT:
	case LSCLK_PMU_ROOT:
	case PCLK_PMU:
		rate = rv1103b_peri_get_clk(priv, clk->id);
		break;
	case ACLK_CRYPTO:
	case HCLK_CRYPTO:
	case HCLK_RK_RNG_NS:
	case HCLK_RK_RNG_S:
	case CLK_CORE_CRYPTO:
	case CLK_PKA_CRYPTO:
		rate = rv1103b_crypto_get_clk(priv, clk->id);
		break;
	case CCLK_SDMMC1:
	case HCLK_SDMMC1:
	case CCLK_SDMMC0:
	case HCLK_SDMMC0:
	case CCLK_EMMC:
	case HCLK_EMMC:
	case SCLK_SFC_2X:
	case HCLK_SFC:
		rate = rv1103b_mmc_get_clk(priv, clk->id);
		break;
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C_PERI:
	case CLK_I2C0:
	case CLK_I2C_PMU:
		rate = rv1103b_i2c_get_clk(priv, clk->id);
		break;
	case CLK_SPI0:
		rate = rv1103b_spi_get_clk(priv, clk->id);
		break;
	case CLK_PWM0:
	case CLK_PWM0_SRC:
	case CLK_PWM1:
	case CLK_PWM2:
		rate = rv1103b_pwm_get_clk(priv, clk->id);
		break;
	case CLK_SARADC:
	case CLK_TSADC_TSEN:
	case CLK_TSADC:
		rate = rv1103b_adc_get_clk(priv, clk->id);
		break;
	case SCLK_UART0:
	case SCLK_UART1:
	case SCLK_UART2:
		rate = rv1103b_uart_get_rate(priv, clk->id);
		break;
	case DCLK_DECOM_SRC:
	case DCLK_DECOM:
		rate = rv1103b_decom_get_clk(priv);
		break;
	case TCLK_WDT_LPMCU:
	case TCLK_WDT_HPMCU:
	case TCLK_WDT_NS:
	case TCLK_WDT_S:
		rate = OSC_HZ;
		break;
	default:
		return -ENOENT;
	}

	return rate;
};

static ulong rv1103b_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rv1103b_clk_priv *priv = dev_get_priv(clk->dev);
	ulong ret = 0;

	if (!priv->gpll_hz) {
		printf("%s gpll=%lu\n", __func__, priv->gpll_hz);
		return -ENOENT;
	}

	switch (clk->id) {
	case PLL_GPLL:
		ret = rockchip_pll_set_rate(&rv1103b_pll_clks[GPLL], priv->cru,
					    GPLL, rate);
		break;
	case ACLK_PERI_SRC:
	case LSCLK_PERI_SRC:
	case PCLK_PERI_ROOT:
	case PCLK_TOP_ROOT:
	case LSCLK_PMU_ROOT:
	case PCLK_PMU:
		ret = rv1103b_peri_set_clk(priv, clk->id, rate);
		break;
	case ACLK_CRYPTO:
	case HCLK_CRYPTO:
	case HCLK_RK_RNG_NS:
	case HCLK_RK_RNG_S:
	case CLK_CORE_CRYPTO:
	case CLK_PKA_CRYPTO:
		ret = rv1103b_crypto_set_clk(priv, clk->id, rate);
		break;
	case CCLK_SDMMC1:
	case HCLK_SDMMC1:
	case CCLK_SDMMC0:
	case HCLK_SDMMC0:
	case CCLK_EMMC:
	case HCLK_EMMC:
	case SCLK_SFC_2X:
	case HCLK_SFC:
		ret = rv1103b_mmc_set_clk(priv, clk->id, rate);
		break;
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C_PERI:
	case CLK_I2C0:
	case CLK_I2C_PMU:
		ret = rv1103b_i2c_set_clk(priv, clk->id, rate);
		break;
	case CLK_SPI0:
		ret = rv1103b_spi_set_clk(priv, clk->id, rate);
		break;
	case CLK_PWM0:
	case CLK_PWM0_SRC:
	case CLK_PWM1:
	case CLK_PWM2:
		ret = rv1103b_pwm_set_clk(priv, clk->id, rate);
		break;
	case CLK_SARADC:
	case CLK_TSADC_TSEN:
	case CLK_TSADC:
		ret = rv1103b_adc_set_clk(priv, clk->id, rate);
		break;
	case SCLK_UART0:
	case SCLK_UART1:
	case SCLK_UART2:
		ret = rv1103b_uart_set_rate(priv, clk->id, rate);
		break;
	case DCLK_DECOM_SRC:
	case DCLK_DECOM:
		rate = rv1103b_decom_set_clk(priv, rate);
		break;
	default:
		return -ENOENT;
	}

	return ret;
};

static int rv1103b_clk_set_parent(struct clk *clk, struct clk *parent)
{
	switch (clk->id) {
	default:
		return -ENOENT;
	}

	return 0;
}

static struct clk_ops rv1103b_clk_ops = {
	.get_rate = rv1103b_clk_get_rate,
	.set_rate = rv1103b_clk_set_rate,
#if (IS_ENABLED(OF_CONTROL)) || (!IS_ENABLED(OF_PLATDATA))
	.set_parent = rv1103b_clk_set_parent,
#endif
};

static void rv1103b_clk_init(struct rv1103b_clk_priv *priv)
{
	int ret;
	u32 div;

	priv->sync_kernel = false;
	priv->gpll_hz = rockchip_pll_get_rate(&rv1103b_pll_clks[GPLL],
					      priv->cru, GPLL);
	if (priv->gpll_hz != GPLL_HZ) {
		ret = rockchip_pll_set_rate(&rv1103b_pll_clks[GPLL], priv->cru,
					    GPLL, GPLL_HZ);
		if (!ret)
			priv->gpll_hz = GPLL_HZ;
	}

	if (!priv->armclk_enter_hz) {
		div = (readl(&priv->cru->clksel_con[37]) &
		       CLK_CORE_GPLL_DIV_MASK) >>
		      CLK_CORE_GPLL_DIV_SHIFT;
		priv->armclk_enter_hz = DIV_TO_RATE(priv->gpll_hz, div);
		priv->armclk_init_hz = priv->armclk_enter_hz;
	}
}

static int rv1103b_clk_probe(struct udevice *dev)
{
	struct rv1103b_clk_priv *priv = dev_get_priv(dev);
	int ret;

#ifdef CONFIG_SPL_BUILD
	/* fix lsclk_prei div */
	writel(BITS_WITH_WMASK(1, 0x1U, 9), RV1103B_CRU_BASE + RV1103B_CLKSEL_CON(31));
	/* fix cpu div */
	writel(BITS_WITH_WMASK(1, 0x7U, 13), RV1103B_CRU_BASE + RV1103B_CLKSEL_CON(37));
	/* fix gpll postdiv1 */
	writel(BITS_WITH_WMASK(1, 0x7U, 12), RV1103B_CRU_BASE + RV1103B_PLL_CON(24));
#endif

	rv1103b_clk_init(priv);

	/* Process 'assigned-{clocks/clock-parents/clock-rates}' properties */
	ret = clk_set_defaults(dev, 1);
	if (ret)
		debug("%s clk_set_defaults failed %d\n", __func__, ret);
	else
		priv->sync_kernel = true;
	return 0;
}

static int rv1103_clk_of_to_plat(struct udevice *dev)
{
	struct rv1103_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);

	return 0;
}

static int rv1103b_clk_bind(struct udevice *dev)
{
	struct udevice *sys_child;
	struct sysreset_reg *priv;
	int ret;

	/* The sysreset driver does not have a device node, so bind it here */
	ret = device_bind_driver(dev, "rockchip_sysreset", "sysreset", &sys_child);
	if (ret) {
		debug("Warning: No sysreset driver: ret=%d\n", ret);
	} else {
		priv = malloc(sizeof(struct sysreset_reg));
		priv->glb_srst_fst_value = offsetof(struct rv1103b_cru, glb_srst_fst);
		priv->glb_srst_snd_value = offsetof(struct rv1103b_cru, glb_srst_snd);
		dev_set_priv(sys_child, priv);
	}

#if CONFIG_IS_ENABLED(RESET_ROCKCHIP)
	ret = offsetof(struct rv1103b_cru, peri_softrst_con[0]);
	ret = rockchip_reset_bind(dev, ret, 12); /* number of reset registers */
	if (ret)
		debug("Warning: software reset driver bind failed\n");
#endif

	return 0;
}

static const struct udevice_id rv1103b_clk_ids[] = {
	{ .compatible = "rockchip,rv1103-cru" },
	{ }
};

U_BOOT_DRIVER(clk_rv1103) = {
	.name		= "clk_rv1103",
	.id		= UCLASS_CLK,
	.of_match	= rv1103b_clk_ids,
	.priv_auto	= sizeof(struct rv1103b_clk_priv),
	.of_to_plat	= rv1103_clk_of_to_plat,
	.ops		= &rv1103b_clk_ops,
	.bind		= rv1103b_clk_bind,
	.probe		= rv1103b_clk_probe,
};

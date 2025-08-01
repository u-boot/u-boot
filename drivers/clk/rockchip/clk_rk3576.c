// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 Fuzhou Rockchip Electronics Co., Ltd
 * Author: Elaine Zhang <zhangqing@rock-chips.com>
 */

#include <bitfield.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/arch-rockchip/cru_rk3576.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rockchip,rk3576-cru.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

static struct rockchip_pll_rate_table rk3576_24m_pll_rates[] = {
	/* _mhz, _p, _m, _s, _k */
	RK3588_PLL_RATE(1500000000, 2, 250, 1, 0),
	RK3588_PLL_RATE(1200000000, 1, 100, 1, 0),
	RK3588_PLL_RATE(1188000000, 2, 198, 1, 0),
	RK3588_PLL_RATE(1150000000, 3, 575, 2, 0),
	RK3588_PLL_RATE(1100000000, 3, 550, 2, 0),
	RK3588_PLL_RATE(1008000000, 2, 336, 2, 0),
	RK3588_PLL_RATE(1000000000, 3, 500, 2, 0),
	RK3588_PLL_RATE(900000000, 2, 300, 2, 0),
	RK3588_PLL_RATE(850000000, 3, 425, 2, 0),
	RK3588_PLL_RATE(816000000, 2, 272, 2, 0),
	RK3588_PLL_RATE(786432000, 2, 262, 2, 9437),
	RK3588_PLL_RATE(786000000, 1, 131, 2, 0),
	RK3588_PLL_RATE(742500000, 4, 495, 2, 0),
	RK3588_PLL_RATE(722534400, 8, 963, 2, 24850),
	RK3588_PLL_RATE(600000000, 2, 200, 2, 0),
	RK3588_PLL_RATE(594000000, 2, 198, 2, 0),
	RK3588_PLL_RATE(200000000, 3, 400, 4, 0),
	RK3588_PLL_RATE(100000000, 3, 400, 5, 0),
	{ /* sentinel */ },
};

static struct rockchip_pll_clock rk3576_pll_clks[] = {
	[BPLL] = PLL(pll_rk3588, PLL_BPLL, RK3576_PLL_CON(0),
		      RK3576_BPLL_MODE_CON0, 0, 15, 0,
		      rk3576_24m_pll_rates),
	[LPLL] = PLL(pll_rk3588, PLL_LPLL, RK3576_LPLL_CON(16),
		     RK3576_LPLL_MODE_CON0, 0, 15, 0, rk3576_24m_pll_rates),
	[VPLL] = PLL(pll_rk3588, PLL_VPLL, RK3576_PLL_CON(88),
		      RK3576_LPLL_MODE_CON0, 4, 15, 0, rk3576_24m_pll_rates),
	[AUPLL] = PLL(pll_rk3588, PLL_AUPLL, RK3576_PLL_CON(96),
		      RK3576_MODE_CON0, 6, 15, 0, rk3576_24m_pll_rates),
	[CPLL] = PLL(pll_rk3588, PLL_CPLL, RK3576_PLL_CON(104),
		     RK3576_MODE_CON0, 8, 15, 0, rk3576_24m_pll_rates),
	[GPLL] = PLL(pll_rk3588, PLL_GPLL, RK3576_PLL_CON(112),
		     RK3576_MODE_CON0, 2, 15, 0, rk3576_24m_pll_rates),
	[PPLL] = PLL(pll_rk3588, PLL_PPLL, RK3576_PMU_PLL_CON(128),
		     RK3576_MODE_CON0, 10, 15, 0, rk3576_24m_pll_rates),
};

#ifdef CONFIG_SPL_BUILD
#ifndef BITS_WITH_WMASK
#define BITS_WITH_WMASK(bits, msk, shift) \
	((bits) << (shift)) | ((msk) << ((shift) + 16))
#endif
#endif

#ifndef CONFIG_SPL_BUILD
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
#endif

static ulong rk3576_bus_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 con, sel, div, rate;

	switch (clk_id) {
	case ACLK_BUS_ROOT:
		con = readl(&cru->clksel_con[55]);
		sel = (con & ACLK_BUS_ROOT_SEL_MASK) >>
		      ACLK_BUS_ROOT_SEL_SHIFT;
		div = (con & ACLK_BUS_ROOT_DIV_MASK) >>
		      ACLK_BUS_ROOT_DIV_SHIFT;
		if (sel == ACLK_BUS_ROOT_SEL_CPLL)
			rate = DIV_TO_RATE(priv->cpll_hz, div);
		else
			rate = DIV_TO_RATE(priv->gpll_hz, div);
		break;
	case HCLK_BUS_ROOT:
		con = readl(&cru->clksel_con[55]);
		sel = (con & HCLK_BUS_ROOT_SEL_MASK) >>
		      HCLK_BUS_ROOT_SEL_SHIFT;
		if (sel == HCLK_BUS_ROOT_SEL_200M)
			rate = 198 * MHz;
		else if (sel == HCLK_BUS_ROOT_SEL_100M)
			rate = 100 * MHz;
		else if (sel == HCLK_BUS_ROOT_SEL_50M)
			rate = 50 * MHz;
		else
			rate = OSC_HZ;
		break;
	case PCLK_BUS_ROOT:
		con = readl(&cru->clksel_con[55]);
		sel = (con & PCLK_BUS_ROOT_SEL_MASK) >>
		      PCLK_BUS_ROOT_SEL_SHIFT;
		if (sel == PCLK_BUS_ROOT_SEL_100M)
			rate = 100 * MHz;
		else if (sel == PCLK_BUS_ROOT_SEL_50M)
			rate = 50 * MHz;
		else
			rate = OSC_HZ;
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong rk3576_bus_set_clk(struct rk3576_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	int src_clk, src_clk_div;

	switch (clk_id) {
	case ACLK_BUS_ROOT:
		if (!(priv->cpll_hz % rate)) {
			src_clk = ACLK_BUS_ROOT_SEL_CPLL;
			src_clk_div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = ACLK_BUS_ROOT_SEL_GPLL;
			src_clk_div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		rk_clrsetreg(&cru->clksel_con[55],
			     ACLK_BUS_ROOT_SEL_MASK,
			     src_clk << ACLK_BUS_ROOT_SEL_SHIFT);
		assert(src_clk_div - 1 <= 31);
		rk_clrsetreg(&cru->clksel_con[55],
			     ACLK_BUS_ROOT_DIV_MASK |
			     ACLK_BUS_ROOT_SEL_MASK,
			     (src_clk <<
			      ACLK_BUS_ROOT_SEL_SHIFT) |
			     (src_clk_div - 1) << ACLK_BUS_ROOT_DIV_SHIFT);
		break;
	case HCLK_BUS_ROOT:
		if (rate >= 198 * MHz)
			src_clk = HCLK_BUS_ROOT_SEL_200M;
		else if (rate >= 99 * MHz)
			src_clk = HCLK_BUS_ROOT_SEL_100M;
		else if (rate >= 50 * MHz)
			src_clk = HCLK_BUS_ROOT_SEL_50M;
		else
			src_clk = HCLK_BUS_ROOT_SEL_OSC;
		rk_clrsetreg(&cru->clksel_con[55],
			     HCLK_BUS_ROOT_SEL_MASK,
			     src_clk << HCLK_BUS_ROOT_SEL_SHIFT);
		break;
	case PCLK_BUS_ROOT:
		if (rate >= 99 * MHz)
			src_clk = PCLK_BUS_ROOT_SEL_100M;
		else if (rate >= 50 * MHz)
			src_clk = PCLK_BUS_ROOT_SEL_50M;
		else
			src_clk = PCLK_BUS_ROOT_SEL_OSC;
		rk_clrsetreg(&cru->clksel_con[55],
			     PCLK_BUS_ROOT_SEL_MASK,
			     src_clk << PCLK_BUS_ROOT_SEL_SHIFT);
		break;
	default:
		printf("do not support this center freq\n");
		return -EINVAL;
	}

	return rk3576_bus_get_clk(priv, clk_id);
}

static ulong rk3576_top_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 con, sel, div, rate, prate;

	switch (clk_id) {
	case ACLK_TOP:
		con = readl(&cru->clksel_con[9]);
		div = (con & ACLK_TOP_DIV_MASK) >>
		      ACLK_TOP_DIV_SHIFT;
		sel = (con & ACLK_TOP_SEL_MASK) >>
		      ACLK_TOP_SEL_SHIFT;
		if (sel == ACLK_TOP_SEL_CPLL)
			prate = priv->cpll_hz;
		else if (sel == ACLK_TOP_SEL_AUPLL)
			prate = priv->aupll_hz;
		else
			prate = priv->gpll_hz;
		return DIV_TO_RATE(prate, div);
	case ACLK_TOP_MID:
		con = readl(&cru->clksel_con[10]);
		div = (con & ACLK_TOP_MID_DIV_MASK) >>
		      ACLK_TOP_MID_DIV_SHIFT;
		sel = (con & ACLK_TOP_MID_SEL_MASK) >>
		      ACLK_TOP_MID_SEL_SHIFT;
		if (sel == ACLK_TOP_MID_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = priv->gpll_hz;
		return DIV_TO_RATE(prate, div);
	case PCLK_TOP_ROOT:
		con = readl(&cru->clksel_con[8]);
		sel = (con & PCLK_TOP_SEL_MASK) >> PCLK_TOP_SEL_SHIFT;
		if (sel == PCLK_TOP_SEL_100M)
			rate = 100 * MHz;
		else if (sel == PCLK_TOP_SEL_50M)
			rate = 50 * MHz;
		else
			rate = OSC_HZ;
		break;
	case HCLK_TOP:
		con = readl(&cru->clksel_con[19]);
		sel = (con & HCLK_TOP_SEL_MASK) >> HCLK_TOP_SEL_SHIFT;
		if (sel == HCLK_TOP_SEL_200M)
			rate = 200 * MHz;
		else if (sel == HCLK_TOP_SEL_100M)
			rate = 100 * MHz;
		else if (sel == HCLK_TOP_SEL_50M)
			rate = 50 * MHz;
		else
			rate = OSC_HZ;
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong rk3576_top_set_clk(struct rk3576_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	int src_clk, src_clk_div;

	switch (clk_id) {
	case ACLK_TOP:
		if (!(priv->cpll_hz % rate)) {
			src_clk = ACLK_TOP_SEL_CPLL;
			src_clk_div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = ACLK_TOP_SEL_GPLL;
			src_clk_div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		assert(src_clk_div - 1 <= 31);
		rk_clrsetreg(&cru->clksel_con[9],
			     ACLK_TOP_DIV_MASK |
			     ACLK_TOP_SEL_MASK,
			     (src_clk <<
			      ACLK_TOP_SEL_SHIFT) |
			     (src_clk_div - 1) << ACLK_TOP_SEL_SHIFT);
		break;
	case ACLK_TOP_MID:
		if (!(priv->cpll_hz % rate)) {
			src_clk = ACLK_TOP_MID_SEL_CPLL;
			src_clk_div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = ACLK_TOP_MID_SEL_GPLL;
			src_clk_div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		rk_clrsetreg(&cru->clksel_con[10],
			     ACLK_TOP_MID_DIV_MASK |
			     ACLK_TOP_MID_SEL_MASK,
			     (ACLK_TOP_MID_SEL_GPLL <<
			      ACLK_TOP_MID_SEL_SHIFT) |
			     (src_clk_div - 1) << ACLK_TOP_MID_DIV_SHIFT);
		break;
	case PCLK_TOP_ROOT:
		if (rate >= 99 * MHz)
			src_clk = PCLK_TOP_SEL_100M;
		else if (rate >= 50 * MHz)
			src_clk = PCLK_TOP_SEL_50M;
		else
			src_clk = PCLK_TOP_SEL_OSC;
		rk_clrsetreg(&cru->clksel_con[8],
			     PCLK_TOP_SEL_MASK,
			     src_clk << PCLK_TOP_SEL_SHIFT);
		break;
	case HCLK_TOP:
		if (rate >= 198 * MHz)
			src_clk = HCLK_TOP_SEL_200M;
		else if (rate >= 99 * MHz)
			src_clk = HCLK_TOP_SEL_100M;
		else if (rate >= 50 * MHz)
			src_clk = HCLK_TOP_SEL_50M;
		else
			src_clk = HCLK_TOP_SEL_OSC;
		rk_clrsetreg(&cru->clksel_con[19],
			     HCLK_TOP_SEL_MASK,
			     src_clk << HCLK_TOP_SEL_SHIFT);
		break;
	default:
		printf("do not support this top freq\n");
		return -EINVAL;
	}

	return rk3576_top_get_clk(priv, clk_id);
}

static ulong rk3576_i2c_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 sel, con;
	ulong rate;

	switch (clk_id) {
	case CLK_I2C0:
		con = readl(&cru->pmuclksel_con[6]);
		sel = (con & CLK_I2C0_SEL_MASK) >> CLK_I2C0_SEL_SHIFT;
		break;
	case CLK_I2C1:
		con = readl(&cru->clksel_con[57]);
		sel = (con & CLK_I2C1_SEL_MASK) >> CLK_I2C1_SEL_SHIFT;
		break;
	case CLK_I2C2:
		con = readl(&cru->clksel_con[57]);
		sel = (con & CLK_I2C2_SEL_MASK) >> CLK_I2C2_SEL_SHIFT;
		break;
	case CLK_I2C3:
		con = readl(&cru->clksel_con[57]);
		sel = (con & CLK_I2C3_SEL_MASK) >> CLK_I2C3_SEL_SHIFT;
		break;
	case CLK_I2C4:
		con = readl(&cru->clksel_con[57]);
		sel = (con & CLK_I2C4_SEL_MASK) >> CLK_I2C4_SEL_SHIFT;
		break;
	case CLK_I2C5:
		con = readl(&cru->clksel_con[57]);
		sel = (con & CLK_I2C5_SEL_MASK) >> CLK_I2C5_SEL_SHIFT;
		break;
	case CLK_I2C6:
		con = readl(&cru->clksel_con[57]);
		sel = (con & CLK_I2C6_SEL_MASK) >> CLK_I2C6_SEL_SHIFT;
		break;
	case CLK_I2C7:
		con = readl(&cru->clksel_con[57]);
		sel = (con & CLK_I2C7_SEL_MASK) >> CLK_I2C7_SEL_SHIFT;
		break;
	case CLK_I2C8:
		con = readl(&cru->clksel_con[57]);
		sel = (con & CLK_I2C8_SEL_MASK) >> CLK_I2C8_SEL_SHIFT;
		break;
	case CLK_I2C9:
		con = readl(&cru->clksel_con[58]);
		sel = (con & CLK_I2C9_SEL_MASK) >> CLK_I2C9_SEL_SHIFT;
		break;

	default:
		return -ENOENT;
	}
	if (sel == CLK_I2C_SEL_200M)
		rate = 200 * MHz;
	else if (sel == CLK_I2C_SEL_100M)
		rate = 100 * MHz;
	else if (sel == CLK_I2C_SEL_50M)
		rate = 50 * MHz;
	else
		rate = OSC_HZ;

	return rate;
}

static ulong rk3576_i2c_set_clk(struct rk3576_clk_priv *priv, ulong clk_id,
				ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	int src_clk;

	if (rate >= 198 * MHz)
		src_clk = CLK_I2C_SEL_200M;
	else if (rate >= 99 * MHz)
		src_clk = CLK_I2C_SEL_100M;
	if (rate >= 50 * MHz)
		src_clk = CLK_I2C_SEL_50M;
	else
		src_clk = CLK_I2C_SEL_OSC;

	switch (clk_id) {
	case CLK_I2C0:
		rk_clrsetreg(&cru->pmuclksel_con[6], CLK_I2C0_SEL_MASK,
			     src_clk << CLK_I2C0_SEL_SHIFT);
		break;
	case CLK_I2C1:
		rk_clrsetreg(&cru->clksel_con[57], CLK_I2C1_SEL_MASK,
			     src_clk << CLK_I2C1_SEL_SHIFT);
		break;
	case CLK_I2C2:
		rk_clrsetreg(&cru->clksel_con[57], CLK_I2C2_SEL_MASK,
			     src_clk << CLK_I2C2_SEL_SHIFT);
		break;
	case CLK_I2C3:
		rk_clrsetreg(&cru->clksel_con[57], CLK_I2C3_SEL_MASK,
			     src_clk << CLK_I2C3_SEL_SHIFT);
		break;
	case CLK_I2C4:
		rk_clrsetreg(&cru->clksel_con[57], CLK_I2C4_SEL_MASK,
			     src_clk << CLK_I2C4_SEL_SHIFT);
		break;
	case CLK_I2C5:
		rk_clrsetreg(&cru->clksel_con[57], CLK_I2C5_SEL_MASK,
			     src_clk << CLK_I2C5_SEL_SHIFT);
		break;
	case CLK_I2C6:
		rk_clrsetreg(&cru->clksel_con[57], CLK_I2C6_SEL_MASK,
			     src_clk << CLK_I2C6_SEL_SHIFT);
		break;
	case CLK_I2C7:
		rk_clrsetreg(&cru->clksel_con[57], CLK_I2C7_SEL_MASK,
			     src_clk << CLK_I2C7_SEL_SHIFT);
		break;
	case CLK_I2C8:
		rk_clrsetreg(&cru->clksel_con[57], CLK_I2C8_SEL_MASK,
			     src_clk << CLK_I2C8_SEL_SHIFT);
	case CLK_I2C9:
		rk_clrsetreg(&cru->clksel_con[58], CLK_I2C9_SEL_MASK,
			     src_clk << CLK_I2C9_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3576_i2c_get_clk(priv, clk_id);
}

static ulong rk3576_spi_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 sel, con;

	switch (clk_id) {
	case CLK_SPI0:
		con = readl(&cru->clksel_con[70]);
		sel = (con & CLK_SPI0_SEL_MASK) >> CLK_SPI0_SEL_SHIFT;
		break;
	case CLK_SPI1:
		con = readl(&cru->clksel_con[71]);
		sel = (con & CLK_SPI1_SEL_MASK) >> CLK_SPI1_SEL_SHIFT;
		break;
	case CLK_SPI2:
		con = readl(&cru->clksel_con[71]);
		sel = (con & CLK_SPI2_SEL_MASK) >> CLK_SPI2_SEL_SHIFT;
		break;
	case CLK_SPI3:
		con = readl(&cru->clksel_con[71]);
		sel = (con & CLK_SPI3_SEL_MASK) >> CLK_SPI3_SEL_SHIFT;
		break;
	case CLK_SPI4:
		con = readl(&cru->clksel_con[71]);
		sel = (con & CLK_SPI4_SEL_MASK) >> CLK_SPI4_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	switch (sel) {
	case CLK_SPI_SEL_200M:
		return 200 * MHz;
	case CLK_SPI_SEL_100M:
		return 100 * MHz;
	case CLK_SPI_SEL_50M:
		return 50 * MHz;
	case CLK_SPI_SEL_OSC:
		return OSC_HZ;
	default:
		return -ENOENT;
	}
}

static ulong rk3576_spi_set_clk(struct rk3576_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	int src_clk;

	if (rate >= 198 * MHz)
		src_clk = CLK_SPI_SEL_200M;
	else if (rate >= 99 * MHz)
		src_clk = CLK_SPI_SEL_100M;
	else if (rate >= 50 * MHz)
		src_clk = CLK_SPI_SEL_50M;
	else
		src_clk = CLK_SPI_SEL_OSC;

	switch (clk_id) {
	case CLK_SPI0:
		rk_clrsetreg(&cru->clksel_con[70],
			     CLK_SPI0_SEL_MASK,
			     src_clk << CLK_SPI0_SEL_SHIFT);
		break;
	case CLK_SPI1:
		rk_clrsetreg(&cru->clksel_con[71],
			     CLK_SPI1_SEL_MASK,
			     src_clk << CLK_SPI1_SEL_SHIFT);
		break;
	case CLK_SPI2:
		rk_clrsetreg(&cru->clksel_con[71],
			     CLK_SPI2_SEL_MASK,
			     src_clk << CLK_SPI2_SEL_SHIFT);
		break;
	case CLK_SPI3:
		rk_clrsetreg(&cru->clksel_con[71],
			     CLK_SPI3_SEL_MASK,
			     src_clk << CLK_SPI3_SEL_SHIFT);
		break;
	case CLK_SPI4:
		rk_clrsetreg(&cru->clksel_con[71],
			     CLK_SPI4_SEL_MASK,
			     src_clk << CLK_SPI4_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3576_spi_get_clk(priv, clk_id);
}

static ulong rk3576_pwm_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 sel, con;

	switch (clk_id) {
	case CLK_PWM1:
		con = readl(&cru->clksel_con[71]);
		sel = (con & CLK_PWM1_SEL_MASK) >> CLK_PWM1_SEL_SHIFT;
		break;
	case CLK_PWM2:
		con = readl(&cru->clksel_con[74]);
		sel = (con & CLK_PWM2_SEL_MASK) >> CLK_PWM2_SEL_SHIFT;
		break;
	case CLK_PMU1PWM:
		con = readl(&cru->pmuclksel_con[5]);
		sel = (con & CLK_PMU1PWM_SEL_MASK) >> CLK_PMU1PWM_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	switch (sel) {
	case CLK_PWM_SEL_100M:
		return 100 * MHz;
	case CLK_PWM_SEL_50M:
		return 50 * MHz;
	case CLK_PWM_SEL_OSC:
		return OSC_HZ;
	default:
		return -ENOENT;
	}
}

static ulong rk3576_pwm_set_clk(struct rk3576_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	int src_clk;

	if (rate >= 99 * MHz)
		src_clk = CLK_PWM_SEL_100M;
	else if (rate >= 50 * MHz)
		src_clk = CLK_PWM_SEL_50M;
	else
		src_clk = CLK_PWM_SEL_OSC;

	switch (clk_id) {
	case CLK_PWM1:
		rk_clrsetreg(&cru->clksel_con[71],
			     CLK_PWM1_SEL_MASK,
			     src_clk << CLK_PWM1_SEL_SHIFT);
		break;
	case CLK_PWM2:
		rk_clrsetreg(&cru->clksel_con[74],
			     CLK_PWM2_SEL_MASK,
			     src_clk << CLK_PWM2_SEL_SHIFT);
		break;
	case CLK_PMU1PWM:
		rk_clrsetreg(&cru->pmuclksel_con[5],
			     CLK_PMU1PWM_SEL_MASK,
			     src_clk << CLK_PMU1PWM_SEL_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3576_pwm_get_clk(priv, clk_id);
}

static ulong rk3576_adc_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 div, sel, con, prate;

	switch (clk_id) {
	case CLK_SARADC:
		con = readl(&cru->clksel_con[58]);
		div = (con & CLK_SARADC_DIV_MASK) >> CLK_SARADC_DIV_SHIFT;
		sel = (con & CLK_SARADC_SEL_MASK) >>
		      CLK_SARADC_SEL_SHIFT;
		if (sel == CLK_SARADC_SEL_OSC)
			prate = OSC_HZ;
		else
			prate = priv->gpll_hz;
		return DIV_TO_RATE(prate, div);
	case CLK_TSADC:
		con = readl(&cru->clksel_con[59]);
		div = (con & CLK_TSADC_DIV_MASK) >>
		      CLK_TSADC_DIV_SHIFT;
		prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	default:
		return -ENOENT;
	}
}

static ulong rk3576_adc_set_clk(struct rk3576_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	int src_clk_div;

	switch (clk_id) {
	case CLK_SARADC:
		if (!(OSC_HZ % rate)) {
			src_clk_div = DIV_ROUND_UP(OSC_HZ, rate);
			assert(src_clk_div - 1 <= 255);
			rk_clrsetreg(&cru->clksel_con[58],
				     CLK_SARADC_SEL_MASK |
				     CLK_SARADC_DIV_MASK,
				     (CLK_SARADC_SEL_OSC <<
				      CLK_SARADC_SEL_SHIFT) |
				     (src_clk_div - 1) <<
				     CLK_SARADC_DIV_SHIFT);
		} else {
			src_clk_div = DIV_ROUND_UP(priv->gpll_hz, rate);
			assert(src_clk_div - 1 <= 255);
			rk_clrsetreg(&cru->clksel_con[59],
				     CLK_SARADC_SEL_MASK |
				     CLK_SARADC_DIV_MASK,
				     (CLK_SARADC_SEL_GPLL <<
				      CLK_SARADC_SEL_SHIFT) |
				     (src_clk_div - 1) <<
				     CLK_SARADC_DIV_SHIFT);
		}
		break;
	case CLK_TSADC:
		src_clk_div = DIV_ROUND_UP(OSC_HZ, rate);
		assert(src_clk_div - 1 <= 255);
		rk_clrsetreg(&cru->clksel_con[58],
			     CLK_TSADC_DIV_MASK,
			     (src_clk_div - 1) <<
			     CLK_TSADC_DIV_SHIFT);
		break;
	default:
		return -ENOENT;
	}
	return rk3576_adc_get_clk(priv, clk_id);
}

static ulong rk3576_mmc_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 sel, con, prate, div = 0;

	switch (clk_id) {
	case CCLK_SRC_SDIO:
	case HCLK_SDIO:
		con = readl(&cru->clksel_con[104]);
		div = (con & CCLK_SDIO_SRC_DIV_MASK) >> CCLK_SDIO_SRC_DIV_SHIFT;
		sel = (con & CCLK_SDIO_SRC_SEL_MASK) >>
		      CCLK_SDIO_SRC_SEL_SHIFT;
		if (sel == CCLK_SDIO_SRC_SEL_GPLL)
			prate = priv->gpll_hz;
		else if (sel == CCLK_SDIO_SRC_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case CCLK_SRC_SDMMC0:
	case HCLK_SDMMC0:
		con = readl(&cru->clksel_con[105]);
		div = (con & CCLK_SDMMC0_SRC_DIV_MASK) >> CCLK_SDMMC0_SRC_DIV_SHIFT;
		sel = (con & CCLK_SDMMC0_SRC_SEL_MASK) >>
		      CCLK_SDMMC0_SRC_SEL_SHIFT;
		if (sel == CCLK_SDMMC0_SRC_SEL_GPLL)
			prate = priv->gpll_hz;
		else if (sel == CCLK_SDMMC0_SRC_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case CCLK_SRC_EMMC:
	case HCLK_EMMC:
		con = readl(&cru->clksel_con[89]);
		div = (con & CCLK_EMMC_DIV_MASK) >> CCLK_EMMC_DIV_SHIFT;
		sel = (con & CCLK_EMMC_SEL_MASK) >>
		      CCLK_EMMC_SEL_SHIFT;
		if (sel == CCLK_EMMC_SEL_GPLL)
			prate = priv->gpll_hz;
		else if (sel == CCLK_EMMC_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case BCLK_EMMC:
		con = readl(&cru->clksel_con[90]);
		sel = (con & BCLK_EMMC_SEL_MASK) >>
		      BCLK_EMMC_SEL_SHIFT;
		if (sel == BCLK_EMMC_SEL_200M)
			prate = 200 * MHz;
		else if (sel == BCLK_EMMC_SEL_100M)
			prate = 100 * MHz;
		else if (sel == BCLK_EMMC_SEL_50M)
			prate = 50 * MHz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case SCLK_FSPI_X2:
		con = readl(&cru->clksel_con[89]);
		div = (con & SCLK_FSPI_DIV_MASK) >> SCLK_FSPI_DIV_SHIFT;
		sel = (con & SCLK_FSPI_SEL_MASK) >>
		      SCLK_FSPI_SEL_SHIFT;
		if (sel == SCLK_FSPI_SEL_GPLL)
			prate = priv->gpll_hz;
		else if (sel == SCLK_FSPI_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case SCLK_FSPI1_X2:
		con = readl(&cru->clksel_con[106]);
		div = (con & SCLK_FSPI_DIV_MASK) >> SCLK_FSPI_DIV_SHIFT;
		sel = (con & SCLK_FSPI_SEL_MASK) >>
		      SCLK_FSPI_SEL_SHIFT;
		if (sel == SCLK_FSPI_SEL_GPLL)
			prate = priv->gpll_hz;
		else if (sel == SCLK_FSPI_SEL_CPLL)
			prate = priv->cpll_hz;
		else
			prate = OSC_HZ;
		return DIV_TO_RATE(prate, div);
	case DCLK_DECOM:
		con = readl(&cru->clksel_con[72]);
		div = (con & DCLK_DECOM_DIV_MASK) >> DCLK_DECOM_DIV_SHIFT;
		sel = (con & DCLK_DECOM_SEL_MASK) >> DCLK_DECOM_SEL_SHIFT;
		if (sel == DCLK_DECOM_SEL_SPLL)
			prate = priv->spll_hz;
		else
			prate = priv->gpll_hz;
		return DIV_TO_RATE(prate, div);

	default:
		return -ENOENT;
	}
}

static ulong rk3576_mmc_set_clk(struct rk3576_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	int src_clk, div = 0;

	switch (clk_id) {
	case CCLK_SRC_SDIO:
	case CCLK_SRC_SDMMC0:
	case CCLK_SRC_EMMC:
	case SCLK_FSPI_X2:
	case SCLK_FSPI1_X2:
	case HCLK_SDMMC0:
	case HCLK_EMMC:
	case HCLK_SDIO:
		if (!(OSC_HZ % rate)) {
			src_clk = SCLK_FSPI_SEL_OSC;
			div = DIV_ROUND_UP(OSC_HZ, rate);
		} else if (!(priv->cpll_hz % rate)) {
			src_clk = SCLK_FSPI_SEL_CPLL;
			div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = SCLK_FSPI_SEL_GPLL;
			div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		break;
	case BCLK_EMMC:
		if (rate >= 198 * MHz)
			src_clk = BCLK_EMMC_SEL_200M;
		else if (rate >= 99 * MHz)
			src_clk = BCLK_EMMC_SEL_100M;
		else if (rate >= 50 * MHz)
			src_clk = BCLK_EMMC_SEL_50M;
		else
			src_clk = BCLK_EMMC_SEL_OSC;
		break;
	case DCLK_DECOM:
		if (!(priv->spll_hz % rate)) {
			src_clk = DCLK_DECOM_SEL_SPLL;
			div = DIV_ROUND_UP(priv->spll_hz, rate);
		} else {
			src_clk = DCLK_DECOM_SEL_GPLL;
			div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		break;
	default:
		return -ENOENT;
	}

	switch (clk_id) {
	case CCLK_SRC_SDIO:
	case HCLK_SDIO:
		rk_clrsetreg(&cru->clksel_con[104],
			     CCLK_SDIO_SRC_SEL_MASK |
			     CCLK_SDIO_SRC_DIV_MASK,
			     (src_clk << CCLK_SDIO_SRC_SEL_SHIFT) |
			     (div - 1) << CCLK_SDIO_SRC_DIV_SHIFT);
		break;
	case CCLK_SRC_SDMMC0:
	case HCLK_SDMMC0:
		rk_clrsetreg(&cru->clksel_con[105],
			     CCLK_SDMMC0_SRC_SEL_MASK |
			     CCLK_SDMMC0_SRC_DIV_MASK,
			     (src_clk << CCLK_SDMMC0_SRC_SEL_SHIFT) |
			     (div - 1) << CCLK_SDMMC0_SRC_DIV_SHIFT);
		break;
	case CCLK_SRC_EMMC:
	case HCLK_EMMC:
		rk_clrsetreg(&cru->clksel_con[89],
			     CCLK_EMMC_DIV_MASK |
			     CCLK_EMMC_SEL_MASK,
			     (src_clk << CCLK_EMMC_SEL_SHIFT) |
			     (div - 1) << CCLK_EMMC_DIV_SHIFT);
		break;
	case SCLK_FSPI_X2:
		rk_clrsetreg(&cru->clksel_con[89],
			     SCLK_FSPI_DIV_MASK |
			     SCLK_FSPI_SEL_MASK,
			     (src_clk << SCLK_FSPI_SEL_SHIFT) |
			     (div - 1) << SCLK_FSPI_DIV_SHIFT);
		break;
	case SCLK_FSPI1_X2:
		rk_clrsetreg(&cru->clksel_con[106],
			     SCLK_FSPI_DIV_MASK |
			     SCLK_FSPI_SEL_MASK,
			     (src_clk << SCLK_FSPI_SEL_SHIFT) |
			     (div - 1) << SCLK_FSPI_DIV_SHIFT);
		break;
	case BCLK_EMMC:
		rk_clrsetreg(&cru->clksel_con[90],
			     BCLK_EMMC_SEL_MASK,
			     src_clk << BCLK_EMMC_SEL_SHIFT);
		break;
	case DCLK_DECOM:
		rk_clrsetreg(&cru->clksel_con[72],
			     DCLK_DECOM_DIV_MASK |
			     DCLK_DECOM_SEL_MASK,
			     (src_clk << DCLK_DECOM_SEL_SHIFT) |
			     (div - 1) << DCLK_DECOM_DIV_SHIFT);
		break;

	default:
		return -ENOENT;
	}

	return rk3576_mmc_get_clk(priv, clk_id);
}

#ifndef CONFIG_SPL_BUILD

static ulong rk3576_aclk_vop_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 div, sel, con, parent = 0;

	switch (clk_id) {
	case ACLK_VOP_ROOT:
	case ACLK_VOP:
		con = readl(&cru->clksel_con[144]);
		div = (con & ACLK_VOP_ROOT_DIV_MASK) >> ACLK_VOP_ROOT_DIV_SHIFT;
		sel = (con & ACLK_VOP_ROOT_SEL_MASK) >> ACLK_VOP_ROOT_SEL_SHIFT;
		if (sel == ACLK_VOP_ROOT_SEL_GPLL)
			parent = priv->gpll_hz;
		else if (sel == ACLK_VOP_ROOT_SEL_CPLL)
			parent = priv->cpll_hz;
		else if (sel == ACLK_VOP_ROOT_SEL_AUPLL)
			parent = priv->aupll_hz;
		else if (sel == ACLK_VOP_ROOT_SEL_SPLL)
			parent = priv->spll_hz;
		else if (sel == ACLK_VOP_ROOT_SEL_LPLL)
			parent = priv->lpll_hz / 2;
		return DIV_TO_RATE(parent, div);
	case ACLK_VO0_ROOT:
		con = readl(&cru->clksel_con[149]);
		div = (con & ACLK_VO0_ROOT_DIV_MASK) >> ACLK_VO0_ROOT_DIV_SHIFT;
		sel = (con & ACLK_VO0_ROOT_SEL_MASK) >> ACLK_VO0_ROOT_SEL_SHIFT;
		if (sel == ACLK_VO0_ROOT_SEL_GPLL)
			parent = priv->gpll_hz;
		else if (sel == ACLK_VO0_ROOT_SEL_CPLL)
			parent = priv->cpll_hz;
		else if (sel == ACLK_VO0_ROOT_SEL_LPLL)
			parent = priv->lpll_hz / 2;
		else if (sel == ACLK_VO0_ROOT_SEL_BPLL)
			parent = priv->bpll_hz / 4;
		return DIV_TO_RATE(parent, div);
	case ACLK_VO1_ROOT:
		con = readl(&cru->clksel_con[158]);
		div = (con & ACLK_VO0_ROOT_DIV_MASK) >> ACLK_VO0_ROOT_DIV_SHIFT;
		sel = (con & ACLK_VO0_ROOT_SEL_MASK) >> ACLK_VO0_ROOT_SEL_SHIFT;
		if (sel == ACLK_VO0_ROOT_SEL_GPLL)
			parent = priv->gpll_hz;
		else if (sel == ACLK_VO0_ROOT_SEL_CPLL)
			parent = priv->cpll_hz;
		else if (sel == ACLK_VO0_ROOT_SEL_LPLL)
			parent = priv->lpll_hz / 2;
		else if (sel == ACLK_VO0_ROOT_SEL_BPLL)
			parent = priv->bpll_hz / 4;
		return DIV_TO_RATE(parent, div);
	case HCLK_VOP_ROOT:
		con = readl(&cru->clksel_con[144]);
		sel = (con & HCLK_VOP_ROOT_SEL_MASK) >> HCLK_VOP_ROOT_SEL_SHIFT;
		if (sel == HCLK_VOP_ROOT_SEL_200M)
			return 200 * MHz;
		else if (sel == HCLK_VOP_ROOT_SEL_100M)
			return 100 * MHz;
		else if (sel == HCLK_VOP_ROOT_SEL_50M)
			return 50 * MHz;
		else
			return OSC_HZ;
	case PCLK_VOP_ROOT:
		con = readl(&cru->clksel_con[144]);
		sel = (con & PCLK_VOP_ROOT_SEL_MASK) >> PCLK_VOP_ROOT_SEL_SHIFT;
		if (sel == PCLK_VOP_ROOT_SEL_100M)
			return 100 * MHz;
		else if (sel == PCLK_VOP_ROOT_SEL_50M)
			return 50 * MHz;
		else
			return OSC_HZ;

	default:
		return -ENOENT;
	}
}

static ulong rk3576_aclk_vop_set_clk(struct rk3576_clk_priv *priv,
				     ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	int src_clk, div;

	switch (clk_id) {
	case ACLK_VOP_ROOT:
	case ACLK_VOP:
		if (rate == 700 * MHz) {
			src_clk = ACLK_VOP_ROOT_SEL_SPLL;
			div = 1;
		} else if (!(priv->cpll_hz % rate)) {
			src_clk = ACLK_VOP_ROOT_SEL_CPLL;
			div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = ACLK_VOP_ROOT_SEL_GPLL;
			div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		rk_clrsetreg(&cru->clksel_con[144],
			     ACLK_VOP_ROOT_DIV_MASK |
			     ACLK_VOP_ROOT_SEL_MASK,
			     (src_clk << ACLK_VOP_ROOT_SEL_SHIFT) |
			     (div - 1) << ACLK_VOP_ROOT_DIV_SHIFT);
		break;
	case ACLK_VO0_ROOT:
		if (!(priv->cpll_hz % rate)) {
			src_clk = ACLK_VO0_ROOT_SEL_CPLL;
			div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = ACLK_VO0_ROOT_SEL_GPLL;
			div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		rk_clrsetreg(&cru->clksel_con[149],
			     ACLK_VO0_ROOT_DIV_MASK |
			     ACLK_VO0_ROOT_SEL_MASK,
			     (src_clk << ACLK_VO0_ROOT_SEL_SHIFT) |
			     (div - 1) << ACLK_VO0_ROOT_DIV_SHIFT);
		break;
	case ACLK_VO1_ROOT:
		if (!(priv->cpll_hz % rate)) {
			src_clk = ACLK_VO0_ROOT_SEL_CPLL;
			div = DIV_ROUND_UP(priv->cpll_hz, rate);
		} else {
			src_clk = ACLK_VO0_ROOT_SEL_GPLL;
			div = DIV_ROUND_UP(priv->gpll_hz, rate);
		}
		rk_clrsetreg(&cru->clksel_con[158],
			     ACLK_VO0_ROOT_DIV_MASK |
			     ACLK_VO0_ROOT_SEL_MASK,
			     (src_clk << ACLK_VO0_ROOT_SEL_SHIFT) |
			     (div - 1) << ACLK_VO0_ROOT_DIV_SHIFT);
		break;
	case HCLK_VOP_ROOT:
		if (rate == 200 * MHz)
			src_clk = HCLK_VOP_ROOT_SEL_200M;
		else if (rate == 100 * MHz)
			src_clk = HCLK_VOP_ROOT_SEL_100M;
		else if (rate == 50 * MHz)
			src_clk = HCLK_VOP_ROOT_SEL_50M;
		else
			src_clk = HCLK_VOP_ROOT_SEL_OSC;
		rk_clrsetreg(&cru->clksel_con[144],
			     HCLK_VOP_ROOT_SEL_MASK,
			     src_clk << HCLK_VOP_ROOT_SEL_SHIFT);
		break;
	case PCLK_VOP_ROOT:
		if (rate == 100 * MHz)
			src_clk = PCLK_VOP_ROOT_SEL_100M;
		else if (rate == 50 * MHz)
			src_clk = PCLK_VOP_ROOT_SEL_50M;
		else
			src_clk = PCLK_VOP_ROOT_SEL_OSC;
		rk_clrsetreg(&cru->clksel_con[144],
			     PCLK_VOP_ROOT_SEL_MASK,
			     src_clk << PCLK_VOP_ROOT_SEL_SHIFT);
		break;

	default:
		return -ENOENT;
	}

	return rk3576_aclk_vop_get_clk(priv, clk_id);
}

static ulong rk3576_dclk_vop_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 div, sel, con, parent;

	switch (clk_id) {
	case DCLK_VP0:
	case DCLK_VP0_SRC:
		con = readl(&cru->clksel_con[145]);
		div = (con & DCLK0_VOP_SRC_DIV_MASK) >> DCLK0_VOP_SRC_DIV_SHIFT;
		sel = (con & DCLK0_VOP_SRC_SEL_MASK) >> DCLK0_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VP1:
	case DCLK_VP1_SRC:
		con = readl(&cru->clksel_con[146]);
		div = (con & DCLK0_VOP_SRC_DIV_MASK) >> DCLK0_VOP_SRC_DIV_SHIFT;
		sel = (con & DCLK0_VOP_SRC_SEL_MASK) >> DCLK0_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VP2:
	case DCLK_VP2_SRC:
		con = readl(&cru->clksel_con[147]);
		div = (con & DCLK0_VOP_SRC_DIV_MASK) >> DCLK0_VOP_SRC_DIV_SHIFT;
		sel = (con & DCLK0_VOP_SRC_SEL_MASK) >> DCLK0_VOP_SRC_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == DCLK_VOP_SRC_SEL_VPLL)
		parent = priv->vpll_hz;
	else if (sel == DCLK_VOP_SRC_SEL_BPLL)
		parent = priv->bpll_hz / 4;
	else if (sel == DCLK_VOP_SRC_SEL_LPLL)
		parent = priv->lpll_hz / 2;
	else if (sel == DCLK_VOP_SRC_SEL_GPLL)
		parent = priv->gpll_hz;
	else
		parent = priv->cpll_hz;

	return DIV_TO_RATE(parent, div);
}

#define RK3576_VOP_PLL_LIMIT_FREQ 600000000

static ulong rk3576_dclk_vop_set_clk(struct rk3576_clk_priv *priv,
				     ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	ulong pll_rate, now, best_rate = 0;
	u32 i, conid, con, sel, div, best_div = 0, best_sel = 0;
	u32 mask, div_shift, sel_shift;

	switch (clk_id) {
	case DCLK_VP0:
	case DCLK_VP0_SRC:
		conid = 145;
		con = readl(&cru->clksel_con[conid]);
		sel = (con & DCLK0_VOP_SRC_SEL_MASK) >> DCLK0_VOP_SRC_SEL_SHIFT;
		mask = DCLK0_VOP_SRC_SEL_MASK | DCLK0_VOP_SRC_DIV_MASK;
		div_shift = DCLK0_VOP_SRC_DIV_SHIFT;
		sel_shift = DCLK0_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VP1:
	case DCLK_VP1_SRC:
		conid = 146;
		con = readl(&cru->clksel_con[conid]);
		sel = (con & DCLK0_VOP_SRC_SEL_MASK) >> DCLK0_VOP_SRC_SEL_SHIFT;
		mask = DCLK0_VOP_SRC_SEL_MASK | DCLK0_VOP_SRC_DIV_MASK;
		div_shift = DCLK0_VOP_SRC_DIV_SHIFT;
		sel_shift = DCLK0_VOP_SRC_SEL_SHIFT;
		break;
	case DCLK_VP2:
	case DCLK_VP2_SRC:
		conid = 147;
		con = readl(&cru->clksel_con[conid]);
		sel = (con & DCLK0_VOP_SRC_SEL_MASK) >> DCLK0_VOP_SRC_SEL_SHIFT;
		mask = DCLK0_VOP_SRC_SEL_MASK | DCLK0_VOP_SRC_DIV_MASK;
		div_shift = DCLK0_VOP_SRC_DIV_SHIFT;
		sel_shift = DCLK0_VOP_SRC_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == DCLK_VOP_SRC_SEL_VPLL) {
		pll_rate = rockchip_pll_get_rate(&rk3576_pll_clks[VPLL],
						 priv->cru, VPLL);
		if (pll_rate >= RK3576_VOP_PLL_LIMIT_FREQ && pll_rate % rate == 0) {
			div = DIV_ROUND_UP(pll_rate, rate);
			rk_clrsetreg(&cru->clksel_con[conid],
				     mask,
				     DCLK_VOP_SRC_SEL_VPLL << sel_shift |
				     ((div - 1) << div_shift));
		} else {
			div = DIV_ROUND_UP(RK3576_VOP_PLL_LIMIT_FREQ, rate);
			if (div % 2)
				div = div + 1;
			rk_clrsetreg(&cru->clksel_con[conid],
				     mask,
				     DCLK_VOP_SRC_SEL_VPLL << sel_shift |
				     ((div - 1) << div_shift));
			rockchip_pll_set_rate(&rk3576_pll_clks[VPLL],
					      priv->cru, VPLL, div * rate);
			priv->vpll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[VPLL],
							      priv->cru, VPLL);
		}
	} else {
		for (i = 0; i <= DCLK_VOP_SRC_SEL_LPLL; i++) {
			switch (i) {
			case DCLK_VOP_SRC_SEL_GPLL:
				pll_rate = priv->gpll_hz;
				break;
			case DCLK_VOP_SRC_SEL_CPLL:
				pll_rate = priv->cpll_hz;
				break;
			case DCLK_VOP_SRC_SEL_BPLL:
				pll_rate = 0;
				break;
			case DCLK_VOP_SRC_SEL_LPLL:
				pll_rate = 0;
				break;
			case DCLK_VOP_SRC_SEL_VPLL:
				pll_rate = 0;
				break;
			default:
				printf("do not support this vop pll sel\n");
				return -EINVAL;
			}

			div = DIV_ROUND_UP(pll_rate, rate);
			if (div > 255)
				continue;
			now = pll_rate / div;
			if (abs(rate - now) < abs(rate - best_rate)) {
				best_rate = now;
				best_div = div;
				best_sel = i;
			}
			debug("p_rate=%lu, best_rate=%lu, div=%u, sel=%u\n",
			      pll_rate, best_rate, best_div, best_sel);
		}

		if (best_rate) {
			rk_clrsetreg(&cru->clksel_con[conid],
				     mask,
				     best_sel << sel_shift |
				     (best_div - 1) << div_shift);
		} else {
			printf("do not support this vop freq %lu\n", rate);
			return -EINVAL;
		}
	}

	return rk3576_dclk_vop_get_clk(priv, clk_id);
}

static ulong rk3576_clk_csihost_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 div, sel, con, parent;

	switch (clk_id) {
	case CLK_DSIHOST0:
		con = readl(&cru->clksel_con[151]);
		div = (con & CLK_DSIHOST0_DIV_MASK) >> CLK_DSIHOST0_DIV_SHIFT;
		sel = (con & CLK_DSIHOST0_SEL_MASK) >> CLK_DSIHOST0_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	if (sel == CLK_DSIHOST0_SEL_VPLL)
		parent = priv->vpll_hz;
	else if (sel == CLK_DSIHOST0_SEL_BPLL)
		parent = priv->bpll_hz / 4;
	else if (sel == CLK_DSIHOST0_SEL_LPLL)
		parent = priv->lpll_hz / 2;
	else if (sel == CLK_DSIHOST0_SEL_GPLL)
		parent = priv->gpll_hz;
	else if (sel == CLK_DSIHOST0_SEL_SPLL)
		parent = priv->spll_hz;
	else
		parent = priv->cpll_hz;

	return DIV_TO_RATE(parent, div);
}

static ulong rk3576_clk_csihost_set_clk(struct rk3576_clk_priv *priv,
					ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	ulong pll_rate, now, best_rate = 0;
	u32 i, con, div, best_div = 0, best_sel = 0;
	u32 mask, div_shift, sel_shift;

	switch (clk_id) {
	case CLK_DSIHOST0:
		con = 151;
		mask = CLK_DSIHOST0_SEL_MASK | CLK_DSIHOST0_DIV_MASK;
		div_shift = CLK_DSIHOST0_DIV_SHIFT;
		sel_shift = CLK_DSIHOST0_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}
	for (i = 0; i <= CLK_DSIHOST0_SEL_LPLL; i++) {
		switch (i) {
		case CLK_DSIHOST0_SEL_GPLL:
			pll_rate = priv->gpll_hz;
			break;
		case CLK_DSIHOST0_SEL_CPLL:
			pll_rate = priv->cpll_hz;
			break;
		case CLK_DSIHOST0_SEL_BPLL:
			pll_rate = 0;
			break;
		case CLK_DSIHOST0_SEL_LPLL:
			pll_rate = 0;
			break;
		case CLK_DSIHOST0_SEL_VPLL:
			pll_rate = 0;
			break;
		case CLK_DSIHOST0_SEL_SPLL:
			pll_rate = priv->spll_hz;
			break;
		default:
			printf("do not support this vop pll sel\n");
			return -EINVAL;
		}

		div = DIV_ROUND_UP(pll_rate, rate);
		if (div > 255)
			continue;
		now = pll_rate / div;
		if (abs(rate - now) < abs(rate - best_rate)) {
			best_rate = now;
			best_div = div;
			best_sel = i;
		}
		debug("p_rate=%lu, best_rate=%lu, div=%u, sel=%u\n",
		      pll_rate, best_rate, best_div, best_sel);
	}

	if (best_rate) {
		rk_clrsetreg(&cru->clksel_con[con],
			     mask,
			     best_sel << sel_shift |
			     (best_div - 1) << div_shift);
	} else {
		printf("do not support this vop freq %lu\n", rate);
		return -EINVAL;
	}

	return rk3576_clk_csihost_get_clk(priv, clk_id);
}

static ulong rk3576_dclk_ebc_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 div, sel, con, parent;
	unsigned long m = 0, n = 0;

	switch (clk_id) {
	case DCLK_EBC:
		con = readl(&cru->clksel_con[123]);
		div = (con & DCLK_EBC_DIV_MASK) >> DCLK_EBC_DIV_SHIFT;
		sel = (con & DCLK_EBC_SEL_MASK) >> DCLK_EBC_SEL_SHIFT;
		if (sel == DCLK_EBC_SEL_CPLL)
			parent = priv->cpll_hz;
		else if (sel == DCLK_EBC_SEL_VPLL)
			parent = priv->vpll_hz;
		else if (sel == DCLK_EBC_SEL_AUPLL)
			parent = priv->aupll_hz;
		else if (sel == DCLK_EBC_SEL_LPLL)
			parent = priv->lpll_hz / 2;
		else if (sel == DCLK_EBC_SEL_GPLL)
			parent = priv->gpll_hz;
		else if (sel == DCLK_EBC_SEL_FRAC_SRC)
			parent = rk3576_dclk_ebc_get_clk(priv, DCLK_EBC_FRAC_SRC);
		else
			parent = OSC_HZ;
		return DIV_TO_RATE(parent, div);
	case DCLK_EBC_FRAC_SRC:
		con = readl(&cru->clksel_con[123]);
		div = readl(&cru->clksel_con[122]);
		sel = (con & DCLK_EBC_FRAC_SRC_SEL_MASK) >> DCLK_EBC_FRAC_SRC_SEL_SHIFT;
		if (sel == DCLK_EBC_FRAC_SRC_SEL_GPLL)
			parent = priv->gpll_hz;
		else if (sel == DCLK_EBC_FRAC_SRC_SEL_CPLL)
			parent = priv->cpll_hz;
		else if (sel == DCLK_EBC_FRAC_SRC_SEL_VPLL)
			parent = priv->vpll_hz;
		else if (sel == DCLK_EBC_FRAC_SRC_SEL_AUPLL)
			parent = priv->aupll_hz;
		else
			parent = OSC_HZ;

		n = div & CLK_UART_FRAC_NUMERATOR_MASK;
		n >>= CLK_UART_FRAC_NUMERATOR_SHIFT;
		m = div & CLK_UART_FRAC_DENOMINATOR_MASK;
		m >>= CLK_UART_FRAC_DENOMINATOR_SHIFT;
		return parent * n / m;
	default:
		return -ENOENT;
	}
}

static ulong rk3576_dclk_ebc_set_clk(struct rk3576_clk_priv *priv,
				     ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	ulong pll_rate, now, best_rate = 0;
	u32 i, con, sel, div, best_div = 0, best_sel = 0;
	unsigned long m = 0, n = 0, val;

	switch (clk_id) {
	case DCLK_EBC:
		con = readl(&cru->clksel_con[123]);
		sel = (con & DCLK_EBC_SEL_MASK) >> DCLK_EBC_SEL_SHIFT;
		if (sel == DCLK_EBC_SEL_VPLL) {
			pll_rate = rockchip_pll_get_rate(&rk3576_pll_clks[VPLL],
							 priv->cru, VPLL);
			if (pll_rate >= RK3576_VOP_PLL_LIMIT_FREQ &&
			    pll_rate % rate == 0) {
				div = DIV_ROUND_UP(pll_rate, rate);
				rk_clrsetreg(&cru->clksel_con[123],
					     DCLK_EBC_DIV_MASK,
					     (div - 1) << DCLK_EBC_DIV_SHIFT);
			} else {
				div = DIV_ROUND_UP(RK3576_VOP_PLL_LIMIT_FREQ,
						   rate);
				if (div % 2)
					div = div + 1;
				rk_clrsetreg(&cru->clksel_con[123],
					     DCLK_EBC_DIV_MASK,
					     (div - 1) << DCLK_EBC_DIV_SHIFT);
				rockchip_pll_set_rate(&rk3576_pll_clks[VPLL],
						      priv->cru,
						      VPLL, div * rate);
				priv->vpll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[VPLL],
								      priv->cru,
								      VPLL);
			}
		} else if (sel == DCLK_EBC_SEL_FRAC_SRC) {
			rk3576_dclk_ebc_set_clk(priv, DCLK_EBC_FRAC_SRC, rate);
			div = rk3576_dclk_ebc_get_clk(priv, DCLK_EBC_FRAC_SRC) / rate;
			rk_clrsetreg(&cru->clksel_con[123],
				     DCLK_EBC_DIV_MASK,
				     (div - 1) << DCLK_EBC_DIV_SHIFT);
		} else {
			for (i = 0; i <= DCLK_EBC_SEL_LPLL; i++) {
				switch (i) {
				case DCLK_EBC_SEL_GPLL:
					pll_rate = priv->gpll_hz;
					break;
				case DCLK_EBC_SEL_CPLL:
					pll_rate = priv->cpll_hz;
					break;
				case DCLK_EBC_SEL_VPLL:
					pll_rate = 0;
					break;
				case DCLK_EBC_SEL_AUPLL:
					pll_rate = priv->aupll_hz;
					break;
				case DCLK_EBC_SEL_LPLL:
					pll_rate = 0;
					break;
				default:
					printf("not support ebc pll sel\n");
					return -EINVAL;
				}

				div = DIV_ROUND_UP(pll_rate, rate);
				if (div > 255)
					continue;
				now = pll_rate / div;
				if (abs(rate - now) < abs(rate - best_rate)) {
					best_rate = now;
					best_div = div;
					best_sel = i;
				}
			}

			if (best_rate) {
				rk_clrsetreg(&cru->clksel_con[123],
					     DCLK_EBC_DIV_MASK |
					     DCLK_EBC_SEL_MASK,
					     best_sel <<
					     DCLK_EBC_SEL_SHIFT |
					     (best_div - 1) <<
					     DCLK_EBC_DIV_SHIFT);
			} else {
				printf("do not support this vop freq %lu\n",
				       rate);
				return -EINVAL;
			}
		}
		break;
	case DCLK_EBC_FRAC_SRC:
		sel = DCLK_EBC_FRAC_SRC_SEL_GPLL;
		div = 1;
		rational_best_approximation(rate, priv->gpll_hz,
					    GENMASK(16 - 1, 0),
					    GENMASK(16 - 1, 0),
					    &m, &n);

		if (m < 4 && m != 0) {
			if (n % 2 == 0)
				val = 1;
			else
				val = DIV_ROUND_UP(4, m);

			n *= val;
			m *= val;
			if (n > 0xffff)
				n = 0xffff;
		}

		rk_clrsetreg(&cru->clksel_con[123],
			     DCLK_EBC_FRAC_SRC_SEL_MASK,
			     (sel << DCLK_EBC_FRAC_SRC_SEL_SHIFT));
		if (m && n) {
			val = m << CLK_UART_FRAC_NUMERATOR_SHIFT | n;
			writel(val, &cru->clksel_con[122]);
		}
		break;
	default:
		return -ENOENT;
	}
	return rk3576_dclk_ebc_get_clk(priv, clk_id);
}

static ulong rk3576_gmac_get_clk(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 con, div, src, p_rate;

	switch (clk_id) {
	case CLK_GMAC0_PTP_REF_SRC:
	case CLK_GMAC0_PTP_REF:
		con = readl(&cru->clksel_con[105]);
		div = (con & CLK_GMAC0_PTP_DIV_MASK) >> CLK_GMAC0_PTP_DIV_SHIFT;
		src = (con & CLK_GMAC0_PTP_SEL_MASK) >> CLK_GMAC0_PTP_SEL_SHIFT;
		if (src == CLK_GMAC0_PTP_SEL_GPLL)
			p_rate = priv->gpll_hz;
		else if (src == CLK_GMAC0_PTP_SEL_CPLL)
			p_rate = priv->cpll_hz;
		else
			p_rate = GMAC0_PTP_REFCLK_IN;
		return DIV_TO_RATE(p_rate, div);
	case CLK_GMAC1_PTP_REF_SRC:
	case CLK_GMAC1_PTP_REF:
		con = readl(&cru->clksel_con[104]);
		div = (con & CLK_GMAC1_PTP_DIV_MASK) >> CLK_GMAC0_PTP_DIV_SHIFT;
		src = (con & CLK_GMAC1_PTP_SEL_MASK) >> CLK_GMAC1_PTP_SEL_SHIFT;
		if (src == CLK_GMAC1_PTP_SEL_GPLL)
			p_rate = priv->gpll_hz;
		else if (src == CLK_GMAC1_PTP_SEL_CPLL)
			p_rate = priv->cpll_hz;
		else
			p_rate = GMAC1_PTP_REFCLK_IN;
		return DIV_TO_RATE(p_rate, div);
	case CLK_GMAC0_125M_SRC:
		con = readl(&cru->clksel_con[30]);
		div = (con & CLK_GMAC0_125M_DIV_MASK) >> CLK_GMAC0_125M_DIV_SHIFT;
		return DIV_TO_RATE(priv->cpll_hz, div);
	case CLK_GMAC1_125M_SRC:
		con = readl(&cru->clksel_con[31]);
		div = (con & CLK_GMAC1_125M_DIV_MASK) >> CLK_GMAC1_125M_DIV_SHIFT;
		return DIV_TO_RATE(priv->cpll_hz, div);
	default:
		return -ENOENT;
	}
}

static ulong rk3576_gmac_set_clk(struct rk3576_clk_priv *priv,
				 ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	int div, src;

	div = DIV_ROUND_UP(priv->cpll_hz, rate);

	switch (clk_id) {
	case CLK_GMAC0_PTP_REF_SRC:
	case CLK_GMAC0_PTP_REF:
		if (rate == GMAC0_PTP_REFCLK_IN) {
			src = CLK_GMAC0_PTP_SEL_REFIN;
			div = 1;
		} else if (!(priv->gpll_hz % rate)) {
			src = CLK_GMAC0_PTP_SEL_GPLL;
			div = priv->gpll_hz / rate;
		} else {
			src = CLK_GMAC0_PTP_SEL_CPLL;
			div = priv->cpll_hz / rate;
		}
		rk_clrsetreg(&cru->clksel_con[105],
			     CLK_GMAC0_PTP_DIV_MASK | CLK_GMAC0_PTP_SEL_MASK,
			     src << CLK_GMAC0_PTP_SEL_SHIFT |
			     (div - 1) << CLK_GMAC0_PTP_DIV_SHIFT);
		break;
	case CLK_GMAC1_PTP_REF_SRC:
	case CLK_GMAC1_PTP_REF:
		if (rate == GMAC1_PTP_REFCLK_IN) {
			src = CLK_GMAC1_PTP_SEL_REFIN;
			div = 1;
		} else if (!(priv->gpll_hz % rate)) {
			src = CLK_GMAC1_PTP_SEL_GPLL;
			div = priv->gpll_hz / rate;
		} else {
			src = CLK_GMAC1_PTP_SEL_CPLL;
			div = priv->cpll_hz / rate;
		}
		rk_clrsetreg(&cru->clksel_con[104],
			     CLK_GMAC1_PTP_DIV_MASK | CLK_GMAC1_PTP_SEL_MASK,
			     src << CLK_GMAC1_PTP_SEL_SHIFT |
			     (div - 1) << CLK_GMAC1_PTP_DIV_SHIFT);
		break;

	case CLK_GMAC0_125M_SRC:
		rk_clrsetreg(&cru->clksel_con[30],
			     CLK_GMAC0_125M_DIV_MASK,
			     (div - 1) << CLK_GMAC0_125M_DIV_SHIFT);
		break;
	case CLK_GMAC1_125M_SRC:
		rk_clrsetreg(&cru->clksel_con[31],
			     CLK_GMAC1_125M_DIV_MASK,
			     (div - 1) << CLK_GMAC1_125M_DIV_SHIFT);
		break;
	default:
		return -ENOENT;
	}

	return rk3576_gmac_get_clk(priv, clk_id);
}

static ulong rk3576_uart_frac_get_rate(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 reg, con, fracdiv, p_src, p_rate;
	unsigned long m, n;

	switch (clk_id) {
	case CLK_UART_FRAC_0:
		reg = 21;
		break;
	case CLK_UART_FRAC_1:
		reg = 23;
		break;
	case CLK_UART_FRAC_2:
		reg = 25;
		break;
	default:
		return -ENOENT;
	}
	con = readl(&cru->clksel_con[reg + 1]);
	p_src = (con & CLK_UART_SRC_SEL_MASK) >> CLK_UART_SRC_SEL_SHIFT;
	if (p_src == CLK_UART_SRC_SEL_GPLL)
		p_rate = priv->gpll_hz;
	else if (p_src == CLK_UART_SRC_SEL_CPLL)
		p_rate = priv->cpll_hz;
	else if (p_src == CLK_UART_SRC_SEL_AUPLL)
		p_rate = priv->aupll_hz;
	else
		p_rate = OSC_HZ;

	fracdiv = readl(&cru->clksel_con[reg]);
	n = fracdiv & CLK_UART_FRAC_NUMERATOR_MASK;
	n >>= CLK_UART_FRAC_NUMERATOR_SHIFT;
	m = fracdiv & CLK_UART_FRAC_DENOMINATOR_MASK;
	m >>= CLK_UART_FRAC_DENOMINATOR_SHIFT;
	return p_rate * n / m;
}

static ulong rk3576_uart_frac_set_rate(struct rk3576_clk_priv *priv,
				       ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	u32 reg, clk_src, p_rate;
	unsigned long m = 0, n = 0, val;

	if (priv->cpll_hz % rate == 0) {
		clk_src = CLK_UART_SRC_SEL_CPLL;
		p_rate = priv->cpll_hz;
	} else if (rate == OSC_HZ) {
		clk_src = CLK_UART_SRC_SEL_OSC;
		p_rate = OSC_HZ;
	} else {
		clk_src = CLK_UART_SRC_SEL_GPLL;
		p_rate = priv->cpll_hz;
	}

	rational_best_approximation(rate, p_rate, GENMASK(16 - 1, 0),
				    GENMASK(16 - 1, 0), &m, &n);

	if (m < 4 && m != 0) {
		if (n % 2 == 0)
			val = 1;
		else
			val = DIV_ROUND_UP(4, m);

		n *= val;
		m *= val;
		if (n > 0xffff)
			n = 0xffff;
	}

	switch (clk_id) {
	case CLK_UART_FRAC_0:
		reg = 21;
		break;
	case CLK_UART_FRAC_1:
		reg = 23;
		break;
	case CLK_UART_FRAC_2:
		reg = 25;
		break;
	default:
		return -ENOENT;
	}

	rk_clrsetreg(&cru->clksel_con[reg + 1],
		     CLK_UART_SRC_SEL_MASK,
		     (clk_src << CLK_UART_SRC_SEL_SHIFT));
	if (m && n) {
		val = m << CLK_UART_FRAC_NUMERATOR_SHIFT | n;
		writel(val, &cru->clksel_con[reg]);
	}

	return rk3576_uart_frac_get_rate(priv, clk_id);
}

static ulong rk3576_uart_get_rate(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 con, div, src, p_rate;

	switch (clk_id) {
	case SCLK_UART0:
		con = readl(&cru->clksel_con[60]);
		break;
	case SCLK_UART1:
		con = readl(&cru->pmuclksel_con[8]);
		src = (con & CLK_UART1_SEL_MASK) >> CLK_UART1_SEL_SHIFT;
		if (src == CLK_UART1_SEL_OSC)
			return OSC_HZ;
		con = readl(&cru->clksel_con[27]);
		break;
	case SCLK_UART2:
		con = readl(&cru->clksel_con[61]);
		break;
	case SCLK_UART3:
		con = readl(&cru->clksel_con[62]);
		break;
	case SCLK_UART4:
		con = readl(&cru->clksel_con[63]);
		break;
	case SCLK_UART5:
		con = readl(&cru->clksel_con[64]);
		break;
	case SCLK_UART6:
		con = readl(&cru->clksel_con[65]);
		break;
	case SCLK_UART7:
		con = readl(&cru->clksel_con[66]);
		break;
	case SCLK_UART8:
		con = readl(&cru->clksel_con[67]);
		break;
	case SCLK_UART9:
		con = readl(&cru->clksel_con[68]);
		break;
	case SCLK_UART10:
		con = readl(&cru->clksel_con[69]);
		break;
	case SCLK_UART11:
		con = readl(&cru->clksel_con[70]);
		break;
	default:
		return -ENOENT;
	}
	if (clk_id == SCLK_UART1) {
		src = (con & CLK_UART1_SRC_SEL_SHIFT) >> CLK_UART1_SRC_SEL_SHIFT;
		div = (con & CLK_UART1_SRC_DIV_MASK) >> CLK_UART1_SRC_DIV_SHIFT;
	} else {
		src = (con & CLK_UART_SEL_MASK) >> CLK_UART_SEL_SHIFT;
		div = (con & CLK_UART_DIV_MASK) >> CLK_UART_DIV_SHIFT;
	}
	if (src == CLK_UART_SEL_GPLL)
		p_rate = priv->gpll_hz;
	else  if (src == CLK_UART_SEL_CPLL)
		p_rate = priv->cpll_hz;
	else  if (src == CLK_UART_SEL_AUPLL)
		p_rate = priv->aupll_hz;
	else  if (src == CLK_UART_SEL_FRAC0)
		p_rate = rk3576_uart_frac_get_rate(priv, CLK_UART_FRAC_0);
	else  if (src == CLK_UART_SEL_FRAC1)
		p_rate = rk3576_uart_frac_get_rate(priv, CLK_UART_FRAC_1);
	else  if (src == CLK_UART_SEL_FRAC2)
		p_rate = rk3576_uart_frac_get_rate(priv, CLK_UART_FRAC_2);
	else
		p_rate = OSC_HZ;

	return DIV_TO_RATE(p_rate, div);
}

static ulong rk3576_uart_set_rate(struct rk3576_clk_priv *priv,
				  ulong clk_id, ulong rate)
{
	struct rk3576_cru *cru = priv->cru;
	u32 reg, clk_src = 0, div = 0;

	if (!(priv->gpll_hz % rate)) {
		clk_src = CLK_UART_SEL_GPLL;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	} else if (!(priv->cpll_hz % rate)) {
		clk_src = CLK_UART_SEL_CPLL;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	} else if (!(rk3576_uart_frac_get_rate(priv, CLK_UART_FRAC_0) % rate)) {
		clk_src = CLK_UART_SEL_FRAC0;
		div = DIV_ROUND_UP(rk3576_uart_frac_get_rate(priv, CLK_UART_FRAC_0), rate);
	} else if (!(rk3576_uart_frac_get_rate(priv, CLK_UART_FRAC_1) % rate)) {
		clk_src = CLK_UART_SEL_FRAC1;
		div = DIV_ROUND_UP(rk3576_uart_frac_get_rate(priv, CLK_UART_FRAC_1), rate);
	} else if (!(rk3576_uart_frac_get_rate(priv, CLK_UART_FRAC_2) % rate)) {
		clk_src = CLK_UART_SEL_FRAC2;
		div = DIV_ROUND_UP(rk3576_uart_frac_get_rate(priv, CLK_UART_FRAC_2), rate);
	} else if (!(OSC_HZ % rate)) {
		clk_src = CLK_UART_SEL_OSC;
		div = DIV_ROUND_UP(OSC_HZ, rate);
	}

	switch (clk_id) {
	case SCLK_UART0:
		reg = 60;
		break;
	case SCLK_UART1:
		if (rate == OSC_HZ) {
			rk_clrsetreg(&cru->pmuclksel_con[8],
				     CLK_UART1_SEL_MASK,
				     CLK_UART1_SEL_OSC << CLK_UART1_SEL_SHIFT);
			return 0;
		}

		rk_clrsetreg(&cru->clksel_con[27],
			     CLK_UART1_SRC_SEL_MASK | CLK_UART1_SRC_DIV_MASK,
			     (clk_src << CLK_UART1_SRC_SEL_SHIFT) |
			     ((div - 1) << CLK_UART1_SRC_DIV_SHIFT));
		rk_clrsetreg(&cru->pmuclksel_con[8],
			     CLK_UART1_SEL_MASK,
			     CLK_UART1_SEL_TOP << CLK_UART1_SEL_SHIFT);
		return 0;
	case SCLK_UART2:
		reg = 61;
		break;
	case SCLK_UART3:
		reg = 62;
		break;
	case SCLK_UART4:
		reg = 63;
		break;
	case SCLK_UART5:
		reg = 64;
		break;
	case SCLK_UART6:
		reg = 65;
		break;
	case SCLK_UART7:
		reg = 66;
		break;
	case SCLK_UART8:
		reg = 67;
		break;
	case SCLK_UART9:
		reg = 68;
		break;
	case SCLK_UART10:
		reg = 69;
		break;
	case SCLK_UART11:
		reg = 70;
		break;
	default:
		return -ENOENT;
	}

	rk_clrsetreg(&cru->clksel_con[reg],
		     CLK_UART_SEL_MASK |
		     CLK_UART_DIV_MASK,
		     (clk_src << CLK_UART_SEL_SHIFT) |
		     ((div - 1) << CLK_UART_DIV_SHIFT));

	return rk3576_uart_get_rate(priv, clk_id);
}
#endif

static ulong rk3576_ufs_ref_get_rate(struct rk3576_clk_priv *priv, ulong clk_id)
{
	struct rk3576_cru *cru = priv->cru;
	u32 src, div;

	src = readl(&cru->pmuclksel_con[3]) & 0x3;
	div = readl(&cru->pmuclksel_con[1]) & 0xff;
	if (src == 0)
		return OSC_HZ;
	else if (src == 2)
		return priv->ppll_hz / (div + 1);
	else
		return 26000000;
}

static ulong rk3576_clk_get_rate(struct clk *clk)
{
	struct rk3576_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	if (!priv->gpll_hz) {
		printf("%s gpll=%lu\n", __func__, priv->gpll_hz);
		return -ENOENT;
	}

	if (!priv->ppll_hz) {
		priv->ppll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[PPLL],
						      priv->cru, PPLL);
	}

	switch (clk->id) {
	case PLL_LPLL:
		rate = rockchip_pll_get_rate(&rk3576_pll_clks[LPLL], priv->cru,
					     LPLL);
		priv->lpll_hz = rate;
		break;
	case PLL_BPLL:
		rate = rockchip_pll_get_rate(&rk3576_pll_clks[BPLL], priv->cru,
					     BPLL);
		priv->bpll_hz = rate;
		break;
	case PLL_GPLL:
		rate = rockchip_pll_get_rate(&rk3576_pll_clks[GPLL], priv->cru,
					     GPLL);
		break;
	case PLL_CPLL:
		rate = rockchip_pll_get_rate(&rk3576_pll_clks[CPLL], priv->cru,
					     CPLL);
		break;
	case PLL_VPLL:
		rate = rockchip_pll_get_rate(&rk3576_pll_clks[VPLL], priv->cru,
					     VPLL);
		break;
	case PLL_AUPLL:
		rate = rockchip_pll_get_rate(&rk3576_pll_clks[AUPLL], priv->cru,
					     AUPLL);
		break;
	case PLL_PPLL:
		rate = rockchip_pll_get_rate(&rk3576_pll_clks[PPLL], priv->cru,
					     PPLL) * 2;
		break;
	case ACLK_BUS_ROOT:
	case HCLK_BUS_ROOT:
	case PCLK_BUS_ROOT:
		rate = rk3576_bus_get_clk(priv, clk->id);
		break;
	case ACLK_TOP:
	case HCLK_TOP:
	case PCLK_TOP_ROOT:
	case ACLK_TOP_MID:
		rate = rk3576_top_get_clk(priv, clk->id);
		break;
	case CLK_I2C0:
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C5:
	case CLK_I2C6:
	case CLK_I2C7:
	case CLK_I2C8:
	case CLK_I2C9:
		rate = rk3576_i2c_get_clk(priv, clk->id);
		break;
	case CLK_SPI0:
	case CLK_SPI1:
	case CLK_SPI2:
	case CLK_SPI3:
	case CLK_SPI4:
		rate = rk3576_spi_get_clk(priv, clk->id);
		break;
	case CLK_PWM1:
	case CLK_PWM2:
	case CLK_PMU1PWM:
		rate = rk3576_pwm_get_clk(priv, clk->id);
		break;
	case CLK_SARADC:
	case CLK_TSADC:
		rate = rk3576_adc_get_clk(priv, clk->id);
		break;
	case CCLK_SRC_SDIO:
	case CCLK_SRC_SDMMC0:
	case CCLK_SRC_EMMC:
	case BCLK_EMMC:
	case SCLK_FSPI_X2:
	case SCLK_FSPI1_X2:
	case DCLK_DECOM:
	case HCLK_SDMMC0:
	case HCLK_EMMC:
	case HCLK_SDIO:
		rate = rk3576_mmc_get_clk(priv, clk->id);
		break;
	case TCLK_EMMC:
	case TCLK_WDT0:
		rate = OSC_HZ;
		break;
#ifndef CONFIG_SPL_BUILD
	case ACLK_VOP_ROOT:
	case ACLK_VOP:
	case ACLK_VO0_ROOT:
	case ACLK_VO1_ROOT:
	case HCLK_VOP_ROOT:
	case PCLK_VOP_ROOT:
		rate = rk3576_aclk_vop_get_clk(priv, clk->id);
		break;
	case DCLK_VP0:
	case DCLK_VP0_SRC:
	case DCLK_VP1:
	case DCLK_VP1_SRC:
	case DCLK_VP2:
	case DCLK_VP2_SRC:
		rate = rk3576_dclk_vop_get_clk(priv, clk->id);
		break;
	case CLK_GMAC0_PTP_REF_SRC:
	case CLK_GMAC1_PTP_REF_SRC:
	case CLK_GMAC0_PTP_REF:
	case CLK_GMAC1_PTP_REF:
	case CLK_GMAC0_125M_SRC:
	case CLK_GMAC1_125M_SRC:
		rate = rk3576_gmac_get_clk(priv, clk->id);
		break;
	case CLK_UART_FRAC_0:
	case CLK_UART_FRAC_1:
	case CLK_UART_FRAC_2:
		rate = rk3576_uart_frac_get_rate(priv, clk->id);
		break;
	case SCLK_UART0:
	case SCLK_UART1:
	case SCLK_UART2:
	case SCLK_UART3:
	case SCLK_UART4:
	case SCLK_UART5:
	case SCLK_UART6:
	case SCLK_UART7:
	case SCLK_UART8:
	case SCLK_UART9:
	case SCLK_UART10:
	case SCLK_UART11:
		rate = rk3576_uart_get_rate(priv, clk->id);
		break;
	case CLK_DSIHOST0:
		rate = rk3576_clk_csihost_get_clk(priv, clk->id);
		break;
	case DCLK_EBC:
	case DCLK_EBC_FRAC_SRC:
		rate = rk3576_dclk_ebc_get_clk(priv, clk->id);
		break;
#endif
	case CLK_REF_UFS_CLKOUT:
	case CLK_REF_OSC_MPHY:
		rate = rk3576_ufs_ref_get_rate(priv, clk->id);
		break;

	default:
		return -ENOENT;
	}

	return rate;
};

static ulong rk3576_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3576_clk_priv *priv = dev_get_priv(clk->dev);
	ulong ret = 0;

	if (!priv->ppll_hz) {
		priv->ppll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[PPLL],
						      priv->cru, PPLL);
	}
	if (!priv->aupll_hz) {
		priv->aupll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[AUPLL],
						       priv->cru, AUPLL);
	}

	switch (clk->id) {
	case PLL_CPLL:
		ret = rockchip_pll_set_rate(&rk3576_pll_clks[CPLL], priv->cru,
					    CPLL, rate);
		priv->cpll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[CPLL],
						      priv->cru, CPLL);
		break;
	case PLL_GPLL:
		ret = rockchip_pll_set_rate(&rk3576_pll_clks[GPLL], priv->cru,
					    GPLL, rate);
		priv->gpll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[GPLL],
						      priv->cru, GPLL);
		break;
	case PLL_VPLL:
		ret = rockchip_pll_set_rate(&rk3576_pll_clks[VPLL], priv->cru,
					    VPLL, rate);
		priv->vpll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[VPLL],
						      priv->cru, VPLL);
		break;
	case PLL_AUPLL:
		ret = rockchip_pll_set_rate(&rk3576_pll_clks[AUPLL], priv->cru,
					    AUPLL, rate);
		priv->aupll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[AUPLL],
						       priv->cru, AUPLL);
		break;
	case PLL_PPLL:
		ret = rockchip_pll_set_rate(&rk3576_pll_clks[PPLL], priv->cru,
					    PPLL, rate);
		priv->ppll_hz = rockchip_pll_get_rate(&rk3576_pll_clks[PPLL],
						      priv->cru, PPLL) * 2;
		break;
	case ACLK_BUS_ROOT:
	case HCLK_BUS_ROOT:
	case PCLK_BUS_ROOT:
		ret = rk3576_bus_set_clk(priv, clk->id, rate);
		break;
	case ACLK_TOP:
	case HCLK_TOP:
	case PCLK_TOP_ROOT:
	case ACLK_TOP_MID:
		ret = rk3576_top_set_clk(priv, clk->id, rate);
		break;
	case CLK_I2C0:
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C5:
	case CLK_I2C6:
	case CLK_I2C7:
	case CLK_I2C8:
	case CLK_I2C9:
		ret = rk3576_i2c_set_clk(priv, clk->id, rate);
		break;
	case CLK_SPI0:
	case CLK_SPI1:
	case CLK_SPI2:
	case CLK_SPI3:
	case CLK_SPI4:
		ret = rk3576_spi_set_clk(priv, clk->id, rate);
		break;
	case CLK_PWM1:
	case CLK_PWM2:
	case CLK_PMU1PWM:
		ret = rk3576_pwm_set_clk(priv, clk->id, rate);
		break;
	case CLK_SARADC:
	case CLK_TSADC:
		ret = rk3576_adc_set_clk(priv, clk->id, rate);
		break;
	case CCLK_SRC_SDIO:
	case CCLK_SRC_SDMMC0:
	case CCLK_SRC_EMMC:
	case BCLK_EMMC:
	case SCLK_FSPI_X2:
	case SCLK_FSPI1_X2:
	case DCLK_DECOM:
	case HCLK_SDMMC0:
	case HCLK_EMMC:
	case HCLK_SDIO:
		ret = rk3576_mmc_set_clk(priv, clk->id, rate);
		break;
	case TCLK_EMMC:
	case TCLK_WDT0:
		ret = OSC_HZ;
		break;

	/* Might occur in cru assigned-clocks, can be ignored here */
	case CLK_AUDIO_FRAC_0:
	case CLK_AUDIO_FRAC_1:
	case CLK_AUDIO_FRAC_0_SRC:
	case CLK_AUDIO_FRAC_1_SRC:
	case CLK_CPLL_DIV2:
	case CLK_CPLL_DIV4:
	case CLK_CPLL_DIV10:
	case FCLK_DDR_CM0_CORE:
	case ACLK_PHP_ROOT:
	case CLK_REF_PCIE0_PHY:
	case CLK_REF_PCIE1_PHY:
		ret = 0;
		break;
#ifndef CONFIG_SPL_BUILD
	case ACLK_VOP_ROOT:
	case ACLK_VOP:
	case ACLK_VO0_ROOT:
	case ACLK_VO1_ROOT:
	case HCLK_VOP_ROOT:
	case PCLK_VOP_ROOT:
		ret = rk3576_aclk_vop_set_clk(priv, clk->id, rate);
		break;
	case DCLK_VP0:
	case DCLK_VP0_SRC:
	case DCLK_VP1:
	case DCLK_VP1_SRC:
	case DCLK_VP2:
	case DCLK_VP2_SRC:
		ret = rk3576_dclk_vop_set_clk(priv, clk->id, rate);
		break;
	case CLK_GMAC0_PTP_REF_SRC:
	case CLK_GMAC1_PTP_REF_SRC:
	case CLK_GMAC0_PTP_REF:
	case CLK_GMAC1_PTP_REF:
	case CLK_GMAC0_125M_SRC:
	case CLK_GMAC1_125M_SRC:
		ret = rk3576_gmac_set_clk(priv, clk->id, rate);
		break;
	case CLK_UART_FRAC_0:
	case CLK_UART_FRAC_1:
	case CLK_UART_FRAC_2:
		ret = rk3576_uart_frac_set_rate(priv, clk->id, rate);
		break;
	case SCLK_UART0:
	case SCLK_UART1:
	case SCLK_UART2:
	case SCLK_UART3:
	case SCLK_UART4:
	case SCLK_UART5:
	case SCLK_UART6:
	case SCLK_UART7:
	case SCLK_UART8:
	case SCLK_UART9:
	case SCLK_UART10:
	case SCLK_UART11:
		ret = rk3576_uart_set_rate(priv, clk->id, rate);
		break;
	case CLK_DSIHOST0:
		ret = rk3576_clk_csihost_set_clk(priv, clk->id, rate);
		break;
	case DCLK_EBC:
	case DCLK_EBC_FRAC_SRC:
		ret = rk3576_dclk_ebc_set_clk(priv, clk->id, rate);
		break;
#endif
	default:
		return -ENOENT;
	}

	return ret;
};

#if (IS_ENABLED(OF_CONTROL)) || (!IS_ENABLED(OF_PLATDATA))
static int __maybe_unused rk3576_dclk_vop_set_parent(struct clk *clk,
						     struct clk *parent)
{
	struct rk3576_clk_priv *priv = dev_get_priv(clk->dev);
	struct rk3576_cru *cru = priv->cru;
	u32 sel;
	const char *clock_dev_name = parent->dev->name;

	if (parent->id == PLL_VPLL)
		sel = 2;
	else if (parent->id == PLL_GPLL)
		sel = 0;
	else if (parent->id == PLL_CPLL)
		sel = 1;
	else if (parent->id == PLL_BPLL)
		sel = 3;
	else
		sel = 4;

	switch (clk->id) {
	case DCLK_VP0_SRC:
		rk_clrsetreg(&cru->clksel_con[145], DCLK0_VOP_SRC_SEL_MASK,
			     sel << DCLK0_VOP_SRC_SEL_SHIFT);
		break;
	case DCLK_VP1_SRC:
		rk_clrsetreg(&cru->clksel_con[146], DCLK0_VOP_SRC_SEL_MASK,
			     sel << DCLK0_VOP_SRC_SEL_SHIFT);
		break;
	case DCLK_VP2_SRC:
		rk_clrsetreg(&cru->clksel_con[147], DCLK0_VOP_SRC_SEL_MASK,
			     sel << DCLK0_VOP_SRC_SEL_SHIFT);
		break;
	case DCLK_VP0:
		if (!strcmp(clock_dev_name, "hdmiphypll_clk0"))
			sel = 1;
		else
			sel = 0;
		rk_clrsetreg(&cru->clksel_con[147], DCLK0_VOP_SEL_MASK,
			     sel << DCLK0_VOP_SEL_SHIFT);
		break;
	case DCLK_VP1:
		if (!strcmp(clock_dev_name, "hdmiphypll_clk0"))
			sel = 1;
		else
			sel = 0;
		rk_clrsetreg(&cru->clksel_con[147], DCLK1_VOP_SEL_MASK,
			     sel << DCLK1_VOP_SEL_SHIFT);
		break;
	case DCLK_VP2:
		if (!strcmp(clock_dev_name, "hdmiphypll_clk0"))
			sel = 1;
		else
			sel = 0;
		rk_clrsetreg(&cru->clksel_con[147], DCLK2_VOP_SEL_MASK,
			     sel << DCLK2_VOP_SEL_SHIFT);
		break;
	case DCLK_EBC:
		if (parent->id == PLL_GPLL)
			sel = 0;
		else if (parent->id == PLL_CPLL)
			sel = 1;
		else if (parent->id == PLL_VPLL)
			sel = 2;
		else if (parent->id == PLL_AUPLL)
			sel = 3;
		else if (parent->id == PLL_LPLL)
			sel = 4;
		else if (parent->id == DCLK_EBC_FRAC_SRC)
			sel = 5;
		else
			sel = 6;
		rk_clrsetreg(&cru->clksel_con[123], DCLK_EBC_SEL_MASK,
			     sel << DCLK_EBC_SEL_SHIFT);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int __maybe_unused rk3576_ufs_ref_set_parent(struct clk *clk,
						    struct clk *parent)
{
	struct rk3576_clk_priv *priv = dev_get_priv(clk->dev);
	struct rk3576_cru *cru = priv->cru;
	u32 sel;
	const char *clock_dev_name = parent->dev->name;

	if (parent->id == CLK_REF_MPHY_26M)
		sel = 2;
	else if (!strcmp(clock_dev_name, "xin24m"))
		sel = 0;
	else
		sel = 1;

	rk_clrsetreg(&cru->pmuclksel_con[3], 0x3, sel << 0);
	return 0;
}

static int rk3576_clk_set_parent(struct clk *clk, struct clk *parent)
{
	switch (clk->id) {
	case DCLK_VP0_SRC:
	case DCLK_VP1_SRC:
	case DCLK_VP2_SRC:
	case DCLK_VP0:
	case DCLK_VP1:
	case DCLK_VP2:
	case DCLK_EBC:
		return rk3576_dclk_vop_set_parent(clk, parent);
	case CLK_REF_OSC_MPHY:
		return rk3576_ufs_ref_set_parent(clk, parent);
	case CLK_AUDIO_FRAC_0_SRC:
	case CLK_AUDIO_FRAC_1_SRC:
		/* Might occur in cru assigned-clocks, can be ignored here */
		return 0;
	default:
		return -ENOENT;
	}

	return 0;
}
#endif

static struct clk_ops rk3576_clk_ops = {
	.get_rate = rk3576_clk_get_rate,
	.set_rate = rk3576_clk_set_rate,
#if (IS_ENABLED(OF_CONTROL)) || (!IS_ENABLED(OF_PLATDATA))
	.set_parent = rk3576_clk_set_parent,
#endif
};

static void rk3576_clk_init(struct rk3576_clk_priv *priv)
{
	int ret;

	priv->spll_hz = 702000000;

	if (priv->cpll_hz != CPLL_HZ) {
		ret = rockchip_pll_set_rate(&rk3576_pll_clks[CPLL], priv->cru,
					    CPLL, CPLL_HZ);
		if (!ret)
			priv->cpll_hz = CPLL_HZ;
	}
	if (priv->gpll_hz != GPLL_HZ) {
		ret = rockchip_pll_set_rate(&rk3576_pll_clks[GPLL], priv->cru,
					    GPLL, GPLL_HZ);
		if (!ret)
			priv->gpll_hz = GPLL_HZ;
	}
	rk_clrsetreg(&priv->cru->clksel_con[123],
		     DCLK_EBC_FRAC_SRC_SEL_MASK,
		     (DCLK_EBC_FRAC_SRC_SEL_GPLL <<
		      DCLK_EBC_FRAC_SRC_SEL_SHIFT));
}

static int rk3576_clk_probe(struct udevice *dev)
{
	struct rk3576_clk_priv *priv = dev_get_priv(dev);
	int ret;

	priv->sync_kernel = false;

#ifdef CONFIG_SPL_BUILD
	/* relase presetn_bigcore_biu/cru/grf */
	writel(0x1c001c00, 0x26010010);
	/* set spll to normal mode */
	writel(BITS_WITH_WMASK(2, 0x7U, 6),
	       RK3576_SCRU_BASE + RK3576_PLL_CON(137));
	writel(BITS_WITH_WMASK(1, 0x3U, 0),
	       RK3576_SCRU_BASE + RK3576_MODE_CON0);
	/* fix ppll\aupll\cpll */
	writel(BITS_WITH_WMASK(2, 0x7U, 6),
	       RK3576_CRU_BASE + RK3576_PMU_PLL_CON(129));
	writel(BITS_WITH_WMASK(2, 0x7U, 6),
	       RK3576_CRU_BASE + RK3576_PLL_CON(97));
	writel(BITS_WITH_WMASK(2, 0x7U, 6),
	       RK3576_CRU_BASE + RK3576_PLL_CON(105));
	writel(BITS_WITH_WMASK(1, 0x3U, 6),
	       RK3576_CRU_BASE + RK3576_MODE_CON0);
	writel(BITS_WITH_WMASK(1, 0x3U, 8),
	       RK3576_CRU_BASE + RK3576_MODE_CON0);
	/* init cci */
	writel(0xffff0000, RK3576_CRU_BASE + RK3576_CCI_CLKSEL_CON(4));
	rockchip_pll_set_rate(&rk3576_pll_clks[BPLL], priv->cru,
			      BPLL, LPLL_HZ);
	if (!priv->armclk_enter_hz) {
		ret = rockchip_pll_set_rate(&rk3576_pll_clks[LPLL], priv->cru,
					    LPLL, LPLL_HZ);
		priv->armclk_enter_hz =
			rockchip_pll_get_rate(&rk3576_pll_clks[LPLL],
					      priv->cru, LPLL);
		priv->armclk_init_hz = priv->armclk_enter_hz;
		rk_clrsetreg(&priv->cru->litclksel_con[0], CLK_LITCORE_DIV_MASK,
			     0 << CLK_LITCORE_DIV_SHIFT);
	}
	/* init cci */
	writel(0xffff20cb, RK3576_CRU_BASE + RK3576_CCI_CLKSEL_CON(4));

	/* Change bigcore rm from 4 to 3 */
	writel(0x001c000c, RK3576_BIGCORE_GRF_BASE + 0x3c);
	writel(0x001c000c, RK3576_BIGCORE_GRF_BASE + 0x44);
	writel(0x00020002, RK3576_BIGCORE_GRF_BASE + 0x38);
	udelay(1);
	writel(0x00020000, RK3576_BIGCORE_GRF_BASE + 0x38);
	/* Change litcore rm from 4 to 3 */
	writel(0x001c000c, RK3576_LITCORE_GRF_BASE + 0x3c);
	writel(0x001c000c, RK3576_LITCORE_GRF_BASE + 0x44);
	writel(0x00020002, RK3576_LITCORE_GRF_BASE + 0x38);
	udelay(1);
	writel(0x00020000, RK3576_LITCORE_GRF_BASE + 0x38);
	/* Change cci rm form 4 to 3 */
	writel(0x001c000c, RK3576_CCI_GRF_BASE + 0x54);
#endif

	rk3576_clk_init(priv);

	/* Process 'assigned-{clocks/clock-parents/clock-rates}' properties */
	ret = clk_set_defaults(dev, 1);
	if (ret)
		debug("%s clk_set_defaults failed %d\n", __func__, ret);
	else
		priv->sync_kernel = true;

	return 0;
}

static int rk3576_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct rk3576_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);

	return 0;
}

static int rk3576_clk_bind(struct udevice *dev)
{
	int ret;
	struct udevice *sys_child;
	struct sysreset_reg *priv;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(dev, "rockchip_sysreset", "sysreset",
				 &sys_child);
	if (ret) {
		debug("Warning: No sysreset driver: ret=%d\n", ret);
	} else {
		priv = malloc(sizeof(struct sysreset_reg));
		priv->glb_srst_fst_value = offsetof(struct rk3576_cru,
						    glb_srst_fst);
		priv->glb_srst_snd_value = offsetof(struct rk3576_cru,
						    glb_srsr_snd);
		dev_set_priv(sys_child, priv);
	}

#if CONFIG_IS_ENABLED(RESET_ROCKCHIP)
	ret = offsetof(struct rk3576_cru, softrst_con[0]);
	ret = rk3576_reset_bind_lut(dev, ret, 32776);
	if (ret)
		debug("Warning: software reset driver bind failed\n");
#endif

	return 0;
}

static const struct udevice_id rk3576_clk_ids[] = {
	{ .compatible = "rockchip,rk3576-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3576_cru) = {
	.name		= "rockchip_rk3576_cru",
	.id		= UCLASS_CLK,
	.of_match	= rk3576_clk_ids,
	.priv_auto	= sizeof(struct rk3576_clk_priv),
	.of_to_plat	= rk3576_clk_ofdata_to_platdata,
	.ops		= &rk3576_clk_ops,
	.bind		= rk3576_clk_bind,
	.probe		= rk3576_clk_probe,
};

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 * Author: Joseph Chen <chenjh@rock-chips.com>
 */

#include <bitfield.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/cru_rk3528.h>
#include <asm/arch-rockchip/hardware.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rockchip,rk3528-cru.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

/*
 *	PLL attention.
 *
 * [FRAC PLL]: GPLL, PPLL, DPLL
 *   - frac mode: refdiv can be 1 or 2 only
 *   - int mode:  refdiv has no special limit
 *   - VCO range: [950, 3800] MHZ
 *
 * [INT PLL]:  CPLL, APLL
 *   - int mode:  refdiv can be 1 or 2 only
 *   - VCO range: [475, 1900] MHZ
 *
 * [PPLL]: normal mode only.
 *
 */
static struct rockchip_pll_rate_table rk3528_pll_rates[] = {
	/* _mhz, _refdiv, _fbdiv, _postdiv1, _postdiv2, _dsmpd, _frac */
	RK3036_PLL_RATE(1896000000, 1, 79, 1, 1, 1, 0),
	RK3036_PLL_RATE(1800000000, 1, 75, 1, 1, 1, 0),
	RK3036_PLL_RATE(1704000000, 1, 71, 1, 1, 1, 0),
	RK3036_PLL_RATE(1608000000, 1, 67, 1, 1, 1, 0),
	RK3036_PLL_RATE(1512000000, 1, 63, 1, 1, 1, 0),
	RK3036_PLL_RATE(1416000000, 1, 59, 1, 1, 1, 0),
	RK3036_PLL_RATE(1296000000, 1, 54, 1, 1, 1, 0),
	RK3036_PLL_RATE(1200000000, 1, 50, 1, 1, 1, 0),
	RK3036_PLL_RATE(1188000000, 1, 99, 2, 1, 1, 0),		/* GPLL */
	RK3036_PLL_RATE(1092000000, 2, 91, 1, 1, 1, 0),
	RK3036_PLL_RATE(1008000000, 1, 42, 1, 1, 1, 0),
	RK3036_PLL_RATE(1000000000, 1, 125, 3, 1, 1, 0),	/* PPLL */
	RK3036_PLL_RATE(996000000, 2, 83, 1, 1, 1, 0),		/* CPLL */
	RK3036_PLL_RATE(960000000, 1, 40, 1, 1, 1, 0),
	RK3036_PLL_RATE(912000000, 1, 76, 2, 1, 1, 0),
	RK3036_PLL_RATE(816000000, 1, 68, 2, 1, 1, 0),
	RK3036_PLL_RATE(600000000, 1, 50, 2, 1, 1, 0),
	RK3036_PLL_RATE(594000000, 2, 99, 2, 1, 1, 0),
	RK3036_PLL_RATE(408000000, 1, 68, 2, 2, 1, 0),
	RK3036_PLL_RATE(312000000, 1, 78, 6, 1, 1, 0),
	RK3036_PLL_RATE(216000000, 1, 72, 4, 2, 1, 0),
	RK3036_PLL_RATE(96000000, 1, 24, 3, 2, 1, 0),
	{ /* sentinel */ },
};

static struct rockchip_pll_clock rk3528_pll_clks[] = {
	[APLL] = PLL(pll_rk3328, PLL_APLL, RK3528_PLL_CON(0),
		     RK3528_MODE_CON, 0, 10, 0, rk3528_pll_rates),

	[CPLL] = PLL(pll_rk3328, PLL_CPLL, RK3528_PLL_CON(8),
		     RK3528_MODE_CON, 2, 10, 0, rk3528_pll_rates),

	[GPLL] = PLL(pll_rk3328, PLL_GPLL, RK3528_PLL_CON(24),
		     RK3528_MODE_CON, 4, 10, 0, rk3528_pll_rates),

	[PPLL] = PLL(pll_rk3328, PLL_PPLL, RK3528_PCIE_PLL_CON(32),
		     RK3528_MODE_CON, 6, 10, ROCKCHIP_PLL_FIXED_MODE, rk3528_pll_rates),

	[DPLL] = PLL(pll_rk3328, PLL_DPLL, RK3528_DDRPHY_PLL_CON(16),
		     RK3528_DDRPHY_MODE_CON, 0, 10, 0, rk3528_pll_rates),
};

#define RK3528_CPUCLK_RATE(_rate, _aclk_m_core, _pclk_dbg)	\
{								\
	.rate = _rate##U,					\
	.aclk_div = (_aclk_m_core),				\
	.pclk_div = (_pclk_dbg),				\
}

/* sign-off: _aclk_m_core: 550M, _pclk_dbg: 137.5M, */
static struct rockchip_cpu_rate_table rk3528_cpu_rates[] = {
	RK3528_CPUCLK_RATE(1896000000, 1, 13),
	RK3528_CPUCLK_RATE(1800000000, 1, 12),
	RK3528_CPUCLK_RATE(1704000000, 1, 11),
	RK3528_CPUCLK_RATE(1608000000, 1, 11),
	RK3528_CPUCLK_RATE(1512000000, 1, 11),
	RK3528_CPUCLK_RATE(1416000000, 1, 9),
	RK3528_CPUCLK_RATE(1296000000, 1, 8),
	RK3528_CPUCLK_RATE(1200000000, 1, 8),
	RK3528_CPUCLK_RATE(1188000000, 1, 8),
	RK3528_CPUCLK_RATE(1092000000, 1, 7),
	RK3528_CPUCLK_RATE(1008000000, 1, 6),
	RK3528_CPUCLK_RATE(1000000000, 1, 6),
	RK3528_CPUCLK_RATE(996000000, 1, 6),
	RK3528_CPUCLK_RATE(960000000, 1, 6),
	RK3528_CPUCLK_RATE(912000000, 1, 6),
	RK3528_CPUCLK_RATE(816000000, 1, 5),
	RK3528_CPUCLK_RATE(600000000, 1, 3),
	RK3528_CPUCLK_RATE(594000000, 1, 3),
	RK3528_CPUCLK_RATE(408000000, 1, 2),
	RK3528_CPUCLK_RATE(312000000, 1, 2),
	RK3528_CPUCLK_RATE(216000000, 1, 1),
	RK3528_CPUCLK_RATE(96000000, 1, 0),
};

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

static int rk3528_armclk_set_clk(struct rk3528_clk_priv *priv, ulong new_rate)
{
	const struct rockchip_cpu_rate_table *rate;
	struct rk3528_cru *cru = priv->cru;
	ulong old_rate;

	rate = rockchip_get_cpu_settings(rk3528_cpu_rates, new_rate);
	if (!rate) {
		printf("%s unsupported rate\n", __func__);
		return -EINVAL;
	}

	/*
	 * set up dependent divisors for DBG and ACLK clocks.
	 */
	old_rate = rockchip_pll_get_rate(&rk3528_pll_clks[APLL], priv->cru, APLL);
	if (old_rate > new_rate) {
		if (rockchip_pll_set_rate(&rk3528_pll_clks[APLL],
					  priv->cru, APLL, new_rate))
			return -EINVAL;

		rk_clrsetreg(&cru->clksel_con[40], RK3528_DIV_PCLK_DBG_MASK,
			     rate->pclk_div << RK3528_DIV_PCLK_DBG_SHIFT);

		rk_clrsetreg(&cru->clksel_con[39], RK3528_DIV_ACLK_M_CORE_MASK,
			     rate->aclk_div << RK3528_DIV_ACLK_M_CORE_SHIFT);
	} else if (old_rate < new_rate) {
		rk_clrsetreg(&cru->clksel_con[40], RK3528_DIV_PCLK_DBG_MASK,
			     rate->pclk_div << RK3528_DIV_PCLK_DBG_SHIFT);

		rk_clrsetreg(&cru->clksel_con[39], RK3528_DIV_ACLK_M_CORE_MASK,
			     rate->aclk_div << RK3528_DIV_ACLK_M_CORE_SHIFT);

		if (rockchip_pll_set_rate(&rk3528_pll_clks[APLL],
					  priv->cru, APLL, new_rate))
			return -EINVAL;
	}

	return 0;
}

static ulong rk3528_ppll_matrix_get_rate(struct rk3528_clk_priv *priv,
					 ulong clk_id)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div, mask, shift;
	void *reg;

	switch (clk_id) {
	case CLK_PPLL_50M_MATRIX:
	case CLK_GMAC1_RMII_VPU:
		mask = PCIE_CLK_MATRIX_50M_SRC_DIV_MASK;
		shift = PCIE_CLK_MATRIX_50M_SRC_DIV_SHIFT;
		reg = &cru->pcieclksel_con[1];
		break;

	case CLK_PPLL_100M_MATRIX:
		mask = PCIE_CLK_MATRIX_100M_SRC_DIV_MASK;
		shift = PCIE_CLK_MATRIX_100M_SRC_DIV_SHIFT;
		reg = &cru->pcieclksel_con[1];
		break;

	case CLK_PPLL_125M_MATRIX:
	case CLK_GMAC1_SRC_VPU:
		mask = CLK_MATRIX_125M_SRC_DIV_MASK;
		shift = CLK_MATRIX_125M_SRC_DIV_SHIFT;
		reg = &cru->clksel_con[60];
		break;

	case CLK_GMAC1_VPU_25M:
		mask = CLK_MATRIX_25M_SRC_DIV_MASK;
		shift = CLK_MATRIX_25M_SRC_DIV_SHIFT;
		reg = &cru->clksel_con[60];
		break;
	default:
		return -ENOENT;
	}

	div = (readl(reg) & mask) >> shift;

	return DIV_TO_RATE(priv->ppll_hz, div);
}

static ulong rk3528_ppll_matrix_set_rate(struct rk3528_clk_priv *priv,
					 ulong clk_id, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 id, div, mask, shift;
	u8 is_pciecru = 0;

	switch (clk_id) {
	case CLK_PPLL_50M_MATRIX:
		id = 1;
		mask = PCIE_CLK_MATRIX_50M_SRC_DIV_MASK;
		shift = PCIE_CLK_MATRIX_50M_SRC_DIV_SHIFT;
		is_pciecru = 1;
		break;

	case CLK_PPLL_100M_MATRIX:
		id = 1;
		mask = PCIE_CLK_MATRIX_100M_SRC_DIV_MASK;
		shift = PCIE_CLK_MATRIX_100M_SRC_DIV_SHIFT;
		is_pciecru = 1;
		break;

	case CLK_PPLL_125M_MATRIX:
		id = 60;
		mask = CLK_MATRIX_125M_SRC_DIV_MASK;
		shift = CLK_MATRIX_125M_SRC_DIV_SHIFT;
		break;
	case CLK_GMAC1_VPU_25M:
		id = 60;
		mask = CLK_MATRIX_25M_SRC_DIV_MASK;
		shift = CLK_MATRIX_25M_SRC_DIV_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	div = DIV_ROUND_UP(priv->ppll_hz, rate);
	if (is_pciecru)
		rk_clrsetreg(&cru->pcieclksel_con[id], mask, (div - 1) << shift);
	else
		rk_clrsetreg(&cru->clksel_con[id], mask, (div - 1) << shift);

	return rk3528_ppll_matrix_get_rate(priv, clk_id);
}

static ulong rk3528_cgpll_matrix_get_rate(struct rk3528_clk_priv *priv,
					  ulong clk_id)
{
	struct rk3528_cru *cru = priv->cru;
	u32 sel, div, mask, shift, con;
	u32 sel_mask = 0, sel_shift;
	u8 is_gpll_parent = 1;
	u8 is_halfdiv = 0;
	ulong prate;

	switch (clk_id) {
	case CLK_MATRIX_50M_SRC:
		con = 0;
		mask = CLK_MATRIX_50M_SRC_DIV_MASK;
		shift = CLK_MATRIX_50M_SRC_DIV_SHIFT;
		is_gpll_parent = 0;
		break;

	case CLK_MATRIX_100M_SRC:
		con = 0;
		mask = CLK_MATRIX_100M_SRC_DIV_MASK;
		shift = CLK_MATRIX_100M_SRC_DIV_SHIFT;
		is_gpll_parent = 0;
		break;

	case CLK_MATRIX_150M_SRC:
		con = 1;
		mask = CLK_MATRIX_150M_SRC_DIV_MASK;
		shift = CLK_MATRIX_150M_SRC_DIV_SHIFT;
		break;

	case CLK_MATRIX_200M_SRC:
		con = 1;
		mask = CLK_MATRIX_200M_SRC_DIV_MASK;
		shift = CLK_MATRIX_200M_SRC_DIV_SHIFT;
		break;

	case CLK_MATRIX_250M_SRC:
		con = 1;
		mask = CLK_MATRIX_250M_SRC_DIV_MASK;
		shift = CLK_MATRIX_250M_SRC_DIV_SHIFT;
		sel_mask = CLK_MATRIX_250M_SRC_SEL_MASK;
		sel_shift = CLK_MATRIX_250M_SRC_SEL_SHIFT;
		break;

	case CLK_MATRIX_300M_SRC:
		con = 2;
		mask = CLK_MATRIX_300M_SRC_DIV_MASK;
		shift = CLK_MATRIX_300M_SRC_DIV_SHIFT;
		break;

	case CLK_MATRIX_339M_SRC:
		con = 2;
		mask = CLK_MATRIX_339M_SRC_DIV_MASK;
		shift = CLK_MATRIX_339M_SRC_DIV_SHIFT;
		is_halfdiv = 1;
		break;

	case CLK_MATRIX_400M_SRC:
		con = 2;
		mask = CLK_MATRIX_400M_SRC_DIV_MASK;
		shift = CLK_MATRIX_400M_SRC_DIV_SHIFT;
		break;

	case CLK_MATRIX_500M_SRC:
		con = 3;
		mask = CLK_MATRIX_500M_SRC_DIV_MASK;
		shift = CLK_MATRIX_500M_SRC_DIV_SHIFT;
		sel_mask = CLK_MATRIX_500M_SRC_SEL_MASK;
		sel_shift = CLK_MATRIX_500M_SRC_SEL_SHIFT;
		break;

	case CLK_MATRIX_600M_SRC:
		con = 4;
		mask = CLK_MATRIX_600M_SRC_DIV_MASK;
		shift = CLK_MATRIX_600M_SRC_DIV_SHIFT;
		break;

	case ACLK_BUS_VOPGL_ROOT:
	case ACLK_BUS_VOPGL_BIU:
		con = 43;
		mask = ACLK_BUS_VOPGL_ROOT_DIV_MASK;
		shift = ACLK_BUS_VOPGL_ROOT_DIV_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	if (sel_mask) {
		sel = (readl(&cru->clksel_con[con]) & sel_mask) >> sel_shift;
		if (sel == CLK_MATRIX_250M_SRC_SEL_CLK_GPLL_MUX) // TODO
			prate = priv->gpll_hz;
		else
			prate = priv->cpll_hz;
	} else {
		if (is_gpll_parent)
			prate = priv->gpll_hz;
		else
			prate = priv->cpll_hz;
	}

	div = (readl(&cru->clksel_con[con]) & mask) >> shift;

	/* NOTE: '-1' to balance the DIV_TO_RATE() 'div+1' */
	return is_halfdiv ? DIV_TO_RATE(prate * 2, (3 + 2 * div) - 1) : DIV_TO_RATE(prate, div);
}

static ulong rk3528_cgpll_matrix_set_rate(struct rk3528_clk_priv *priv,
					  ulong clk_id, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 sel, div, mask, shift, con;
	u32 sel_mask = 0, sel_shift;
	u8 is_gpll_parent = 1;
	u8 is_halfdiv = 0;
	ulong prate = 0;

	switch (clk_id) {
	case CLK_MATRIX_50M_SRC:
		con = 0;
		mask = CLK_MATRIX_50M_SRC_DIV_MASK;
		shift = CLK_MATRIX_50M_SRC_DIV_SHIFT;
		is_gpll_parent = 0;
		break;

	case CLK_MATRIX_100M_SRC:
		con = 0;
		mask = CLK_MATRIX_100M_SRC_DIV_MASK;
		shift = CLK_MATRIX_100M_SRC_DIV_SHIFT;
		is_gpll_parent = 0;
		break;

	case CLK_MATRIX_150M_SRC:
		con = 1;
		mask = CLK_MATRIX_150M_SRC_DIV_MASK;
		shift = CLK_MATRIX_150M_SRC_DIV_SHIFT;
		break;

	case CLK_MATRIX_200M_SRC:
		con = 1;
		mask = CLK_MATRIX_200M_SRC_DIV_MASK;
		shift = CLK_MATRIX_200M_SRC_DIV_SHIFT;
		break;

	case CLK_MATRIX_250M_SRC:
		con = 1;
		mask = CLK_MATRIX_250M_SRC_DIV_MASK;
		shift = CLK_MATRIX_250M_SRC_DIV_SHIFT;
		sel_mask = CLK_MATRIX_250M_SRC_SEL_MASK;
		sel_shift = CLK_MATRIX_250M_SRC_SEL_SHIFT;
		break;

	case CLK_MATRIX_300M_SRC:
		con = 2;
		mask = CLK_MATRIX_300M_SRC_DIV_MASK;
		shift = CLK_MATRIX_300M_SRC_DIV_SHIFT;
		break;

	case CLK_MATRIX_339M_SRC:
		con = 2;
		mask = CLK_MATRIX_339M_SRC_DIV_MASK;
		shift = CLK_MATRIX_339M_SRC_DIV_SHIFT;
		is_halfdiv = 1;
		break;

	case CLK_MATRIX_400M_SRC:
		con = 2;
		mask = CLK_MATRIX_400M_SRC_DIV_MASK;
		shift = CLK_MATRIX_400M_SRC_DIV_SHIFT;
		break;

	case CLK_MATRIX_500M_SRC:
		con = 3;
		mask = CLK_MATRIX_500M_SRC_DIV_MASK;
		shift = CLK_MATRIX_500M_SRC_DIV_SHIFT;
		sel_mask = CLK_MATRIX_500M_SRC_SEL_MASK;
		sel_shift = CLK_MATRIX_500M_SRC_SEL_SHIFT;
		break;

	case CLK_MATRIX_600M_SRC:
		con = 4;
		mask = CLK_MATRIX_600M_SRC_DIV_MASK;
		shift = CLK_MATRIX_600M_SRC_DIV_SHIFT;
		break;

	case ACLK_BUS_VOPGL_ROOT:
	case ACLK_BUS_VOPGL_BIU:
		con = 43;
		mask = ACLK_BUS_VOPGL_ROOT_DIV_MASK;
		shift = ACLK_BUS_VOPGL_ROOT_DIV_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	if (sel_mask) {
		if (priv->gpll_hz % rate == 0) {
			sel = CLK_MATRIX_250M_SRC_SEL_CLK_GPLL_MUX; // TODO
			prate = priv->gpll_hz;
		} else {
			sel = CLK_MATRIX_250M_SRC_SEL_CLK_CPLL_MUX;
			prate = priv->cpll_hz;
		}
	} else {
		if (is_gpll_parent)
			prate = priv->gpll_hz;
		else
			prate = priv->cpll_hz;
	}

	if (is_halfdiv)
		/* NOTE: '+1' to balance the following rk_clrsetreg() 'div-1' */
		div = DIV_ROUND_UP((prate * 2) - (3 * rate), 2 * rate) + 1;
	else
		div = DIV_ROUND_UP(prate, rate);

	rk_clrsetreg(&cru->clksel_con[con], mask, (div - 1) << shift);
	if (sel_mask)
		rk_clrsetreg(&cru->clksel_con[con], sel_mask, sel << sel_shift);

	return rk3528_cgpll_matrix_get_rate(priv, clk_id);
}

static ulong rk3528_i2c_get_clk(struct rk3528_clk_priv *priv, ulong clk_id)
{
	struct rk3528_cru *cru = priv->cru;
	u32 id, sel, con, mask, shift;
	u8 is_pmucru = 0;
	ulong rate;

	switch (clk_id) {
	case CLK_I2C0:
		id = 79;
		mask = CLK_I2C0_SEL_MASK;
		shift = CLK_I2C0_SEL_SHIFT;
		break;

	case CLK_I2C1:
		id = 79;
		mask = CLK_I2C1_SEL_MASK;
		shift = CLK_I2C1_SEL_SHIFT;
		break;

	case CLK_I2C2:
		id = 0;
		mask = CLK_I2C2_SEL_MASK;
		shift = CLK_I2C2_SEL_SHIFT;
		is_pmucru = 1;
		break;

	case CLK_I2C3:
		id = 63;
		mask = CLK_I2C3_SEL_MASK;
		shift = CLK_I2C3_SEL_SHIFT;
		break;

	case CLK_I2C4:
		id = 85;
		mask = CLK_I2C4_SEL_MASK;
		shift = CLK_I2C4_SEL_SHIFT;
		break;

	case CLK_I2C5:
		id = 63;
		mask = CLK_I2C5_SEL_MASK;
		shift = CLK_I2C5_SEL_SHIFT;
		break;

	case CLK_I2C6:
		id = 64;
		mask = CLK_I2C6_SEL_MASK;
		shift = CLK_I2C6_SEL_SHIFT;
		break;

	case CLK_I2C7:
		id = 86;
		mask = CLK_I2C7_SEL_MASK;
		shift = CLK_I2C7_SEL_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	if (is_pmucru)
		con = readl(&cru->pmuclksel_con[id]);
	else
		con = readl(&cru->clksel_con[id]);
	sel = (con & mask) >> shift;
	if (sel == CLK_I2C3_SEL_CLK_MATRIX_200M_SRC)
		rate = 200 * MHz;
	else if (sel == CLK_I2C3_SEL_CLK_MATRIX_100M_SRC)
		rate = 100 * MHz;
	else if (sel == CLK_I2C3_SEL_CLK_MATRIX_50M_SRC)
		rate = 50 * MHz;
	else
		rate = OSC_HZ;

	return rate;
}

static ulong rk3528_i2c_set_clk(struct rk3528_clk_priv *priv, ulong clk_id,
				ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 id, sel, mask, shift;
	u8 is_pmucru = 0;

	if (rate >= 198 * MHz)
		sel = CLK_I2C3_SEL_CLK_MATRIX_200M_SRC;
	else if (rate >= 99 * MHz)
		sel = CLK_I2C3_SEL_CLK_MATRIX_100M_SRC;
	else if (rate >= 50 * MHz)
		sel = CLK_I2C3_SEL_CLK_MATRIX_50M_SRC;
	else
		sel = CLK_I2C3_SEL_XIN_OSC0_FUNC;

	switch (clk_id) {
	case CLK_I2C0:
		id = 79;
		mask = CLK_I2C0_SEL_MASK;
		shift = CLK_I2C0_SEL_SHIFT;
		break;

	case CLK_I2C1:
		id = 79;
		mask = CLK_I2C1_SEL_MASK;
		shift = CLK_I2C1_SEL_SHIFT;
		break;

	case CLK_I2C2:
		id = 0;
		mask = CLK_I2C2_SEL_MASK;
		shift = CLK_I2C2_SEL_SHIFT;
		is_pmucru = 1;
		break;

	case CLK_I2C3:
		id = 63;
		mask = CLK_I2C3_SEL_MASK;
		shift = CLK_I2C3_SEL_SHIFT;
		break;

	case CLK_I2C4:
		id = 85;
		mask = CLK_I2C4_SEL_MASK;
		shift = CLK_I2C4_SEL_SHIFT;
		break;

	case CLK_I2C5:
		id = 63;
		mask = CLK_I2C5_SEL_MASK;
		shift = CLK_I2C5_SEL_SHIFT;
		break;

	case CLK_I2C6:
		id = 64;
		mask = CLK_I2C6_SEL_MASK;
		shift = CLK_I2C6_SEL_SHIFT;
		break;

	case CLK_I2C7:
		id = 86;
		mask = CLK_I2C7_SEL_MASK;
		shift = CLK_I2C7_SEL_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	if (is_pmucru)
		rk_clrsetreg(&cru->pmuclksel_con[id], mask, sel << shift);
	else
		rk_clrsetreg(&cru->clksel_con[id], mask, sel << shift);

	return rk3528_i2c_get_clk(priv, clk_id);
}

static ulong rk3528_spi_get_clk(struct rk3528_clk_priv *priv, ulong clk_id)
{
	struct rk3528_cru *cru = priv->cru;
	u32 id, sel, con, mask, shift;
	ulong rate;

	switch (clk_id) {
	case CLK_SPI0:
		id = 79;
		mask = CLK_SPI0_SEL_MASK;
		shift = CLK_SPI0_SEL_SHIFT;
		break;

	case CLK_SPI1:
		id = 63;
		mask = CLK_SPI1_SEL_MASK;
		shift = CLK_SPI1_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	con = readl(&cru->clksel_con[id]);
	sel = (con & mask) >> shift;
	if (sel == CLK_SPI1_SEL_CLK_MATRIX_200M_SRC)
		rate = 200 * MHz;
	else if (sel == CLK_SPI1_SEL_CLK_MATRIX_100M_SRC)
		rate = 100 * MHz;
	else if (sel == CLK_SPI1_SEL_CLK_MATRIX_50M_SRC)
		rate = 50 * MHz;
	else
		rate = OSC_HZ;

	return rate;
}

static ulong rk3528_spi_set_clk(struct rk3528_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 id, sel, mask, shift;

	if (rate >= 198 * MHz)
		sel = CLK_SPI1_SEL_CLK_MATRIX_200M_SRC;
	else if (rate >= 99 * MHz)
		sel = CLK_SPI1_SEL_CLK_MATRIX_100M_SRC;
	else if (rate >= 50 * MHz)
		sel = CLK_SPI1_SEL_CLK_MATRIX_50M_SRC;
	else
		sel = CLK_SPI1_SEL_XIN_OSC0_FUNC;

	switch (clk_id) {
	case CLK_SPI0:
		id = 79;
		mask = CLK_SPI0_SEL_MASK;
		shift = CLK_SPI0_SEL_SHIFT;
		break;

	case CLK_SPI1:
		id = 63;
		mask = CLK_SPI1_SEL_MASK;
		shift = CLK_SPI1_SEL_SHIFT;
		break;
	default:
		return -ENOENT;
	}

	rk_clrsetreg(&cru->clksel_con[id], mask, sel << shift);

	return rk3528_spi_get_clk(priv, clk_id);
}

static ulong rk3528_pwm_get_clk(struct rk3528_clk_priv *priv, ulong clk_id)
{
	struct rk3528_cru *cru = priv->cru;
	u32 id, sel, con, mask, shift;
	ulong rate;

	switch (clk_id) {
	case CLK_PWM0:
		id = 44;
		mask = CLK_PWM0_SEL_MASK;
		shift = CLK_PWM0_SEL_SHIFT;
		break;

	case CLK_PWM1:
		id = 44;
		mask = CLK_PWM1_SEL_MASK;
		shift = CLK_PWM1_SEL_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	con = readl(&cru->clksel_con[id]);
	sel = (con & mask) >> shift;
	if (sel == CLK_PWM0_SEL_CLK_MATRIX_100M_SRC)
		rate = 100 * MHz;
	if (sel == CLK_PWM0_SEL_CLK_MATRIX_50M_SRC)
		rate = 50 * MHz;
	else
		rate = OSC_HZ;

	return rate;
}

static ulong rk3528_pwm_set_clk(struct rk3528_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 id, sel, mask, shift;

	if (rate >= 99 * MHz)
		sel = CLK_PWM0_SEL_CLK_MATRIX_100M_SRC;
	else if (rate >= 50 * MHz)
		sel = CLK_PWM0_SEL_CLK_MATRIX_50M_SRC;
	else
		sel = CLK_PWM0_SEL_XIN_OSC0_FUNC;

	switch (clk_id) {
	case CLK_PWM0:
		id = 44;
		mask = CLK_PWM0_SEL_MASK;
		shift = CLK_PWM0_SEL_SHIFT;
		break;

	case CLK_PWM1:
		id = 44;
		mask = CLK_PWM1_SEL_MASK;
		shift = CLK_PWM1_SEL_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	rk_clrsetreg(&cru->clksel_con[id], mask, sel << shift);

	return rk3528_pwm_get_clk(priv, clk_id);
}

static ulong rk3528_adc_get_clk(struct rk3528_clk_priv *priv, ulong clk_id)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div, con;

	con = readl(&cru->clksel_con[74]);
	switch (clk_id) {
	case CLK_SARADC:
		div = (con & CLK_SARADC_DIV_MASK) >>
			CLK_SARADC_DIV_SHIFT;
		break;

	case CLK_TSADC_TSEN:
		div = (con & CLK_TSADC_TSEN_DIV_MASK) >>
			CLK_TSADC_TSEN_DIV_SHIFT;
		break;

	case CLK_TSADC:
		div = (con & CLK_TSADC_DIV_MASK) >>
			CLK_TSADC_DIV_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	return DIV_TO_RATE(OSC_HZ, div);
}

static ulong rk3528_adc_set_clk(struct rk3528_clk_priv *priv,
				ulong clk_id, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div, mask, shift;

	switch (clk_id) {
	case CLK_SARADC:
		mask = CLK_SARADC_DIV_MASK;
		shift =	CLK_SARADC_DIV_SHIFT;
		break;

	case CLK_TSADC_TSEN:
		mask = CLK_TSADC_TSEN_DIV_MASK;
		shift =	CLK_TSADC_TSEN_DIV_SHIFT;
		break;

	case CLK_TSADC:
		mask = CLK_TSADC_DIV_MASK;
		shift =	CLK_TSADC_DIV_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	div = DIV_ROUND_UP(OSC_HZ, rate);
	rk_clrsetreg(&cru->clksel_con[74], mask, (div - 1) << shift);

	return rk3528_adc_get_clk(priv, clk_id);
}

static ulong rk3528_sdmmc_get_clk(struct rk3528_clk_priv *priv, ulong clk_id)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div, sel, con;
	ulong prate;

	con = readl(&cru->clksel_con[85]);
	div = (con & CCLK_SRC_SDMMC0_DIV_MASK) >>
		CCLK_SRC_SDMMC0_DIV_SHIFT;
	sel = (con & CCLK_SRC_SDMMC0_SEL_MASK) >>
		CCLK_SRC_SDMMC0_SEL_SHIFT;

	if (sel == CCLK_SRC_SDMMC0_SEL_CLK_GPLL_MUX)
		prate = priv->gpll_hz;
	else if (sel == CCLK_SRC_SDMMC0_SEL_CLK_CPLL_MUX)
		prate = priv->cpll_hz;
	else
		prate = OSC_HZ;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3528_sdmmc_set_clk(struct rk3528_clk_priv *priv,
				  ulong clk_id, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div, sel;

	if (OSC_HZ % rate == 0) {
		div = DIV_ROUND_UP(OSC_HZ, rate);
		sel = CCLK_SRC_SDMMC0_SEL_XIN_OSC0_FUNC;
	} else if ((priv->cpll_hz % rate) == 0) {
		div = DIV_ROUND_UP(priv->cpll_hz, rate);
		sel = CCLK_SRC_SDMMC0_SEL_CLK_CPLL_MUX;
	} else {
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
		sel = CCLK_SRC_SDMMC0_SEL_CLK_GPLL_MUX;
	}

	assert(div - 1 <= 63);
	rk_clrsetreg(&cru->clksel_con[85],
		     CCLK_SRC_SDMMC0_SEL_MASK |
		     CCLK_SRC_SDMMC0_DIV_MASK,
		     sel << CCLK_SRC_SDMMC0_SEL_SHIFT |
		     (div - 1) << CCLK_SRC_SDMMC0_DIV_SHIFT);

	return rk3528_sdmmc_get_clk(priv, clk_id);
}

static ulong rk3528_sfc_get_clk(struct rk3528_clk_priv *priv)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div, sel, con, parent;

	con = readl(&cru->clksel_con[61]);
	div = (con & SCLK_SFC_DIV_MASK) >>
		SCLK_SFC_DIV_SHIFT;
	sel = (con & SCLK_SFC_SEL_MASK) >>
		SCLK_SFC_SEL_SHIFT;
	if (sel == SCLK_SFC_SEL_CLK_GPLL_MUX)
		parent = priv->gpll_hz;
	else if (sel == SCLK_SFC_SEL_CLK_CPLL_MUX)
		parent = priv->cpll_hz;
	else
		parent = OSC_HZ;

	return DIV_TO_RATE(parent, div);
}

static ulong rk3528_sfc_set_clk(struct rk3528_clk_priv *priv, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	int div, sel;

	if (OSC_HZ % rate == 0) {
		div = DIV_ROUND_UP(OSC_HZ, rate);
		sel = SCLK_SFC_SEL_XIN_OSC0_FUNC;
	} else if ((priv->cpll_hz % rate) == 0) {
		div = DIV_ROUND_UP(priv->cpll_hz, rate);
		sel = SCLK_SFC_SEL_CLK_CPLL_MUX;
	} else {
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
		sel = SCLK_SFC_SEL_CLK_GPLL_MUX;
	}

	assert(div - 1 <= 63);
	rk_clrsetreg(&cru->clksel_con[61],
		     SCLK_SFC_SEL_MASK |
		     SCLK_SFC_DIV_MASK,
		     sel << SCLK_SFC_SEL_SHIFT |
		     (div - 1) << SCLK_SFC_DIV_SHIFT);

	return rk3528_sfc_get_clk(priv);
}

static ulong rk3528_emmc_get_clk(struct rk3528_clk_priv *priv)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div, sel, con, parent;

	con = readl(&cru->clksel_con[62]);
	div = (con & CCLK_SRC_EMMC_DIV_MASK) >>
		CCLK_SRC_EMMC_DIV_SHIFT;
	sel = (con & CCLK_SRC_EMMC_SEL_MASK) >>
		CCLK_SRC_EMMC_SEL_SHIFT;

	if (sel == CCLK_SRC_EMMC_SEL_CLK_GPLL_MUX)
		parent = priv->gpll_hz;
	else if (sel == CCLK_SRC_EMMC_SEL_CLK_CPLL_MUX)
		parent = priv->cpll_hz;
	else
		parent = OSC_HZ;

	return DIV_TO_RATE(parent, div);
}

static ulong rk3528_emmc_set_clk(struct rk3528_clk_priv *priv, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div, sel;

	if (OSC_HZ % rate == 0) {
		div = DIV_ROUND_UP(OSC_HZ, rate);
		sel = CCLK_SRC_EMMC_SEL_XIN_OSC0_FUNC;
	} else if ((priv->cpll_hz % rate) == 0) {
		div = DIV_ROUND_UP(priv->cpll_hz, rate);
		sel = CCLK_SRC_EMMC_SEL_CLK_CPLL_MUX;
	} else {
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
		sel = CCLK_SRC_EMMC_SEL_CLK_GPLL_MUX;
	}

	assert(div - 1 <= 63);
	rk_clrsetreg(&cru->clksel_con[62],
		     CCLK_SRC_EMMC_SEL_MASK |
		     CCLK_SRC_EMMC_DIV_MASK,
		     sel << CCLK_SRC_EMMC_SEL_SHIFT |
		     (div - 1) << CCLK_SRC_EMMC_DIV_SHIFT);

	return rk3528_emmc_get_clk(priv);
}

static ulong rk3528_dclk_vop_get_clk(struct rk3528_clk_priv *priv, ulong clk_id)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div_mask, div_shift;
	u32 sel_mask, sel_shift;
	u32 id, con, sel, div;
	ulong prate;

	switch (clk_id) {
	case DCLK_VOP0:
		id = 32;
		sel_mask = DCLK_VOP_SRC0_SEL_MASK;
		sel_shift = DCLK_VOP_SRC0_SEL_SHIFT;
		/* FIXME if need src: clk_hdmiphy_pixel_io */
		div_mask = DCLK_VOP_SRC0_DIV_MASK;
		div_shift = DCLK_VOP_SRC0_DIV_SHIFT;
		break;

	case DCLK_VOP1:
		id = 33;
		sel_mask = DCLK_VOP_SRC1_SEL_MASK;
		sel_shift = DCLK_VOP_SRC1_SEL_SHIFT;
		div_mask = DCLK_VOP_SRC1_DIV_MASK;
		div_shift = DCLK_VOP_SRC1_DIV_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	con = readl(&cru->clksel_con[id]);
	div = (con & div_mask) >> div_shift;
	sel = (con & sel_mask) >> sel_shift;
	if (sel == DCLK_VOP_SRC_SEL_CLK_GPLL_MUX)
		prate = priv->gpll_hz;
	else
		prate = priv->cpll_hz;

	return DIV_TO_RATE(prate, div);
}

static ulong rk3528_dclk_vop_set_clk(struct rk3528_clk_priv *priv,
				     ulong clk_id, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 div_mask, div_shift;
	u32 sel_mask, sel_shift;
	u32 id, sel, div;
	ulong prate;

	switch (clk_id) {
	case DCLK_VOP0:
		id = 32;
		sel_mask = DCLK_VOP_SRC0_SEL_MASK;
		sel_shift = DCLK_VOP_SRC0_SEL_SHIFT;
		/* FIXME if need src: clk_hdmiphy_pixel_io */
		div_mask = DCLK_VOP_SRC0_DIV_MASK;
		div_shift = DCLK_VOP_SRC0_DIV_SHIFT;
		break;

	case DCLK_VOP1:
		id = 33;
		sel_mask = DCLK_VOP_SRC1_SEL_MASK;
		sel_shift = DCLK_VOP_SRC1_SEL_SHIFT;
		div_mask = DCLK_VOP_SRC1_DIV_MASK;
		div_shift = DCLK_VOP_SRC1_DIV_SHIFT;
		break;

	default:
		return -ENOENT;
	}

	if ((priv->gpll_hz % rate) == 0) {
		prate = priv->gpll_hz;
		sel = (DCLK_VOP_SRC_SEL_CLK_GPLL_MUX << sel_shift) & sel_mask;
	} else {
		prate = priv->cpll_hz;
		sel = (DCLK_VOP_SRC_SEL_CLK_CPLL_MUX << sel_shift) & sel_mask;
	}

	div = ((DIV_ROUND_UP(prate, rate) - 1) << div_shift) & div_mask;
	rk_clrsetreg(&cru->clksel_con[id], sel, div);

	return rk3528_dclk_vop_get_clk(priv, clk_id);
}

static ulong rk3528_uart_get_rate(struct rk3528_clk_priv *priv, ulong clk_id)
{
	struct rk3528_cru *cru = priv->cru;
	u32 sel_shift, sel_mask, div_shift, div_mask;
	u32 sel, id, con, frac_div, div;
	ulong m, n, rate;

	switch (clk_id) {
	case SCLK_UART0:
		id = 6;
		sel_shift = SCLK_UART0_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART0_SRC_SEL_MASK;
		div_shift = CLK_UART0_SRC_DIV_SHIFT;
		div_mask = CLK_UART0_SRC_DIV_MASK;
		break;

	case SCLK_UART1:
		id = 8;
		sel_shift = SCLK_UART1_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART1_SRC_SEL_MASK;
		div_shift = CLK_UART1_SRC_DIV_SHIFT;
		div_mask = CLK_UART1_SRC_DIV_MASK;
		break;

	case SCLK_UART2:
		id = 10;
		sel_shift = SCLK_UART2_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART2_SRC_SEL_MASK;
		div_shift = CLK_UART2_SRC_DIV_SHIFT;
		div_mask = CLK_UART2_SRC_DIV_MASK;
		break;

	case SCLK_UART3:
		id = 12;
		sel_shift = SCLK_UART3_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART3_SRC_SEL_MASK;
		div_shift = CLK_UART3_SRC_DIV_SHIFT;
		div_mask = CLK_UART3_SRC_DIV_MASK;
		break;

	case SCLK_UART4:
		id = 14;
		sel_shift = SCLK_UART4_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART4_SRC_SEL_MASK;
		div_shift = CLK_UART4_SRC_DIV_SHIFT;
		div_mask = CLK_UART4_SRC_DIV_MASK;
		break;

	case SCLK_UART5:
		id = 16;
		sel_shift = SCLK_UART5_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART5_SRC_SEL_MASK;
		div_shift = CLK_UART5_SRC_DIV_SHIFT;
		div_mask = CLK_UART5_SRC_DIV_MASK;
		break;

	case SCLK_UART6:
		id = 18;
		sel_shift = SCLK_UART6_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART6_SRC_SEL_MASK;
		div_shift = CLK_UART6_SRC_DIV_SHIFT;
		div_mask = CLK_UART6_SRC_DIV_MASK;
		break;

	case SCLK_UART7:
		id = 20;
		sel_shift = SCLK_UART7_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART7_SRC_SEL_MASK;
		div_shift = CLK_UART7_SRC_DIV_SHIFT;
		div_mask = CLK_UART7_SRC_DIV_MASK;
		break;

	default:
		return -ENOENT;
	}

	con = readl(&cru->clksel_con[id - 2]);
	div = (con & div_mask) >> div_shift;

	con = readl(&cru->clksel_con[id]);
	sel = (con & sel_mask) >> sel_shift;

	if (sel == SCLK_UART0_SRC_SEL_CLK_UART0_SRC) {
		rate = DIV_TO_RATE(priv->gpll_hz, div);
	} else if (sel == SCLK_UART0_SRC_SEL_CLK_UART0_FRAC) {
		frac_div = readl(&cru->clksel_con[id - 1]);
		n = (frac_div & 0xffff0000) >> 16;
		m = frac_div & 0x0000ffff;
		rate = DIV_TO_RATE(priv->gpll_hz, div) * n / m;
	} else {
		rate = OSC_HZ;
	}

	return rate;
}

static ulong rk3528_uart_set_rate(struct rk3528_clk_priv *priv,
				  ulong clk_id, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 sel_shift, sel_mask, div_shift, div_mask;
	u32 sel, id, div;
	ulong m = 0, n = 0, val;

	if (rate == OSC_HZ) {
		sel = SCLK_UART0_SRC_SEL_XIN_OSC0_FUNC;
		div = DIV_ROUND_UP(OSC_HZ, rate);
	} else if (priv->gpll_hz % rate == 0) {
		sel = SCLK_UART0_SRC_SEL_CLK_UART0_SRC;
		div = DIV_ROUND_UP(priv->gpll_hz, rate);
	} else {
		sel = SCLK_UART0_SRC_SEL_CLK_UART0_FRAC;
		div = 2;
		rational_best_approximation(rate, priv->gpll_hz / div,
					    GENMASK(16 - 1, 0),
					    GENMASK(16 - 1, 0),
					    &n, &m);
	}

	switch (clk_id) {
	case SCLK_UART0:
		id = 6;
		sel_shift = SCLK_UART0_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART0_SRC_SEL_MASK;
		div_shift = CLK_UART0_SRC_DIV_SHIFT;
		div_mask = CLK_UART0_SRC_DIV_MASK;
		break;

	case SCLK_UART1:
		id = 8;
		sel_shift = SCLK_UART1_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART1_SRC_SEL_MASK;
		div_shift = CLK_UART1_SRC_DIV_SHIFT;
		div_mask = CLK_UART1_SRC_DIV_MASK;
		break;

	case SCLK_UART2:
		id = 10;
		sel_shift = SCLK_UART2_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART2_SRC_SEL_MASK;
		div_shift = CLK_UART2_SRC_DIV_SHIFT;
		div_mask = CLK_UART2_SRC_DIV_MASK;
		break;

	case SCLK_UART3:
		id = 12;
		sel_shift = SCLK_UART3_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART3_SRC_SEL_MASK;
		div_shift = CLK_UART3_SRC_DIV_SHIFT;
		div_mask = CLK_UART3_SRC_DIV_MASK;
		break;

	case SCLK_UART4:
		id = 14;
		sel_shift = SCLK_UART4_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART4_SRC_SEL_MASK;
		div_shift = CLK_UART4_SRC_DIV_SHIFT;
		div_mask = CLK_UART4_SRC_DIV_MASK;
		break;

	case SCLK_UART5:
		id = 16;
		sel_shift = SCLK_UART5_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART5_SRC_SEL_MASK;
		div_shift = CLK_UART5_SRC_DIV_SHIFT;
		div_mask = CLK_UART5_SRC_DIV_MASK;
		break;

	case SCLK_UART6:
		id = 18;
		sel_shift = SCLK_UART6_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART6_SRC_SEL_MASK;
		div_shift = CLK_UART6_SRC_DIV_SHIFT;
		div_mask = CLK_UART6_SRC_DIV_MASK;
		break;

	case SCLK_UART7:
		id = 20;
		sel_shift = SCLK_UART7_SRC_SEL_SHIFT;
		sel_mask = SCLK_UART7_SRC_SEL_MASK;
		div_shift = CLK_UART7_SRC_DIV_SHIFT;
		div_mask = CLK_UART7_SRC_DIV_MASK;
		break;

	default:
		return -ENOENT;
	}

	rk_clrsetreg(&cru->clksel_con[id - 2], div_mask, (div - 1) << div_shift);
	rk_clrsetreg(&cru->clksel_con[id], sel_mask, sel << sel_shift);
	if (m && n) {
		val = n << 16 | m;
		writel(val, &cru->clksel_con[id - 1]);
	}

	return rk3528_uart_get_rate(priv, clk_id);
}

static ulong rk3528_clk_get_rate(struct clk *clk)
{
	struct rk3528_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	if (!priv->gpll_hz || !priv->cpll_hz) {
		printf("%s: gpll=%lu, cpll=%ld\n",
		       __func__, priv->gpll_hz, priv->cpll_hz);
		return -ENOENT;
	}

	switch (clk->id) {
	case PLL_APLL:
	case ARMCLK:
		rate = rockchip_pll_get_rate(&rk3528_pll_clks[APLL], priv->cru,
					     APLL);
		break;
	case PLL_CPLL:
		rate = rockchip_pll_get_rate(&rk3528_pll_clks[CPLL], priv->cru,
					     CPLL);
		break;
	case PLL_GPLL:
		rate = rockchip_pll_get_rate(&rk3528_pll_clks[GPLL], priv->cru,
					     GPLL);
		break;

	case PLL_PPLL:
		rate = rockchip_pll_get_rate(&rk3528_pll_clks[PPLL], priv->cru,
					     PPLL);
		break;
	case PLL_DPLL:
		rate = rockchip_pll_get_rate(&rk3528_pll_clks[DPLL], priv->cru,
					     DPLL);
		break;

	case TCLK_EMMC:
	case TCLK_WDT_NS:
		rate = OSC_HZ;
		break;
	case CLK_I2C0:
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C5:
	case CLK_I2C6:
	case CLK_I2C7:
		rate = rk3528_i2c_get_clk(priv, clk->id);
		break;
	case CLK_SPI0:
	case CLK_SPI1:
		rate = rk3528_spi_get_clk(priv, clk->id);
		break;
	case CLK_PWM0:
	case CLK_PWM1:
		rate = rk3528_pwm_get_clk(priv, clk->id);
		break;
	case CLK_SARADC:
	case CLK_TSADC:
	case CLK_TSADC_TSEN:
		rate = rk3528_adc_get_clk(priv, clk->id);
		break;
	case CCLK_SRC_EMMC:
		rate = rk3528_emmc_get_clk(priv);
		break;
	case HCLK_SDMMC0:
	case CCLK_SRC_SDMMC0:
		rate = rk3528_sdmmc_get_clk(priv, clk->id);
		break;
	case SCLK_SFC:
		rate = rk3528_sfc_get_clk(priv);
		break;
	case DCLK_VOP0:
	case DCLK_VOP1:
		rate = rk3528_dclk_vop_get_clk(priv, clk->id);
		break;
	case DCLK_CVBS:
		rate = rk3528_dclk_vop_get_clk(priv, DCLK_VOP1) / 4;
		break;
	case DCLK_4X_CVBS:
		rate = rk3528_dclk_vop_get_clk(priv, DCLK_VOP1);
		break;
	case SCLK_UART0:
	case SCLK_UART1:
	case SCLK_UART2:
	case SCLK_UART3:
	case SCLK_UART4:
	case SCLK_UART5:
	case SCLK_UART6:
	case SCLK_UART7:
		rate = rk3528_uart_get_rate(priv, clk->id);
		break;
	case CLK_MATRIX_50M_SRC:
	case CLK_MATRIX_100M_SRC:
	case CLK_MATRIX_150M_SRC:
	case CLK_MATRIX_200M_SRC:
	case CLK_MATRIX_250M_SRC:
	case CLK_MATRIX_300M_SRC:
	case CLK_MATRIX_339M_SRC:
	case CLK_MATRIX_400M_SRC:
	case CLK_MATRIX_500M_SRC:
	case CLK_MATRIX_600M_SRC:
	case ACLK_BUS_VOPGL_BIU:
		rate = rk3528_cgpll_matrix_get_rate(priv, clk->id);
		break;
	case CLK_PPLL_50M_MATRIX:
	case CLK_PPLL_100M_MATRIX:
	case CLK_PPLL_125M_MATRIX:
	case CLK_GMAC1_VPU_25M:
	case CLK_GMAC1_RMII_VPU:
	case CLK_GMAC1_SRC_VPU:
		rate = rk3528_ppll_matrix_get_rate(priv, clk->id);
		break;
	default:
		return -ENOENT;
	}

	return rate;
};

static ulong rk3528_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3528_clk_priv *priv = dev_get_priv(clk->dev);
	ulong ret = 0;

	if (!priv->gpll_hz) {
		printf("%s gpll=%lu\n", __func__, priv->gpll_hz);
		return -ENOENT;
	}

	switch (clk->id) {
	case PLL_APLL:
	case ARMCLK:
		if (priv->armclk_hz)
			rk3528_armclk_set_clk(priv, rate);
		priv->armclk_hz = rate;
		break;
	case PLL_CPLL:
		ret = rockchip_pll_set_rate(&rk3528_pll_clks[CPLL], priv->cru,
					    CPLL, rate);
		priv->cpll_hz = rockchip_pll_get_rate(&rk3528_pll_clks[CPLL],
						      priv->cru, CPLL);
		break;
	case PLL_GPLL:
		ret = rockchip_pll_set_rate(&rk3528_pll_clks[GPLL], priv->cru,
					    GPLL, rate);
		priv->gpll_hz = rockchip_pll_get_rate(&rk3528_pll_clks[GPLL],
						      priv->cru, GPLL);
		break;
	case PLL_PPLL:
		ret = rockchip_pll_set_rate(&rk3528_pll_clks[PPLL], priv->cru,
					    PPLL, rate);
		priv->ppll_hz = rockchip_pll_get_rate(&rk3528_pll_clks[PPLL],
						      priv->cru, PPLL);
		break;
	case TCLK_EMMC:
	case TCLK_WDT_NS:
		return (rate == OSC_HZ) ? 0 : -EINVAL;
	case CLK_I2C0:
	case CLK_I2C1:
	case CLK_I2C2:
	case CLK_I2C3:
	case CLK_I2C4:
	case CLK_I2C5:
	case CLK_I2C6:
	case CLK_I2C7:
		ret = rk3528_i2c_set_clk(priv, clk->id, rate);
		break;
	case CLK_SPI0:
	case CLK_SPI1:
		ret = rk3528_spi_set_clk(priv, clk->id, rate);
		break;
	case CLK_PWM0:
	case CLK_PWM1:
		ret = rk3528_pwm_set_clk(priv, clk->id, rate);
		break;
	case CLK_SARADC:
	case CLK_TSADC:
	case CLK_TSADC_TSEN:
		ret = rk3528_adc_set_clk(priv, clk->id, rate);
		break;
	case HCLK_SDMMC0:
	case CCLK_SRC_SDMMC0:
		ret = rk3528_sdmmc_set_clk(priv, clk->id, rate);
		break;
	case SCLK_SFC:
		ret = rk3528_sfc_set_clk(priv, rate);
		break;
	case CCLK_SRC_EMMC:
		ret = rk3528_emmc_set_clk(priv, rate);
		break;
	case DCLK_VOP0:
	case DCLK_VOP1:
		ret = rk3528_dclk_vop_set_clk(priv, clk->id, rate);
		break;
	case SCLK_UART0:
	case SCLK_UART1:
	case SCLK_UART2:
	case SCLK_UART3:
	case SCLK_UART4:
	case SCLK_UART5:
	case SCLK_UART6:
	case SCLK_UART7:
		ret = rk3528_uart_set_rate(priv, clk->id, rate);
		break;
	case CLK_MATRIX_50M_SRC:
	case CLK_MATRIX_100M_SRC:
	case CLK_MATRIX_150M_SRC:
	case CLK_MATRIX_200M_SRC:
	case CLK_MATRIX_250M_SRC:
	case CLK_MATRIX_300M_SRC:
	case CLK_MATRIX_339M_SRC:
	case CLK_MATRIX_400M_SRC:
	case CLK_MATRIX_500M_SRC:
	case CLK_MATRIX_600M_SRC:
	case ACLK_BUS_VOPGL_BIU:
		ret = rk3528_cgpll_matrix_set_rate(priv, clk->id, rate);
		break;
	case CLK_PPLL_50M_MATRIX:
	case CLK_PPLL_100M_MATRIX:
	case CLK_PPLL_125M_MATRIX:
	case CLK_GMAC1_VPU_25M:
		ret = rk3528_ppll_matrix_set_rate(priv, clk->id, rate);
		break;
	case CLK_GMAC1_RMII_VPU:
	case CLK_GMAC1_SRC_VPU:
		/* dummy set */
		ret = rk3528_ppll_matrix_get_rate(priv, clk->id);
		break;

	/* Might occur in cru assigned-clocks, can be ignored here */
	case ACLK_BUS_VOPGL_ROOT:
	case BCLK_EMMC:
	case CLK_REF_PCIE_INNER_PHY:
	case XIN_OSC0_DIV:
		ret = 0;
		break;
	default:
		return -ENOENT;
	}

	return ret;
};

static struct clk_ops rk3528_clk_ops = {
	.get_rate = rk3528_clk_get_rate,
	.set_rate = rk3528_clk_set_rate,
};

#ifdef CONFIG_XPL_BUILD

#define COREGRF_BASE	0xff300000
#define PVTPLL_CON0_L	0x0
#define PVTPLL_CON0_H	0x4

static int rk3528_cpu_pvtpll_set_rate(struct rk3528_clk_priv *priv, ulong rate)
{
	struct rk3528_cru *cru = priv->cru;
	u32 length;

	if (rate >= 1200000000)
		length = 8;
	else if (rate >= 1008000000)
		length = 11;
	else
		length = 17;

	/* set pclk dbg div to 9 */
	rk_clrsetreg(&cru->clksel_con[40], RK3528_DIV_PCLK_DBG_MASK,
		     9 << RK3528_DIV_PCLK_DBG_SHIFT);
	/* set aclk_m_core div to 1 */
	rk_clrsetreg(&cru->clksel_con[39], RK3528_DIV_ACLK_M_CORE_MASK,
		     1 << RK3528_DIV_ACLK_M_CORE_SHIFT);

	/* set ring sel = 1 */
	writel(0x07000000 | (1 << 8), COREGRF_BASE + PVTPLL_CON0_L);
	/* set length */
	writel(0x007f0000 | length, COREGRF_BASE + PVTPLL_CON0_H);
	/* enable pvtpll */
	writel(0x00020002, COREGRF_BASE + PVTPLL_CON0_L);
	/* start monitor */
	writel(0x00010001, COREGRF_BASE + PVTPLL_CON0_L);

	/* set core mux pvtpll */
	writel(0x00010001, &cru->clksel_con[40]);
	writel(0x00100010, &cru->clksel_con[39]);

	/* set pclk dbg div to 8 */
	rk_clrsetreg(&cru->clksel_con[40], RK3528_DIV_PCLK_DBG_MASK,
		     8 << RK3528_DIV_PCLK_DBG_SHIFT);

	return 0;
}
#endif

static int rk3528_clk_init(struct rk3528_clk_priv *priv)
{
	int ret;

	priv->sync_kernel = false;

#ifdef CONFIG_XPL_BUILD
	/*
	 * BOOTROM:
	 *	CPU 1902/2(postdiv1)=546M
	 *	CPLL 996/2(postdiv1)=498M
	 *	GPLL 1188/2(postdiv1)=594M
	 *	   |-- clk_matrix_200m_src_div=1 => rate: 300M
	 *	   |-- clk_matrix_300m_src_div=2 => rate: 200M
	 *
	 * Avoid overclocking when change GPLL rate:
	 *	Change clk_matrix_200m_src_div to 5.
	 *	Change clk_matrix_300m_src_div to 3.
	 */
	writel(0x01200120, &priv->cru->clksel_con[1]);
	writel(0x00030003, &priv->cru->clksel_con[2]);

	if (!priv->armclk_enter_hz) {
		priv->armclk_enter_hz =
			rockchip_pll_get_rate(&rk3528_pll_clks[APLL],
					      priv->cru, APLL);
		priv->armclk_init_hz = priv->armclk_enter_hz;
	}

	if (priv->armclk_init_hz != APLL_HZ) {
		ret = rk3528_armclk_set_clk(priv, APLL_HZ);
		if (!ret)
			priv->armclk_init_hz = APLL_HZ;
	}

	if (!rk3528_cpu_pvtpll_set_rate(priv, CPU_PVTPLL_HZ)) {
		debug("cpu pvtpll %d KHz\n", CPU_PVTPLL_HZ / 1000);
		priv->armclk_init_hz = CPU_PVTPLL_HZ;
	}
#endif

	if (priv->cpll_hz != CPLL_HZ) {
		ret = rockchip_pll_set_rate(&rk3528_pll_clks[CPLL], priv->cru,
					    CPLL, CPLL_HZ);
		if (!ret)
			priv->cpll_hz = CPLL_HZ;
	}

	if (priv->gpll_hz != GPLL_HZ) {
		ret = rockchip_pll_set_rate(&rk3528_pll_clks[GPLL], priv->cru,
					    GPLL, GPLL_HZ);
		if (!ret)
			priv->gpll_hz = GPLL_HZ;
	}

	if (priv->ppll_hz != PPLL_HZ) {
		ret = rockchip_pll_set_rate(&rk3528_pll_clks[PPLL], priv->cru,
					    PPLL, PPLL_HZ);
		if (!ret)
			priv->ppll_hz = PPLL_HZ;
	}

#ifdef CONFIG_XPL_BUILD
	/* Init to override bootrom config */
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_50M_SRC,   50000000);
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_100M_SRC, 100000000);
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_150M_SRC, 150000000);
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_200M_SRC, 200000000);
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_250M_SRC, 250000000);
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_300M_SRC, 300000000);
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_339M_SRC, 340000000);
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_400M_SRC, 400000000);
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_500M_SRC, 500000000);
	rk3528_cgpll_matrix_set_rate(priv, CLK_MATRIX_600M_SRC, 600000000);
	rk3528_cgpll_matrix_set_rate(priv, ACLK_BUS_VOPGL_BIU,  500000000);

	/* The default rate is 100Mhz, it's not friendly for remote IR module */
	rk3528_pwm_set_clk(priv, CLK_PWM0, 24000000);
	rk3528_pwm_set_clk(priv, CLK_PWM1, 24000000);
#endif
	return 0;
}

static int rk3528_clk_probe(struct udevice *dev)
{
	struct rk3528_clk_priv *priv = dev_get_priv(dev);
	int ret;

	ret = rk3528_clk_init(priv);
	if (ret)
		return ret;

	/* Process 'assigned-{clocks/clock-parents/clock-rates}' properties */
	ret = clk_set_defaults(dev, 1);
	if (ret)
		debug("%s clk_set_defaults failed %d\n", __func__, ret);
	else
		priv->sync_kernel = true;

	return 0;
}

static int rk3528_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct rk3528_clk_priv *priv = dev_get_priv(dev);

	priv->cru = dev_read_addr_ptr(dev);

	return 0;
}

static int rk3528_clk_bind(struct udevice *dev)
{
	struct udevice *sys_child;
	struct sysreset_reg *priv;
	int ret;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(dev, "rockchip_sysreset", "sysreset",
				 &sys_child);
	if (ret) {
		debug("Warning: No sysreset driver: ret=%d\n", ret);
	} else {
		priv = malloc(sizeof(struct sysreset_reg));
		priv->glb_srst_fst_value = offsetof(struct rk3528_cru,
						    glb_srst_fst);
		priv->glb_srst_snd_value = offsetof(struct rk3528_cru,
						    glb_srst_snd);
		dev_set_priv(sys_child, priv);
	}

#if CONFIG_IS_ENABLED(RESET_ROCKCHIP)
	ret = offsetof(struct rk3528_cru, softrst_con[0]);
	ret = rk3528_reset_bind_lut(dev, ret, 47);
	if (ret)
		debug("Warning: software reset driver bind failed\n");
#endif

	return 0;
}

static const struct udevice_id rk3528_clk_ids[] = {
	{ .compatible = "rockchip,rk3528-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3528_cru) = {
	.name		= "rockchip_rk3528_cru",
	.id		= UCLASS_CLK,
	.of_match	= rk3528_clk_ids,
	.priv_auto	= sizeof(struct rk3528_clk_priv),
	.of_to_plat	= rk3528_clk_ofdata_to_platdata,
	.ops		= &rk3528_clk_ops,
	.bind		= rk3528_clk_bind,
	.probe		= rk3528_clk_probe,
};

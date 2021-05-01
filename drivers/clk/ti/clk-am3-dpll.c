// SPDX-License-Identifier: GPL-2.0+
/*
 * TI DPLL clock support
 *
 * Copyright (C) 2020 Dario Binacchi <dariobin@libero.it>
 *
 * Loosely based on Linux kernel drivers/clk/ti/dpll.c
 */

#include <common.h>
#include <clk.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <hang.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include "clk.h"

struct clk_ti_am3_dpll_drv_data {
	ulong max_rate;
};

struct clk_ti_am3_dpll_priv {
	struct clk_ti_reg clkmode_reg;
	struct clk_ti_reg idlest_reg;
	struct clk_ti_reg clksel_reg;
	struct clk clk_bypass;
	struct clk clk_ref;
	u16 last_rounded_mult;
	u8 last_rounded_div;
	ulong max_rate;
};

static ulong clk_ti_am3_dpll_round_rate(struct clk *clk, ulong rate)
{
	struct clk_ti_am3_dpll_priv *priv = dev_get_priv(clk->dev);
	ulong ret, ref_rate, r;
	int m, d, err_min, err;
	int mult = INT_MAX, div = INT_MAX;

	if (priv->max_rate && rate > priv->max_rate) {
		dev_warn(clk->dev, "%ld is to high a rate, lowered to %ld\n",
			 rate, priv->max_rate);
		rate = priv->max_rate;
	}

	ret = -EFAULT;
	err = rate;
	err_min = rate;
	ref_rate = clk_get_rate(&priv->clk_ref);
	for (d = 1; err_min && d <= 128; d++) {
		for (m = 2; m <= 2047; m++) {
			r = (ref_rate * m) / d;
			err = abs(r - rate);
			if (err < err_min) {
				err_min = err;
				ret = r;
				mult = m;
				div = d;

				if (err == 0)
					break;
			} else if (r > rate) {
				break;
			}
		}
	}

	priv->last_rounded_mult = mult;
	priv->last_rounded_div = div;
	dev_dbg(clk->dev, "rate=%ld, best_rate=%ld, mult=%d, div=%d\n", rate,
		ret, mult, div);
	return ret;
}

static void clk_ti_am3_dpll_clken(struct clk_ti_am3_dpll_priv *priv,
				  u8 clken_bits)
{
	u32 v;

	v = clk_ti_readl(&priv->clkmode_reg);
	v &= ~CM_CLKMODE_DPLL_DPLL_EN_MASK;
	v |= clken_bits << CM_CLKMODE_DPLL_EN_SHIFT;
	clk_ti_writel(v, &priv->clkmode_reg);
}

static int clk_ti_am3_dpll_state(struct clk *clk, u8 state)
{
	struct clk_ti_am3_dpll_priv *priv = dev_get_priv(clk->dev);
	u32 i = 0, v;

	do {
		v = clk_ti_readl(&priv->idlest_reg) & ST_DPLL_CLK_MASK;
		if (v == state) {
			dev_dbg(clk->dev, "transition to '%s' in %d loops\n",
				state ? "locked" : "bypassed", i);
			return 1;
		}

	} while (++i < LDELAY);

	dev_err(clk->dev, "failed transition to '%s'\n",
		state ? "locked" : "bypassed");
	return 0;
}

static ulong clk_ti_am3_dpll_set_rate(struct clk *clk, ulong rate)
{
	struct clk_ti_am3_dpll_priv *priv = dev_get_priv(clk->dev);
	u32 v;
	ulong round_rate;

	round_rate = clk_ti_am3_dpll_round_rate(clk, rate);
	if (IS_ERR_VALUE(round_rate))
		return round_rate;

	v = clk_ti_readl(&priv->clksel_reg);

	/* enter bypass mode */
	clk_ti_am3_dpll_clken(priv, DPLL_EN_MN_BYPASS);

	/* wait for bypass mode */
	clk_ti_am3_dpll_state(clk, 0);

	/* set M & N */
	v &= ~CM_CLKSEL_DPLL_M_MASK;
	v |= (priv->last_rounded_mult << CM_CLKSEL_DPLL_M_SHIFT) &
		CM_CLKSEL_DPLL_M_MASK;

	v &= ~CM_CLKSEL_DPLL_N_MASK;
	v |= ((priv->last_rounded_div - 1) << CM_CLKSEL_DPLL_N_SHIFT) &
		CM_CLKSEL_DPLL_N_MASK;

	clk_ti_writel(v, &priv->clksel_reg);

	/* lock dpll */
	clk_ti_am3_dpll_clken(priv, DPLL_EN_LOCK);

	/* wait till the dpll locks */
	if (!clk_ti_am3_dpll_state(clk, ST_DPLL_CLK_MASK))
		hang();

	return round_rate;
}

static ulong clk_ti_am3_dpll_get_rate(struct clk *clk)
{
	struct clk_ti_am3_dpll_priv *priv = dev_get_priv(clk->dev);
	u64 rate;
	u32 m, n, v;

	/* Return bypass rate if DPLL is bypassed */
	v = clk_ti_readl(&priv->clkmode_reg);
	v &= CM_CLKMODE_DPLL_EN_MASK;
	v >>= CM_CLKMODE_DPLL_EN_SHIFT;

	switch (v) {
	case DPLL_EN_MN_BYPASS:
	case DPLL_EN_LOW_POWER_BYPASS:
	case DPLL_EN_FAST_RELOCK_BYPASS:
		rate = clk_get_rate(&priv->clk_bypass);
		dev_dbg(clk->dev, "rate=%lld\n", rate);
		return rate;
	}

	v = clk_ti_readl(&priv->clksel_reg);
	m = v & CM_CLKSEL_DPLL_M_MASK;
	m >>= CM_CLKSEL_DPLL_M_SHIFT;
	n = v & CM_CLKSEL_DPLL_N_MASK;
	n >>= CM_CLKSEL_DPLL_N_SHIFT;

	rate = clk_get_rate(&priv->clk_ref) * m;
	do_div(rate, n + 1);
	dev_dbg(clk->dev, "rate=%lld\n", rate);
	return rate;
}

const struct clk_ops clk_ti_am3_dpll_ops = {
	.round_rate = clk_ti_am3_dpll_round_rate,
	.get_rate = clk_ti_am3_dpll_get_rate,
	.set_rate = clk_ti_am3_dpll_set_rate,
};

static int clk_ti_am3_dpll_remove(struct udevice *dev)
{
	struct clk_ti_am3_dpll_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_release_all(&priv->clk_bypass, 1);
	if (err) {
		dev_err(dev, "failed to release bypass clock\n");
		return err;
	}

	err = clk_release_all(&priv->clk_ref, 1);
	if (err) {
		dev_err(dev, "failed to release reference clock\n");
		return err;
	}

	return 0;
}

static int clk_ti_am3_dpll_probe(struct udevice *dev)
{
	struct clk_ti_am3_dpll_priv *priv = dev_get_priv(dev);
	int err;

	err = clk_get_by_index(dev, 0, &priv->clk_ref);
	if (err) {
		dev_err(dev, "failed to get reference clock\n");
		return err;
	}

	err = clk_get_by_index(dev, 1, &priv->clk_bypass);
	if (err) {
		dev_err(dev, "failed to get bypass clock\n");
		return err;
	}

	return 0;
}

static int clk_ti_am3_dpll_of_to_plat(struct udevice *dev)
{
	struct clk_ti_am3_dpll_priv *priv = dev_get_priv(dev);
	struct clk_ti_am3_dpll_drv_data *data =
		(struct clk_ti_am3_dpll_drv_data *)dev_get_driver_data(dev);
	int err;

	priv->max_rate = data->max_rate;

	err = clk_ti_get_reg_addr(dev, 0, &priv->clkmode_reg);
	if (err) {
		dev_err(dev, "failed to get clkmode register address\n");
		return err;
	}

	err = clk_ti_get_reg_addr(dev, 1, &priv->idlest_reg);
	if (err) {
		dev_err(dev, "failed to get idlest register\n");
		return -EINVAL;
	}

	err = clk_ti_get_reg_addr(dev, 2, &priv->clksel_reg);
	if (err) {
		dev_err(dev, "failed to get clksel register\n");
		return err;
	}

	return 0;
}

static const struct clk_ti_am3_dpll_drv_data dpll_no_gate_data = {
	.max_rate = 1000000000
};

static const struct clk_ti_am3_dpll_drv_data dpll_no_gate_j_type_data = {
	.max_rate = 2000000000
};

static const struct clk_ti_am3_dpll_drv_data dpll_core_data = {
	.max_rate = 1000000000
};

static const struct udevice_id clk_ti_am3_dpll_of_match[] = {
	{.compatible = "ti,am3-dpll-core-clock",
	 .data = (ulong)&dpll_core_data},
	{.compatible = "ti,am3-dpll-no-gate-clock",
	 .data = (ulong)&dpll_no_gate_data},
	{.compatible = "ti,am3-dpll-no-gate-j-type-clock",
	 .data = (ulong)&dpll_no_gate_j_type_data},
	{}
};

U_BOOT_DRIVER(clk_ti_am3_dpll) = {
	.name = "ti_am3_dpll_clock",
	.id = UCLASS_CLK,
	.of_match = clk_ti_am3_dpll_of_match,
	.of_to_plat = clk_ti_am3_dpll_of_to_plat,
	.probe = clk_ti_am3_dpll_probe,
	.remove = clk_ti_am3_dpll_remove,
	.priv_auto = sizeof(struct clk_ti_am3_dpll_priv),
	.ops = &clk_ti_am3_dpll_ops,
};

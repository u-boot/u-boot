// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2026 NXP
 *
 */

#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/devres.h>
#include <linux/bitops.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <clk.h>
#include <div64.h>
#include <linux/printk.h>
#include <linux/bitfield.h>

#include "clk.h"

#define PLL_CFG0		0x0
#define PLL_CFG1		0x4

#define PLL_LOCK_STATUS		BIT(31)
#define PLL_PD_MASK		BIT(19)
#define PLL_BYPASS_MASK		BIT(14)
#define PLL_NEWDIV_VAL		BIT(12)
#define PLL_NEWDIV_ACK		BIT(11)
#define PLL_FRAC_DIV_MASK	GENMASK(30, 7)
#define PLL_INT_DIV_MASK	GENMASK(6, 0)
#define PLL_OUTPUT_DIV_MASK	GENMASK(4, 0)
#define PLL_FRAC_DENOM		0x1000000

#define PLL_FRAC_LOCK_TIMEOUT	10000
#define PLL_FRAC_ACK_TIMEOUT	500000

struct clk_frac_pll {
	struct clk			clk;
	void __iomem			*base;
};

#define to_clk_frac_pll(_clk) container_of(_clk, struct clk_frac_pll, clk)

static int clk_wait_lock(struct clk_frac_pll *pll)
{
	u32 val;

	return readl_poll_timeout(pll->base, val, val & PLL_LOCK_STATUS,
					PLL_FRAC_LOCK_TIMEOUT);
}

static int clk_wait_ack(struct clk_frac_pll *pll)
{
	u32 val;

	/* return directly if the pll is in powerdown or in bypass */
	if (readl_relaxed(pll->base) & (PLL_PD_MASK | PLL_BYPASS_MASK))
		return 0;

	/* Wait for the pll's divfi and divff to be reloaded */
	return readl_poll_timeout(pll->base, val, val & PLL_NEWDIV_ACK,
					PLL_FRAC_ACK_TIMEOUT);
}

static int clk_pll_prepare(struct clk *clk)
{
	struct clk_frac_pll *pll = to_clk_frac_pll(clk);
	u32 val;

	val = readl_relaxed(pll->base + PLL_CFG0);
	val &= ~PLL_PD_MASK;
	writel_relaxed(val, pll->base + PLL_CFG0);

	return clk_wait_lock(pll);
}

static int clk_pll_unprepare(struct clk *clk)
{
	struct clk_frac_pll *pll = to_clk_frac_pll(clk);
	u32 val;

	val = readl_relaxed(pll->base + PLL_CFG0);
	val |= PLL_PD_MASK;
	writel_relaxed(val, pll->base + PLL_CFG0);

	return 0;
}

static unsigned long clk_pll_recalc_rate(struct clk *clk)
{
	struct clk_frac_pll *pll = to_clk_frac_pll(clk);
	u32 val, divff, divfi, divq;
	ulong parent_rate = clk_get_parent_rate(clk);
	u64 temp64 = parent_rate;
	u64 rate;

	val = readl_relaxed(pll->base + PLL_CFG0);
	divq = (FIELD_GET(PLL_OUTPUT_DIV_MASK, val) + 1) * 2;
	val = readl_relaxed(pll->base + PLL_CFG1);
	divff = FIELD_GET(PLL_FRAC_DIV_MASK, val);
	divfi = FIELD_GET(PLL_INT_DIV_MASK, val);

	temp64 *= 8;
	temp64 *= divff;
	do_div(temp64, PLL_FRAC_DENOM);
	do_div(temp64, divq);

	rate = parent_rate * 8 * (divfi + 1);
	do_div(rate, divq);
	rate += temp64;

	return rate;
}

static ulong clk_pll_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk_frac_pll *pll = to_clk_frac_pll(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 val, divfi, divff;
	u64 temp64;
	int ret;

	parent_rate *= 8;
	rate *= 2;
	divfi = rate / parent_rate;
	temp64 = parent_rate * divfi;
	temp64 = rate - temp64;
	temp64 *= PLL_FRAC_DENOM;
	do_div(temp64, parent_rate);
	divff = temp64;

	val = readl_relaxed(pll->base + PLL_CFG1);
	val &= ~(PLL_FRAC_DIV_MASK | PLL_INT_DIV_MASK);
	val |= (divff << 7) | (divfi - 1);
	writel_relaxed(val, pll->base + PLL_CFG1);

	val = readl_relaxed(pll->base + PLL_CFG0);
	val &= ~0x1f;
	writel_relaxed(val, pll->base + PLL_CFG0);

	/* Set the NEV_DIV_VAL to reload the DIVFI and DIVFF */
	val = readl_relaxed(pll->base + PLL_CFG0);
	val |= PLL_NEWDIV_VAL;
	writel_relaxed(val, pll->base + PLL_CFG0);

	ret = clk_wait_ack(pll);

	/* clear the NEV_DIV_VAL */
	val = readl_relaxed(pll->base + PLL_CFG0);
	val &= ~PLL_NEWDIV_VAL;
	writel_relaxed(val, pll->base + PLL_CFG0);

	if (ret)
		return ret;

	return clk_pll_recalc_rate(clk);
}

static const struct clk_ops clk_frac_pll_ops = {
	.enable		= clk_pll_prepare,
	.disable	= clk_pll_unprepare,
	.set_rate	= clk_pll_set_rate,
	.get_rate	= clk_pll_recalc_rate,
};

struct clk *imx_clk_frac_pll(const char *name, const char *parent_name,
			     void __iomem *base)
{
	struct clk_frac_pll *pll;
	struct clk *clk;
	int ret;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	pll->base = base;
	clk = &pll->clk;

	ret = clk_register(clk, "imx_clk_frac_pll", name, parent_name);
	if (ret) {
		pr_err("%s: failed to register pll %s %d\n",
		       __func__, name, ret);
		kfree(pll);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(clk_frac_pll) = {
	.name	= "imx_clk_frac_pll",
	.id	= UCLASS_CLK,
	.ops	= &clk_frac_pll_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

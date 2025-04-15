// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <div64.h>
#include <linux/bitfield.h>
#include <linux/clk-provider.h>
#include <linux/kernel.h>

#include "clk-common.h"
#include "clk-pll.h"

#define PLL_PRE_DIV_MIN      1
#define PLL_PRE_DIV_MAX      127
#define PLL_POST_DIV_MIN     1
#define PLL_POST_DIV_MAX     127
#define PLL_DIV_MIN          6
#define PLL_DIV_MAX          127
#define PLL_ICTRL_MIN        0
#define PLL_ICTRL_MAX        7
#define PLL_MODE_MIN         0
#define PLL_MODE_MAX         3
#define FOR_RANGE(x, RANGE) for (x = RANGE##_MIN; x <= RANGE##_MAX; x++)

#define PLL_ICTRL GENMASK(26, 24)
#define PLL_DIV_SEL GENMASK(23, 17)
#define PLL_SEL_MODE GENMASK(16, 15)
#define PLL_POST_DIV_SEL GENMASK(14, 8)
#define PLL_PRE_DIV_SEL GENMASK(6, 0)
#define PLL_MASK_ALL (PLL_ICTRL | PLL_DIV_SEL | PLL_SEL_MODE | PLL_POST_DIV_SEL | PLL_PRE_DIV_SEL)

/* IPLL */
#define to_clk_ipll(dev) container_of(dev, struct cv1800b_clk_ipll, clk)

static int cv1800b_ipll_enable(struct clk *clk)
{
	struct cv1800b_clk_ipll *pll = to_clk_ipll(clk);

	cv1800b_clk_clrbit(pll->base, &pll->pll_pwd);
	return 0;
}

static int cv1800b_ipll_disable(struct clk *clk)
{
	struct cv1800b_clk_ipll *pll = to_clk_ipll(clk);

	cv1800b_clk_setbit(pll->base, &pll->pll_pwd);
	return 0;
}

static ulong cv1800b_ipll_get_rate(struct clk *clk)
{
	struct cv1800b_clk_ipll *pll = to_clk_ipll(clk);

	ulong parent_rate = clk_get_parent_rate(clk);
	u32 reg = readl(pll->base + pll->pll_reg);
	u32 pre_div = FIELD_GET(PLL_PRE_DIV_SEL, reg);
	u32 post_div = FIELD_GET(PLL_POST_DIV_SEL, reg);
	u32 div = FIELD_GET(PLL_DIV_SEL, reg);

	return DIV_ROUND_DOWN_ULL(parent_rate * div, pre_div * post_div);
}

static ulong cv1800b_ipll_set_rate(struct clk *clk, ulong rate)
{
	struct cv1800b_clk_ipll *pll = to_clk_ipll(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 pre_div, post_div, div;
	u32 pre_div_sel, post_div_sel, div_sel;
	ulong new_rate, best_rate = 0;
	u32 mode, ictrl;
	u32 test, val;

	FOR_RANGE(pre_div, PLL_PRE_DIV)
	{
		FOR_RANGE(post_div, PLL_POST_DIV)
		{
			FOR_RANGE(div, PLL_DIV)
			{
				new_rate =
					DIV_ROUND_DOWN_ULL(parent_rate * div, pre_div * post_div);
				if (rate - new_rate < rate - best_rate) {
					best_rate = new_rate;
					pre_div_sel = pre_div;
					post_div_sel = post_div;
					div_sel = div;
				}
			}
		}
	}

	FOR_RANGE(mode, PLL_MODE)
	{
		FOR_RANGE(ictrl, PLL_ICTRL)
		{
			test = 184 * (1 + mode) * (1 + ictrl) / 2;
			if (test > 20 * div_sel && test < 35 * div_sel) {
				val = FIELD_PREP(PLL_PRE_DIV_SEL, pre_div_sel) |
				      FIELD_PREP(PLL_POST_DIV_SEL, post_div_sel) |
				      FIELD_PREP(PLL_DIV_SEL, div_sel) |
				      FIELD_PREP(PLL_ICTRL, ictrl) |
				      FIELD_PREP(PLL_SEL_MODE, mode);
				clrsetbits_le32(pll->base + pll->pll_reg, PLL_MASK_ALL, val);
				return best_rate;
			}
		}
	}

	return -EINVAL;
}

const struct clk_ops cv1800b_ipll_ops = {
	.enable = cv1800b_ipll_enable,
	.disable = cv1800b_ipll_disable,
	.get_rate = cv1800b_ipll_get_rate,
	.set_rate = cv1800b_ipll_set_rate,
};

U_BOOT_DRIVER(cv1800b_clk_ipll) = {
	.name = "cv1800b_clk_ipll",
	.id = UCLASS_CLK,
	.ops = &cv1800b_ipll_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* FPLL */
#define to_clk_fpll(dev) container_of(dev, struct cv1800b_clk_fpll, ipll.clk)

static ulong cv1800b_fpll_get_rate(struct clk *clk)
{
	struct cv1800b_clk_fpll *pll = to_clk_fpll(clk);
	u32 val, syn_set;
	u32 pre_div, post_div, div;
	u8 mult = 1;
	ulong divisor, remainder, rate;

	if (!cv1800b_clk_getbit(pll->ipll.base, &pll->syn.en))
		return cv1800b_ipll_get_rate(clk);

	syn_set = readl(pll->ipll.base + pll->syn.set);
	if (syn_set == 0)
		return 0;

	val = readl(pll->ipll.base + pll->ipll.pll_reg);
	pre_div = FIELD_GET(PLL_PRE_DIV_SEL, val);
	post_div = FIELD_GET(PLL_POST_DIV_SEL, val);
	div = FIELD_GET(PLL_DIV_SEL, val);

	if (cv1800b_clk_getbit(pll->ipll.base, &pll->syn.clk_half))
		mult = 2;

	divisor = (ulong)pre_div * post_div * syn_set;
	rate = (clk_get_parent_rate(clk) * div) << 25;
	remainder = rate % divisor;
	rate /= divisor;
	return rate * mult + DIV_ROUND_CLOSEST_ULL(remainder * mult, divisor);
}

static ulong cv1800b_find_syn(ulong rate, ulong parent_rate, ulong pre_div, ulong post_div,
			      ulong div, u32 *syn)
{
	u32 syn_min = (4 << 26) + 1;
	u32 syn_max = U32_MAX;
	u32 mid;
	ulong new_rate;
	u32 mult = 1;
	ulong divisor, remainder;

	while (syn_min < syn_max) {
		mid = ((ulong)syn_min + syn_max) / 2;
		divisor = pre_div * post_div * mid;
		new_rate = (parent_rate * div) << 25;
		remainder = do_div(new_rate, divisor);
		new_rate = new_rate * mult + DIV_ROUND_CLOSEST_ULL(remainder * mult, divisor);
		if (new_rate > rate) {
			syn_max = mid + 1;
		} else if (new_rate < rate) {
			syn_min = mid - 1;
		} else {
			syn_min = mid;
			break;
		}
	}
	*syn = syn_min;
	return new_rate;
}

static ulong cv1800b_fpll_set_rate(struct clk *clk, ulong rate)
{
	struct cv1800b_clk_fpll *pll = to_clk_fpll(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 pre_div, post_div, div;
	u32 pre_div_sel, post_div_sel, div_sel;
	u32 syn, syn_sel;
	ulong new_rate, best_rate = 0;
	u32 mult = 1;
	u32 mode, ictrl;

	if (!cv1800b_clk_getbit(pll->ipll.base, &pll->syn.en))
		return cv1800b_ipll_set_rate(clk, rate);

	if (cv1800b_clk_getbit(pll->ipll.base, &pll->syn.clk_half))
		mult = 2;

	FOR_RANGE(pre_div, PLL_PRE_DIV)
	{
		FOR_RANGE(post_div, PLL_POST_DIV)
		{
			FOR_RANGE(div, PLL_DIV)
			{
				new_rate = cv1800b_find_syn(rate, parent_rate, pre_div, post_div,
							    div, &syn);
				if (rate - new_rate < rate - best_rate) {
					best_rate = new_rate;
					pre_div_sel = pre_div;
					post_div_sel = post_div;
					div_sel = div;
					syn_sel = syn;
				}
			}
		}
	}

	FOR_RANGE(mode, PLL_MODE)
	{
		FOR_RANGE(ictrl, PLL_ICTRL)
		{
			u32 test = 184 * (1 + mode) * (1 + ictrl) / 2;

			if (test > 10 * div_sel && test <= 24 * div_sel) {
				u32 val = FIELD_PREP(PLL_PRE_DIV_SEL, pre_div_sel) |
					  FIELD_PREP(PLL_POST_DIV_SEL, post_div_sel) |
					  FIELD_PREP(PLL_DIV_SEL, div_sel) |
					  FIELD_PREP(PLL_ICTRL, ictrl) |
					  FIELD_PREP(PLL_SEL_MODE, mode);
				clrsetbits_le32(pll->ipll.base + pll->ipll.pll_reg, PLL_MASK_ALL,
						val);
				writel(syn_sel, pll->ipll.base + pll->syn.set);
				return best_rate;
			}
		}
	}

	return -EINVAL;
}

static int cv1800b_fpll_set_parent(struct clk *clk, struct clk *parent)
{
	struct cv1800b_clk_fpll *pll = to_clk_fpll(clk);

	if (parent->id == CV1800B_CLK_BYPASS)
		cv1800b_clk_setbit(pll->ipll.base, &pll->syn.en);
	else
		cv1800b_clk_clrbit(pll->ipll.base, &pll->syn.en);

	return 0;
}

const struct clk_ops cv1800b_fpll_ops = {
	.enable = cv1800b_ipll_enable,
	.disable = cv1800b_ipll_disable,
	.get_rate = cv1800b_fpll_get_rate,
	.set_rate = cv1800b_fpll_set_rate,
	.set_parent = cv1800b_fpll_set_parent,
};

U_BOOT_DRIVER(cv1800b_clk_fpll) = {
	.name = "cv1800b_clk_fpll",
	.id = UCLASS_CLK,
	.ops = &cv1800b_fpll_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

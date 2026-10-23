// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek common clock driver
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/string.h>

#include "clk-mtk.h"

#define REG_CON0			0
#define REG_CON1			4

#define CON0_BASE_EN			BIT(0)
#define CON0_PWR_ON			BIT(0)
#define CON0_ISO_EN			BIT(1)
#define CON1_PCW_CHG			BIT(31)

#define POSTDIV_MASK			0x7
#define INTEGER_BITS			7

/* scpsys clock off control */
#define CLK_SCP_CFG0			0x200
#define CLK_SCP_CFG1			0x204
#define SCP_ARMCK_OFF_EN		GENMASK(9, 0)
#define SCP_AXICK_DCM_DIS_EN		BIT(0)
#define SCP_AXICK_26M_SEL_EN		BIT(4)

static bool mtk_clk_tree_type_is_provider(enum mtk_clk_tree_type type)
{
	return type != MTK_CLK_TREE_NONE && type < MTK_CLK_TREE_NUM_TYPES;
}

static enum mtk_clk_tree_type mtk_clk_tree_type_from_parent_flags(u16 flags)
{
	switch (flags & CLK_PARENT_MASK) {
	case CLK_PARENT_APMIXED:
		return MTK_CLK_TREE_APMIXED;
	case CLK_PARENT_TOPCKGEN:
		return MTK_CLK_TREE_TOPCKGEN;
	case CLK_PARENT_INFRASYS:
		return MTK_CLK_TREE_INFRASYS;
	default:
		return MTK_CLK_TREE_NONE;
	}
}

static struct udevice *mtk_clk_tree_get_provider(enum mtk_clk_tree_type type)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	if (!mtk_clk_tree_type_is_provider(type))
		return NULL;

	ret = uclass_get(UCLASS_CLK, &uc);
	if (ret)
		return ERR_PTR(ret);

	uclass_foreach_dev(dev, uc) {
		const struct mtk_clk_tree *tree;

		if (dev_get_driver_ops(dev) != &mtk_clk_ops)
			continue;

		tree = (const void *)dev_get_driver_data(dev);
		if (tree->type != type)
			continue;

		ret = device_probe(dev);
		if (ret)
			return ERR_PTR(ret);

		return dev;
	}

	return ERR_PTR(-ENOENT);
}

static struct udevice *mtk_clk_parent_get_provider(u16 flags)
{
	return mtk_clk_tree_get_provider(mtk_clk_tree_type_from_parent_flags(flags));
}

/* shared functions */

static const int mtk_common_clk_of_xlate(struct clk *clk,
					 struct ofnode_phandle_args *args,
					 const struct mtk_clk_tree *tree)
{
	int id;

	if (args->args_count != 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	id = args->args[0];

	/* Remap the clk ID to the one expected by driver */
	if (tree->id_offs_map) {
		if (id >= tree->id_offs_map_size)
			return -ENOENT;

		id = tree->id_offs_map[id];
	}

	/* Some IDs in the map may not be valid. */
	if (id < 0)
		return -ENOENT;

	clk->id = id;
	clk->data = 0;

	return 0;
}

static int mtk_common_clk_get_unmapped_id(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_clk_tree *tree = priv->tree;
	int i;

	if (!tree->id_offs_map)
		return clk->id;

	/* Perform reverse lookup of unmapped ID. */
	for (i = 0; i < tree->id_offs_map_size; i++) {
		if (tree->id_offs_map[i] == clk->id)
			return i;
	}

	return -ENOENT;
}

static bool mtk_clk_id_is_pll(const struct mtk_clk_tree *tree, int mapped_id)
{
	return tree->plls && mapped_id < tree->num_plls;
}

static bool mtk_clk_id_is_fclk(const struct mtk_clk_tree *tree, int mapped_id)
{
	return tree->fclks && mapped_id >= tree->fclks_offs &&
	       mapped_id < tree->fclks_offs + tree->num_fclks;
}

static bool mtk_clk_id_is_fdiv(const struct mtk_clk_tree *tree, int mapped_id)
{
	return tree->fdivs && mapped_id >= tree->fdivs_offs &&
	       mapped_id < tree->fdivs_offs + tree->num_fdivs;
}

static bool mtk_clk_id_is_mux(const struct mtk_clk_tree *tree, int mapped_id)
{
	return tree->muxes && mapped_id >= tree->muxes_offs &&
	       mapped_id < tree->muxes_offs + tree->num_muxes;
}

static bool mtk_clk_id_is_gate(const struct mtk_clk_tree *tree, int mapped_id)
{
	return tree->gates && mapped_id >= tree->gates_offs &&
	       mapped_id < tree->gates_offs + tree->num_gates;
}

static int mtk_clk_of_xlate(struct clk *clk, struct ofnode_phandle_args *args)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_clk_tree *tree = priv->tree;
	int ret;

	ret = mtk_common_clk_of_xlate(clk, args, tree);
	if (ret)
		return ret;

	if (!mtk_clk_id_is_pll(tree, clk->id) && !mtk_clk_id_is_fclk(tree, clk->id) &&
	    !mtk_clk_id_is_fdiv(tree, clk->id) && !mtk_clk_id_is_mux(tree, clk->id) &&
	    !mtk_clk_id_is_gate(tree, clk->id))
		return -ENOENT;

	return 0;
}

static int mtk_gate_enable(void __iomem *base, const struct mtk_gate *gate)
{
	u32 bit = BIT(gate->shift);

	switch (gate->flags & CLK_GATE_MASK) {
	case CLK_GATE_SETCLR:
		writel(bit, base + gate->regs->clr_ofs);
		break;
	case CLK_GATE_SETCLR_INV:
		writel(bit, base + gate->regs->set_ofs);
		break;
	case CLK_GATE_NO_SETCLR:
		clrsetbits_le32(base + gate->regs->sta_ofs, bit, 0);
		break;
	case CLK_GATE_NO_SETCLR_INV:
		clrsetbits_le32(base + gate->regs->sta_ofs, bit, bit);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static int mtk_gate_disable(void __iomem *base, const struct mtk_gate *gate)
{
	u32 bit = BIT(gate->shift);

	switch (gate->flags & CLK_GATE_MASK) {
	case CLK_GATE_SETCLR:
		writel(bit, base + gate->regs->set_ofs);
		break;
	case CLK_GATE_SETCLR_INV:
		writel(bit, base + gate->regs->clr_ofs);
		break;
	case CLK_GATE_NO_SETCLR:
		clrsetbits_le32(base + gate->regs->sta_ofs, bit, bit);
		break;
	case CLK_GATE_NO_SETCLR_INV:
		clrsetbits_le32(base + gate->regs->sta_ofs, bit, 0);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static ulong mtk_ext_clock_get_rate(const struct mtk_clk_tree *tree, int id)
{
	if (!tree->ext_clk_rates || id >= tree->num_ext_clks)
		return -ENOENT;

	return tree->ext_clk_rates[id];
}

/*
 * Build @out so that it refers to clock @parent on the provider selected by
 * @flags, which lets the generic clk API be used to walk up the tree even
 * when the parent lives on a different provider device (a topckgen divider
 * feeding off an apmixedsys PLL, say).
 *
 * CLK_PARENT_EXT parents are not backed by a provider at all; callers have
 * to handle them with mtk_ext_clock_get_rate() instead.
 */
static int mtk_clk_lookup_parent(struct clk *clk, int parent, u16 flags,
				 struct clk *out)
{
	struct ofnode_phandle_args args = {
		.args_count = 1,
		.args = { parent },
	};
	struct udevice *pdev;

	if ((flags & CLK_PARENT_MASK) == CLK_PARENT_EXT)
		return -EINVAL;

	pdev = mtk_clk_parent_get_provider(flags);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	memset(out, 0, sizeof(*out));
	out->dev = pdev ?: clk->dev;
	args.node = dev_ofnode(out->dev);

	return ((struct clk_ops *)out->dev->driver->ops)->of_xlate(out, &args);
}

static ulong mtk_find_parent_rate(struct mtk_clk_priv *priv, struct clk *clk,
				  const int parent, u16 flags)
{
	struct clk pclk;
	int ret;

	if ((flags & CLK_PARENT_MASK) == CLK_PARENT_EXT)
		return mtk_ext_clock_get_rate(priv->tree, parent);

	ret = mtk_clk_lookup_parent(clk, parent, flags, &pclk);
	if (ret)
		return ret;

	return clk_get_rate(&pclk);
}

static ulong mtk_clk_mux_get_rate(struct clk *clk, u32 off)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_composite *mux = &priv->tree->muxes[off];
	const struct mtk_parent *parent;
	u32 index;

	index = readl(priv->base + mux->mux_reg);
	index &= mux->mux_mask << mux->mux_shift;
	index = index >> mux->mux_shift;
	parent = &mux->parent[index];

	return mtk_find_parent_rate(priv, clk, parent->id, parent->flags);
}

static void mtk_clk_mux_write_index(void __iomem *base, u32 index,
				    const struct mtk_composite *mux)
{
	u32 val;

	if (mux->flags & CLK_MUX_SETCLR_UPD) {
		val = (mux->mux_mask << mux->mux_shift);
		writel(val, base + mux->mux_clr_reg);

		val = (index << mux->mux_shift);
		writel(val, base + mux->mux_set_reg);

		if (mux->upd_shift >= 0)
			writel(BIT(mux->upd_shift), base + mux->upd_reg);
	} else {
		/* switch mux to a select parent */
		val = readl(base + mux->mux_reg);
		val &= ~(mux->mux_mask << mux->mux_shift);

		val |= index << mux->mux_shift;
		writel(val, base + mux->mux_reg);
	}
}

static int mtk_clk_mux_set_parent(void __iomem *base, u32 parent,
				  u32 parent_type,
				  const struct mtk_composite *mux)
{
	u32 index = 0;

	/*
	 * Assume parent_type in clk_tree to be always set. If it's not, assume
	 * parent clk ID clash is not possible.
	 */
	while (mux->parent[index].id != parent ||
	       (parent_type && (mux->parent[index].flags & CLK_PARENT_MASK) !=
		parent_type))
		if (++index == mux->num_parents)
			return -EINVAL;

	mtk_clk_mux_write_index(base, index, mux);

	return 0;
}

#if CONFIG_IS_ENABLED(CMD_CLK)
static void mtk_clk_print_mapped_id(int unmapped_id, int mapped_id, bool has_map)
{
	/*
	 * If there is a ID map, then having unmapped and mapped IDs differ is
	 * expected. On the other hand, if there is no map, then they should be
	 * the same, and it is a programming error if they differ.
	 */
	if (has_map)
		printf(", (mapped ID: %u)", mapped_id);
	else if (unmapped_id != mapped_id)
		printf(", (error! should be %u)", mapped_id);
}

static void mtk_clk_print_rate(struct udevice *dev, int mapped_id)
{
	struct clk clk = {
		.dev = dev,
		.id = mapped_id,
	};
	ulong rate = clk_get_rate(&clk);

	if (IS_ERR_VALUE(rate))
		printf(", error! clk_get_rate() failed: %d", (int)rate);
	else
		printf(", Rate: %lu Hz", rate);
}

static void mtk_clk_print_parent(const char *prefix, int parent, u32 flags)
{
	const char *parent_type_str;

	switch (flags & CLK_PARENT_MASK) {
	case CLK_PARENT_APMIXED:
		parent_type_str = "apmixedsys";
		break;
	case CLK_PARENT_TOPCKGEN:
		parent_type_str = "topckgen";
		break;
	case CLK_PARENT_INFRASYS:
		parent_type_str = "infrasys";
		break;
	case CLK_PARENT_EXT:
		parent_type_str = "ext";
		break;
	default:
		parent_type_str = "default";
		break;
	}

	printf("%s%s-%u", prefix, parent_type_str, parent);
}

static void mtk_clk_print_single_parent(int parent, u32 flags)
{
	mtk_clk_print_parent(", Parent: ", parent, flags);
}

static void mtk_clk_print_mux_parents(struct mtk_clk_priv *priv,
				      const struct mtk_composite *mux)
{
	const char *prefix = "";
	u32 selected;
	int i;

	printf(", Parents: ");

	selected = readl(priv->base + mux->mux_reg);
	selected &= mux->mux_mask << mux->mux_shift;
	selected >>= mux->mux_shift;

	/* Print parents separated by "/" and selected parent enclosed in "*"s */
	for (i = 0; i < mux->num_parents; i++) {
		const struct mtk_parent *parent = &mux->parent[i];

		if (i == selected) {
			printf("%s", prefix);
			prefix = "*";
		}

		mtk_clk_print_parent(prefix, parent->id, parent->flags);

		prefix = "/";

		if (i == selected)
			printf("*");
	}
}
#endif

/* PLL functions */

static unsigned long __mtk_pll_recalc_rate(const struct mtk_pll_data *pll,
					   u32 fin, u32 pcw, int postdiv)
{
	int pcwbits = pll->pcwbits;
	int pcwfbits;
	int ibits;
	u64 vco;
	u8 c = 0;

	/* The fractional part of the PLL divider. */
	ibits = pll->pcwibits ? pll->pcwibits : INTEGER_BITS;
	pcwfbits = pcwbits > ibits ? pcwbits - ibits : 0;

	vco = (u64)fin * pcw;

	if (pcwfbits && (vco & GENMASK(pcwfbits - 1, 0)))
		c = 1;

	vco >>= pcwfbits;

	if (c)
		vco++;

	return ((unsigned long)vco + postdiv - 1) / postdiv;
}

/**
 * MediaTek PLLs are configured through their pcw value. The pcw value
 * describes a divider in the PLL feedback loop which consists of 7 bits
 * for the integer part and the remaining bits (if present) for the
 * fractional part. Also they have a 3 bit power-of-two post divider.
 */
static void mtk_pll_set_rate_regs(struct mtk_clk_priv *priv, u32 id,
				  u32 pcw, int postdiv)
{
	const struct mtk_pll_data *pll;
	u32 val, chg;

	pll = &priv->tree->plls[id];

	/* set postdiv */
	val = readl(priv->base + pll->pd_reg);
	val &= ~(POSTDIV_MASK << pll->pd_shift);
	val |= (ffs(postdiv) - 1) << pll->pd_shift;

	/* postdiv and pcw need to set at the same time if on same register */
	if (pll->pd_reg != pll->pcw_reg) {
		writel(val, priv->base + pll->pd_reg);
		val = readl(priv->base + pll->pcw_reg);
	}

	/* set pcw */
	val &= ~GENMASK(pll->pcw_shift + pll->pcwbits - 1, pll->pcw_shift);
	val |= pcw << pll->pcw_shift;

	if (pll->pcw_chg_reg) {
		chg = readl(priv->base + pll->pcw_chg_reg);
		chg |= CON1_PCW_CHG;
		writel(val, priv->base + pll->pcw_reg);
		writel(chg, priv->base + pll->pcw_chg_reg);
	} else {
		val |= CON1_PCW_CHG;
		writel(val, priv->base + pll->pcw_reg);
	}

	udelay(20);
}

/**
 * mtk_pll_calc_values - calculate good values for a given input frequency.
 * @pll:	The PLL data
 * @xtal_rate:	The PLL input (reference) frequency
 * @pcw:	The pcw value (output)
 * @postdiv:	The post divider (output)
 * @freq:	The desired target frequency
 */
static void mtk_pll_calc_values(const struct mtk_pll_data *pll,
				ulong xtal_rate, u32 *pcw, u32 *postdiv,
				ulong freq)
{
	unsigned long fmin;
	u64 _pcw;
	int ibits;
	u32 val;

	fmin = pll->fmin ? pll->fmin : 1000 * MHZ;

	if (freq > pll->fmax)
		freq = pll->fmax;

	for (val = 0; val < 5; val++) {
		*postdiv = 1 << val;
		if ((u64)freq * *postdiv >= fmin)
			break;
	}

	/* _pcw = freq * postdiv / xtal_rate * 2^pcwfbits */
	ibits = pll->pcwibits ? pll->pcwibits : INTEGER_BITS;
	_pcw = ((u64)freq << val) << (pll->pcwbits - ibits);
	do_div(_pcw, xtal_rate);

	*pcw = (u32)_pcw;
}

static ulong mtk_pll_get_rate(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_parent *parent;
	const struct mtk_pll_data *pll;
	unsigned long xtal_rate;
	u32 postdiv;
	u32 pcw;

	parent = &priv->tree->pll_parent;
	xtal_rate = mtk_find_parent_rate(priv, clk, parent->id, parent->flags);
	if (IS_ERR_VALUE(xtal_rate))
		return xtal_rate;

	pll = &priv->tree->plls[clk->id];

	postdiv = (readl(priv->base + pll->pd_reg) >> pll->pd_shift) &
		   POSTDIV_MASK;
	postdiv = 1 << postdiv;

	pcw = readl(priv->base + pll->pcw_reg) >> pll->pcw_shift;
	pcw &= GENMASK(pll->pcwbits - 1, 0);

	return __mtk_pll_recalc_rate(pll, xtal_rate, pcw, postdiv);
}

/*
 * Report the rate the PLL would actually end up at if asked for @rate,
 * without touching the hardware. The requested rate is rarely hit exactly
 * because it has to be expressed as a pcw/post-divider pair, and it is
 * clamped to the PLL's fmax.
 */
static ulong mtk_pll_round_rate(struct clk *clk, ulong rate)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_parent *parent = &priv->tree->pll_parent;
	const struct mtk_pll_data *pll = &priv->tree->plls[clk->id];
	ulong xtal_rate;
	u32 pcw = 0;
	u32 postdiv;

	xtal_rate = mtk_find_parent_rate(priv, clk, parent->id, parent->flags);
	if (IS_ERR_VALUE(xtal_rate))
		return xtal_rate;

	mtk_pll_calc_values(pll, xtal_rate, &pcw, &postdiv, rate);

	return __mtk_pll_recalc_rate(pll, xtal_rate, pcw, postdiv);
}

static ulong mtk_pll_set_rate(struct clk *clk, ulong rate)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_parent *parent = &priv->tree->pll_parent;
	ulong xtal_rate;
	u32 pcw = 0;
	u32 postdiv;

	xtal_rate = mtk_find_parent_rate(priv, clk, parent->id, parent->flags);
	if (IS_ERR_VALUE(xtal_rate))
		return xtal_rate;

	mtk_pll_calc_values(&priv->tree->plls[clk->id], xtal_rate, &pcw,
			    &postdiv, rate);
	mtk_pll_set_rate_regs(priv, clk->id, pcw, postdiv);

	return mtk_pll_get_rate(clk);
}

static int mtk_pll_enable(void __iomem *base, const struct mtk_pll_data *pll)
{
	u32 r;

	r = readl(base + pll->pwr_reg) | CON0_PWR_ON;
	writel(r, base + pll->pwr_reg);
	udelay(1);

	r = readl(base + pll->pwr_reg) & ~CON0_ISO_EN;
	writel(r, base + pll->pwr_reg);
	udelay(1);

	r = readl(base + pll->reg + REG_CON0);
	r |= pll->en_mask;
	writel(r, base + pll->reg + REG_CON0);

	udelay(20);

	if (pll->flags & CLK_PLL_HAVE_RST_BAR) {
		r = readl(base + pll->reg + REG_CON0);
		r |= pll->rst_bar_mask;
		writel(r, base + pll->reg + REG_CON0);
	}

	return 0;
}

static int mtk_pll_disable(void __iomem *base, const struct mtk_pll_data *pll)
{
	u32 r;

	if (pll->flags & CLK_PLL_HAVE_RST_BAR) {
		r = readl(base + pll->reg + REG_CON0);
		r &= ~pll->rst_bar_mask;
		writel(r, base + pll->reg + REG_CON0);
	}

	r = readl(base + pll->reg + REG_CON0);
	r &= ~CON0_BASE_EN;
	writel(r, base + pll->reg + REG_CON0);

	r = readl(base + pll->pwr_reg) | CON0_ISO_EN;
	writel(r, base + pll->pwr_reg);

	r = readl(base + pll->pwr_reg) & ~CON0_PWR_ON;
	writel(r, base + pll->pwr_reg);

	return 0;
}

static ulong mtk_factor_recalc_rate(const struct mtk_fixed_factor *fdiv,
				    ulong parent_rate)
{
	u64 rate = parent_rate * fdiv->mult;

	do_div(rate, fdiv->div);

	return rate;
}

static ulong mtk_factor_get_rate(struct clk *clk, u32 off)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_fixed_factor *fdiv = &priv->tree->fdivs[off];
	ulong rate;

	rate = mtk_find_parent_rate(priv, clk, fdiv->parent, fdiv->flags);
	if (IS_ERR_VALUE(rate))
		return rate;

	return mtk_factor_recalc_rate(fdiv, rate);
}

static ulong mtk_clk_get_rate(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_clk_tree *tree = priv->tree;

	if (mtk_clk_id_is_pll(tree, clk->id))
		return mtk_pll_get_rate(clk);

	if (mtk_clk_id_is_fclk(tree, clk->id))
		return tree->fclks[clk->id - tree->fclks_offs].rate;

	if (mtk_clk_id_is_fdiv(tree, clk->id))
		return mtk_factor_get_rate(clk, clk->id - tree->fdivs_offs);

	if (mtk_clk_id_is_mux(tree, clk->id))
		return mtk_clk_mux_get_rate(clk, clk->id - tree->muxes_offs);

	if (mtk_clk_id_is_gate(tree, clk->id)) {
		const struct mtk_gate *gate = &tree->gates[clk->id - tree->gates_offs];

		return mtk_find_parent_rate(priv, clk, gate->parent, gate->flags);
	}

	return -ENOENT;
}

/*
 * Pass a rate request on to the parent of a gate. The request stops at a PLL,
 * which reports the rate it already runs at: PLLs are shared, so only a
 * request made directly on one may retune it.
 *
 * @apply selects whether to program the hardware or only report what the
 * result would have been, so that the round and set paths below cannot
 * disagree. Both return the resulting rate.
 */
static ulong mtk_clk_parent_rate_request(struct clk *clk, int parent, u16 flags,
					 ulong rate, bool apply)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	struct mtk_clk_priv *ppriv;
	struct clk pclk;
	int ret;

	/* an external clock runs at a fixed rate: take it or leave it */
	if ((flags & CLK_PARENT_MASK) == CLK_PARENT_EXT)
		return mtk_ext_clock_get_rate(priv->tree, parent);

	ret = mtk_clk_lookup_parent(clk, parent, flags, &pclk);
	if (ret)
		return ret;

	ppriv = dev_get_priv(pclk.dev);
	if (mtk_clk_id_is_pll(ppriv->tree, pclk.id))
		return clk_get_rate(&pclk);

	if (!apply)
		return clk_round_rate(&pclk, rate);

	return clk_set_rate(&pclk, rate);
}

/*
 * Select the fastest mux input that does not exceed @rate, as the Linux
 * clk_mux_determine_rate_flags() does without CLK_MUX_ROUND_CLOSEST.
 *
 * A mux only ever picks between its inputs: it must not retune whatever PLL
 * happens to sit behind one of them, because those are shared - the audio
 * PLLs in particular feed unrelated blocks, and a display driver has no
 * business moving them. Linux sets CLK_SET_RATE_PARENT on these muxes, but
 * relies on rate change notifiers and exclusivity that U-Boot does not
 * have to keep that safe. A driver that needs a rate no input currently
 * provides has to set its own PLL first, the one its "pll" clock names, and
 * then ask the mux for the resulting rate.
 *
 * An input running at 0 Hz is never selected, as in Linux.
 *
 * Unlike Linux, the new input is not enabled before the mux switches to
 * it, and a PLL reports the rate its dividers are set to even while it is
 * powered off. The known users already set and enable the PLL they need
 * before asking the mux, so this is fine for now.
 */
static ulong mtk_clk_mux_rate_request(struct clk *clk, ulong rate, bool apply)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_composite *mux;
	ulong best_rate = 0;
	u32 best_index = 0;
	u32 i;

	mux = &priv->tree->muxes[clk->id - priv->tree->muxes_offs];

	for (i = 0; i < mux->num_parents; i++) {
		const struct mtk_parent *parent = &mux->parent[i];
		ulong prate;

		prate = mtk_find_parent_rate(priv, clk, parent->id,
					     parent->flags);
		if (IS_ERR_VALUE(prate) || !prate || prate > rate)
			continue;

		if (prate > best_rate) {
			best_index = i;
			best_rate = prate;
		}

		if (prate == rate)
			break;
	}

	if (!best_rate)
		return -EINVAL;

	if (apply)
		mtk_clk_mux_write_index(priv->base, best_index, mux);

	return best_rate;
}

/*
 * Only a PLL can change its own frequency, and only when asked directly.
 * Fixed clocks and fixed factors report the rate they run at, a mux picks
 * between its inputs and a gate passes the request on to its parent.
 */
static ulong mtk_clk_rate_request(struct clk *clk, ulong rate, bool apply)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_clk_tree *tree = priv->tree;

	if (mtk_clk_id_is_pll(tree, clk->id))
		return apply ? mtk_pll_set_rate(clk, rate) :
			       mtk_pll_round_rate(clk, rate);

	if (mtk_clk_id_is_fclk(tree, clk->id))
		return tree->fclks[clk->id - tree->fclks_offs].rate;

	if (mtk_clk_id_is_fdiv(tree, clk->id))
		return mtk_factor_get_rate(clk, clk->id - tree->fdivs_offs);

	if (mtk_clk_id_is_mux(tree, clk->id))
		return mtk_clk_mux_rate_request(clk, rate, apply);

	if (mtk_clk_id_is_gate(tree, clk->id)) {
		const struct mtk_gate *gate;

		gate = &tree->gates[clk->id - tree->gates_offs];

		return mtk_clk_parent_rate_request(clk, gate->parent,
						   gate->flags, rate, apply);
	}

	return -ENOENT;
}

static ulong mtk_clk_round_rate(struct clk *clk, ulong rate)
{
	return mtk_clk_rate_request(clk, rate, false);
}

static ulong mtk_clk_set_rate(struct clk *clk, ulong rate)
{
	return mtk_clk_rate_request(clk, rate, true);
}

static int mtk_clk_mux_enable(void __iomem *base,
			      const struct mtk_composite *mux)
{
	u32 val;

	if (mux->gate_shift < 0)
		return 0;

	/* enable clock gate */
	if (mux->flags & CLK_MUX_SETCLR_UPD) {
		val = BIT(mux->gate_shift);
		writel(val, base + mux->mux_clr_reg);
	} else {
		val = readl(base + mux->gate_reg);
		val &= ~BIT(mux->gate_shift);
		writel(val, base + mux->gate_reg);
	}

	if (mux->flags & CLK_MUX_DOMAIN_SCPSYS) {
		/* enable scpsys clock off control */
		writel(SCP_ARMCK_OFF_EN, base + CLK_SCP_CFG0);
		writel(SCP_AXICK_DCM_DIS_EN | SCP_AXICK_26M_SEL_EN,
		       base + CLK_SCP_CFG1);
	}

	return 0;
}

static int mtk_clk_mux_disable(void __iomem *base,
			       const struct mtk_composite *mux)
{
	u32 val;

	if (mux->gate_shift < 0)
		return 0;

	/* disable clock gate */
	if (mux->flags & CLK_MUX_SETCLR_UPD) {
		val = BIT(mux->gate_shift);
		writel(val, base + mux->mux_set_reg);
	} else {
		val = readl(base + mux->gate_reg);
		val |= BIT(mux->gate_shift);
		writel(val, base + mux->gate_reg);
	}

	return 0;
}

static int mtk_clk_enable(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_clk_tree *tree = priv->tree;

	if (mtk_clk_id_is_pll(tree, clk->id))
		return mtk_pll_enable(priv->base, &tree->plls[clk->id]);

	if (mtk_clk_id_is_mux(tree, clk->id))
		return mtk_clk_mux_enable(priv->base,
					  &tree->muxes[clk->id - tree->muxes_offs]);

	if (mtk_clk_id_is_gate(tree, clk->id))
		return mtk_gate_enable(priv->base,
				       &tree->gates[clk->id - tree->gates_offs]);

	return 0;
}

static int mtk_clk_disable(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_clk_tree *tree = priv->tree;

	if (mtk_clk_id_is_pll(tree, clk->id))
		return mtk_pll_disable(priv->base, &tree->plls[clk->id]);

	if (mtk_clk_id_is_mux(tree, clk->id))
		return mtk_clk_mux_disable(priv->base,
					   &tree->muxes[clk->id - tree->muxes_offs]);

	if (mtk_clk_id_is_gate(tree, clk->id))
		return mtk_gate_disable(priv->base,
					&tree->gates[clk->id - tree->gates_offs]);

	return 0;
}

static int mtk_common_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct mtk_clk_priv *parent_priv = dev_get_priv(parent->dev);
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	int parent_unmapped_id;
	u32 parent_type;

	if (!mtk_clk_id_is_mux(priv->tree, clk->id))
		return 0;

	if (!parent_priv)
		return 0;

	parent_unmapped_id = mtk_common_clk_get_unmapped_id(parent);
	if (parent_unmapped_id < 0)
		return parent_unmapped_id;

	parent_type = parent_priv->tree->flags & CLK_PARENT_MASK;
	return mtk_clk_mux_set_parent(priv->base, parent_unmapped_id, parent_type,
			&priv->tree->muxes[clk->id - priv->tree->muxes_offs]);
}

#if CONFIG_IS_ENABLED(CMD_CLK)
static void mtk_clk_dump(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);
	const struct mtk_clk_tree *tree = priv->tree;
	u32 i;

	for (i = 0; i < tree->num_plls; i++) {
		const struct mtk_pll_data *pll = &tree->plls[i];

		printf("[PLL%u] DT: %u", i, pll->id);
		mtk_clk_print_mapped_id(pll->id, i, tree->id_offs_map);
		mtk_clk_print_rate(dev, i);
		printf("\n");
	}

	for (i = 0; i < tree->num_fclks; i++) {
		const struct mtk_fixed_clk *fclk = &tree->fclks[i];

		printf("[FCLK%u] DT: %u", i, fclk->id);
		mtk_clk_print_mapped_id(fclk->id, i + tree->fclks_offs, tree->id_offs_map);
		mtk_clk_print_rate(dev, i + tree->fclks_offs);
		mtk_clk_print_single_parent(fclk->parent, fclk->flags);
		printf("\n");
	}

	for (i = 0; i < tree->num_fdivs; i++) {
		const struct mtk_fixed_factor *fdiv = &tree->fdivs[i];

		printf("[FDIV%u] DT: %u", i, fdiv->id);
		mtk_clk_print_mapped_id(fdiv->id, i + tree->fdivs_offs, tree->id_offs_map);
		mtk_clk_print_rate(dev, i + tree->fdivs_offs);
		mtk_clk_print_single_parent(fdiv->parent, fdiv->flags);
		printf(", Mult: %u, Div: %u\n", fdiv->mult, fdiv->div);
	}

	for (i = 0; i < tree->num_muxes; i++) {
		const struct mtk_composite *mux = &tree->muxes[i];

		printf("[MUX%u] DT: %u", i, mux->id);
		mtk_clk_print_mapped_id(mux->id, i + tree->muxes_offs, tree->id_offs_map);
		mtk_clk_print_rate(dev, i + tree->muxes_offs);
		mtk_clk_print_mux_parents(priv, mux);
		printf("\n");
	}

	for (i = 0; i < tree->num_gates; i++) {
		const struct mtk_gate *gate = &tree->gates[i];

		printf("[GATE%u] DT: %u", i, gate->id);
		mtk_clk_print_mapped_id(gate->id, i + tree->gates_offs, tree->id_offs_map);
		mtk_clk_print_rate(dev, i + tree->gates_offs);
		mtk_clk_print_single_parent(gate->parent, gate->flags);
		printf("\n");
	}
}
#endif

const struct clk_ops mtk_clk_ops = {
	.of_xlate = mtk_clk_of_xlate,
	.enable = mtk_clk_enable,
	.disable = mtk_clk_disable,
	.set_rate = mtk_clk_set_rate,
	.round_rate = mtk_clk_round_rate,
	.get_rate = mtk_clk_get_rate,
	.set_parent = mtk_common_clk_set_parent,
#if CONFIG_IS_ENABLED(CMD_CLK)
	.dump = mtk_clk_dump,
#endif
};

int mtk_clk_probe(struct udevice *dev)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);
	const struct mtk_clk_tree *tree = (void *)dev_get_driver_data(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	priv->tree = tree;

	return 0;
}

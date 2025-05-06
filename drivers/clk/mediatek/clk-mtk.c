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
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>

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

/* shared functions */

static int mtk_clk_get_id(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	int id = clk->id;

	/* Remap the clk ID to the one expected by driver */
	if (priv->tree->id_offs_map)
		id = priv->tree->id_offs_map[id];

	return id;
}

static int mtk_dummy_enable(struct clk *clk)
{
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

/*
 * In case the rate change propagation to parent clocks is undesirable,
 * this function is recursively called to find the parent to calculate
 * the accurate frequency.
 */
static ulong mtk_clk_find_parent_rate(struct clk *clk, int id,
				      struct udevice *pdev)
{
	struct clk parent = { .id = id, };

	if (pdev)
		parent.dev = pdev;
	else
		parent.dev = clk->dev;

	return clk_get_rate(&parent);
}

static int mtk_clk_mux_set_parent(void __iomem *base, u32 parent,
				  u32 parent_type,
				  const struct mtk_composite *mux)
{
	u32 val, index = 0;

	if (mux->flags & CLK_PARENT_MIXED) {
		/*
		 * Assume parent_type in clk_tree to be always set with
		 * CLK_PARENT_MIXED implementation. If it's not, assume
		 * not parent clk ID clash is possible.
		 */
		while (mux->parent_flags[index].id != parent ||
		       (parent_type && (mux->parent_flags[index].flags & CLK_PARENT_MASK) !=
			parent_type))
			if (++index == mux->num_parents)
				return -EINVAL;
	} else {
		while (mux->parent[index] != parent)
			if (++index == mux->num_parents)
				return -EINVAL;
	}

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

	return 0;
}

/* apmixedsys functions */

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
 * @priv:	The mtk priv struct
 * @id:		The clk id
 * @pcw:	The pcw value (output)
 * @postdiv:	The post divider (output)
 * @freq:	The desired target frequency
 */
static void mtk_pll_calc_values(struct mtk_clk_priv *priv, u32 id,
				u32 *pcw, u32 *postdiv, u32 freq)
{
	const struct mtk_pll_data *pll;
	unsigned long fmin;
	u64 _pcw;
	int ibits;
	u32 val;

	pll = &priv->tree->plls[id];
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
	do_div(_pcw, priv->tree->xtal2_rate);

	*pcw = (u32)_pcw;
}

static ulong mtk_apmixedsys_set_rate(struct clk *clk, ulong rate)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	int id = mtk_clk_get_id(clk);
	u32 pcw = 0;
	u32 postdiv;

	if (priv->tree->gates && id >= priv->tree->gates_offs)
		return -EINVAL;

	mtk_pll_calc_values(priv, id, &pcw, &postdiv, rate);
	mtk_pll_set_rate_regs(priv, id, pcw, postdiv);

	return 0;
}

static ulong mtk_apmixedsys_get_rate(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_pll_data *pll;
	int id = mtk_clk_get_id(clk);
	const struct mtk_gate *gate;
	u32 postdiv;
	u32 pcw;

	/* GATE handling */
	if (priv->tree->gates && id >= priv->tree->gates_offs) {
		gate = &priv->tree->gates[id - priv->tree->gates_offs];
		return mtk_clk_find_parent_rate(clk, gate->parent, NULL);
	}

	pll = &priv->tree->plls[id];

	postdiv = (readl(priv->base + pll->pd_reg) >> pll->pd_shift) &
		   POSTDIV_MASK;
	postdiv = 1 << postdiv;

	pcw = readl(priv->base + pll->pcw_reg) >> pll->pcw_shift;
	pcw &= GENMASK(pll->pcwbits - 1, 0);

	return __mtk_pll_recalc_rate(pll, priv->tree->xtal2_rate,
				     pcw, postdiv);
}

static int mtk_apmixedsys_enable(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_pll_data *pll;
	int id = mtk_clk_get_id(clk);
	const struct mtk_gate *gate;
	u32 r;

	/* GATE handling */
	if (priv->tree->gates && id >= priv->tree->gates_offs) {
		gate = &priv->tree->gates[id - priv->tree->gates_offs];
		return mtk_gate_enable(priv->base, gate);
	}

	pll = &priv->tree->plls[id];

	r = readl(priv->base + pll->pwr_reg) | CON0_PWR_ON;
	writel(r, priv->base + pll->pwr_reg);
	udelay(1);

	r = readl(priv->base + pll->pwr_reg) & ~CON0_ISO_EN;
	writel(r, priv->base + pll->pwr_reg);
	udelay(1);

	r = readl(priv->base + pll->reg + REG_CON0);
	r |= pll->en_mask;
	writel(r, priv->base + pll->reg + REG_CON0);

	udelay(20);

	if (pll->flags & HAVE_RST_BAR) {
		r = readl(priv->base + pll->reg + REG_CON0);
		r |= pll->rst_bar_mask;
		writel(r, priv->base + pll->reg + REG_CON0);
	}

	return 0;
}

static int mtk_apmixedsys_disable(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_pll_data *pll;
	int id = mtk_clk_get_id(clk);
	const struct mtk_gate *gate;
	u32 r;

	/* GATE handling */
	if (priv->tree->gates && id >= priv->tree->gates_offs) {
		gate = &priv->tree->gates[id - priv->tree->gates_offs];
		return mtk_gate_disable(priv->base, gate);
	}

	pll = &priv->tree->plls[id];

	if (pll->flags & HAVE_RST_BAR) {
		r = readl(priv->base + pll->reg + REG_CON0);
		r &= ~pll->rst_bar_mask;
		writel(r, priv->base + pll->reg + REG_CON0);
	}

	r = readl(priv->base + pll->reg + REG_CON0);
	r &= ~CON0_BASE_EN;
	writel(r, priv->base + pll->reg + REG_CON0);

	r = readl(priv->base + pll->pwr_reg) | CON0_ISO_EN;
	writel(r, priv->base + pll->pwr_reg);

	r = readl(priv->base + pll->pwr_reg) & ~CON0_PWR_ON;
	writel(r, priv->base + pll->pwr_reg);

	return 0;
}

/* topckgen functions */

static ulong mtk_factor_recalc_rate(const struct mtk_fixed_factor *fdiv,
				    ulong parent_rate)
{
	u64 rate = parent_rate * fdiv->mult;

	do_div(rate, fdiv->div);

	return rate;
}

static ulong mtk_topckgen_get_factor_rate(struct clk *clk, u32 off)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_fixed_factor *fdiv = &priv->tree->fdivs[off];
	ulong rate;

	switch (fdiv->flags & CLK_PARENT_MASK) {
	case CLK_PARENT_APMIXED:
		rate = mtk_clk_find_parent_rate(clk, fdiv->parent,
						priv->parent);
		break;
	case CLK_PARENT_TOPCKGEN:
		rate = mtk_clk_find_parent_rate(clk, fdiv->parent, NULL);
		break;

	case CLK_PARENT_XTAL:
	default:
		rate = priv->tree->xtal_rate;
	}

	return mtk_factor_recalc_rate(fdiv, rate);
}

static ulong mtk_infrasys_get_factor_rate(struct clk *clk, u32 off)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_fixed_factor *fdiv = &priv->tree->fdivs[off];
	ulong rate;

	switch (fdiv->flags & CLK_PARENT_MASK) {
	case CLK_PARENT_TOPCKGEN:
		rate = mtk_clk_find_parent_rate(clk, fdiv->parent,
						priv->parent);
		break;
	case CLK_PARENT_XTAL:
		rate = priv->tree->xtal_rate;
		break;
	default:
		rate = mtk_clk_find_parent_rate(clk, fdiv->parent, NULL);
	}

	return mtk_factor_recalc_rate(fdiv, rate);
}

static ulong mtk_topckgen_find_parent_rate(struct mtk_clk_priv *priv, struct clk *clk,
					   const int parent, u16 flags)
{
	switch (flags & CLK_PARENT_MASK) {
	case CLK_PARENT_XTAL:
		return priv->tree->xtal_rate;
	case CLK_PARENT_APMIXED:
		return mtk_clk_find_parent_rate(clk, parent, priv->parent);
	default:
		return mtk_clk_find_parent_rate(clk, parent, NULL);
	}
}

static ulong mtk_topckgen_get_mux_rate(struct clk *clk, u32 off)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_composite *mux = &priv->tree->muxes[off];
	u32 index;

	index = readl(priv->base + mux->mux_reg);
	index &= mux->mux_mask << mux->mux_shift;
	index = index >> mux->mux_shift;

	/*
	 * Parents can be either from APMIXED or TOPCKGEN,
	 * inspect the mtk_parent struct to check the source
	 */
	if (mux->flags & CLK_PARENT_MIXED) {
		const struct mtk_parent *parent = &mux->parent_flags[index];

		return mtk_topckgen_find_parent_rate(priv, clk, parent->id,
						     parent->flags);
	}

	if (mux->parent[index] == CLK_XTAL &&
	    !(priv->tree->flags & CLK_BYPASS_XTAL))
		return priv->tree->xtal_rate;

	return mtk_topckgen_find_parent_rate(priv, clk, mux->parent[index],
					     mux->flags);
}

static ulong mtk_find_parent_rate(struct mtk_clk_priv *priv, struct clk *clk,
				  const int parent, u16 flags)
{
	switch (flags & CLK_PARENT_MASK) {
	case CLK_PARENT_XTAL:
		return priv->tree->xtal_rate;
	/* Assume the second level parent is always APMIXED */
	case CLK_PARENT_APMIXED:
		priv = dev_get_priv(priv->parent);
		fallthrough;
	case CLK_PARENT_TOPCKGEN:
		return mtk_clk_find_parent_rate(clk, parent, priv->parent);
	default:
		return mtk_clk_find_parent_rate(clk, parent, NULL);
	}
}

static ulong mtk_infrasys_get_mux_rate(struct clk *clk, u32 off)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_composite *mux = &priv->tree->muxes[off];
	u32 index;

	index = readl(priv->base + mux->mux_reg);
	index &= mux->mux_mask << mux->mux_shift;
	index = index >> mux->mux_shift;

	/*
	 * Parents can be either from TOPCKGEN or INFRACFG,
	 * inspect the mtk_parent struct to check the source
	 */
	if (mux->flags & CLK_PARENT_MIXED) {
		const struct mtk_parent *parent = &mux->parent_flags[index];

		return mtk_find_parent_rate(priv, clk, parent->id, parent->flags);
	}

	if (mux->parent[index] == CLK_XTAL &&
	    !(priv->tree->flags & CLK_BYPASS_XTAL))
		return priv->tree->xtal_rate;

	return mtk_find_parent_rate(priv, clk, mux->parent[index], mux->flags);
}

static ulong mtk_topckgen_get_rate(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	int id = mtk_clk_get_id(clk);

	if (id < priv->tree->fdivs_offs)
		return priv->tree->fclks[id].rate;
	else if (id < priv->tree->muxes_offs)
		return mtk_topckgen_get_factor_rate(clk, id -
						    priv->tree->fdivs_offs);
	else
		return mtk_topckgen_get_mux_rate(clk, id -
						 priv->tree->muxes_offs);
}

static ulong mtk_infrasys_get_rate(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	int id = mtk_clk_get_id(clk);
	ulong rate;

	if (id < priv->tree->fdivs_offs) {
		rate = priv->tree->fclks[id].rate;
	} else if (id < priv->tree->muxes_offs) {
		rate = mtk_infrasys_get_factor_rate(clk, id -
						    priv->tree->fdivs_offs);
	/* No gates defined or ID is a MUX */
	} else if (!priv->tree->gates || id < priv->tree->gates_offs) {
		rate = mtk_infrasys_get_mux_rate(clk, id -
						 priv->tree->muxes_offs);
	/* Only valid with muxes + gates implementation */
	} else {
		struct udevice *parent = NULL;
		const struct mtk_gate *gate;

		gate = &priv->tree->gates[id - priv->tree->gates_offs];
		if (gate->flags & CLK_PARENT_TOPCKGEN)
			parent = priv->parent;
		/*
		 * Assume xtal_rate to be declared if some gates have
		 * XTAL as parent
		 */
		else if (gate->flags & CLK_PARENT_XTAL)
			return priv->tree->xtal_rate;

		rate = mtk_clk_find_parent_rate(clk, gate->parent, parent);
	}

	return rate;
}

static int mtk_clk_mux_enable(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_composite *mux;
	int id = mtk_clk_get_id(clk);
	u32 val;

	if (id < priv->tree->muxes_offs)
		return 0;

	mux = &priv->tree->muxes[id - priv->tree->muxes_offs];
	if (mux->gate_shift < 0)
		return 0;

	/* enable clock gate */
	if (mux->flags & CLK_MUX_SETCLR_UPD) {
		val = BIT(mux->gate_shift);
		writel(val, priv->base + mux->mux_clr_reg);
	} else {
		val = readl(priv->base + mux->gate_reg);
		val &= ~BIT(mux->gate_shift);
		writel(val, priv->base + mux->gate_reg);
	}

	if (mux->flags & CLK_DOMAIN_SCPSYS) {
		/* enable scpsys clock off control */
		writel(SCP_ARMCK_OFF_EN, priv->base + CLK_SCP_CFG0);
		writel(SCP_AXICK_DCM_DIS_EN | SCP_AXICK_26M_SEL_EN,
		       priv->base + CLK_SCP_CFG1);
	}

	return 0;
}

static int mtk_clk_mux_disable(struct clk *clk)
{
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	const struct mtk_composite *mux;
	int id = mtk_clk_get_id(clk);
	u32 val;

	if (id < priv->tree->muxes_offs)
		return 0;

	mux = &priv->tree->muxes[id - priv->tree->muxes_offs];
	if (mux->gate_shift < 0)
		return 0;

	/* disable clock gate */
	if (mux->flags & CLK_MUX_SETCLR_UPD) {
		val = BIT(mux->gate_shift);
		writel(val, priv->base + mux->mux_set_reg);
	} else {
		val = readl(priv->base + mux->gate_reg);
		val |= BIT(mux->gate_shift);
		writel(val, priv->base + mux->gate_reg);
	}

	return 0;
}

static int mtk_common_clk_set_parent(struct clk *clk, struct clk *parent)
{
	struct mtk_clk_priv *parent_priv = dev_get_priv(parent->dev);
	struct mtk_clk_priv *priv = dev_get_priv(clk->dev);
	int id = mtk_clk_get_id(clk);
	u32 parent_type;

	if (id < priv->tree->muxes_offs)
		return 0;

	if (!parent_priv)
		return 0;

	parent_type = parent_priv->tree->flags & CLK_PARENT_MASK;
	return mtk_clk_mux_set_parent(priv->base, parent->id, parent_type,
			&priv->tree->muxes[id - priv->tree->muxes_offs]);
}

/* CG functions */

static int mtk_clk_gate_enable(struct clk *clk)
{
	struct mtk_cg_priv *priv = dev_get_priv(clk->dev);
	int id = mtk_clk_get_id(clk);
	const struct mtk_gate *gate;

	if (id < priv->tree->gates_offs)
		return -EINVAL;

	gate = &priv->gates[id - priv->tree->gates_offs];
	return mtk_gate_enable(priv->base, gate);
}

static int mtk_clk_infrasys_enable(struct clk *clk)
{
	struct mtk_cg_priv *priv = dev_get_priv(clk->dev);
	int id = mtk_clk_get_id(clk);
	const struct mtk_gate *gate;

	/* MUX handling */
	if (!priv->tree->gates || id < priv->tree->gates_offs)
		return mtk_clk_mux_enable(clk);

	gate = &priv->tree->gates[id - priv->tree->gates_offs];
	return mtk_gate_enable(priv->base, gate);
}

static int mtk_clk_gate_disable(struct clk *clk)
{
	struct mtk_cg_priv *priv = dev_get_priv(clk->dev);
	int id = mtk_clk_get_id(clk);
	const struct mtk_gate *gate;

	if (id < priv->tree->gates_offs)
		return -EINVAL;

	gate = &priv->gates[id - priv->tree->gates_offs];
	return mtk_gate_disable(priv->base, gate);
}

static int mtk_clk_infrasys_disable(struct clk *clk)
{
	struct mtk_cg_priv *priv = dev_get_priv(clk->dev);
	int id = mtk_clk_get_id(clk);
	const struct mtk_gate *gate;

	/* MUX handling */
	if (!priv->tree->gates || id < priv->tree->gates_offs)
		return mtk_clk_mux_disable(clk);

	gate = &priv->tree->gates[id - priv->tree->gates_offs];
	return mtk_gate_disable(priv->base, gate);
}

static ulong mtk_clk_gate_get_rate(struct clk *clk)
{
	struct mtk_cg_priv *priv = dev_get_priv(clk->dev);
	struct udevice *parent = priv->parent;
	int id = mtk_clk_get_id(clk);
	const struct mtk_gate *gate;

	if (id < priv->tree->gates_offs)
		return -EINVAL;

	gate = &priv->gates[id - priv->tree->gates_offs];
	/*
	 * With requesting a TOPCKGEN parent, make sure the dev parent
	 * is actually topckgen. This might not be the case for an
	 * infracfg-ao implementation where:
	 * parent = infracfg
	 * parent->parent = topckgen
	 */
	if (gate->flags & CLK_PARENT_TOPCKGEN &&
	    parent->driver != DM_DRIVER_GET(mtk_clk_topckgen)) {
		priv = dev_get_priv(parent);
		parent = priv->parent;
	/*
	 * Assume xtal_rate to be declared if some gates have
	 * XTAL as parent
	 */
	} else if (gate->flags & CLK_PARENT_XTAL) {
		return priv->tree->xtal_rate;
	}

	return mtk_clk_find_parent_rate(clk, gate->parent, parent);
}

const struct clk_ops mtk_clk_apmixedsys_ops = {
	.enable = mtk_apmixedsys_enable,
	.disable = mtk_apmixedsys_disable,
	.set_rate = mtk_apmixedsys_set_rate,
	.get_rate = mtk_apmixedsys_get_rate,
};

const struct clk_ops mtk_clk_fixed_pll_ops = {
	.enable = mtk_dummy_enable,
	.disable = mtk_dummy_enable,
	.get_rate = mtk_topckgen_get_rate,
};

const struct clk_ops mtk_clk_topckgen_ops = {
	.enable = mtk_clk_mux_enable,
	.disable = mtk_clk_mux_disable,
	.get_rate = mtk_topckgen_get_rate,
	.set_parent = mtk_common_clk_set_parent,
};

const struct clk_ops mtk_clk_infrasys_ops = {
	.enable = mtk_clk_infrasys_enable,
	.disable = mtk_clk_infrasys_disable,
	.get_rate = mtk_infrasys_get_rate,
	.set_parent = mtk_common_clk_set_parent,
};

const struct clk_ops mtk_clk_gate_ops = {
	.enable = mtk_clk_gate_enable,
	.disable = mtk_clk_gate_disable,
	.get_rate = mtk_clk_gate_get_rate,
};

static int mtk_common_clk_init_drv(struct udevice *dev,
				   const struct mtk_clk_tree *tree,
				   const struct driver *drv)
{
	struct mtk_clk_priv *priv = dev_get_priv(dev);
	struct udevice *parent;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	ret = uclass_get_device_by_phandle(UCLASS_CLK, dev, "clock-parent", &parent);
	if (ret || !parent) {
		ret = uclass_get_device_by_driver(UCLASS_CLK, drv, &parent);
		if (ret || !parent)
			return -ENOENT;
	}

	priv->parent = parent;
	priv->tree = tree;

	return 0;
}

int mtk_common_clk_init(struct udevice *dev,
			const struct mtk_clk_tree *tree)
{
	return mtk_common_clk_init_drv(dev, tree,
				       DM_DRIVER_GET(mtk_clk_apmixedsys));
}

int mtk_common_clk_infrasys_init(struct udevice *dev,
				 const struct mtk_clk_tree *tree)
{
	return mtk_common_clk_init_drv(dev, tree,
				       DM_DRIVER_GET(mtk_clk_topckgen));
}

int mtk_common_clk_gate_init(struct udevice *dev,
			     const struct mtk_clk_tree *tree,
			     const struct mtk_gate *gates)
{
	struct mtk_cg_priv *priv = dev_get_priv(dev);
	struct udevice *parent;
	int ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENOENT;

	ret = uclass_get_device_by_phandle(UCLASS_CLK, dev, "clock-parent", &parent);
	if (ret || !parent) {
		ret = uclass_get_device_by_driver(UCLASS_CLK,
				DM_DRIVER_GET(mtk_clk_topckgen), &parent);
		if (ret || !parent)
			return -ENOENT;
	}

	priv->parent = parent;
	priv->tree = tree;
	priv->gates = gates;

	return 0;
}

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023 Inochi Amaoto <inochiama@outlook.com>
 */

#include <dm.h>
#include <div64.h>
#include <linux/clk-provider.h>
#include <linux/io.h>

#include "clk-common.h"
#include "clk-ip.h"

static int get_parent_index(struct clk *clk, const char *const *parent_name,
			    u8 num_parents)
{
	const char *name = clk_hw_get_name(clk);
	int i;

	for (i = 0; i < num_parents; i++) {
		if (!strcmp(name, parent_name[i]))
			return i;
	}

	return -1;
}

/* GATE */
#define to_cv1800b_clk_gate(_clk) \
	container_of(_clk, struct cv1800b_clk_gate, clk)

static int gate_enable(struct clk *clk)
{
	struct cv1800b_clk_gate *gate = to_cv1800b_clk_gate(clk);

	return cv1800b_clk_setbit(gate->base, &gate->gate);
}

static int gate_disable(struct clk *clk)
{
	struct cv1800b_clk_gate *gate = to_cv1800b_clk_gate(clk);

	return cv1800b_clk_clrbit(gate->base, &gate->gate);
}

static ulong gate_get_rate(struct clk *clk)
{
	return clk_get_parent_rate(clk);
}

const struct clk_ops cv1800b_clk_gate_ops = {
	.disable = gate_disable,
	.enable = gate_enable,
	.get_rate = gate_get_rate,
};

U_BOOT_DRIVER(cv1800b_clk_gate) = {
	.name = "cv1800b_clk_gate",
	.id = UCLASS_CLK,
	.ops = &cv1800b_clk_gate_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* DIV */
#define CLK_DIV_EN_FACTOR BIT(3)

#define to_cv1800b_clk_div(_clk) container_of(_clk, struct cv1800b_clk_div, clk)

static int div_enable(struct clk *clk)
{
	struct cv1800b_clk_div *div = to_cv1800b_clk_div(clk);

	return cv1800b_clk_setbit(div->base, &div->gate);
}

static int div_disable(struct clk *clk)
{
	struct cv1800b_clk_div *div = to_cv1800b_clk_div(clk);

	return cv1800b_clk_clrbit(div->base, &div->gate);
}

static ulong div_get_rate(struct clk *clk)
{
	struct cv1800b_clk_div *div = to_cv1800b_clk_div(clk);
	ulong val;

	if (div->div_init == 0 ||
	    readl(div->base + div->div.offset) & CLK_DIV_EN_FACTOR)
		val = cv1800b_clk_getfield(div->base, &div->div);
	else
		val = div->div_init;

	return DIV_ROUND_UP_ULL(clk_get_parent_rate(clk), val);
}

static ulong div_set_rate(struct clk *clk, ulong rate)
{
	struct cv1800b_clk_div *div = to_cv1800b_clk_div(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 val;

	val = DIV_ROUND_UP_ULL(parent_rate, rate);
	val = min_t(u32, val, clk_div_mask(div->div.width));

	cv1800b_clk_setfield(div->base, &div->div, val);
	if (div->div_init > 0)
		setbits_le32(div->base + div->div.offset, CLK_DIV_EN_FACTOR);

	return DIV_ROUND_UP_ULL(parent_rate, val);
}

const struct clk_ops cv1800b_clk_div_ops = {
	.disable = div_disable,
	.enable = div_enable,
	.get_rate = div_get_rate,
	.set_rate = div_set_rate,
};

U_BOOT_DRIVER(cv1800b_clk_div) = {
	.name = "cv1800b_clk_div",
	.id = UCLASS_CLK,
	.ops = &cv1800b_clk_div_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

#define to_cv1800b_clk_bypass_div(_clk) \
	container_of(_clk, struct cv1800b_clk_bypass_div, div.clk)

static ulong bypass_div_get_rate(struct clk *clk)
{
	struct cv1800b_clk_bypass_div *div = to_cv1800b_clk_bypass_div(clk);

	if (cv1800b_clk_getbit(div->div.base, &div->bypass))
		return 0;

	return div_get_rate(clk);
}

static ulong bypass_div_set_rate(struct clk *clk, ulong rate)
{
	struct cv1800b_clk_bypass_div *div = to_cv1800b_clk_bypass_div(clk);

	if (cv1800b_clk_getbit(div->div.base, &div->bypass))
		return 0;

	return div_set_rate(clk, rate);
}

static int bypass_div_set_parent(struct clk *clk, struct clk *pclk)
{
	struct cv1800b_clk_bypass_div *div = to_cv1800b_clk_bypass_div(clk);

	if (pclk->id == CV1800B_CLK_BYPASS) {
		cv1800b_clk_setbit(div->div.base, &div->bypass);
		return 0;
	}

	if (strcmp(clk_hw_get_name(pclk), div->div.parent_name))
		return -EINVAL;

	cv1800b_clk_clrbit(div->div.base, &div->bypass);
	return 0;
}

const struct clk_ops cv1800b_clk_bypass_div_ops = {
	.disable = div_disable,
	.enable = div_enable,
	.get_rate = bypass_div_get_rate,
	.set_rate = bypass_div_set_rate,
	.set_parent = bypass_div_set_parent,
};

U_BOOT_DRIVER(cv1800b_clk_bypass_div) = {
	.name = "cv1800b_clk_bypass_div",
	.id = UCLASS_CLK,
	.ops = &cv1800b_clk_bypass_div_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* FIXED DIV */
#define to_cv1800b_clk_fixed_div(_clk) \
	container_of(_clk, struct cv1800b_clk_fixed_div, clk)

static int fixed_div_enable(struct clk *clk)
{
	struct cv1800b_clk_fixed_div *div = to_cv1800b_clk_fixed_div(clk);

	return cv1800b_clk_setbit(div->base, &div->gate);
}

static int fixed_div_disable(struct clk *clk)
{
	struct cv1800b_clk_fixed_div *div = to_cv1800b_clk_fixed_div(clk);

	return cv1800b_clk_clrbit(div->base, &div->gate);
}

static ulong fixed_div_get_rate(struct clk *clk)
{
	struct cv1800b_clk_fixed_div *div = to_cv1800b_clk_fixed_div(clk);

	return DIV_ROUND_UP_ULL(clk_get_parent_rate(clk), div->div);
}

const struct clk_ops cv1800b_clk_fixed_div_ops = {
	.disable = fixed_div_disable,
	.enable = fixed_div_enable,
	.get_rate = fixed_div_get_rate,
};

U_BOOT_DRIVER(cv1800b_clk_fixed_div) = {
	.name = "cv1800b_clk_fixed_div",
	.id = UCLASS_CLK,
	.ops = &cv1800b_clk_fixed_div_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

#define to_cv1800b_clk_bypass_fixed_div(_clk) \
	container_of(_clk, struct cv1800b_clk_bypass_fixed_div, div.clk)

static ulong bypass_fixed_div_get_rate(struct clk *clk)
{
	struct cv1800b_clk_bypass_fixed_div *div =
		to_cv1800b_clk_bypass_fixed_div(clk);

	if (cv1800b_clk_getbit(div->div.base, &div->bypass))
		return 0;

	return fixed_div_get_rate(clk);
}

static int bypass_fixed_div_set_parent(struct clk *clk, struct clk *pclk)
{
	struct cv1800b_clk_bypass_fixed_div *div =
		to_cv1800b_clk_bypass_fixed_div(clk);

	if (pclk->id == CV1800B_CLK_BYPASS) {
		cv1800b_clk_setbit(div->div.base, &div->bypass);
		return 0;
	}

	if (strcmp(clk_hw_get_name(pclk), div->div.parent_name))
		return -EINVAL;

	cv1800b_clk_clrbit(div->div.base, &div->bypass);
	return 0;
}

const struct clk_ops cv1800b_clk_bypass_fixed_div_ops = {
	.disable = fixed_div_disable,
	.enable = fixed_div_enable,
	.get_rate = bypass_fixed_div_get_rate,
	.set_parent = bypass_fixed_div_set_parent,
};

U_BOOT_DRIVER(cv1800b_clk_bypass_fixed_div) = {
	.name = "cv1800b_clk_bypass_fixed_div",
	.id = UCLASS_CLK,
	.ops = &cv1800b_clk_bypass_fixed_div_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* MUX */
#define to_cv1800b_clk_mux(_clk) container_of(_clk, struct cv1800b_clk_mux, clk)

static int mux_enable(struct clk *clk)
{
	struct cv1800b_clk_mux *mux = to_cv1800b_clk_mux(clk);

	return cv1800b_clk_setbit(mux->base, &mux->gate);
}

static int mux_disable(struct clk *clk)
{
	struct cv1800b_clk_mux *mux = to_cv1800b_clk_mux(clk);

	return cv1800b_clk_clrbit(mux->base, &mux->gate);
}

static ulong mux_get_rate(struct clk *clk)
{
	struct cv1800b_clk_mux *mux = to_cv1800b_clk_mux(clk);
	ulong val;

	if (mux->div_init == 0 ||
	    readl(mux->base + mux->div.offset) & CLK_DIV_EN_FACTOR)
		val = cv1800b_clk_getfield(mux->base, &mux->div);
	else
		val = mux->div_init;

	return DIV_ROUND_UP_ULL(clk_get_parent_rate(clk), val);
}

static ulong mux_set_rate(struct clk *clk, ulong rate)
{
	struct cv1800b_clk_mux *mux = to_cv1800b_clk_mux(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	ulong val;

	val = DIV_ROUND_UP_ULL(parent_rate, rate);
	val = min_t(u32, val, clk_div_mask(mux->div.width));

	cv1800b_clk_setfield(mux->base, &mux->div, val);
	if (mux->div_init > 0)
		setbits_le32(mux->base + mux->div.offset, CLK_DIV_EN_FACTOR);

	return DIV_ROUND_UP_ULL(parent_rate, val);
}

static int mux_set_parent(struct clk *clk, struct clk *pclk)
{
	struct cv1800b_clk_mux *mux = to_cv1800b_clk_mux(clk);
	int index = get_parent_index(pclk, mux->parent_names, mux->num_parents);

	if (index < 0)
		return -EINVAL;

	cv1800b_clk_setfield(mux->base, &mux->mux, index);
	return 0;
}

const struct clk_ops cv1800b_clk_mux_ops = {
	.disable = mux_disable,
	.enable = mux_enable,
	.get_rate = mux_get_rate,
	.set_rate = mux_set_rate,
	.set_parent = mux_set_parent,
};

U_BOOT_DRIVER(cv1800b_clk_mux) = {
	.name = "cv1800b_clk_mux",
	.id = UCLASS_CLK,
	.ops = &cv1800b_clk_mux_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

#define to_cv1800b_clk_bypass_mux(_clk) \
	container_of(_clk, struct cv1800b_clk_bypass_mux, mux.clk)

static ulong bypass_mux_get_rate(struct clk *clk)
{
	struct cv1800b_clk_bypass_mux *mux = to_cv1800b_clk_bypass_mux(clk);

	if (cv1800b_clk_getbit(mux->mux.base, &mux->bypass))
		return 0;

	return mux_get_rate(clk);
}

static ulong bypass_mux_set_rate(struct clk *clk, ulong rate)
{
	struct cv1800b_clk_bypass_mux *mux = to_cv1800b_clk_bypass_mux(clk);

	if (cv1800b_clk_getbit(mux->mux.base, &mux->bypass))
		return 0;

	return mux_set_rate(clk, rate);
}

static int bypass_mux_set_parent(struct clk *clk, struct clk *pclk)
{
	struct cv1800b_clk_bypass_mux *mux = to_cv1800b_clk_bypass_mux(clk);
	int index;

	if (pclk->id == CV1800B_CLK_BYPASS) {
		cv1800b_clk_setbit(mux->mux.base, &mux->bypass);
		return 0;
	}

	index = get_parent_index(pclk, mux->mux.parent_names,
				 mux->mux.num_parents);
	if (index < 0)
		return -EINVAL;

	cv1800b_clk_clrbit(mux->mux.base, &mux->bypass);
	cv1800b_clk_setfield(mux->mux.base, &mux->mux.mux, index);
	return 0;
}

const struct clk_ops cv1800b_clk_bypass_mux_ops = {
	.disable = mux_disable,
	.enable = mux_enable,
	.get_rate = bypass_mux_get_rate,
	.set_rate = bypass_mux_set_rate,
	.set_parent = bypass_mux_set_parent,
};

U_BOOT_DRIVER(cv1800b_clk_bypass_mux) = {
	.name = "cv1800b_clk_bypass_mux",
	.id = UCLASS_CLK,
	.ops = &cv1800b_clk_bypass_mux_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* MMUX */
#define to_cv1800b_clk_mmux(_clk) \
	container_of(_clk, struct cv1800b_clk_mmux, clk)

static int mmux_enable(struct clk *clk)
{
	struct cv1800b_clk_mmux *mmux = to_cv1800b_clk_mmux(clk);

	return cv1800b_clk_setbit(mmux->base, &mmux->gate);
}

static int mmux_disable(struct clk *clk)
{
	struct cv1800b_clk_mmux *mmux = to_cv1800b_clk_mmux(clk);

	return cv1800b_clk_clrbit(mmux->base, &mmux->gate);
}

static ulong mmux_get_rate(struct clk *clk)
{
	struct cv1800b_clk_mmux *mmux = to_cv1800b_clk_mmux(clk);
	int clk_sel = 1;
	ulong reg, val;

	if (cv1800b_clk_getbit(mmux->base, &mmux->bypass))
		return 0;

	if (cv1800b_clk_getbit(mmux->base, &mmux->clk_sel))
		clk_sel = 0;

	reg = readl(mmux->base + mmux->div[clk_sel].offset);

	if (mmux->div_init[clk_sel] == 0 || reg & CLK_DIV_EN_FACTOR)
		val = cv1800b_clk_getfield(mmux->base, &mmux->div[clk_sel]);
	else
		val = mmux->div_init[clk_sel];

	return DIV_ROUND_UP_ULL(clk_get_parent_rate(clk), val);
}

static ulong mmux_set_rate(struct clk *clk, ulong rate)
{
	struct cv1800b_clk_mmux *mmux = to_cv1800b_clk_mmux(clk);
	int clk_sel = 1;
	ulong parent_rate = clk_get_parent_rate(clk);
	ulong val;

	if (cv1800b_clk_getbit(mmux->base, &mmux->bypass))
		return 0;

	if (cv1800b_clk_getbit(mmux->base, &mmux->clk_sel))
		clk_sel = 0;

	val = DIV_ROUND_UP_ULL(parent_rate, rate);
	val = min_t(u32, val, clk_div_mask(mmux->div[clk_sel].width));

	cv1800b_clk_setfield(mmux->base, &mmux->div[clk_sel], val);
	if (mmux->div_init[clk_sel] > 0)
		setbits_le32(mmux->base + mmux->div[clk_sel].offset,
			     CLK_DIV_EN_FACTOR);

	return DIV_ROUND_UP_ULL(parent_rate, val);
}

static int mmux_set_parent(struct clk *clk, struct clk *pclk)
{
	struct cv1800b_clk_mmux *mmux = to_cv1800b_clk_mmux(clk);
	const char *pname = clk_hw_get_name(pclk);
	int i;
	u8 clk_sel, index;

	if (pclk->id == CV1800B_CLK_BYPASS) {
		cv1800b_clk_setbit(mmux->base, &mmux->bypass);
		return 0;
	}

	for (i = 0; i < mmux->num_parents; i++) {
		if (!strcmp(pname, mmux->parent_infos[i].name))
			break;
	}

	if (i == mmux->num_parents)
		return -EINVAL;

	clk_sel = mmux->parent_infos[i].clk_sel;
	index = mmux->parent_infos[i].index;
	cv1800b_clk_clrbit(mmux->base, &mmux->bypass);
	if (clk_sel)
		cv1800b_clk_clrbit(mmux->base, &mmux->clk_sel);
	else
		cv1800b_clk_setbit(mmux->base, &mmux->clk_sel);

	cv1800b_clk_setfield(mmux->base, &mmux->mux[clk_sel], index);
	return 0;
}

const struct clk_ops cv1800b_clk_mmux_ops = {
	.disable = mmux_disable,
	.enable = mmux_enable,
	.get_rate = mmux_get_rate,
	.set_rate = mmux_set_rate,
	.set_parent = mmux_set_parent,
};

U_BOOT_DRIVER(cv1800b_clk_mmux) = {
	.name = "cv1800b_clk_mmux",
	.id = UCLASS_CLK,
	.ops = &cv1800b_clk_mmux_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* AUDIO CLK */
#define to_cv1800b_clk_audio(_clk) \
	container_of(_clk, struct cv1800b_clk_audio, clk)

static int aclk_enable(struct clk *clk)
{
	struct cv1800b_clk_audio *aclk = to_cv1800b_clk_audio(clk);

	cv1800b_clk_setbit(aclk->base, &aclk->src_en);
	cv1800b_clk_setbit(aclk->base, &aclk->output_en);
	return 0;
}

static int aclk_disable(struct clk *clk)
{
	struct cv1800b_clk_audio *aclk = to_cv1800b_clk_audio(clk);

	cv1800b_clk_clrbit(aclk->base, &aclk->src_en);
	cv1800b_clk_clrbit(aclk->base, &aclk->output_en);
	return 0;
}

static ulong aclk_get_rate(struct clk *clk)
{
	struct cv1800b_clk_audio *aclk = to_cv1800b_clk_audio(clk);
	u64 parent_rate = clk_get_parent_rate(clk);
	u32 m, n;

	if (!cv1800b_clk_getbit(aclk->base, &aclk->div_en))
		return 0;

	m = cv1800b_clk_getfield(aclk->base, &aclk->m);
	n = cv1800b_clk_getfield(aclk->base, &aclk->n);

	return DIV_ROUND_UP_ULL(n * parent_rate, m * 2);
}

static u32 gcd(u32 a, u32 b)
{
	u32 t;

	while (b != 0) {
		t = a % b;
		a = b;
		b = t;
	}
	return a;
}

static void aclk_determine_mn(ulong parent_rate, ulong rate, u32 *m, u32 *n)
{
	u32 tm = parent_rate / 2;
	u32 tn = rate;
	u32 tcommon = gcd(tm, tn);
	*m = tm / tcommon;
	*n = tn / tcommon;
}

static ulong aclk_set_rate(struct clk *clk, ulong rate)
{
	struct cv1800b_clk_audio *aclk = to_cv1800b_clk_audio(clk);
	ulong parent_rate = clk_get_parent_rate(clk);
	u32 m, n;

	aclk_determine_mn(parent_rate, rate, &m, &n);

	cv1800b_clk_setfield(aclk->base, &aclk->m, m);
	cv1800b_clk_setfield(aclk->base, &aclk->n, n);

	cv1800b_clk_setbit(aclk->base, &aclk->div_en);
	cv1800b_clk_setbit(aclk->base, &aclk->div_up);

	return DIV_ROUND_UP_ULL(parent_rate * n, m * 2);
}

const struct clk_ops cv1800b_clk_audio_ops = {
	.disable = aclk_disable,
	.enable = aclk_enable,
	.get_rate = aclk_get_rate,
	.set_rate = aclk_set_rate,
};

U_BOOT_DRIVER(cv1800b_clk_audio) = {
	.name = "cv1800b_clk_audio",
	.id = UCLASS_CLK,
	.ops = &cv1800b_clk_audio_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

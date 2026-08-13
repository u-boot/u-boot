// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023 SberDevices, Inc.
 * Author: Igor Prusov <ivprusov@salutedevices.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <regmap.h>

#include "clk_meson.h"

static const struct meson_clk_info *meson_clk_get_info(struct clk *clk,
						       enum meson_clk_type type)
{
	struct meson_clk_data *data = (void *)dev_get_driver_data(clk->dev);
	const struct meson_clk_info *info;

	if (clk->id >= data->num_clocks)
		return ERR_PTR(-EINVAL);

	info = data->clocks[clk->id];
	if (!info)
		return ERR_PTR(-ENOENT);

	if (type != MESON_CLK_ANY && type != info->type)
		return ERR_PTR(-EINVAL);

	return info;
}

static int meson_set_gate(struct clk *clk, bool on)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;

	debug("%s: %sabling %lu\n", __func__, on ? "en" : "dis", clk->id);

	info = meson_clk_get_info(clk, MESON_CLK_ANY);
	if (IS_ERR(info))
		return PTR_ERR(info);

	SET_PARM_VALUE(priv, info->parm, on);

	return 0;
}

int meson_clk_enable(struct clk *clk)
{
	return meson_set_gate(clk, true);
}

int meson_clk_disable(struct clk *clk)
{
	return meson_set_gate(clk, false);
}

static ulong meson_div_get_rate(struct clk *clk)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	u16 n;
	ulong rate;
	const struct meson_clk_info *info;
	struct clk parent;

	info = meson_clk_get_info(clk, MESON_CLK_DIV);
	if (IS_ERR(info))
		return PTR_ERR(info);

	/* Actual divider value is (field value + 1), hence the increment */
	n = GET_PARM_VALUE(priv, info->parm) + 1;

	parent.dev = clk->dev;
	parent.id = info->parents[0];
	rate = meson_clk_get_rate(&parent);

	return rate / n;
}

int meson_clk_get_parent(struct clk *clk)
{
	uint reg = 0;
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;

	info = meson_clk_get_info(clk, MESON_CLK_ANY);
	if (IS_ERR(info))
		return PTR_ERR(info);

	/* For muxes we read currently selected parent from register,
	 * for other types there is always only one element in parents array.
	 */
	if (info->type == MESON_CLK_MUX) {
		reg = GET_PARM_VALUE(priv, info->parm);
		if (IS_ERR_VALUE(reg))
			return reg;
	}

	return info->parents[reg];
}

static ulong meson_pll_get_rate(struct clk *clk)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;
	const struct parm *pm, *pn;
	ulong parent_rate_mhz;
	struct clk parent;
	u16 n, m;

	info = meson_clk_get_info(clk, MESON_CLK_ANY);
	if (IS_ERR(info))
		return PTR_ERR(info);

	pm = &info->parm[0];
	pn = &info->parm[1];

	n = GET_PARM_VALUE(priv, pn);
	m = GET_PARM_VALUE(priv, pm);

	if (n == 0)
		return -EINVAL;

	parent.dev = clk->dev;
	parent.id = info->parents[0];
	parent_rate_mhz = meson_clk_get_rate(&parent) / 1000000;

	return parent_rate_mhz * m / n * 1000000;
}

ulong meson_clk_get_rate(struct clk *clk)
{
	struct clk parent;
	const struct meson_clk_info *info;

	if (IS_ERR_VALUE(clk->id))
		return clk->id;

	info = meson_clk_get_info(clk, MESON_CLK_ANY);
	if (IS_ERR(info))
		return PTR_ERR(info);

	switch (info->type) {
	case MESON_CLK_PLL:
		return meson_pll_get_rate(clk);
	case MESON_CLK_GATE:
	case MESON_CLK_MUX:
		parent.dev = clk->dev;
		parent.id = meson_clk_get_parent(clk);
		return meson_clk_get_rate(&parent);
	case MESON_CLK_DIV:
		return meson_div_get_rate(clk);
	case MESON_CLK_FIXED_DIV:
		parent.dev = clk->dev;
		parent.id = meson_clk_get_parent(clk);
		return mult_frac(meson_clk_get_rate(&parent), info->mult,
				 info->div);
	case MESON_CLK_EXTERNAL: {
		int ret;
		struct clk external_clk;

		ret = clk_get_by_name(clk->dev, info->name, &external_clk);
		if (ret)
			return ret;

		return clk_get_rate(&external_clk);
	}
	default:
		return -EINVAL;
	}
}

/* This implements rate propagation for dividers placed after multiplexer:
 *  ---------|\
 *     ..... | |---DIV--
 *  ---------|/
 */
ulong meson_composite_set_rate(struct clk *clk, ulong rate)
{
	unsigned int i, best_div_val;
	unsigned long best_delta, best_parent;
	const struct meson_clk_info *div;
	const struct meson_clk_info *mux;
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct clk mux_clk;

	div = meson_clk_get_info(clk, MESON_CLK_DIV);
	if (IS_ERR(div))
		return PTR_ERR(div);

	mux_clk.dev = clk->dev;
	mux_clk.id = div->parents[0];
	mux = meson_clk_get_info(&mux_clk, MESON_CLK_MUX);
	if (IS_ERR(mux))
		return PTR_ERR(mux);

	best_parent = -EINVAL;
	best_delta = ULONG_MAX;
	for (i = 0; i < (1 << mux->parm->width); i++) {
		unsigned long parent_rate, delta;
		unsigned int div_val;
		struct clk parent = {
			.dev = clk->dev,
			.id =  mux->parents[i],
		};

		parent_rate = meson_clk_get_rate(&parent);
		if (IS_ERR_VALUE(parent_rate))
			continue;

		/* If overflow, try to use max divider value */
		div_val = min(DIV_ROUND_CLOSEST(parent_rate, rate),
			      (1UL << div->parm->width));

		delta = abs(rate - (parent_rate / div_val));
		if (delta < best_delta) {
			best_delta = delta;
			best_div_val = div_val;
			best_parent = i;
		}
	}

	if (IS_ERR_VALUE(best_parent))
		return best_parent;

	SET_PARM_VALUE(priv, mux->parm, best_parent);
	/* Divider is set to (field value + 1), hence the decrement */
	SET_PARM_VALUE(priv, div->parm, best_div_val - 1);

	return 0;
}

ulong meson_mux_set_rate(struct clk *clk, ulong rate)
{
	int i;
	ulong ret = -EINVAL;
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;

	info = meson_clk_get_info(clk, MESON_CLK_MUX);
	if (IS_ERR(info))
		return PTR_ERR(info);

	for (i = 0; i < (1 << info->parm->width); i++) {
		struct clk parent = {
			.dev = clk->dev,
			.id = info->parents[i],
		};

		ret = clk_set_rate(&parent, rate);
		if (!ret) {
			SET_PARM_VALUE(priv, info->parm, i);
			break;
		}
	}

	return ret;
}

int meson_clk_set_parent(struct clk *clk, struct clk *parent)
{
	unsigned int i, parent_index;
	struct meson_clk *priv = dev_get_priv(clk->dev);
	const struct meson_clk_info *info;

	info = meson_clk_get_info(clk, MESON_CLK_MUX);
	if (IS_ERR(info))
		return PTR_ERR(info);

	parent_index = -EINVAL;
	for (i = 0; i < (1 << info->parm->width); i++) {
		if (parent->id == info->parents[i]) {
			parent_index = i;
			break;
		}
	}

	if (IS_ERR_VALUE(parent_index))
		return parent_index;

	SET_PARM_VALUE(priv, info->parm, parent_index);

	return 0;
}

#if IS_ENABLED(CONFIG_CMD_CLK)
static const char *meson_clk_get_name(struct clk *clk)
{
	const struct meson_clk_info *info;

	info = meson_clk_get_info(clk, MESON_CLK_ANY);

	return IS_ERR(info) ? "unknown" : info->name;
}

static int meson_clk_dump_single(struct clk *clk)
{
	const struct meson_clk_info *info;
	struct meson_clk *priv;
	unsigned long rate;
	char *state, frequency[80];
	struct clk parent;

	priv = dev_get_priv(clk->dev);

	info = meson_clk_get_info(clk, MESON_CLK_ANY);
	if (IS_ERR(info) || !info->name)
		return -EINVAL;

	rate = clk_get_rate(clk);
	if (IS_ERR_VALUE(rate))
		sprintf(frequency, "unknown");
	else
		sprintf(frequency, "%lu", rate);

	if (info->type == MESON_CLK_GATE)
		state = GET_PARM_VALUE(priv, info->parm) ? "enabled" : "disabled";
	else
		state = "N/A";

	parent.dev = clk->dev;
	parent.id = meson_clk_get_parent(clk);
	printf("%15s%20s%20s%15s\n",
	       info->name,
	       frequency,
	       meson_clk_get_name(&parent),
	       state);

	return 0;
}

void meson_clk_dump(struct udevice *dev)
{
	int i;
	struct meson_clk_data *data;
	const char *sep = "--------------------";

	printf("%s:\n", dev->name);
	printf("%.15s%s%s%.15s\n", sep, sep, sep, sep);
	printf("%15s%20s%20s%15s\n", "clk", "frequency", "parent", "state");
	printf("%.15s%s%s%.15s\n", sep, sep, sep, sep);

	data = (struct meson_clk_data *)dev_get_driver_data(dev);
	for (i = 0; i < data->num_clocks; i++) {
		meson_clk_dump_single(&(struct clk){
			.dev = dev,
			.id = i
		});
	}
}
#endif

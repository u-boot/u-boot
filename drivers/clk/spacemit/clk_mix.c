// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2024 SpacemiT Technology Co. Ltd
 * Copyright (c) 2024-2025 Haylen Chu <heylenay@4d2.org>
 * Copyright (c) 2025 Junhui Liu <junhui.liu@pigmoral.tech>
 *  Authors: Haylen Chu <heylenay@4d2.org>
 *
 * MIX clock type is the combination of mux, factor or divider, and gate
 */

#include <dm/device.h>
#include <dm/uclass.h>
#include <div64.h>
#include <regmap.h>
#include <linux/clk-provider.h>
#include <linux/kernel.h>

#include "clk_mix.h"

#define UBOOT_DM_SPACEMIT_CLK_GATE		"spacemit_clk_gate"
#define UBOOT_DM_SPACEMIT_CLK_FACTOR		"spacemit_clk_factor"
#define UBOOT_DM_SPACEMIT_CLK_MUX		"spacemit_clk_mux"
#define UBOOT_DM_SPACEMIT_CLK_DIV		"spacemit_clk_div"
#define UBOOT_DM_SPACEMIT_CLK_FACTOR_GATE	"spacemit_clk_factor_gate"
#define UBOOT_DM_SPACEMIT_CLK_MUX_GATE		"spacemit_clk_mux_gate"
#define UBOOT_DM_SPACEMIT_CLK_DIV_GATE		"spacemit_clk_div_gate"
#define UBOOT_DM_SPACEMIT_CLK_MUX_DIV		"spacemit_clk_mux_div"
#define UBOOT_DM_SPACEMIT_CLK_MUX_DIV_GATE	"spacemit_clk_mux_div_gate"

#define MIX_FC_TIMEOUT_US	10000
#define MIX_FC_DELAY_US		5

int ccu_gate_disable(struct clk *clk)
{
	struct ccu_mix *mix = clk_to_ccu_mix(clk);

	ccu_update(&mix->common, ctrl, mix->gate.mask, 0);

	return 0;
}

int ccu_gate_enable(struct clk *clk)
{
	struct ccu_mix *mix = clk_to_ccu_mix(clk);
	struct ccu_gate_config *gate = &mix->gate;

	ccu_update(&mix->common, ctrl, gate->mask, gate->mask);

	return 0;
}

static unsigned long ccu_factor_recalc_rate(struct clk *clk)
{
	struct ccu_mix *mix = clk_to_ccu_mix(clk);

	return clk_get_parent_rate(clk) * mix->factor.mul / mix->factor.div;
}

static unsigned long ccu_div_recalc_rate(struct clk *clk)
{
	struct ccu_mix *mix = clk_to_ccu_mix(clk);
	struct ccu_div_config *div = &mix->div;
	unsigned long val;

	val = ccu_read(&mix->common, ctrl) >> div->shift;
	val &= (1 << div->width) - 1;

	return divider_recalc_rate(clk, clk_get_parent_rate(clk), val, NULL, 0, div->width);
}

/*
 * Some clocks require a "FC" (frequency change) bit to be set after changing
 * their rates or reparenting. This bit will be automatically cleared by
 * hardware in MIX_FC_TIMEOUT_US, which indicates the operation is completed.
 */
static int ccu_mix_trigger_fc(struct clk *clk)
{
	struct ccu_common *common = clk_to_ccu_common(clk);
	unsigned int val;

	if (common->reg_fc)
		return 0;

	ccu_update(common, fc, common->mask_fc, common->mask_fc);

	return regmap_read_poll_timeout(common->regmap, common->reg_fc,
					val, !(val & common->mask_fc),
					MIX_FC_DELAY_US,
					MIX_FC_TIMEOUT_US);
}

static unsigned long
ccu_mix_calc_best_rate(struct clk *clk, unsigned long rate,
		       struct clk **best_parent,
		       unsigned long *best_parent_rate,
		       u32 *div_val)
{
	struct ccu_common *common = clk_to_ccu_common(clk);
	struct ccu_mix *mix = clk_to_ccu_mix(clk);
	unsigned int parent_num = common->num_parents;
	struct ccu_div_config *div = &mix->div;
	u32 div_max = 1 << div->width;
	unsigned long best_rate = 0;

	for (int i = 0; i < parent_num; i++) {
		struct udevice *parent_dev;
		unsigned long parent_rate;
		struct clk *parent;

		if (uclass_get_device_by_name(UCLASS_CLK, common->parents[i],
					      &parent_dev))
			continue;
		parent = dev_get_clk_ptr(parent_dev);
		if (!parent)
			continue;

		parent_rate = clk_get_rate(parent);

		for (int j = 1; j <= div_max; j++) {
			unsigned long tmp = DIV_ROUND_CLOSEST_ULL(parent_rate, j);

			if (abs(tmp - rate) < abs(best_rate - rate)) {
				best_rate = tmp;

				if (div_val)
					*div_val = j - 1;

				if (best_parent) {
					*best_parent      = parent;
					*best_parent_rate = parent_rate;
				}
			}
		}
	}

	return best_rate;
}

static unsigned long ccu_mix_set_rate(struct clk *clk, unsigned long rate)
{
	struct ccu_mix *mix = clk_to_ccu_mix(clk);
	struct ccu_common *common = &mix->common;
	struct ccu_div_config *div = &mix->div;
	u32 current_div, target_div, mask;

	ccu_mix_calc_best_rate(clk, rate, NULL, NULL, &target_div);

	current_div = ccu_read(common, ctrl) >> div->shift;
	current_div &= (1 << div->width) - 1;

	if (current_div == target_div)
		return 0;

	mask = GENMASK(div->width + div->shift - 1, div->shift);

	ccu_update(common, ctrl, mask, target_div << div->shift);

	return ccu_mix_trigger_fc(clk);
}

static u8 ccu_mux_get_parent(struct clk *clk)
{
	struct ccu_mix *mix = clk_to_ccu_mix(clk);
	struct ccu_mux_config *mux = &mix->mux;
	u8 parent;

	parent = ccu_read(&mix->common, ctrl) >> mux->shift;
	parent &= (1 << mux->width) - 1;

	return parent;
}

static int ccu_mux_set_parent(struct clk *clk, struct clk *parent)
{
	struct ccu_common *common = clk_to_ccu_common(clk);
	struct ccu_mix *mix = clk_to_ccu_mix(clk);
	struct ccu_mux_config *mux = &mix->mux;
	u32 mask;
	int i = 0;

	mask = GENMASK(mux->width + mux->shift - 1, mux->shift);

	for (i = 0; i < common->num_parents; i++) {
		if (!strcmp(parent->dev->name, common->parents[i]))
			break;
	}

	if (i == common->num_parents)
		return -EINVAL;

	ccu_update(&mix->common, ctrl, mask, i << mux->shift);

	return ccu_mix_trigger_fc(clk);
}

int spacemit_gate_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_GATE,
			    common->name, common->parents[0]);
}

static const struct clk_ops spacemit_clk_gate_ops = {
	.disable	= ccu_gate_disable,
	.enable		= ccu_gate_enable,
	.get_rate	= clk_generic_get_rate,
};

U_BOOT_DRIVER(spacemit_clk_gate) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_GATE,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_gate_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

int spacemit_factor_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_FACTOR,
			    common->name, common->parents[0]);
}

static const struct clk_ops spacemit_clk_factor_ops = {
	.get_rate	= ccu_factor_recalc_rate,
};

U_BOOT_DRIVER(spacemit_clk_factor) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_FACTOR,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_factor_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

int spacemit_mux_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;
	u8 index;

	index = ccu_mux_get_parent(clk);
	if (index >= common->num_parents)
		index = 0;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_MUX,
			    common->name, common->parents[index]);
}

static const struct clk_ops spacemit_clk_mux_ops = {
	.set_parent	= ccu_mux_set_parent,
	.get_rate	= clk_generic_get_rate,
};

U_BOOT_DRIVER(spacemit_clk_mux) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_MUX,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_mux_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

int spacemit_div_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_DIV,
			    common->name, common->parents[0]);
}

static const struct clk_ops spacemit_clk_div_ops = {
	.get_rate	= ccu_div_recalc_rate,
	.set_rate	= ccu_mix_set_rate,
};

U_BOOT_DRIVER(spacemit_clk_div) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_DIV,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_div_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

int spacemit_factor_gate_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_FACTOR_GATE,
			    common->name, common->parents[0]);
}

static const struct clk_ops spacemit_clk_factor_gate_ops = {
	.disable	= ccu_gate_disable,
	.enable		= ccu_gate_enable,
	.get_rate	= ccu_factor_recalc_rate,
};

U_BOOT_DRIVER(spacemit_clk_factor_gate) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_FACTOR_GATE,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_factor_gate_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

int spacemit_mux_gate_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;
	u8 index;

	index = ccu_mux_get_parent(clk);
	if (index >= common->num_parents)
		index = 0;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_MUX_GATE,
			    common->name, common->parents[index]);
}

static const struct clk_ops spacemit_clk_mux_gate_ops = {
	.disable	= ccu_gate_disable,
	.enable		= ccu_gate_enable,
	.set_parent	= ccu_mux_set_parent,
	.get_rate	= clk_generic_get_rate,
};

U_BOOT_DRIVER(spacemit_clk_mux_gate) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_MUX_GATE,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_mux_gate_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

int spacemit_div_gate_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_DIV_GATE,
			    common->name, common->parents[0]);
}

static const struct clk_ops spacemit_clk_div_gate_ops = {
	.disable	= ccu_gate_disable,
	.enable		= ccu_gate_enable,
	.get_rate	= ccu_div_recalc_rate,
	.set_rate	= ccu_mix_set_rate,
};

U_BOOT_DRIVER(spacemit_clk_div_gate) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_DIV_GATE,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_div_gate_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

int spacemit_mux_div_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;
	u8 index;

	index = ccu_mux_get_parent(clk);
	if (index >= common->num_parents)
		index = 0;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_MUX_DIV,
			    common->name, common->parents[index]);
}

static const struct clk_ops spacemit_clk_mux_div_ops = {
	.set_parent	= ccu_mux_set_parent,
	.get_rate	= ccu_div_recalc_rate,
	.set_rate	= ccu_mix_set_rate,
};

U_BOOT_DRIVER(spacemit_clk_mux_div) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_MUX_DIV,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_mux_div_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

int spacemit_mux_div_gate_init(struct ccu_common *common)
{
	struct clk *clk = &common->clk;
	u8 index;

	index = ccu_mux_get_parent(clk);
	if (index >= common->num_parents)
		index = 0;

	return clk_register(clk, UBOOT_DM_SPACEMIT_CLK_MUX_DIV_GATE,
			    common->name, common->parents[index]);
}

static const struct clk_ops spacemit_clk_mux_div_gate_ops = {
	.disable	= ccu_gate_disable,
	.enable		= ccu_gate_enable,
	.set_parent	= ccu_mux_set_parent,
	.get_rate	= ccu_div_recalc_rate,
	.set_rate	= ccu_mix_set_rate,
};

U_BOOT_DRIVER(spacemit_clk_mux_div_gate) = {
	.name	= UBOOT_DM_SPACEMIT_CLK_MUX_DIV_GATE,
	.id	= UCLASS_CLK,
	.ops	= &spacemit_clk_mux_div_gate_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

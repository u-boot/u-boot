/*
 * Copyright (c) 2014-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#define pr_fmt(fmt) "tegra-xusb-padctl: " fmt

#include <common.h>
#include <errno.h>

#include "xusb-padctl-common.h"

#include <asm/arch/clock.h>

int tegra_xusb_phy_prepare(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->prepare)
		return phy->ops->prepare(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_enable(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->enable)
		return phy->ops->enable(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_disable(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->disable)
		return phy->ops->disable(phy);

	return phy ? -ENOSYS : -EINVAL;
}

int tegra_xusb_phy_unprepare(struct tegra_xusb_phy *phy)
{
	if (phy && phy->ops && phy->ops->unprepare)
		return phy->ops->unprepare(phy);

	return phy ? -ENOSYS : -EINVAL;
}

struct tegra_xusb_phy *tegra_xusb_phy_get(unsigned int type)
{
	struct tegra_xusb_phy *phy;
	int i;

	for (i = 0; i < padctl.socdata->num_phys; i++) {
		phy = &padctl.socdata->phys[i];
		if (phy->type != type)
			continue;
		return phy;
	}

	return NULL;
}

static const struct tegra_xusb_padctl_lane *
tegra_xusb_padctl_find_lane(struct tegra_xusb_padctl *padctl, const char *name)
{
	unsigned int i;

	for (i = 0; i < padctl->socdata->num_lanes; i++)
		if (strcmp(name, padctl->socdata->lanes[i].name) == 0)
			return &padctl->socdata->lanes[i];

	return NULL;
}

static int
tegra_xusb_padctl_group_parse_dt(struct tegra_xusb_padctl *padctl,
				 struct tegra_xusb_padctl_group *group,
				 const void *fdt, int node)
{
	unsigned int i;
	int len, err;

	group->name = fdt_get_name(fdt, node, &len);

	len = fdt_count_strings(fdt, node, "nvidia,lanes");
	if (len < 0) {
		error("failed to parse \"nvidia,lanes\" property");
		return -EINVAL;
	}

	group->num_pins = len;

	for (i = 0; i < group->num_pins; i++) {
		err = fdt_get_string_index(fdt, node, "nvidia,lanes", i,
					   &group->pins[i]);
		if (err < 0) {
			error("failed to read string from \"nvidia,lanes\" property");
			return -EINVAL;
		}
	}

	group->num_pins = len;

	err = fdt_get_string(fdt, node, "nvidia,function", &group->func);
	if (err < 0) {
		error("failed to parse \"nvidia,func\" property");
		return -EINVAL;
	}

	group->iddq = fdtdec_get_int(fdt, node, "nvidia,iddq", -1);

	return 0;
}

static int tegra_xusb_padctl_find_function(struct tegra_xusb_padctl *padctl,
					   const char *name)
{
	unsigned int i;

	for (i = 0; i < padctl->socdata->num_functions; i++)
		if (strcmp(name, padctl->socdata->functions[i]) == 0)
			return i;

	return -ENOENT;
}

static int
tegra_xusb_padctl_lane_find_function(struct tegra_xusb_padctl *padctl,
				     const struct tegra_xusb_padctl_lane *lane,
				     const char *name)
{
	unsigned int i;
	int func;

	func = tegra_xusb_padctl_find_function(padctl, name);
	if (func < 0)
		return func;

	for (i = 0; i < lane->num_funcs; i++)
		if (lane->funcs[i] == func)
			return i;

	return -ENOENT;
}

static int
tegra_xusb_padctl_group_apply(struct tegra_xusb_padctl *padctl,
			      const struct tegra_xusb_padctl_group *group)
{
	unsigned int i;

	for (i = 0; i < group->num_pins; i++) {
		const struct tegra_xusb_padctl_lane *lane;
		unsigned int func;
		u32 value;

		lane = tegra_xusb_padctl_find_lane(padctl, group->pins[i]);
		if (!lane) {
			error("no lane for pin %s", group->pins[i]);
			continue;
		}

		func = tegra_xusb_padctl_lane_find_function(padctl, lane,
							    group->func);
		if (func < 0) {
			error("function %s invalid for lane %s: %d",
			      group->func, lane->name, func);
			continue;
		}

		value = padctl_readl(padctl, lane->offset);

		/* set pin function */
		value &= ~(lane->mask << lane->shift);
		value |= func << lane->shift;

		/*
		 * Set IDDQ if supported on the lane and specified in the
		 * configuration.
		 */
		if (lane->iddq > 0 && group->iddq >= 0) {
			if (group->iddq != 0)
				value &= ~(1 << lane->iddq);
			else
				value |= 1 << lane->iddq;
		}

		padctl_writel(padctl, value, lane->offset);
	}

	return 0;
}

static int
tegra_xusb_padctl_config_apply(struct tegra_xusb_padctl *padctl,
			       struct tegra_xusb_padctl_config *config)
{
	unsigned int i;

	for (i = 0; i < config->num_groups; i++) {
		const struct tegra_xusb_padctl_group *group;
		int err;

		group = &config->groups[i];

		err = tegra_xusb_padctl_group_apply(padctl, group);
		if (err < 0) {
			error("failed to apply group %s: %d",
			      group->name, err);
			continue;
		}
	}

	return 0;
}

static int
tegra_xusb_padctl_config_parse_dt(struct tegra_xusb_padctl *padctl,
				  struct tegra_xusb_padctl_config *config,
				  const void *fdt, int node)
{
	int subnode;

	config->name = fdt_get_name(fdt, node, NULL);

	fdt_for_each_subnode(fdt, subnode, node) {
		struct tegra_xusb_padctl_group *group;
		int err;

		group = &config->groups[config->num_groups];

		err = tegra_xusb_padctl_group_parse_dt(padctl, group, fdt,
						       subnode);
		if (err < 0) {
			error("failed to parse group %s", group->name);
			return err;
		}

		config->num_groups++;
	}

	return 0;
}

static int tegra_xusb_padctl_parse_dt(struct tegra_xusb_padctl *padctl,
				      const void *fdt, int node)
{
	int subnode, err;

	err = fdt_get_resource(fdt, node, "reg", 0, &padctl->regs);
	if (err < 0) {
		error("registers not found");
		return err;
	}

	fdt_for_each_subnode(fdt, subnode, node) {
		struct tegra_xusb_padctl_config *config = &padctl->config;

		err = tegra_xusb_padctl_config_parse_dt(padctl, config, fdt,
							subnode);
		if (err < 0) {
			error("failed to parse entry %s: %d",
			      config->name, err);
			continue;
		}
	}

	return 0;
}

struct tegra_xusb_padctl padctl;

int tegra_xusb_process_nodes(const void *fdt, int nodes[], unsigned int count,
	const struct tegra_xusb_padctl_soc *socdata)
{
	unsigned int i;
	int err;

	for (i = 0; i < count; i++) {
		if (!fdtdec_get_is_enabled(fdt, nodes[i]))
			continue;

		padctl.socdata = socdata;

		err = tegra_xusb_padctl_parse_dt(&padctl, fdt, nodes[i]);
		if (err < 0) {
			error("failed to parse DT: %d", err);
			continue;
		}

		/* deassert XUSB padctl reset */
		reset_set_enable(PERIPH_ID_XUSB_PADCTL, 0);

		err = tegra_xusb_padctl_config_apply(&padctl, &padctl.config);
		if (err < 0) {
			error("failed to apply pinmux: %d", err);
			continue;
		}

		/* only a single instance is supported */
		break;
	}

	return 0;
}

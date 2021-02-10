// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <generic-phy.h>

static inline struct phy_ops *phy_dev_ops(struct udevice *dev)
{
	return (struct phy_ops *)dev->driver->ops;
}

static int generic_phy_xlate_offs_flags(struct phy *phy,
					struct ofnode_phandle_args *args)
{
	debug("%s(phy=%p)\n", __func__, phy);

	if (args->args_count > 1) {
		debug("Invaild args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	if (args->args_count)
		phy->id = args->args[0];
	else
		phy->id = 0;

	return 0;
}

int generic_phy_get_by_index_nodev(ofnode node, int index, struct phy *phy)
{
	struct ofnode_phandle_args args;
	struct phy_ops *ops;
	struct udevice *phydev;
	int i, ret;

	debug("%s(node=%s, index=%d, phy=%p)\n",
	      __func__, ofnode_get_name(node), index, phy);

	assert(phy);
	phy->dev = NULL;
	ret = ofnode_parse_phandle_with_args(node, "phys", "#phy-cells", 0,
					     index, &args);
	if (ret) {
		debug("%s: dev_read_phandle_with_args failed: err=%d\n",
		      __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_PHY, args.node, &phydev);
	if (ret) {
		debug("%s: uclass_get_device_by_ofnode failed: err=%d\n",
		      __func__, ret);

		/* Check if args.node's parent is a PHY provider */
		ret = uclass_get_device_by_ofnode(UCLASS_PHY,
						  ofnode_get_parent(args.node),
						  &phydev);
		if (ret)
			return ret;

		/* insert phy idx at first position into args array */
		for (i = args.args_count; i >= 1 ; i--)
			args.args[i] = args.args[i - 1];

		args.args_count++;
		args.args[0] = ofnode_read_u32_default(args.node, "reg", -1);
	}

	phy->dev = phydev;

	ops = phy_dev_ops(phydev);

	if (ops->of_xlate)
		ret = ops->of_xlate(phy, &args);
	else
		ret = generic_phy_xlate_offs_flags(phy, &args);
	if (ret) {
		debug("of_xlate() failed: %d\n", ret);
		goto err;
	}

	return 0;

err:
	return ret;
}

int generic_phy_get_by_index(struct udevice *dev, int index,
			     struct phy *phy)
{
	return generic_phy_get_by_index_nodev(dev_ofnode(dev), index, phy);
}

int generic_phy_get_by_name(struct udevice *dev, const char *phy_name,
			    struct phy *phy)
{
	int index;

	debug("%s(dev=%p, name=%s, phy=%p)\n", __func__, dev, phy_name, phy);

	index = dev_read_stringlist_search(dev, "phy-names", phy_name);
	if (index < 0) {
		debug("dev_read_stringlist_search() failed: %d\n", index);
		return index;
	}

	return generic_phy_get_by_index(dev, index, phy);
}

int generic_phy_init(struct phy *phy)
{
	struct phy_ops const *ops;
	int ret;

	if (!generic_phy_valid(phy))
		return 0;
	ops = phy_dev_ops(phy->dev);
	if (!ops->init)
		return 0;
	ret = ops->init(phy);
	if (ret)
		dev_err(phy->dev, "PHY: Failed to init %s: %d.\n",
			phy->dev->name, ret);

	return ret;
}

int generic_phy_reset(struct phy *phy)
{
	struct phy_ops const *ops;
	int ret;

	if (!generic_phy_valid(phy))
		return 0;
	ops = phy_dev_ops(phy->dev);
	if (!ops->reset)
		return 0;
	ret = ops->reset(phy);
	if (ret)
		dev_err(phy->dev, "PHY: Failed to reset %s: %d.\n",
			phy->dev->name, ret);

	return ret;
}

int generic_phy_exit(struct phy *phy)
{
	struct phy_ops const *ops;
	int ret;

	if (!generic_phy_valid(phy))
		return 0;
	ops = phy_dev_ops(phy->dev);
	if (!ops->exit)
		return 0;
	ret = ops->exit(phy);
	if (ret)
		dev_err(phy->dev, "PHY: Failed to exit %s: %d.\n",
			phy->dev->name, ret);

	return ret;
}

int generic_phy_power_on(struct phy *phy)
{
	struct phy_ops const *ops;
	int ret;

	if (!generic_phy_valid(phy))
		return 0;
	ops = phy_dev_ops(phy->dev);
	if (!ops->power_on)
		return 0;
	ret = ops->power_on(phy);
	if (ret)
		dev_err(phy->dev, "PHY: Failed to power on %s: %d.\n",
			phy->dev->name, ret);

	return ret;
}

int generic_phy_power_off(struct phy *phy)
{
	struct phy_ops const *ops;
	int ret;

	if (!generic_phy_valid(phy))
		return 0;
	ops = phy_dev_ops(phy->dev);
	if (!ops->power_off)
		return 0;
	ret = ops->power_off(phy);
	if (ret)
		dev_err(phy->dev, "PHY: Failed to power off %s: %d.\n",
			phy->dev->name, ret);

	return ret;
}

int generic_phy_configure(struct phy *phy, void *params)
{
	struct phy_ops const *ops;

	if (!generic_phy_valid(phy))
		return 0;
	ops = phy_dev_ops(phy->dev);

	return ops->configure ? ops->configure(phy, params) : 0;
}

int generic_phy_get_bulk(struct udevice *dev, struct phy_bulk *bulk)
{
	int i, ret, count;

	bulk->count = 0;

	/* Return if no phy declared */
	if (!dev_read_prop(dev, "phys", NULL))
		return 0;

	count = dev_count_phandle_with_args(dev, "phys", "#phy-cells", 0);
	if (count < 1)
		return count;

	bulk->phys = devm_kcalloc(dev, count, sizeof(struct phy), GFP_KERNEL);
	if (!bulk->phys)
		return -ENOMEM;

	for (i = 0; i < count; i++) {
		ret = generic_phy_get_by_index(dev, i, &bulk->phys[i]);
		if (ret) {
			pr_err("Failed to get PHY%d for %s\n", i, dev->name);
			return ret;
		}
		bulk->count++;
	}

	return 0;
}

int generic_phy_init_bulk(struct phy_bulk *bulk)
{
	struct phy *phys = bulk->phys;
	int i, ret;

	for (i = 0; i < bulk->count; i++) {
		ret = generic_phy_init(&phys[i]);
		if (ret) {
			pr_err("Can't init PHY%d\n", i);
			goto phys_init_err;
		}
	}

	return 0;

phys_init_err:
	for (; i > 0; i--)
		generic_phy_exit(&phys[i - 1]);

	return ret;
}

int generic_phy_exit_bulk(struct phy_bulk *bulk)
{
	struct phy *phys = bulk->phys;
	int i, ret = 0;

	for (i = 0; i < bulk->count; i++)
		ret |= generic_phy_exit(&phys[i]);

	return ret;
}

int generic_phy_power_on_bulk(struct phy_bulk *bulk)
{
	struct phy *phys = bulk->phys;
	int i, ret;

	for (i = 0; i < bulk->count; i++) {
		ret = generic_phy_power_on(&phys[i]);
		if (ret) {
			pr_err("Can't power on PHY%d\n", i);
			goto phys_poweron_err;
		}
	}

	return 0;

phys_poweron_err:
	for (; i > 0; i--)
		generic_phy_power_off(&phys[i - 1]);

	return ret;
}

int generic_phy_power_off_bulk(struct phy_bulk *bulk)
{
	struct phy *phys = bulk->phys;
	int i, ret = 0;

	for (i = 0; i < bulk->count; i++)
		ret |= generic_phy_power_off(&phys[i]);

	return ret;
}

UCLASS_DRIVER(phy) = {
	.id		= UCLASS_PHY,
	.name		= "phy",
};

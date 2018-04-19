// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <syscon.h>
#include <dm.h>
#include <errno.h>
#include <regmap.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>
#include <linux/err.h>

/*
 * Caution:
 * This API requires the given device has alerady been bound to syscon driver.
 * For example,
 *    compatible = "syscon", "simple-mfd";
 * works, but
 *    compatible = "simple-mfd", "syscon";
 * does not.  The behavior is different from Linux.
 */
struct regmap *syscon_get_regmap(struct udevice *dev)
{
	struct syscon_uc_info *priv;

	if (device_get_uclass_id(dev) != UCLASS_SYSCON)
		return ERR_PTR(-ENOEXEC);
	priv = dev_get_uclass_priv(dev);
	return priv->regmap;
}

static int syscon_pre_probe(struct udevice *dev)
{
	struct syscon_uc_info *priv = dev_get_uclass_priv(dev);

	/*
	 * With OF_PLATDATA we really have no way of knowing the format of
	 * the device-specific platform data. So we assume that it starts with
	 * a 'reg' member, and this holds a single address and size. Drivers
	 * using OF_PLATDATA will need to ensure that this is true.
	 */
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct syscon_base_platdata *plat = dev_get_platdata(dev);

	return regmap_init_mem_platdata(dev, plat->reg, ARRAY_SIZE(plat->reg),
					&priv->regmap);
#else
	return regmap_init_mem(dev_ofnode(dev), &priv->regmap);
#endif
}

int syscon_get_by_driver_data(ulong driver_data, struct udevice **devp)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	*devp = NULL;
	ret = uclass_get(UCLASS_SYSCON, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		if (dev->driver_data == driver_data) {
			*devp = dev;
			return device_probe(dev);
		}
	}

	return -ENODEV;
}

struct regmap *syscon_get_regmap_by_driver_data(ulong driver_data)
{
	struct syscon_uc_info *priv;
	struct udevice *dev;
	int ret;

	ret = syscon_get_by_driver_data(driver_data, &dev);
	if (ret)
		return ERR_PTR(ret);
	priv = dev_get_uclass_priv(dev);

	return priv->regmap;
}

void *syscon_get_first_range(ulong driver_data)
{
	struct regmap *map;

	map = syscon_get_regmap_by_driver_data(driver_data);
	if (IS_ERR(map))
		return map;
	return regmap_get_range(map, 0);
}

UCLASS_DRIVER(syscon) = {
	.id		= UCLASS_SYSCON,
	.name		= "syscon",
	.per_device_auto_alloc_size = sizeof(struct syscon_uc_info),
	.pre_probe = syscon_pre_probe,
};

static const struct udevice_id generic_syscon_ids[] = {
	{ .compatible = "syscon" },
	{ }
};

U_BOOT_DRIVER(generic_syscon) = {
	.name	= "syscon",
	.id	= UCLASS_SYSCON,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind           = dm_scan_fdt_dev,
#endif
	.of_match = generic_syscon_ids,
};

/*
 * Linux-compatible syscon-to-regmap
 * The syscon node can be bound to another driver, but still works
 * as a syscon provider.
 */
static LIST_HEAD(syscon_list);

struct syscon {
	ofnode node;
	struct regmap *regmap;
	struct list_head list;
};

static struct syscon *of_syscon_register(ofnode node)
{
	struct syscon *syscon;
	int ret;

	if (!ofnode_device_is_compatible(node, "syscon"))
		return ERR_PTR(-EINVAL);

	syscon = malloc(sizeof(*syscon));
	if (!syscon)
		return ERR_PTR(-ENOMEM);

	ret = regmap_init_mem(node, &syscon->regmap);
	if (ret) {
		free(syscon);
		return ERR_PTR(ret);
	}

	list_add_tail(&syscon->list, &syscon_list);

	return syscon;
}

struct regmap *syscon_node_to_regmap(ofnode node)
{
	struct syscon *entry, *syscon = NULL;

	list_for_each_entry(entry, &syscon_list, list)
		if (ofnode_equal(entry->node, node)) {
			syscon = entry;
			break;
		}

	if (!syscon)
		syscon = of_syscon_register(node);

	if (IS_ERR(syscon))
		return ERR_CAST(syscon);

	return syscon->regmap;
}

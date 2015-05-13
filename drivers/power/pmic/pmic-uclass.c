/*
 * Copyright (C) 2014-2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <errno.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <power/pmic.h>
#include <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong str_get_num(const char *ptr, const char *maxptr)
{
	if (!ptr || !maxptr)
		return 0;

	while (!isdigit(*ptr) && ptr++ < maxptr);

	return simple_strtoul(ptr, NULL, 0);
}

int pmic_bind_children(struct udevice *pmic, int offset,
		       const struct pmic_child_info *child_info)
{
	const struct pmic_child_info *info;
	const void *blob = gd->fdt_blob;
	struct driver *drv;
	struct udevice *child;
	const char *node_name;
	int node_name_len;
	int bind_count = 0;
	int node;
	int prefix_len;
	int ret;

	debug("%s for '%s' at node offset: %d\n", __func__, pmic->name,
	      pmic->of_offset);

	for (node = fdt_first_subnode(blob, offset);
	     node > 0;
	     node = fdt_next_subnode(blob, node)) {
		node_name = fdt_get_name(blob, node, &node_name_len);

		debug("* Found child node: '%s' at offset:%d\n", node_name,
								 node);

		child = NULL;
		for (info = child_info; info->prefix && info->driver; info++) {
			prefix_len = strlen(info->prefix);
			if (strncasecmp(info->prefix, node_name, prefix_len))
				continue;

			debug("  - compatible prefix: '%s'\n", info->prefix);

			drv = lists_driver_lookup_name(info->driver);
			if (!drv) {
				debug("  - driver: '%s' not found!\n",
				      info->driver);
				continue;
			}

			debug("  - found child driver: '%s'\n", drv->name);

			ret = device_bind(pmic, drv, node_name, NULL,
					  node, &child);
			if (ret) {
				debug("  - child binding error: %d\n", ret);
				continue;
			}

			debug("  - bound child device: '%s'\n", child->name);

			child->driver_data = str_get_num(node_name +
							 prefix_len,
							 node_name +
							 node_name_len);

			debug("  - set 'child->driver_data': %lu\n",
			      child->driver_data);
			break;
		}

		if (child)
			bind_count++;
		else
			debug("  - compatible prefix not found\n");
	}

	debug("Bound: %d childs for PMIC: '%s'\n", bind_count, pmic->name);
	return bind_count;
}

int pmic_get(const char *name, struct udevice **devp)
{
	return uclass_get_device_by_name(UCLASS_PMIC, name, devp);
}

int pmic_reg_count(struct udevice *dev)
{
	const struct dm_pmic_ops *ops = dev_get_driver_ops(dev);

	if (!ops || !ops->reg_count)
		return -ENOSYS;

	return ops->reg_count(dev);
}

int pmic_read(struct udevice *dev, uint reg, uint8_t *buffer, int len)
{
	const struct dm_pmic_ops *ops = dev_get_driver_ops(dev);

	if (!buffer)
		return -EFAULT;

	if (!ops || !ops->read)
		return -ENOSYS;

	return ops->read(dev, reg, buffer, len);
}

int pmic_write(struct udevice *dev, uint reg, const uint8_t *buffer, int len)
{
	const struct dm_pmic_ops *ops = dev_get_driver_ops(dev);

	if (!buffer)
		return -EFAULT;

	if (!ops || !ops->write)
		return -ENOSYS;

	return ops->write(dev, reg, buffer, len);
}

UCLASS_DRIVER(pmic) = {
	.id		= UCLASS_PMIC,
	.name		= "pmic",
};

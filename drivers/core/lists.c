/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/platdata.h>
#include <dm/uclass.h>
#include <dm/util.h>
#include <fdtdec.h>
#include <linux/compiler.h>

struct driver *lists_driver_lookup_name(const char *name)
{
	struct driver *drv =
		ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (!strcmp(name, entry->name))
			return entry;
	}

	/* Not found */
	return NULL;
}

struct uclass_driver *lists_uclass_lookup(enum uclass_id id)
{
	struct uclass_driver *uclass =
		ll_entry_start(struct uclass_driver, uclass);
	const int n_ents = ll_entry_count(struct uclass_driver, uclass);
	struct uclass_driver *entry;

	for (entry = uclass; entry != uclass + n_ents; entry++) {
		if (entry->id == id)
			return entry;
	}

	return NULL;
}

int lists_bind_drivers(struct udevice *parent, bool pre_reloc_only)
{
	struct driver_info *info =
		ll_entry_start(struct driver_info, driver_info);
	const int n_ents = ll_entry_count(struct driver_info, driver_info);
	struct driver_info *entry;
	struct udevice *dev;
	int result = 0;
	int ret;

	for (entry = info; entry != info + n_ents; entry++) {
		ret = device_bind_by_name(parent, pre_reloc_only, entry, &dev);
		if (ret && ret != -EPERM) {
			dm_warn("No match for driver '%s'\n", entry->name);
			if (!result || ret != -ENOENT)
				result = ret;
		}
	}

	return result;
}

int device_bind_driver(struct udevice *parent, const char *drv_name,
		       const char *dev_name, struct udevice **devp)
{
	return device_bind_driver_to_node(parent, drv_name, dev_name, -1, devp);
}

int device_bind_driver_to_node(struct udevice *parent, const char *drv_name,
			       const char *dev_name, int node,
			       struct udevice **devp)
{
	struct driver *drv;
	int ret;

	drv = lists_driver_lookup_name(drv_name);
	if (!drv) {
		debug("Cannot find driver '%s'\n", drv_name);
		return -ENOENT;
	}
	ret = device_bind(parent, drv, dev_name, NULL, node, devp);
	if (ret) {
		debug("Cannot create device named '%s' (err=%d)\n",
		      dev_name, ret);
		return ret;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
/**
 * driver_check_compatible() - Check if a driver is compatible with this node
 *
 * @param blob:		Device tree pointer
 * @param offset:	Offset of node in device tree
 * @param of_match:	List of compatible strings to match
 * @param of_idp:	Returns the match that was found
 * @return 0 if there is a match, -ENOENT if no match, -ENODEV if the node
 * does not have a compatible string, other error <0 if there is a device
 * tree error
 */
static int driver_check_compatible(const void *blob, int offset,
				   const struct udevice_id *of_match,
				   const struct udevice_id **of_idp)
{
	int ret;

	*of_idp = NULL;
	if (!of_match)
		return -ENOENT;

	while (of_match->compatible) {
		ret = fdt_node_check_compatible(blob, offset,
						of_match->compatible);
		if (!ret) {
			*of_idp = of_match;
			return 0;
		} else if (ret == -FDT_ERR_NOTFOUND) {
			return -ENODEV;
		} else if (ret < 0) {
			return -EINVAL;
		}
		of_match++;
	}

	return -ENOENT;
}

int lists_bind_fdt(struct udevice *parent, const void *blob, int offset,
		   struct udevice **devp)
{
	struct driver *driver = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	const struct udevice_id *id;
	struct driver *entry;
	struct udevice *dev;
	bool found = false;
	const char *name;
	int result = 0;
	int ret = 0;

	dm_dbg("bind node %s\n", fdt_get_name(blob, offset, NULL));
	if (devp)
		*devp = NULL;
	for (entry = driver; entry != driver + n_ents; entry++) {
		ret = driver_check_compatible(blob, offset, entry->of_match,
					      &id);
		name = fdt_get_name(blob, offset, NULL);
		if (ret == -ENOENT) {
			continue;
		} else if (ret == -ENODEV) {
			dm_dbg("Device '%s' has no compatible string\n", name);
			break;
		} else if (ret) {
			dm_warn("Device tree error at offset %d\n", offset);
			result = ret;
			break;
		}

		dm_dbg("   - found match at '%s'\n", entry->name);
		ret = device_bind(parent, entry, name, NULL, offset, &dev);
		if (ret) {
			dm_warn("Error binding driver '%s': %d\n", entry->name,
				ret);
			return ret;
		} else {
			dev->driver_data = id->data;
			found = true;
			if (devp)
				*devp = dev;
		}
		break;
	}

	if (!found && !result && ret != -ENODEV) {
		dm_dbg("No match for node '%s'\n",
		       fdt_get_name(blob, offset, NULL));
	}

	return result;
}
#endif

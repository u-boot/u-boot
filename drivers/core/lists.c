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
#include <dm/platdata.h>
#include <dm/uclass.h>
#include <dm/util.h>
#include <linux/compiler.h>

struct driver *lists_driver_lookup_name(const char *name)
{
	struct driver *drv =
		ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;
	int len;

	if (!drv || !n_ents)
		return NULL;

	len = strlen(name);

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (strncmp(name, entry->name, len))
			continue;

		/* Full match */
		if (len == strlen(entry->name))
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

	if ((id == UCLASS_INVALID) || !uclass)
		return NULL;

	for (entry = uclass; entry != uclass + n_ents; entry++) {
		if (entry->id == id)
			return entry;
	}

	return NULL;
}

int lists_bind_drivers(struct device *parent)
{
	struct driver_info *info =
		ll_entry_start(struct driver_info, driver_info);
	const int n_ents = ll_entry_count(struct driver_info, driver_info);
	struct driver_info *entry;
	struct device *dev;
	int result = 0;
	int ret;

	for (entry = info; entry != info + n_ents; entry++) {
		ret = device_bind_by_name(parent, entry, &dev);
		if (ret) {
			dm_warn("No match for driver '%s'\n", entry->name);
			if (!result || ret != -ENOENT)
				result = ret;
		}
	}

	return result;
}

#ifdef CONFIG_OF_CONTROL
/**
 * driver_check_compatible() - Check if a driver is compatible with this node
 *
 * @param blob:		Device tree pointer
 * @param offset:	Offset of node in device tree
 * @param of_matchL	List of compatible strings to match
 * @return 0 if there is a match, -ENOENT if no match, -ENODEV if the node
 * does not have a compatible string, other error <0 if there is a device
 * tree error
 */
static int driver_check_compatible(const void *blob, int offset,
				   const struct device_id *of_match)
{
	int ret;

	if (!of_match)
		return -ENOENT;

	while (of_match->compatible) {
		ret = fdt_node_check_compatible(blob, offset,
						of_match->compatible);
		if (!ret)
			return 0;
		else if (ret == -FDT_ERR_NOTFOUND)
			return -ENODEV;
		else if (ret < 0)
			return -EINVAL;
		of_match++;
	}

	return -ENOENT;
}

int lists_bind_fdt(struct device *parent, const void *blob, int offset)
{
	struct driver *driver = ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;
	struct device *dev;
	const char *name;
	int result = 0;
	int ret;

	dm_dbg("bind node %s\n", fdt_get_name(blob, offset, NULL));
	for (entry = driver; entry != driver + n_ents; entry++) {
		ret = driver_check_compatible(blob, offset, entry->of_match);
		if (ret == -ENOENT) {
			continue;
		} else if (ret == -ENODEV) {
			break;
		} else if (ret) {
			dm_warn("Device tree error at offset %d\n", offset);
			if (!result || ret != -ENOENT)
				result = ret;
			break;
		}

		name = fdt_get_name(blob, offset, NULL);
		dm_dbg("   - found match at '%s'\n", entry->name);
		ret = device_bind(parent, entry, name, NULL, offset, &dev);
		if (ret) {
			dm_warn("No match for driver '%s'\n", entry->name);
			if (!result || ret != -ENOENT)
				result = ret;
		}
	}

	return result;
}
#endif

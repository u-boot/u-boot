/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/platdata.h>
#include <dm/uclass.h>
#include <dm/util.h>
#include <linux/list.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct driver_info root_info = {
	.name		= "root_driver",
};

struct device *dm_root(void)
{
	if (!gd->dm_root) {
		dm_warn("Virtual root driver does not exist!\n");
		return NULL;
	}

	return gd->dm_root;
}

int dm_init(void)
{
	int ret;

	if (gd->dm_root) {
		dm_warn("Virtual root driver already exists!\n");
		return -EINVAL;
	}
	INIT_LIST_HEAD(&gd->uclass_root);

	ret = device_bind_by_name(NULL, &root_info, &gd->dm_root);
	if (ret)
		return ret;

	return 0;
}

int dm_scan_platdata(void)
{
	int ret;

	ret = lists_bind_drivers(gd->dm_root);
	if (ret == -ENOENT) {
		dm_warn("Some drivers were not found\n");
		ret = 0;
	}
	if (ret)
		return ret;

	return 0;
}

#ifdef CONFIG_OF_CONTROL
int dm_scan_fdt(const void *blob)
{
	int offset = 0;
	int ret = 0, err;
	int depth = 0;

	do {
		offset = fdt_next_node(blob, offset, &depth);
		if (offset > 0 && depth == 1) {
			err = lists_bind_fdt(gd->dm_root, blob, offset);
			if (err && !ret)
				ret = err;
		}
	} while (offset > 0);

	if (ret)
		dm_warn("Some drivers failed to bind\n");

	return ret;
}
#endif

/* This is the root driver - all drivers are children of this */
U_BOOT_DRIVER(root_driver) = {
	.name	= "root_driver",
	.id	= UCLASS_ROOT,
};

/* This is the root uclass */
UCLASS_DRIVER(root) = {
	.name	= "root",
	.id	= UCLASS_ROOT,
};

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
#include <libfdt.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/platdata.h>
#include <dm/root.h>
#include <dm/uclass.h>
#include <dm/util.h>
#include <linux/list.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct driver_info root_info = {
	.name		= "root_driver",
};

struct udevice *dm_root(void)
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
	INIT_LIST_HEAD(&DM_UCLASS_ROOT_NON_CONST);

	ret = device_bind_by_name(NULL, false, &root_info, &DM_ROOT_NON_CONST);
	if (ret)
		return ret;
	ret = device_probe(DM_ROOT_NON_CONST);
	if (ret)
		return ret;

	return 0;
}

int dm_uninit(void)
{
	device_remove(dm_root());
	device_unbind(dm_root());

	return 0;
}

int dm_scan_platdata(bool pre_reloc_only)
{
	int ret;

	ret = lists_bind_drivers(DM_ROOT_NON_CONST, pre_reloc_only);
	if (ret == -ENOENT) {
		dm_warn("Some drivers were not found\n");
		ret = 0;
	}
	if (ret)
		return ret;

	return 0;
}

#ifdef CONFIG_OF_CONTROL
int dm_scan_fdt_node(struct udevice *parent, const void *blob, int offset,
		     bool pre_reloc_only)
{
	int ret = 0, err;

	for (offset = fdt_first_subnode(blob, offset);
	     offset > 0;
	     offset = fdt_next_subnode(blob, offset)) {
		if (pre_reloc_only &&
		    !fdt_getprop(blob, offset, "u-boot,dm-pre-reloc", NULL))
			continue;
		err = lists_bind_fdt(parent, blob, offset, NULL);
		if (err && !ret)
			ret = err;
	}

	if (ret)
		dm_warn("Some drivers failed to bind\n");

	return ret;
}

int dm_scan_fdt(const void *blob, bool pre_reloc_only)
{
	return dm_scan_fdt_node(gd->dm_root, blob, 0, pre_reloc_only);
}
#endif

__weak int dm_scan_other(bool pre_reloc_only)
{
	return 0;
}

int dm_init_and_scan(bool pre_reloc_only)
{
	int ret;

	ret = dm_init();
	if (ret) {
		debug("dm_init() failed: %d\n", ret);
		return ret;
	}
	ret = dm_scan_platdata(pre_reloc_only);
	if (ret) {
		debug("dm_scan_platdata() failed: %d\n", ret);
		return ret;
	}
#ifdef CONFIG_OF_CONTROL
	ret = dm_scan_fdt(gd->fdt_blob, pre_reloc_only);
	if (ret) {
		debug("dm_scan_fdt() failed: %d\n", ret);
		return ret;
	}
#endif
	ret = dm_scan_other(pre_reloc_only);
	if (ret)
		return ret;

	return 0;
}

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

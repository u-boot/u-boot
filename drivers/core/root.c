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
#include <fdtdec.h>
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

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
void fix_drivers(void)
{
	struct driver *drv =
		ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (entry->of_match)
			entry->of_match = (const struct udevice_id *)
				((u32)entry->of_match + gd->reloc_off);
		if (entry->bind)
			entry->bind += gd->reloc_off;
		if (entry->probe)
			entry->probe += gd->reloc_off;
		if (entry->remove)
			entry->remove += gd->reloc_off;
		if (entry->unbind)
			entry->unbind += gd->reloc_off;
		if (entry->ofdata_to_platdata)
			entry->ofdata_to_platdata += gd->reloc_off;
		if (entry->child_post_bind)
			entry->child_post_bind += gd->reloc_off;
		if (entry->child_pre_probe)
			entry->child_pre_probe += gd->reloc_off;
		if (entry->child_post_remove)
			entry->child_post_remove += gd->reloc_off;
		/* OPS are fixed in every uclass post_probe function */
		if (entry->ops)
			entry->ops += gd->reloc_off;
	}
}

void fix_uclass(void)
{
	struct uclass_driver *uclass =
		ll_entry_start(struct uclass_driver, uclass);
	const int n_ents = ll_entry_count(struct uclass_driver, uclass);
	struct uclass_driver *entry;

	for (entry = uclass; entry != uclass + n_ents; entry++) {
		if (entry->post_bind)
			entry->post_bind += gd->reloc_off;
		if (entry->pre_unbind)
			entry->pre_unbind += gd->reloc_off;
		if (entry->pre_probe)
			entry->pre_probe += gd->reloc_off;
		if (entry->post_probe)
			entry->post_probe += gd->reloc_off;
		if (entry->pre_remove)
			entry->pre_remove += gd->reloc_off;
		if (entry->child_post_bind)
			entry->child_post_bind += gd->reloc_off;
		if (entry->child_pre_probe)
			entry->child_pre_probe += gd->reloc_off;
		if (entry->init)
			entry->init += gd->reloc_off;
		if (entry->destroy)
			entry->destroy += gd->reloc_off;
		/* FIXME maybe also need to fix these ops */
		if (entry->ops)
			entry->ops += gd->reloc_off;
	}
}
#endif

int dm_init(void)
{
	int ret;

	if (gd->dm_root) {
		dm_warn("Virtual root driver already exists!\n");
		return -EINVAL;
	}
	INIT_LIST_HEAD(&DM_UCLASS_ROOT_NON_CONST);

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	fix_drivers();
	fix_uclass();
#endif

	ret = device_bind_by_name(NULL, false, &root_info, &DM_ROOT_NON_CONST);
	if (ret)
		return ret;
#if CONFIG_IS_ENABLED(OF_CONTROL)
	DM_ROOT_NON_CONST->of_offset = 0;
#endif
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

	return ret;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
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
		if (!fdtdec_get_is_enabled(blob, offset)) {
			dm_dbg("   - ignoring disabled device\n");
			continue;
		}
		err = lists_bind_fdt(parent, blob, offset, NULL);
		if (err && !ret) {
			ret = err;
			debug("%s: ret=%d\n", fdt_get_name(blob, offset, NULL),
			      ret);
		}
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

	if (CONFIG_IS_ENABLED(OF_CONTROL)) {
		ret = dm_scan_fdt(gd->fdt_blob, pre_reloc_only);
		if (ret) {
			debug("dm_scan_fdt() failed: %d\n", ret);
			return ret;
		}
	}

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

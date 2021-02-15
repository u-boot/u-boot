// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <dm/acpi.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/of.h>
#include <dm/of_access.h>
#include <dm/platdata.h>
#include <dm/read.h>
#include <dm/root.h>
#include <dm/uclass.h>
#include <dm/util.h>
#include <linux/list.h>

DECLARE_GLOBAL_DATA_PTR;

static struct driver_info root_info = {
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

void dm_fixup_for_gd_move(struct global_data *new_gd)
{
	/* The sentinel node has moved, so update things that point to it */
	if (gd->dm_root) {
		new_gd->uclass_root->next->prev = new_gd->uclass_root;
		new_gd->uclass_root->prev->next = new_gd->uclass_root;
	}
}

void fix_drivers(void)
{
	struct driver *drv =
		ll_entry_start(struct driver, driver);
	const int n_ents = ll_entry_count(struct driver, driver);
	struct driver *entry;

	for (entry = drv; entry != drv + n_ents; entry++) {
		if (entry->of_match)
			entry->of_match = (const struct udevice_id *)
				((ulong)entry->of_match + gd->reloc_off);
		if (entry->bind)
			entry->bind += gd->reloc_off;
		if (entry->probe)
			entry->probe += gd->reloc_off;
		if (entry->remove)
			entry->remove += gd->reloc_off;
		if (entry->unbind)
			entry->unbind += gd->reloc_off;
		if (entry->of_to_plat)
			entry->of_to_plat += gd->reloc_off;
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
		ll_entry_start(struct uclass_driver, uclass_driver);
	const int n_ents = ll_entry_count(struct uclass_driver, uclass_driver);
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

void fix_devices(void)
{
	struct driver_info *dev =
		ll_entry_start(struct driver_info, driver_info);
	const int n_ents = ll_entry_count(struct driver_info, driver_info);
	struct driver_info *entry;

	for (entry = dev; entry != dev + n_ents; entry++) {
		if (entry->plat)
			entry->plat += gd->reloc_off;
	}
}

int dm_init(bool of_live)
{
	int ret;

	if (IS_ENABLED(CONFIG_OF_TRANSLATE_ZERO_SIZE_CELLS))
		gd->dm_flags |= GD_DM_FLG_SIZE_CELLS_0;

	if (gd->dm_root) {
		dm_warn("Virtual root driver already exists!\n");
		return -EINVAL;
	}
	gd->uclass_root = &DM_UCLASS_ROOT_S_NON_CONST;
	INIT_LIST_HEAD(DM_UCLASS_ROOT_NON_CONST);

	if (IS_ENABLED(CONFIG_NEEDS_MANUAL_RELOC)) {
		fix_drivers();
		fix_uclass();
		fix_devices();
	}

	ret = device_bind_by_name(NULL, false, &root_info, &DM_ROOT_NON_CONST);
	if (ret)
		return ret;
	if (CONFIG_IS_ENABLED(OF_CONTROL))
		dev_set_ofnode(DM_ROOT_NON_CONST, ofnode_root());
	ret = device_probe(DM_ROOT_NON_CONST);
	if (ret)
		return ret;

	return 0;
}

int dm_uninit(void)
{
	/* Remove non-vital devices first */
	device_remove(dm_root(), DM_REMOVE_NON_VITAL);
	device_remove(dm_root(), DM_REMOVE_NORMAL);
	device_unbind(dm_root());
	gd->dm_root = NULL;

	return 0;
}

#if CONFIG_IS_ENABLED(DM_DEVICE_REMOVE)
int dm_remove_devices_flags(uint flags)
{
	device_remove(dm_root(), flags);

	return 0;
}
#endif

int dm_scan_plat(bool pre_reloc_only)
{
	int ret;

	if (CONFIG_IS_ENABLED(OF_PLATDATA)) {
		struct driver_rt *dyn;
		int n_ents;

		n_ents = ll_entry_count(struct driver_info, driver_info);
		dyn = calloc(n_ents, sizeof(struct driver_rt));
		if (!dyn)
			return -ENOMEM;
		gd_set_dm_driver_rt(dyn);
	}

	ret = lists_bind_drivers(DM_ROOT_NON_CONST, pre_reloc_only);
	if (ret == -ENOENT) {
		dm_warn("Some drivers were not found\n");
		ret = 0;
	}

	return ret;
}

#if CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)
/**
 * dm_scan_fdt_node() - Scan the device tree and bind drivers for a node
 *
 * This scans the subnodes of a device tree node and and creates a driver
 * for each one.
 *
 * @parent: Parent device for the devices that will be created
 * @node: Node to scan
 * @pre_reloc_only: If true, bind only drivers with the DM_FLAG_PRE_RELOC
 * flag. If false bind all drivers.
 * @return 0 if OK, -ve on error
 */
static int dm_scan_fdt_node(struct udevice *parent, ofnode parent_node,
			    bool pre_reloc_only)
{
	int ret = 0, err;
	ofnode node;

	if (!ofnode_valid(parent_node))
		return 0;

	for (node = ofnode_first_subnode(parent_node);
	     ofnode_valid(node);
	     node = ofnode_next_subnode(node)) {
		const char *node_name = ofnode_get_name(node);

		if (!ofnode_is_enabled(node)) {
			pr_debug("   - ignoring disabled device\n");
			continue;
		}
		err = lists_bind_fdt(parent, node, NULL, pre_reloc_only);
		if (err && !ret) {
			ret = err;
			debug("%s: ret=%d\n", node_name, ret);
		}
	}

	if (ret)
		dm_warn("Some drivers failed to bind\n");

	return ret;
}

int dm_scan_fdt_dev(struct udevice *dev)
{
	return dm_scan_fdt_node(dev, dev_ofnode(dev),
				gd->flags & GD_FLG_RELOC ? false : true);
}

int dm_scan_fdt(bool pre_reloc_only)
{
	return dm_scan_fdt_node(gd->dm_root, ofnode_root(), pre_reloc_only);
}

static int dm_scan_fdt_ofnode_path(const char *path, bool pre_reloc_only)
{
	ofnode node;

	node = ofnode_path(path);

	return dm_scan_fdt_node(gd->dm_root, node, pre_reloc_only);
}

int dm_extended_scan(bool pre_reloc_only)
{
	int ret, i;
	const char * const nodes[] = {
		"/chosen",
		"/clocks",
		"/firmware"
	};

	ret = dm_scan_fdt(pre_reloc_only);
	if (ret) {
		debug("dm_scan_fdt() failed: %d\n", ret);
		return ret;
	}

	/* Some nodes aren't devices themselves but may contain some */
	for (i = 0; i < ARRAY_SIZE(nodes); i++) {
		ret = dm_scan_fdt_ofnode_path(nodes[i], pre_reloc_only);
		if (ret) {
			debug("dm_scan_fdt() scan for %s failed: %d\n",
			      nodes[i], ret);
			return ret;
		}
	}

	return ret;
}
#endif

__weak int dm_scan_other(bool pre_reloc_only)
{
	return 0;
}

/**
 * dm_scan() - Scan tables to bind devices
 *
 * Runs through the driver_info tables and binds the devices it finds. Then runs
 * through the devicetree nodes. Finally calls dm_scan_other() to add any
 * special devices
 *
 * @pre_reloc_only: If true, bind only nodes with special devicetree properties,
 * or drivers with the DM_FLAG_PRE_RELOC flag. If false bind all drivers.
 */
static int dm_scan(bool pre_reloc_only)
{
	int ret;

	ret = dm_scan_plat(pre_reloc_only);
	if (ret) {
		debug("dm_scan_plat() failed: %d\n", ret);
		return ret;
	}

	if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)) {
		ret = dm_extended_scan(pre_reloc_only);
		if (ret) {
			debug("dm_extended_scan() failed: %d\n", ret);
			return ret;
		}
	}

	ret = dm_scan_other(pre_reloc_only);
	if (ret)
		return ret;

	return 0;
}

int dm_init_and_scan(bool pre_reloc_only)
{
	int ret;

	ret = dm_init(CONFIG_IS_ENABLED(OF_LIVE));
	if (ret) {
		debug("dm_init() failed: %d\n", ret);
		return ret;
	}
	ret = dm_scan(pre_reloc_only);
	if (ret) {
		log_debug("dm_scan() failed: %d\n", ret);
		return ret;
	}

	return 0;
}

#ifdef CONFIG_ACPIGEN
static int root_acpi_get_name(const struct udevice *dev, char *out_name)
{
	return acpi_copy_name(out_name, "\\_SB");
}

struct acpi_ops root_acpi_ops = {
	.get_name	= root_acpi_get_name,
};
#endif

/* This is the root driver - all drivers are children of this */
U_BOOT_DRIVER(root_driver) = {
	.name	= "root_driver",
	.id	= UCLASS_ROOT,
	ACPI_OPS_PTR(&root_acpi_ops)
};

/* This is the root uclass */
UCLASS_DRIVER(root) = {
	.name	= "root",
	.id	= UCLASS_ROOT,
};

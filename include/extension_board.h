/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#ifndef __EXTENSION_SUPPORT_H
#define __EXTENSION_SUPPORT_H

#include <alist.h>
#include <dm/device.h>
#include <linux/list.h>
#include <dm/platdata.h>

extern struct list_head extension_list;

/**
 * extension_get_list - Get the extension list
 * Return: The extension alist pointer, or NULL if no such list exists.
 *
 * The caller must not free the list.
 */
struct alist *extension_get_list(void);

/**
 * extension_probe - Probe extension device
 * @dev: Extension device that needs to be probed
 * Return: Zero on success, negative on failure.
 */
int extension_probe(struct udevice *dev);

/**
 * extension_remove - Remove extension device
 * @dev: Extension device that needs to be removed
 * Return: Zero on success, negative on failure.
 */
int extension_remove(struct udevice *dev);

/**
 * extension_scan - Scan extension boards available.
 * Return: Zero on success, negative on failure.
 */
int extension_scan(void);

/**
 * extension_apply - Apply extension board overlay to the devicetree
 * @working_fdt: Pointer to working flattened device tree
 * @size: Size of the devicetree overlay
 * Return: Zero on success, negative on failure.
 */
int extension_apply(struct fdt_header *working_fdt, ulong size);

/**
 * extension - Description fields of an extension board
 * @name: Name of the extension
 * @owner: Owner of the extension
 * @version: Version of the extension
 * @overlay: Devicetree overlay name to be loaded for this extension
 * @other: Other information of this extension
 */
struct extension {
	char name[32];
	char owner[32];
	char version[32];
	char overlay[64];
	char other[32];
};

struct extension_ops {
	/**
	 * scan - Add system-specific function to scan extension boards.
	 * @dev: extension device
	 * @extension_list: alist of extension to expand
	 * Return: The number of extension or a negative value in case of
	 *	   error.
	 */
	int (*scan)(struct udevice *dev, struct alist *extension_list);
};

#define extension_get_ops(dev)  ((struct extension_ops *)(dev)->driver->ops)

/* Currently, only one extension driver enabled at a time is supported */
#define U_BOOT_EXTENSION(_name, _scan_func) \
	U_BOOT_DRIVER(_name) = { \
		.name = #_name, \
		.id = UCLASS_EXTENSION, \
		.probe = extension_probe, \
		.remove = extension_remove, \
		.ops = &(struct extension_ops) { \
		       .scan = _scan_func, \
		       }, \
		.priv_auto = sizeof(struct alist), \
	}

#endif /* __EXTENSION_SUPPORT_H */

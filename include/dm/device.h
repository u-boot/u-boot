/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 * Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_DEVICE_H
#define _DM_DEVICE_H

#include <dm/uclass-id.h>
#include <linker_lists.h>
#include <linux/list.h>

struct driver_info;

/* Driver is active (probed). Cleared when it is removed */
#define DM_FLAG_ACTIVATED	(1 << 0)

/* DM is responsible for allocating and freeing platdata */
#define DM_FLAG_ALLOC_PDATA	(1 << 1)

/**
 * struct udevice - An instance of a driver
 *
 * This holds information about a device, which is a driver bound to a
 * particular port or peripheral (essentially a driver instance).
 *
 * A device will come into existence through a 'bind' call, either due to
 * a U_BOOT_DEVICE() macro (in which case platdata is non-NULL) or a node
 * in the device tree (in which case of_offset is >= 0). In the latter case
 * we translate the device tree information into platdata in a function
 * implemented by the driver ofdata_to_platdata method (called just before the
 * probe method if the device has a device tree node.
 *
 * All three of platdata, priv and uclass_priv can be allocated by the
 * driver, or you can use the auto_alloc_size members of struct driver and
 * struct uclass_driver to have driver model do this automatically.
 *
 * @driver: The driver used by this device
 * @name: Name of device, typically the FDT node name
 * @platdata: Configuration data for this device
 * @of_offset: Device tree node offset for this device (- for none)
 * @parent: Parent of this device, or NULL for the top level device
 * @priv: Private data for this device
 * @uclass: Pointer to uclass for this device
 * @uclass_priv: The uclass's private data for this device
 * @uclass_node: Used by uclass to link its devices
 * @child_head: List of children of this device
 * @sibling_node: Next device in list of all devices
 * @flags: Flags for this device DM_FLAG_...
 */
struct udevice {
	struct driver *driver;
	const char *name;
	void *platdata;
	int of_offset;
	struct udevice *parent;
	void *priv;
	struct uclass *uclass;
	void *uclass_priv;
	struct list_head uclass_node;
	struct list_head child_head;
	struct list_head sibling_node;
	uint32_t flags;
};

/* Returns the operations for a device */
#define device_get_ops(dev)	(dev->driver->ops)

/* Returns non-zero if the device is active (probed and not removed) */
#define device_active(dev)	((dev)->flags & DM_FLAG_ACTIVATED)

/**
 * struct udevice_id - Lists the compatible strings supported by a driver
 * @compatible: Compatible string
 * @data: Data for this compatible string
 */
struct udevice_id {
	const char *compatible;
	ulong data;
};

/**
 * struct driver - A driver for a feature or peripheral
 *
 * This holds methods for setting up a new device, and also removing it.
 * The device needs information to set itself up - this is provided either
 * by platdata or a device tree node (which we find by looking up
 * matching compatible strings with of_match).
 *
 * Drivers all belong to a uclass, representing a class of devices of the
 * same type. Common elements of the drivers can be implemented in the uclass,
 * or the uclass can provide a consistent interface to the drivers within
 * it.
 *
 * @name: Device name
 * @id: Identiies the uclass we belong to
 * @of_match: List of compatible strings to match, and any identifying data
 * for each.
 * @bind: Called to bind a device to its driver
 * @probe: Called to probe a device, i.e. activate it
 * @remove: Called to remove a device, i.e. de-activate it
 * @unbind: Called to unbind a device from its driver
 * @ofdata_to_platdata: Called before probe to decode device tree data
 * @priv_auto_alloc_size: If non-zero this is the size of the private data
 * to be allocated in the device's ->priv pointer. If zero, then the driver
 * is responsible for allocating any data required.
 * @platdata_auto_alloc_size: If non-zero this is the size of the
 * platform data to be allocated in the device's ->platdata pointer.
 * This is typically only useful for device-tree-aware drivers (those with
 * an of_match), since drivers which use platdata will have the data
 * provided in the U_BOOT_DEVICE() instantiation.
 * ops: Driver-specific operations. This is typically a list of function
 * pointers defined by the driver, to implement driver functions required by
 * the uclass.
 */
struct driver {
	char *name;
	enum uclass_id id;
	const struct udevice_id *of_match;
	int (*bind)(struct udevice *dev);
	int (*probe)(struct udevice *dev);
	int (*remove)(struct udevice *dev);
	int (*unbind)(struct udevice *dev);
	int (*ofdata_to_platdata)(struct udevice *dev);
	int priv_auto_alloc_size;
	int platdata_auto_alloc_size;
	const void *ops;	/* driver-specific operations */
};

/* Declare a new U-Boot driver */
#define U_BOOT_DRIVER(__name)						\
	ll_entry_declare(struct driver, __name, driver)

/**
 * dev_get_platdata() - Get the platform data for a device
 *
 * This checks that dev is not NULL, but no other checks for now
 *
 * @dev		Device to check
 * @return platform data, or NULL if none
 */
void *dev_get_platdata(struct udevice *dev);

/**
 * dev_get_priv() - Get the private data for a device
 *
 * This checks that dev is not NULL, but no other checks for now
 *
 * @dev		Device to check
 * @return private data, or NULL if none
 */
void *dev_get_priv(struct udevice *dev);

#endif

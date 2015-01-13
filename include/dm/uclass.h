/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_UCLASS_H
#define _DM_UCLASS_H

#include <dm/uclass-id.h>
#include <linker_lists.h>
#include <linux/list.h>

/**
 * struct uclass - a U-Boot drive class, collecting together similar drivers
 *
 * A uclass provides an interface to a particular function, which is
 * implemented by one or more drivers. Every driver belongs to a uclass even
 * if it is the only driver in that uclass. An example uclass is GPIO, which
 * provides the ability to change read inputs, set and clear outputs, etc.
 * There may be drivers for on-chip SoC GPIO banks, I2C GPIO expanders and
 * PMIC IO lines, all made available in a unified way through the uclass.
 *
 * @priv: Private data for this uclass
 * @uc_drv: The driver for the uclass itself, not to be confused with a
 * 'struct driver'
 * @dev_head: List of devices in this uclass (devices are attached to their
 * uclass when their bind method is called)
 * @sibling_node: Next uclass in the linked list of uclasses
 */
struct uclass {
	void *priv;
	struct uclass_driver *uc_drv;
	struct list_head dev_head;
	struct list_head sibling_node;
};

struct udevice;

/**
 * struct uclass_driver - Driver for the uclass
 *
 * A uclass_driver provides a consistent interface to a set of related
 * drivers.
 *
 * @name: Name of uclass driver
 * @id: ID number of this uclass
 * @post_bind: Called after a new device is bound to this uclass
 * @pre_unbind: Called before a device is unbound from this uclass
 * @post_probe: Called after a new device is probed
 * @pre_remove: Called before a device is removed
 * @init: Called to set up the uclass
 * @destroy: Called to destroy the uclass
 * @priv_auto_alloc_size: If non-zero this is the size of the private data
 * to be allocated in the uclass's ->priv pointer. If zero, then the uclass
 * driver is responsible for allocating any data required.
 * @per_device_auto_alloc_size: Each device can hold private data owned
 * by the uclass. If required this will be automatically allocated if this
 * value is non-zero.
 * @ops: Uclass operations, providing the consistent interface to devices
 * within the uclass.
 */
struct uclass_driver {
	const char *name;
	enum uclass_id id;
	int (*post_bind)(struct udevice *dev);
	int (*pre_unbind)(struct udevice *dev);
	int (*post_probe)(struct udevice *dev);
	int (*pre_remove)(struct udevice *dev);
	int (*init)(struct uclass *class);
	int (*destroy)(struct uclass *class);
	int priv_auto_alloc_size;
	int per_device_auto_alloc_size;
	const void *ops;
};

/* Declare a new uclass_driver */
#define UCLASS_DRIVER(__name)						\
	ll_entry_declare(struct uclass_driver, __name, uclass)

/**
 * uclass_get() - Get a uclass based on an ID, creating it if needed
 *
 * Every uclass is identified by an ID, a number from 0 to n-1 where n is
 * the number of uclasses. This function allows looking up a uclass by its
 * ID.
 *
 * @key: ID to look up
 * @ucp: Returns pointer to uclass (there is only one per ID)
 * @return 0 if OK, -ve on error
 */
int uclass_get(enum uclass_id key, struct uclass **ucp);

/**
 * uclass_get_device() - Get a uclass device based on an ID and index
 *
 * The device is probed to activate it ready for use.
 *
 * @id: ID to look up
 * @index: Device number within that uclass (0=first)
 * @devp: Returns pointer to device (there is only one per for each ID)
 * @return 0 if OK, -ve on error
 */
int uclass_get_device(enum uclass_id id, int index, struct udevice **devp);

/**
 * uclass_get_device_by_seq() - Get a uclass device based on an ID and sequence
 *
 * If an active device has this sequence it will be returned. If there is no
 * such device then this will check for a device that is requesting this
 * sequence.
 *
 * The device is probed to activate it ready for use.
 *
 * @id: ID to look up
 * @seq: Sequence number to find (0=first)
 * @devp: Returns pointer to device (there is only one for each seq)
 * @return 0 if OK, -ve on error
 */
int uclass_get_device_by_seq(enum uclass_id id, int seq, struct udevice **devp);

/**
 * uclass_get_device_by_of_offset() - Get a uclass device by device tree node
 *
 * This searches the devices in the uclass for one attached to the given
 * device tree node.
 *
 * The device is probed to activate it ready for use.
 *
 * @id: ID to look up
 * @node: Device tree offset to search for (if -ve then -ENODEV is returned)
 * @devp: Returns pointer to device (there is only one for each node)
 * @return 0 if OK, -ve on error
 */
int uclass_get_device_by_of_offset(enum uclass_id id, int node,
				   struct udevice **devp);

/**
 * uclass_first_device() - Get the first device in a uclass
 *
 * @id: Uclass ID to look up
 * @devp: Returns pointer to the first device in that uclass, or NULL if none
 * @return 0 if OK (found or not found), -1 on error
 */
int uclass_first_device(enum uclass_id id, struct udevice **devp);

/**
 * uclass_next_device() - Get the next device in a uclass
 *
 * @devp: On entry, pointer to device to lookup. On exit, returns pointer
 * to the next device in the same uclass, or NULL if none
 * @return 0 if OK (found or not found), -1 on error
 */
int uclass_next_device(struct udevice **devp);

/**
 * uclass_resolve_seq() - Resolve a device's sequence number
 *
 * On entry dev->seq is -1, and dev->req_seq may be -1 (to allocate a
 * sequence number automatically, or >= 0 to select a particular number.
 * If the requested sequence number is in use, then this device will
 * be allocated another one.
 *
 * Note that the device's seq value is not changed by this function.
 *
 * @dev: Device for which to allocate sequence number
 * @return sequence number allocated, or -ve on error
 */
int uclass_resolve_seq(struct udevice *dev);

/**
 * uclass_foreach_dev() - Helper function to iteration through devices
 *
 * This creates a for() loop which works through the available devices in
 * a uclass in order from start to end.
 *
 * @pos: struct udevice * to hold the current device. Set to NULL when there
 * are no more devices.
 * @uc: uclass to scan
 */
#define uclass_foreach_dev(pos, uc)					\
	for (pos = list_entry((&(uc)->dev_head)->next, typeof(*pos),	\
			uclass_node);					\
	     prefetch(pos->uclass_node.next),				\
			&pos->uclass_node != (&(uc)->dev_head);		\
	     pos = list_entry(pos->uclass_node.next, typeof(*pos),	\
			uclass_node))

#endif

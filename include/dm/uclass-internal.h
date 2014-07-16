/*
 * Copyright (c) 2013 Google, Inc
 *
 * (C) Copyright 2012
 * Pavel Herrmann <morpheus.ibis@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _DM_UCLASS_INTERNAL_H
#define _DM_UCLASS_INTERNAL_H

/**
 * uclass_find_device() - Return n-th child of uclass
 * @id:		Id number of the uclass
 * @index:	Position of the child in uclass's list
 * #devp:	Returns pointer to device, or NULL on error
 *
 * The device is not prepared for use - this is an internal function
 *
 * @return the uclass pointer of a child at the given index or
 * return NULL on error.
 */
int uclass_find_device(enum uclass_id id, int index, struct udevice **devp);

/**
 * uclass_bind_device() - Associate device with a uclass
 *
 * Connect the device into uclass's list of devices.
 *
 * @dev:	Pointer to the device
 * #return 0 on success, -ve on error
 */
int uclass_bind_device(struct udevice *dev);

/**
 * uclass_unbind_device() - Deassociate device with a uclass
 *
 * Disconnect the device from uclass's list of devices.
 *
 * @dev:	Pointer to the device
 * #return 0 on success, -ve on error
 */
int uclass_unbind_device(struct udevice *dev);

/**
 * uclass_post_probe_device() - Deal with a device that has just been probed
 *
 * Perform any post-processing of a probed device that is needed by the
 * uclass.
 *
 * @dev:	Pointer to the device
 * #return 0 on success, -ve on error
 */
int uclass_post_probe_device(struct udevice *dev);

/**
 * uclass_pre_remove_device() - Handle a device which is about to be removed
 *
 * Perform any pre-processing of a device that is about to be removed.
 *
 * @dev:	Pointer to the device
 * #return 0 on success, -ve on error
 */
int uclass_pre_remove_device(struct udevice *dev);

/**
 * uclass_find() - Find uclass by its id
 *
 * @id:		Id to serach for
 * @return pointer to uclass, or NULL if not found
 */
struct uclass *uclass_find(enum uclass_id key);

/**
 * uclass_destroy() - Destroy a uclass
 *
 * Destroy a uclass and all its devices
 *
 * @uc: uclass to destroy
 * @return 0 on success, -ve on error
 */
int uclass_destroy(struct uclass *uc);

#endif

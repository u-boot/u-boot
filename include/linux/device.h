// SPDX-License-Identifier: GPL-2.0
/*
 * device.h - generic, centralized driver model
 *
 * U-Boot: compat header derived from Linux
 *
 * Copyright (c) 2001-2003 Patrick Mochel <mochel@osdl.org>
 * Copyright (c) 2004-2009 Greg Kroah-Hartman <gregkh@suse.de>
 * Copyright (c) 2008-2009 Novell Inc.
 *
 * See Documentation/driver-api/driver-model/ for more information.
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <dm/device.h>

/**
 * dev_name - Return a device's name.
 * @dev: Device with name to get.
 * Return: The kobject name of the device, or its initial name if unavailable.
 */
static inline const char *dev_name(const struct udevice *dev)
{
	return dev->name;
}

#endif /* _DEVICE_H_ */

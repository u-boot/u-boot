/*
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <misc.h>

/*
 * Implement a  miscellaneous uclass for those do not fit other more
 * general classes. A set of generic read, write and ioctl methods may
 * be used to access the device.
 */

int misc_read(struct udevice *dev, int offset, void *buf, int size)
{
	const struct misc_ops *ops = device_get_ops(dev);

	if (!ops->read)
		return -ENOSYS;

	return ops->read(dev, offset, buf, size);
}

int misc_write(struct udevice *dev, int offset, void *buf, int size)
{
	const struct misc_ops *ops = device_get_ops(dev);

	if (!ops->write)
		return -ENOSYS;

	return ops->write(dev, offset, buf, size);
}

int misc_ioctl(struct udevice *dev, unsigned long request, void *buf)
{
	const struct misc_ops *ops = device_get_ops(dev);

	if (!ops->ioctl)
		return -ENOSYS;

	return ops->ioctl(dev, request, buf);
}

UCLASS_DRIVER(misc) = {
	.id		= UCLASS_MISC,
	.name		= "misc",
};

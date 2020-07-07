// SPDX-License-Identifier: GPL-2.0
/*
 * Generation of tables for particular device types
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot file of the same name
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <acpi/acpi_device.h>
#include <dm/acpi.h>

/**
 * acpi_device_path_fill() - Find the root device and build a path from there
 *
 * This recursively reaches back to the root device and progressively adds path
 * elements until the device is reached.
 *
 * @dev: Device to return path of
 * @buf: Buffer to hold the path
 * @buf_len: Length of buffer
 * @cur: Current position in the buffer
 * @return new position in buffer after adding @dev, or -ve on error
 */
static int acpi_device_path_fill(const struct udevice *dev, char *buf,
				 size_t buf_len, int cur)
{
	char name[ACPI_NAME_MAX];
	int next = 0;
	int ret;

	ret = acpi_get_name(dev, name);
	if (ret)
		return ret;

	/*
	 * Make sure this name segment will fit, including the path segment
	 * separator and possible NULL terminator, if this is the last segment.
	 */
	if (cur + strlen(name) + 2 > buf_len)
		return -ENOSPC;

	/* Walk up the tree to the root device */
	if (dev_get_parent(dev)) {
		next = acpi_device_path_fill(dev_get_parent(dev), buf, buf_len,
					     cur);
		if (next < 0)
			return next;
	}

	/* Fill in the path from the root device */
	next += snprintf(buf + next, buf_len - next, "%s%s",
			 dev_get_parent(dev) && *name ? "." : "", name);

	return next;
}

int acpi_device_path(const struct udevice *dev, char *buf, int maxlen)
{
	int ret;

	ret = acpi_device_path_fill(dev, buf, maxlen, 0);
	if (ret < 0)
		return ret;

	return 0;
}

int acpi_device_scope(const struct udevice *dev, char *scope, int maxlen)
{
	int ret;

	if (!dev_get_parent(dev))
		return log_msg_ret("noparent", -EINVAL);

	ret = acpi_device_path_fill(dev_get_parent(dev), scope, maxlen, 0);
	if (ret < 0)
		return log_msg_ret("fill", ret);

	return 0;
}

enum acpi_dev_status acpi_device_status(const struct udevice *dev)
{
	return ACPI_DSTATUS_ALL_ON;
}

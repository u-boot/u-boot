/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Generation of tables for particular device types
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot file of the same name
 */

#ifndef __ACPI_DEVICE_H
#define __ACPI_DEVICE_H

#include <linux/bitops.h>

struct udevice;

/* Length of a full path to an ACPI device */
#define ACPI_PATH_MAX		30

/* Values that can be returned for ACPI device _STA method */
enum acpi_dev_status {
	ACPI_DSTATUS_PRESENT		= BIT(0),
	ACPI_DSTATUS_ENABLED		= BIT(1),
	ACPI_DSTATUS_SHOW_IN_UI		= BIT(2),
	ACPI_DSTATUS_OK			= BIT(3),
	ACPI_DSTATUS_HAS_BATTERY	= BIT(4),

	ACPI_DSTATUS_ALL_OFF	= 0,
	ACPI_DSTATUS_HIDDEN_ON	= ACPI_DSTATUS_PRESENT | ACPI_DSTATUS_ENABLED |
		ACPI_DSTATUS_OK,
	ACPI_DSTATUS_ALL_ON	= ACPI_DSTATUS_HIDDEN_ON |
		ACPI_DSTATUS_SHOW_IN_UI,
};

/**
 * acpi_device_path() - Get the full path to an ACPI device
 *
 * This gets the full path in the form XXXX.YYYY.ZZZZ where XXXX is the root
 * and ZZZZ is the device. All parent devices are added to the path.
 *
 * @dev: Device to check
 * @buf: Buffer to place the path in (should be ACPI_PATH_MAX long)
 * @maxlen: Size of buffer (typically ACPI_PATH_MAX)
 * @return 0 if OK, -ve on error
 */
int acpi_device_path(const struct udevice *dev, char *buf, int maxlen);

/**
 * acpi_device_scope() - Get the scope of an ACPI device
 *
 * This gets the scope which is the full path of the parent device, as per
 * acpi_device_path().
 *
 * @dev: Device to check
 * @buf: Buffer to place the path in (should be ACPI_PATH_MAX long)
 * @maxlen: Size of buffer (typically ACPI_PATH_MAX)
 * @return 0 if OK, -EINVAL if the device has no parent, other -ve on other
 *	error
 */
int acpi_device_scope(const struct udevice *dev, char *scope, int maxlen);

/**
 * acpi_device_status() - Get the status of a device
 *
 * This currently just returns ACPI_DSTATUS_ALL_ON. It does not support
 * inactive or hidden devices.
 *
 * @dev: Device to check
 * @return device status, as ACPI_DSTATUS_...
 */
enum acpi_dev_status acpi_device_status(const struct udevice *dev);

#endif

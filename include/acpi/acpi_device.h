/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Generation of tables for particular device types
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot file of the same name
 */

#ifndef __ACPI_DEVICE_H
#define __ACPI_DEVICE_H

struct udevice;

/* Length of a full path to an ACPI device */
#define ACPI_PATH_MAX		30

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

#endif

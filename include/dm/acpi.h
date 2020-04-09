/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Core ACPI (Advanced Configuration and Power Interface) support
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __DM_ACPI_H__
#define __DM_ACPI_H__

/* Allow operations to be optional for ACPI */
#if CONFIG_IS_ENABLED(ACPIGEN)
#define ACPI_OPS_PTR(_ptr)	.acpi_ops	= _ptr,
#else
#define ACPI_OPS_PTR(_ptr)
#endif

/* Length of an ACPI name string, excluding nul terminator */
#define ACPI_NAME_LEN	4

/* Length of an ACPI name string including nul terminator */
#define ACPI_NAME_MAX	(ACPI_NAME_LEN + 1)

/**
 * struct acpi_ops - ACPI operations supported by driver model
 */
struct acpi_ops {
	/**
	 * get_name() - Obtain the ACPI name of a device
	 *
	 * @dev: Device to check
	 * @out_name: Place to put the name, must hold at least ACPI_NAME_MAX
	 *	bytes
	 * @return 0 if OK, -ENOENT if no name is available, other -ve value on
	 *	other error
	 */
	int (*get_name)(const struct udevice *dev, char *out_name);
};

#define device_get_acpi_ops(dev)	((dev)->driver->acpi_ops)

/**
 * acpi_get_name() - Obtain the ACPI name of a device
 *
 * @dev: Device to check
 * @out_name: Place to put the name, must hold at least ACPI_NAME_MAX
 *	bytes
 * @return 0 if OK, -ENOENT if no name is available, other -ve value on
 *	other error
 */
int acpi_get_name(const struct udevice *dev, char *out_name);

/**
 * acpi_copy_name() - Copy an ACPI name to an output buffer
 *
 * This convenience function can be used to return a literal string as a name
 * in functions that implement the get_name() method.
 *
 * For example:
 *
 *	static int mydev_get_name(const struct udevice *dev, char *out_name)
 *	{
 *		return acpi_copy_name(out_name, "WIBB");
 *	}
 *
 * @out_name: Place to put the name
 * @name: Name to copy
 * @return 0 (always)
 */
int acpi_copy_name(char *out_name, const char *name);

#endif

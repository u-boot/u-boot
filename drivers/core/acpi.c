// SPDX-License-Identifier: GPL-2.0+
/*
 * Core driver model support for ACPI table generation
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEOGRY	LOGC_ACPI

#include <common.h>
#include <dm.h>
#include <dm/acpi.h>
#include <dm/root.h>

int acpi_copy_name(char *out_name, const char *name)
{
	strncpy(out_name, name, ACPI_NAME_LEN);
	out_name[ACPI_NAME_LEN] = '\0';

	return 0;
}

int acpi_get_name(const struct udevice *dev, char *out_name)
{
	struct acpi_ops *aops;

	aops = device_get_acpi_ops(dev);
	if (aops && aops->get_name)
		return aops->get_name(dev, out_name);

	return -ENOSYS;
}

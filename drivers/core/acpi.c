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
#include <log.h>
#include <dm/acpi.h>
#include <dm/device-internal.h>
#include <dm/root.h>

/* Type of method to call */
enum method_t {
	METHOD_WRITE_TABLES,
};

/* Prototype for all methods */
typedef int (*acpi_method)(const struct udevice *dev, struct acpi_ctx *ctx);

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

acpi_method acpi_get_method(struct udevice *dev, enum method_t method)
{
	struct acpi_ops *aops;

	aops = device_get_acpi_ops(dev);
	if (aops) {
		switch (method) {
		case METHOD_WRITE_TABLES:
			return aops->write_tables;
		}
	}

	return NULL;
}

int acpi_recurse_method(struct acpi_ctx *ctx, struct udevice *parent,
			enum method_t method)
{
	struct udevice *dev;
	acpi_method func;
	int ret;

	func = acpi_get_method(parent, method);
	if (func) {
		log_debug("\n");
		log_debug("- %s %p\n", parent->name, func);
		ret = device_ofdata_to_platdata(parent);
		if (ret)
			return log_msg_ret("ofdata", ret);
		ret = func(parent, ctx);
		if (ret)
			return log_msg_ret("func", ret);
	}
	device_foreach_child(dev, parent) {
		ret = acpi_recurse_method(ctx, dev, method);
		if (ret)
			return log_msg_ret("recurse", ret);
	}

	return 0;
}

int acpi_write_dev_tables(struct acpi_ctx *ctx)
{
	int ret;

	log_debug("Writing device tables\n");
	ret = acpi_recurse_method(ctx, dm_root(), METHOD_WRITE_TABLES);
	log_debug("Writing finished, err=%d\n", ret);

	return ret;
}

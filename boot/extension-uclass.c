// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025 Köry Maincent <kory.maincent@bootlin.com>
 */

#include <alist.h>
#include <command.h>
#include <env.h>
#include <extension_board.h>
#include <fdt_support.h>
#include <malloc.h>
#include <mapmem.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass.h>

struct alist *extension_get_list(void)
{
	struct udevice *dev;

	if (uclass_first_device_err(UCLASS_EXTENSION, &dev))
		return NULL;

	return dev_get_priv(dev);
}

int extension_probe(struct udevice *dev)
{
	struct alist *extension_list = dev_get_priv(dev);

	alist_init_struct(extension_list, struct extension);
	return 0;
}

int extension_remove(struct udevice *dev)
{
	struct alist *extension_list = dev_get_priv(dev);

	alist_uninit(extension_list);
	return 0;
}

int extension_scan(void)
{
	struct alist *extension_list = extension_get_list();
	const struct extension_ops *ops;
	struct udevice *dev;
	int ret;

	ret = uclass_first_device_err(UCLASS_EXTENSION, &dev);
	if (ret)
		return ret;

	if (!extension_list)
		return -ENODEV;

	ops = extension_get_ops(dev);
	alist_empty(extension_list);
	return ops->scan(dev, extension_list);
}

int extension_apply(struct fdt_header *working_fdt, ulong size)
{
	struct fdt_header *blob;
	ulong overlay_addr;
	int ret;

	overlay_addr = env_get_hex("extension_overlay_addr", 0);
	if (!overlay_addr) {
		printf("Environment extension_overlay_addr is missing\n");
		return -EINVAL;
	}

	fdt_shrink_to_minimum(working_fdt, size);

	blob = map_sysmem(overlay_addr, 0);
	if (!fdt_valid(&blob)) {
		printf("Invalid overlay devicetree\n");
		return -EINVAL;
	}

	/* Apply method prints messages on error */
	ret = fdt_overlay_apply_verbose(working_fdt, blob);
	if (ret)
		printf("Failed to apply overlay\n");

	return ret;
}

UCLASS_DRIVER(extension) = {
	.name	= "extension",
	.id	= UCLASS_EXTENSION,
};

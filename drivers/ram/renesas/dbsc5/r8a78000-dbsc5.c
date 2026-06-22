// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Renesas Electronics Corp.
 *
 * Portions Copyright (C) 2026 Synopsys, Inc. Used with permission. All rights reserved.
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <errno.h>
#include <linux/sizes.h>
#include <ram.h>
#include "r8a78000-dbsc5.h"

static int renesas_dbsc5_probe(struct udevice *dev)
{
	struct udevice *ddev, *vdev, *edev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_RAM, "dbsc5_dram", &ddev);
	if (ret)
		return ret;

	ret = uclass_get_device_by_name(UCLASS_RAM, "ram@b8940000", &vdev);
	if (ret)
		return ret;

	ret = uclass_get_device_by_name(UCLASS_RAM, "dbsc5_ecc", &edev);
	if (ret)
		return ret;

	return 0;
}

int renesas_dbsc5_bind(struct udevice *dev)
{
	struct udevice *ramdev, *eccdev;
	struct driver *ramdrv, *eccdrv;
	int ret;

	ramdrv = lists_driver_lookup_name("dbsc5_dram");
	if (!ramdrv)
		return -ENOENT;

	eccdrv = lists_driver_lookup_name("dbsc5_ecc");
	if (!eccdrv)
		return -ENOENT;

	ret = device_bind_with_driver_data(dev, ramdrv, "dbsc5_dram",
					   dev_get_driver_data(dev),
					   dev_ofnode(dev), &ramdev);
	if (ret)
		return ret;

	ret = device_bind_with_driver_data(dev, eccdrv, "dbsc5_ecc", 0,
					   dev_ofnode(dev), &eccdev);
	if (ret)
		device_unbind(ramdev);

	return ret;
}

static const struct udevice_id renesas_dbsc5_ids[] = {
	{ .compatible = "renesas,r8a78000-dbsc", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(renesas_dbsc5) = {
	.name		= "dbsc5",
	.id		= UCLASS_NOP,
	.of_match	= renesas_dbsc5_ids,
	.bind		= renesas_dbsc5_bind,
	.probe		= renesas_dbsc5_probe,
};

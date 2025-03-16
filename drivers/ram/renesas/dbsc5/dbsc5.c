// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Renesas Electronics Corp.
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <errno.h>
#include <linux/sizes.h>
#include <ram.h>
#include "dbsc5.h"

static int renesas_dbsc5_probe(struct udevice *dev)
{
	struct udevice *pdev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_RAM, "dbsc5_dram", &pdev);
	if (ret)
		return ret;

	ret = uclass_get_device_by_name(UCLASS_NOP, "dbsc5_qos", &pdev);
	if (ret)
		return ret;

	return 0;
}

int renesas_dbsc5_bind(struct udevice *dev)
{
	struct udevice *ramdev, *qosdev;
	struct driver *ramdrv, *qosdrv;
	int ret;

	ramdrv = lists_driver_lookup_name("dbsc5_dram");
	if (!ramdrv)
		return -ENOENT;


	qosdrv = lists_driver_lookup_name("dbsc5_qos");
	if (!qosdrv)
		return -ENOENT;

	ret = device_bind_with_driver_data(dev, ramdrv, "dbsc5_dram",
					   dev_get_driver_data(dev),
					   dev_ofnode(dev), &ramdev);
	if (ret)
		return ret;

	ret = device_bind_with_driver_data(dev, qosdrv, "dbsc5_qos", 0,
					   dev_ofnode(dev), &qosdev);
	if (ret)
		device_unbind(ramdev);

	return ret;
}

struct renesas_dbsc5_data r8a779g0_dbsc5_data = {
	.clock_node = "renesas,r8a779g0-cpg-mssr",
	.reset_node = "renesas,r8a779g0-rst",
	.otp_node = "renesas,r8a779g0-otp",
};

static const struct udevice_id renesas_dbsc5_ids[] = {
	{
		.compatible = "renesas,r8a779g0-dbsc",
		.data = (ulong)&r8a779g0_dbsc5_data
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(renesas_dbsc5) = {
	.name		= "dbsc5",
	.id		= UCLASS_NOP,
	.of_match	= renesas_dbsc5_ids,
	.bind		= renesas_dbsc5_bind,
	.probe		= renesas_dbsc5_probe,
};

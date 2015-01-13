/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <ns16550.h>
#include <serial.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct udevice_id x86_serial_ids[] = {
	{ .compatible = "x86-uart" },
	{ }
};

static int x86_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct ns16550_platdata *plat = dev_get_platdata(dev);
	int ret;

	ret = ns16550_serial_ofdata_to_platdata(dev);
	if (ret)
		return ret;

	plat->clock = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
				     "clock-frequency", 1843200);

	return 0;
}

U_BOOT_DRIVER(serial_ns16550) = {
	.name	= "serial_x86",
	.id	= UCLASS_SERIAL,
	.of_match = x86_serial_ids,
	.ofdata_to_platdata = x86_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct ns16550_platdata),
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.probe = ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
};

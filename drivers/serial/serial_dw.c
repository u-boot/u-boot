/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <serial.h>

static const struct udevice_id dw_serial_ids[] = {
	{ .compatible = "snps,dw-apb-uart" },
	{ }
};

static int dw_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct ns16550_platdata *plat = dev_get_platdata(dev);
	int ret;

	ret = ns16550_serial_ofdata_to_platdata(dev);
	if (ret)
		return ret;
	plat->clock = CONFIG_SYS_NS16550_CLK;

	return 0;
}

U_BOOT_DRIVER(serial_ns16550) = {
	.name	= "serial_dw",
	.id	= UCLASS_SERIAL,
	.of_match = dw_serial_ids,
	.ofdata_to_platdata = dw_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct ns16550_platdata),
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.probe = ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
};

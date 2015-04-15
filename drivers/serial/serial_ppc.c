/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <serial.h>

static const struct udevice_id ppc_serial_ids[] = {
	{ .compatible = "ns16550" },
	{ }
};

static int ppc_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct ns16550_platdata *plat = dev_get_platdata(dev);
	int ret;

	ret = ns16550_serial_ofdata_to_platdata(dev);
	if (ret)
		return ret;
	plat->clock = get_serial_clock();

	return 0;
}

U_BOOT_DRIVER(serial_ns16550) = {
	.name	= "serial_ppc",
	.id	= UCLASS_SERIAL,
	.of_match = ppc_serial_ids,
	.ofdata_to_platdata = ppc_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct ns16550_platdata),
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.probe = ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

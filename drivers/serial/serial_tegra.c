/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <serial.h>

#ifdef CONFIG_OF_CONTROL
static const struct udevice_id tegra_serial_ids[] = {
	{ .compatible = "nvidia,tegra20-uart" },
	{ }
};

static int tegra_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct ns16550_platdata *plat = dev_get_platdata(dev);
	int ret;

	ret = ns16550_serial_ofdata_to_platdata(dev);
	if (ret)
		return ret;
	plat->clock = V_NS16550_CLK;

	return 0;
}
#else
struct ns16550_platdata tegra_serial = {
	.base = CONFIG_SYS_NS16550_COM1,
	.reg_shift = 2,
	.clock = V_NS16550_CLK,
};

U_BOOT_DEVICE(ns16550_serial) = {
	"serial_tegra20", &tegra_serial
};
#endif

U_BOOT_DRIVER(serial_ns16550) = {
	.name	= "serial_tegra20",
	.id	= UCLASS_SERIAL,
#ifdef CONFIG_OF_CONTROL
	.of_match = tegra_serial_ids,
	.ofdata_to_platdata = tegra_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct ns16550_platdata),
#endif
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.probe = ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

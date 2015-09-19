/*
 * Copyright (c) 2015 Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <ns16550.h>
#include <serial.h>
#include <asm/arch/clock.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id keystone_serial_ids[] = {
	{ .compatible = "ti,keystone-uart" },
	{ .compatible = "ns16550a" },
	{ }
};

static int keystone_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct ns16550_platdata *plat = dev_get_platdata(dev);
	int ret;

	ret = ns16550_serial_ofdata_to_platdata(dev);
	if (ret)
		return ret;
	plat->clock = CONFIG_SYS_NS16550_CLK;
	return 0;
}
#endif

U_BOOT_DRIVER(serial_keystone_ns16550) = {
	.name	= "serial_keystone",
	.id	= UCLASS_SERIAL,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match = of_match_ptr(keystone_serial_ids),
	.ofdata_to_platdata = of_match_ptr(keystone_serial_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct ns16550_platdata),
#endif
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.probe = ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

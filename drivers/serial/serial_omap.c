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

#ifdef CONFIG_OF_CONTROL
static const struct udevice_id omap_serial_ids[] = {
	{ .compatible = "ti,omap3-uart" },
	{ }
};

static int omap_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct ns16550_platdata *plat = dev_get_platdata(dev);
	int ret;

	ret = ns16550_serial_ofdata_to_platdata(dev);
	if (ret)
		return ret;
	plat->clock = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
				     "clock-frequency", -1);
	plat->reg_shift = 2;

	return 0;
}
#endif

U_BOOT_DRIVER(serial_omap_ns16550) = {
	.name	= "serial_omap",
	.id	= UCLASS_SERIAL,
	.of_match = of_match_ptr(omap_serial_ids),
	.ofdata_to_platdata = of_match_ptr(omap_serial_ofdata_to_platdata),
	.platdata_auto_alloc_size = sizeof(struct ns16550_platdata),
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.probe = ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};

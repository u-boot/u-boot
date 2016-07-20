/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <dm/root.h>
#include "pmc.h"

DECLARE_GLOBAL_DATA_PTR;

static int at91_pmc_bind(struct udevice *dev)
{
	return dm_scan_fdt_node(dev, gd->fdt_blob, dev->of_offset, false);
}

static const struct udevice_id at91_pmc_match[] = {
	{ .compatible = "atmel,sama5d2-pmc" },
	{}
};

U_BOOT_DRIVER(at91_pmc) = {
	.name = "at91-pmc-core",
	.id = UCLASS_CLK,
	.of_match = at91_pmc_match,
	.bind = at91_pmc_bind,
};

int at91_pmc_core_probe(struct udevice *dev)
{
	struct pmc_platdata *plat = dev_get_platdata(dev);

	dev = dev_get_parent(dev);

	plat->reg_base = (struct at91_pmc *)dev_get_addr_ptr(dev);

	return 0;
}

int at91_pmc_clk_node_bind(struct udevice *dev)
{
	const void *fdt = gd->fdt_blob;
	int offset = dev->of_offset;
	const char *name;
	int ret;

	for (offset = fdt_first_subnode(fdt, offset);
	     offset > 0;
	     offset = fdt_next_subnode(fdt, offset)) {
		name = fdt_get_name(fdt, offset, NULL);
		if (!name)
			return -EINVAL;

		ret = device_bind_driver_to_node(dev, "clk", name,
						 offset, NULL);
		if (ret)
			return ret;
	}

	return 0;
}

U_BOOT_DRIVER(clk_generic) = {
	.id	= UCLASS_CLK,
	.name	= "clk",
};

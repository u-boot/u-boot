/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/root.h>

DECLARE_GLOBAL_DATA_PTR;

static int at91_sckc_clk_bind(struct udevice *dev)
{
	return dm_scan_fdt_node(dev, gd->fdt_blob, dev->of_offset, false);
}

static const struct udevice_id at91_sckc_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-sckc" },
	{}
};

U_BOOT_DRIVER(at91_sckc_clk) = {
	.name = "at91_sckc_clk",
	.id = UCLASS_CLK,
	.of_match = at91_sckc_clk_match,
	.bind = at91_sckc_clk_bind,
};

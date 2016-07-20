/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <linux/io.h>
#include <mach/at91_pmc.h>
#include "pmc.h"

#define SYSTEM_MAX_ID		31

static inline int is_pck(int id)
{
	return (id >= 8) && (id <= 15);
}

static int at91_system_clk_enable(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	u32 mask;

	if (clk->id > SYSTEM_MAX_ID)
		return -EINVAL;

	mask = BIT(clk->id);

	writel(mask, &pmc->scer);

	/**
	 * For the programmable clocks the Ready status in the PMC
	 * status register should be checked after enabling.
	 * For other clocks this is unnecessary.
	 */
	if (!is_pck(clk->id))
		return 0;

	while (!(readl(&pmc->sr) & mask))
		;

	return 0;
}

static struct clk_ops at91_system_clk_ops = {
	.enable = at91_system_clk_enable,
};

static int at91_system_clk_bind(struct udevice *dev)
{
	return at91_pmc_clk_node_bind(dev);
}

static int at91_system_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id at91_system_clk_match[] = {
	{ .compatible = "atmel,at91rm9200-clk-system" },
	{}
};

U_BOOT_DRIVER(at91_system_clk) = {
	.name = "at91-system-clk",
	.id = UCLASS_CLK,
	.of_match = at91_system_clk_match,
	.bind = at91_system_clk_bind,
	.probe = at91_system_clk_probe,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &at91_system_clk_ops,
};

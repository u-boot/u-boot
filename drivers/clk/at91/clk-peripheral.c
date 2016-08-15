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

#define PERIPHERAL_ID_MIN	2
#define PERIPHERAL_ID_MAX	31
#define PERIPHERAL_MASK(id)	(1 << ((id) & PERIPHERAL_ID_MAX))

static int sam9x5_periph_clk_enable(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;

	if (clk->id < PERIPHERAL_ID_MIN)
		return -1;

	writel(clk->id & AT91_PMC_PCR_PID_MASK, &pmc->pcr);
	setbits_le32(&pmc->pcr, AT91_PMC_PCR_CMD_WRITE | AT91_PMC_PCR_EN);

	return 0;
}

static struct clk_ops sam9x5_periph_clk_ops = {
	.enable = sam9x5_periph_clk_enable,
};

static int sam9x5_periph_clk_bind(struct udevice *dev)
{
	return at91_pmc_clk_node_bind(dev);
}

static int sam9x5_periph_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id sam9x5_periph_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-peripheral" },
	{}
};

U_BOOT_DRIVER(sam9x5_periph_clk) = {
	.name = "sam9x5-periph-clk",
	.id = UCLASS_CLK,
	.of_match = sam9x5_periph_clk_match,
	.bind = sam9x5_periph_clk_bind,
	.probe = sam9x5_periph_clk_probe,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &sam9x5_periph_clk_ops,
};

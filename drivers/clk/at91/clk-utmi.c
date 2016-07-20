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

DECLARE_GLOBAL_DATA_PTR;

#define UTMI_FIXED_MUL		40

static int utmi_clk_enable(struct clk *clk)
{
	struct pmc_platdata *plat = dev_get_platdata(clk->dev);
	struct at91_pmc *pmc = plat->reg_base;
	u32 tmp;

	if (readl(&pmc->sr) & AT91_PMC_LOCKU)
		return 0;

	tmp = readl(&pmc->uckr);
	tmp |= AT91_PMC_UPLLEN |
	       AT91_PMC_UPLLCOUNT |
	       AT91_PMC_BIASEN;
	writel(tmp, &pmc->uckr);

	while (!(readl(&pmc->sr) & AT91_PMC_LOCKU))
		;

	return 0;
}

static ulong utmi_clk_get_rate(struct clk *clk)
{
	return gd->arch.main_clk_rate_hz * UTMI_FIXED_MUL;
}

static struct clk_ops utmi_clk_ops = {
	.enable = utmi_clk_enable,
	.get_rate = utmi_clk_get_rate,
};

static int utmi_clk_probe(struct udevice *dev)
{
	return at91_pmc_core_probe(dev);
}

static const struct udevice_id utmi_clk_match[] = {
	{ .compatible = "atmel,at91sam9x5-clk-utmi" },
	{}
};

U_BOOT_DRIVER(at91sam9x5_utmi_clk) = {
	.name = "at91sam9x5-utmi-clk",
	.id = UCLASS_CLK,
	.of_match = utmi_clk_match,
	.probe = utmi_clk_probe,
	.platdata_auto_alloc_size = sizeof(struct pmc_platdata),
	.ops = &utmi_clk_ops,
};

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Author: Greg Malysa <greg.malysa@timesys.com>
 *
 * Ported from Linux: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 */

#include <clk.h>
#include <clk-uclass.h>
#include <asm/io.h>
#include <dm/device.h>
#include <linux/compiler_types.h>
#include <linux/err.h>
#include <linux/kernel.h>

#include "clk.h"

#define ADI_CLK_PLL_GENERIC "adi_clk_pll_generic"

struct clk_sc5xx_cgu_pll {
	struct clk clk;
	void __iomem *base;
	u32 mask;
	u32 max;
	u32 m_offset;
	u8 shift;
	bool half_m;
};

#define to_clk_sc5xx_cgu_pll(_clk) container_of(_clk, struct clk_sc5xx_cgu_pll, clk)

static unsigned long sc5xx_cgu_pll_get_rate(struct clk *clk)
{
	struct clk_sc5xx_cgu_pll *pll = to_clk_sc5xx_cgu_pll(dev_get_clk_ptr(clk->dev));
	unsigned long parent_rate = clk_get_parent_rate(clk);

	u32 reg = readl(pll->base);
	u32 m = ((reg & pll->mask) >> pll->shift) + pll->m_offset;

	if (m == 0)
		m = pll->max;

	if (pll->half_m)
		return parent_rate * m * 2;
	return parent_rate * m;
}

static const struct clk_ops clk_sc5xx_cgu_pll_ops = {
	.get_rate = sc5xx_cgu_pll_get_rate,
};

struct clk *sc5xx_cgu_pll(const char *name, const char *parent_name,
			  void __iomem *base, u8 shift, u8 width, u32 m_offset,
			  bool half_m)
{
	struct clk_sc5xx_cgu_pll *pll;
	struct clk *clk;
	int ret;
	char *drv_name = ADI_CLK_PLL_GENERIC;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	pll->base = base;
	pll->shift = shift;
	pll->mask = GENMASK(width - 1, 0) << shift;
	pll->max = pll->mask + 1;
	pll->m_offset = m_offset;
	pll->half_m = half_m;

	clk = &pll->clk;

	ret = clk_register(clk, drv_name, name, parent_name);
	if (ret) {
		pr_err("Failed to register %s in %s: %d\n", name, __func__, ret);
		kfree(pll);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(clk_adi_pll_generic) = {
	.name	= ADI_CLK_PLL_GENERIC,
	.id	= UCLASS_CLK,
	.ops	= &clk_sc5xx_cgu_pll_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

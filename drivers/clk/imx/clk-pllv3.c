// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 */

#include <common.h>
#include <asm/io.h>
#include <malloc.h>
#include <clk-uclass.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <clk.h>
#include "clk.h"

#define UBOOT_DM_CLK_IMX_PLLV3 "imx_clk_pllv3"

struct clk_pllv3 {
	struct clk	clk;
	void __iomem	*base;
	u32		div_mask;
	u32		div_shift;
};

#define to_clk_pllv3(_clk) container_of(_clk, struct clk_pllv3, clk)

static ulong clk_pllv3_get_rate(struct clk *clk)
{
	struct clk_pllv3 *pll = to_clk_pllv3(dev_get_clk_ptr(clk->dev));
	unsigned long parent_rate = clk_get_parent_rate(clk);

	u32 div = (readl(pll->base) >> pll->div_shift) & pll->div_mask;

	return (div == 1) ? parent_rate * 22 : parent_rate * 20;
}

static const struct clk_ops clk_pllv3_generic_ops = {
	.get_rate       = clk_pllv3_get_rate,
};

struct clk *imx_clk_pllv3(enum imx_pllv3_type type, const char *name,
			  const char *parent_name, void __iomem *base,
			  u32 div_mask)
{
	struct clk_pllv3 *pll;
	struct clk *clk;
	char *drv_name;
	int ret;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	switch (type) {
	case IMX_PLLV3_GENERIC:
	case IMX_PLLV3_USB:
		drv_name = UBOOT_DM_CLK_IMX_PLLV3;
		break;
	default:
		kfree(pll);
		return ERR_PTR(-ENOTSUPP);
	}

	pll->base = base;
	pll->div_mask = div_mask;
	clk = &pll->clk;

	ret = clk_register(clk, drv_name, name, parent_name);
	if (ret) {
		kfree(pll);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(clk_pllv3_generic) = {
	.name	= UBOOT_DM_CLK_IMX_PLLV3,
	.id	= UCLASS_CLK,
	.ops	= &clk_pllv3_generic_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

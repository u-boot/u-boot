// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Common Clock Framework [CCF] driver for Sandbox
 */

#include <common.h>
#include <dm.h>
#include <clk.h>
#include <asm/clk.h>
#include <clk-uclass.h>
#include <linux/clk-provider.h>
#include <sandbox-clk.h>

/*
 * Sandbox implementation of CCF primitives necessary for clk-uclass testing
 *
 * --- Sandbox PLLv3 ---
 */
struct clk_pllv3 {
	struct clk	clk;
	u32		div_mask;
	u32		div_shift;
};

static ulong clk_pllv3_get_rate(struct clk *clk)
{
	unsigned long parent_rate = clk_get_parent_rate(clk);

	return parent_rate * 24;
}

static const struct clk_ops clk_pllv3_generic_ops = {
	.get_rate       = clk_pllv3_get_rate,
};

struct clk *sandbox_clk_pllv3(enum sandbox_pllv3_type type, const char *name,
			      const char *parent_name, void __iomem *base,
			      u32 div_mask)
{
	struct clk_pllv3 *pll;
	struct clk *clk;
	char *drv_name = "sandbox_clk_pllv3";
	int ret;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	pll->div_mask = div_mask;
	clk = &pll->clk;

	ret = clk_register(clk, drv_name, name, parent_name);
	if (ret) {
		kfree(pll);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(sandbox_clk_pll_generic) = {
	.name	= "sandbox_clk_pllv3",
	.id	= UCLASS_CLK,
	.ops	= &clk_pllv3_generic_ops,
};

/* --- Sandbox PLLv3 --- */
/* --- Sandbox Gate  --- */
struct clk_gate2 {
	struct clk clk;
	bool	state;
};

#define to_clk_gate2(_clk) container_of(_clk, struct clk_gate2, clk)

static int clk_gate2_enable(struct clk *clk)
{
	struct clk_gate2 *gate = to_clk_gate2(dev_get_clk_ptr(clk->dev));

	gate->state = 1;
	return 0;
}

static int clk_gate2_disable(struct clk *clk)
{
	struct clk_gate2 *gate = to_clk_gate2(dev_get_clk_ptr(clk->dev));

	gate->state = 0;
	return 0;
}

static const struct clk_ops clk_gate2_ops = {
	.enable = clk_gate2_enable,
	.disable = clk_gate2_disable,
	.get_rate = clk_generic_get_rate,
};

struct clk *sandbox_clk_register_gate2(struct device *dev, const char *name,
				       const char *parent_name,
				       unsigned long flags, void __iomem *reg,
				       u8 bit_idx, u8 cgr_val,
				       u8 clk_gate2_flags)
{
	struct clk_gate2 *gate;
	struct clk *clk;
	int ret;

	gate = kzalloc(sizeof(*gate), GFP_KERNEL);
	if (!gate)
		return ERR_PTR(-ENOMEM);

	gate->state = 0;
	clk = &gate->clk;

	ret = clk_register(clk, "sandbox_clk_gate2", name, parent_name);
	if (ret) {
		kfree(gate);
		return ERR_PTR(ret);
	}

	return clk;
}

U_BOOT_DRIVER(sandbox_clk_gate2) = {
	.name	= "sandbox_clk_gate2",
	.id	= UCLASS_CLK,
	.ops	= &clk_gate2_ops,
};

/* --- Sandbox Gate --- */
/* The CCF core driver itself */
static const struct udevice_id sandbox_clk_ccf_test_ids[] = {
	{ .compatible = "sandbox,clk-ccf" },
	{ }
};

static const char *const usdhc_sels[] = { "pll3_60m", "pll3_80m", };

static int sandbox_clk_ccf_probe(struct udevice *dev)
{
	void *base = NULL;
	u32 reg;

	clk_dm(SANDBOX_CLK_PLL3,
	       sandbox_clk_pllv3(SANDBOX_PLLV3_USB, "pll3_usb_otg", "osc",
				 base + 0x10, 0x3));

	clk_dm(SANDBOX_CLK_PLL3_60M,
	       sandbox_clk_fixed_factor("pll3_60m",  "pll3_usb_otg",   1, 8));

	clk_dm(SANDBOX_CLK_PLL3_80M,
	       sandbox_clk_fixed_factor("pll3_80m",  "pll3_usb_otg",   1, 6));

	/* The HW adds +1 to the divider value (2+1) is the divider */
	reg = (2 << 19);
	clk_dm(SANDBOX_CLK_ECSPI_ROOT,
	       sandbox_clk_divider("ecspi_root", "pll3_60m", &reg, 19, 6));

	clk_dm(SANDBOX_CLK_ECSPI1,
	       sandbox_clk_gate2("ecspi1", "ecspi_root", base + 0x6c, 0));

	/* Select 'pll3_60m' */
	reg = 0;
	clk_dm(SANDBOX_CLK_USDHC1_SEL,
	       sandbox_clk_mux("usdhc1_sel", &reg, 16, 1, usdhc_sels,
			       ARRAY_SIZE(usdhc_sels)));

	/* Select 'pll3_80m' */
	reg = BIT(17);
	clk_dm(SANDBOX_CLK_USDHC2_SEL,
	       sandbox_clk_mux("usdhc2_sel", &reg, 17, 1, usdhc_sels,
			       ARRAY_SIZE(usdhc_sels)));

	return 0;
}

U_BOOT_DRIVER(sandbox_clk_ccf) = {
	.name = "sandbox_clk_ccf",
	.id = UCLASS_CLK,
	.probe = sandbox_clk_ccf_probe,
	.of_match = sandbox_clk_ccf_test_ids,
};

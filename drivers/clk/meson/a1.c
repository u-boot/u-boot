// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2023 SberDevices, Inc.
 * Author: Igor Prusov <ivprusov@salutedevices.com>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <regmap.h>
#include <asm/arch/clock-a1.h>
#include <dt-bindings/clock/amlogic,a1-pll-clkc.h>
#include <dt-bindings/clock/amlogic,a1-peripherals-clkc.h>
#include "clk_meson.h"

/*
 * This driver supports both PLL and peripherals clock sources.
 * Following operations are supported:
 * - calculating clock frequency on a limited tree
 * - reading muxes and dividers
 * - enabling/disabling gates without propagation
 * - reparenting without rate propagation, only on muxes
 * - setting rates with limited reparenting, only on dividers with mux parent
 */

#define NR_CLKS				154
#define NR_PLL_CLKS			11

/* External clock IDs. Those should not overlap with regular IDs */
#define EXTERNAL_XTAL			(NR_CLKS + 0)
#define EXTERNAL_FCLK_DIV2		(NR_CLKS + 1)
#define EXTERNAL_FCLK_DIV3		(NR_CLKS + 2)
#define EXTERNAL_FCLK_DIV5		(NR_CLKS + 3)
#define EXTERNAL_FCLK_DIV7		(NR_CLKS + 4)

#define EXTERNAL_FIXPLL_IN		(NR_PLL_CLKS + 1)

/* A1 peripherals clocks */
static const struct meson_clk_info *meson_clocks[] = {
	[CLKID_SPIFC_SEL] = CLK_MUX("spifc_sel", A1_SPIFC_CLK_CTRL, 9, 2, {
		EXTERNAL_FCLK_DIV2,
		EXTERNAL_FCLK_DIV3,
		EXTERNAL_FCLK_DIV5,
		-ENOENT,
	}),
	[CLKID_SPIFC_SEL2] = CLK_MUX("spifc_sel2", A1_SPIFC_CLK_CTRL, 15, 1, {
		CLKID_SPIFC_DIV,
		EXTERNAL_XTAL,
	}),
	[CLKID_USB_BUS_SEL] = CLK_MUX("usb_bus_sel", A1_USB_BUSCLK_CTRL, 9, 2, {
		-ENOENT,
		CLKID_SYS,
		EXTERNAL_FCLK_DIV3,
		EXTERNAL_FCLK_DIV5,
	}),
	[CLKID_SYS] = CLK_MUX("sys", A1_SYS_CLK_CTRL0, 31, 1, {
		CLKID_SYS_A,
		CLKID_SYS_B,
	}),
	[CLKID_SYS_A_SEL] = CLK_MUX("sys_a_sel", A1_SYS_CLK_CTRL0, 10, 3, {
		-ENOENT,
		EXTERNAL_FCLK_DIV2,
		EXTERNAL_FCLK_DIV3,
		EXTERNAL_FCLK_DIV5,
		-ENOENT,
		-ENOENT,
		-ENOENT,
		-ENOENT,
	}),
	[CLKID_SYS_B_SEL] = CLK_MUX("sys_b_sel", A1_SYS_CLK_CTRL0, 26, 3, {
		-ENOENT,
		EXTERNAL_FCLK_DIV2,
		EXTERNAL_FCLK_DIV3,
		EXTERNAL_FCLK_DIV5,
		-ENOENT,
		-ENOENT,
		-ENOENT,
		-ENOENT,
	}),

	[CLKID_SPIFC_DIV] = CLK_DIV("spifc_div", A1_SPIFC_CLK_CTRL, 0, 8,
		CLKID_SPIFC_SEL
	),
	[CLKID_USB_BUS_DIV] = CLK_DIV("usb_bus_div", A1_USB_BUSCLK_CTRL, 0, 8,
		CLKID_USB_BUS_SEL
	),
	[CLKID_SYS_A_DIV] = CLK_DIV("sys_a_div", A1_SYS_CLK_CTRL0, 0, 10,
		CLKID_SYS_A_SEL
	),
	[CLKID_SYS_B_DIV] = CLK_DIV("sys_b_div", A1_SYS_CLK_CTRL0, 16, 10,
		CLKID_SYS_B_SEL
	),

	[CLKID_SPIFC] = CLK_GATE("spifc", A1_SPIFC_CLK_CTRL, 8,
		CLKID_SPIFC_SEL2
	),
	[CLKID_USB_BUS] = CLK_GATE("usb_bus", A1_USB_BUSCLK_CTRL, 8,
		CLKID_USB_BUS_DIV
	),
	[CLKID_SYS_A] = CLK_GATE("sys_a", A1_SYS_CLK_CTRL0, 13,
		CLKID_SYS_A_DIV
	),
	[CLKID_SYS_B] = CLK_GATE("sys_b", A1_SYS_CLK_CTRL0, 29,
		CLKID_SYS_B_DIV
	),
	[CLKID_FIXPLL_IN] = CLK_GATE("fixpll_in", A1_SYS_OSCIN_CTRL, 1,
		EXTERNAL_XTAL
	),
	[CLKID_USB_PHY_IN] = CLK_GATE("usb_phy_in", A1_SYS_OSCIN_CTRL, 2,
		EXTERNAL_XTAL
	),
	[CLKID_USB_CTRL_IN] = CLK_GATE("usb_ctrl_in", A1_SYS_OSCIN_CTRL, 3,
		EXTERNAL_XTAL
	),
	[CLKID_USB_CTRL] = CLK_GATE("usb_ctrl", A1_SYS_CLK_EN0, 28,
		CLKID_SYS
	),
	[CLKID_USB_PHY] = CLK_GATE("usb_phy", A1_SYS_CLK_EN0, 27,
		CLKID_SYS
	),
	[CLKID_SARADC] = CLK_GATE("saradc", A1_SAR_ADC_CLK_CTR, 8,
		-ENOENT
	),
	[CLKID_SARADC_EN] = CLK_GATE("saradc_en", A1_SYS_CLK_EN0, 13,
		CLKID_SYS
	),

	[EXTERNAL_XTAL] = CLK_EXTERNAL("xtal"),
	[EXTERNAL_FCLK_DIV2] = CLK_EXTERNAL("fclk_div2"),
	[EXTERNAL_FCLK_DIV3] = CLK_EXTERNAL("fclk_div3"),
	[EXTERNAL_FCLK_DIV5] = CLK_EXTERNAL("fclk_div5"),
	[EXTERNAL_FCLK_DIV7] = CLK_EXTERNAL("fclk_div7"),
};

/* A1 PLL clocks */
static const struct meson_clk_info *meson_pll_clocks[] = {
	[EXTERNAL_FIXPLL_IN] = CLK_EXTERNAL("fixpll_in"),

	[CLKID_FIXED_PLL_DCO] = CLK_PLL("fixed_pll_dco", EXTERNAL_FIXPLL_IN, {
			{A1_ANACTRL_FIXPLL_CTRL0, 0, 8},
			{A1_ANACTRL_FIXPLL_CTRL0, 10, 5},
	}),

	[CLKID_FCLK_DIV2_DIV] = CLK_DIV_FIXED("fclk_div2_div", 2,
		CLKID_FIXED_PLL
	),
	[CLKID_FCLK_DIV3_DIV] = CLK_DIV_FIXED("fclk_div3_div", 3,
		CLKID_FIXED_PLL
	),
	[CLKID_FCLK_DIV5_DIV] = CLK_DIV_FIXED("fclk_div5_div", 5,
		CLKID_FIXED_PLL
	),
	[CLKID_FCLK_DIV7_DIV] = CLK_DIV_FIXED("fclk_div7_div", 7,
		CLKID_FIXED_PLL
	),

	[CLKID_FIXED_PLL] = CLK_GATE("fixed_pll", A1_ANACTRL_FIXPLL_CTRL0, 20,
		CLKID_FIXED_PLL_DCO
	),
	[CLKID_FCLK_DIV2] = CLK_GATE("fclk_div2", A1_ANACTRL_FIXPLL_CTRL0, 21,
		CLKID_FCLK_DIV2_DIV
	),
	[CLKID_FCLK_DIV3] = CLK_GATE("fclk_div3", A1_ANACTRL_FIXPLL_CTRL0, 22,
		CLKID_FCLK_DIV3_DIV
	),
	[CLKID_FCLK_DIV5] = CLK_GATE("fclk_div5", A1_ANACTRL_FIXPLL_CTRL0, 23,
		CLKID_FCLK_DIV5_DIV
	),
	[CLKID_FCLK_DIV7] = CLK_GATE("fclk_div7", A1_ANACTRL_FIXPLL_CTRL0, 24,
		CLKID_FCLK_DIV7_DIV
	),
};

/* Rate propagation is implemented for a subcection of a clock tree, that is
 * required at boot stage.
 */
static ulong meson_clk_set_rate(struct clk *clk, ulong rate)
{
	switch (clk->id) {
	case CLKID_SPIFC_DIV:
	case CLKID_USB_BUS_DIV:
		return meson_composite_set_rate(clk, rate);
	case CLKID_SPIFC:
	case CLKID_USB_BUS: {
		struct clk parent = {
			.dev = clk->dev,
			.id = meson_clk_get_parent(clk),
		};

		return meson_clk_set_rate(&parent, rate);
	}
	case CLKID_SPIFC_SEL2:
		return meson_mux_set_rate(clk, rate);
	}

	return -EINVAL;
}

static int meson_clk_probe(struct udevice *dev)
{
	struct meson_clk *priv = dev_get_priv(dev);

	return regmap_init_mem(dev_ofnode(dev), &priv->map);
}

struct meson_clk_data meson_a1_peripherals_info = {
	.clocks = meson_clocks,
	.num_clocks = ARRAY_SIZE(meson_clocks),
};

struct meson_clk_data meson_a1_pll_info = {
	.clocks = meson_pll_clocks,
	.num_clocks = ARRAY_SIZE(meson_pll_clocks),
};

static const struct udevice_id meson_clk_ids[] = {
	{
		.compatible = "amlogic,a1-peripherals-clkc",
		.data = (ulong)&meson_a1_peripherals_info,
	},
	{
		.compatible = "amlogic,a1-pll-clkc",
		.data = (ulong)&meson_a1_pll_info,
	},
	{ }
};

static struct clk_ops meson_clk_ops = {
	.disable	= meson_clk_disable,
	.enable		= meson_clk_enable,
	.get_rate	= meson_clk_get_rate,
	.set_rate	= meson_clk_set_rate,
	.set_parent	= meson_clk_set_parent,
#if IS_ENABLED(CONFIG_CMD_CLK)
	.dump		= meson_clk_dump,
#endif
};

U_BOOT_DRIVER(meson_clk_a1) = {
	.name		= "meson-clk-a1",
	.id		= UCLASS_CLK,
	.of_match	= meson_clk_ids,
	.priv_auto	= sizeof(struct meson_clk),
	.ops		= &meson_clk_ops,
	.probe		= meson_clk_probe,
};

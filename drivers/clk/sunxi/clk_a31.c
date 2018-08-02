// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2018 Amarula Solutions B.V.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/arch/ccu.h>
#include <dt-bindings/clock/sun6i-a31-ccu.h>
#include <dt-bindings/reset/sun6i-a31-ccu.h>

static struct ccu_clk_gate a31_gates[] = {
	[CLK_AHB1_OTG]		= GATE(0x060, BIT(24)),
	[CLK_AHB1_EHCI0]	= GATE(0x060, BIT(26)),
	[CLK_AHB1_EHCI1]	= GATE(0x060, BIT(27)),
	[CLK_AHB1_OHCI0]	= GATE(0x060, BIT(29)),
	[CLK_AHB1_OHCI1]	= GATE(0x060, BIT(30)),
	[CLK_AHB1_OHCI2]	= GATE(0x060, BIT(31)),

	[CLK_USB_PHY0]		= GATE(0x0cc, BIT(8)),
	[CLK_USB_PHY1]		= GATE(0x0cc, BIT(9)),
	[CLK_USB_PHY2]		= GATE(0x0cc, BIT(10)),
	[CLK_USB_OHCI0]		= GATE(0x0cc, BIT(16)),
	[CLK_USB_OHCI1]		= GATE(0x0cc, BIT(17)),
	[CLK_USB_OHCI2]		= GATE(0x0cc, BIT(18)),
};

static struct ccu_reset a31_resets[] = {
	[RST_USB_PHY0]		= RESET(0x0cc, BIT(0)),
	[RST_USB_PHY1]		= RESET(0x0cc, BIT(1)),
	[RST_USB_PHY2]		= RESET(0x0cc, BIT(2)),

	[RST_AHB1_OTG]		= RESET(0x2c0, BIT(24)),
	[RST_AHB1_EHCI0]	= RESET(0x2c0, BIT(26)),
	[RST_AHB1_EHCI1]	= RESET(0x2c0, BIT(27)),
	[RST_AHB1_OHCI0]	= RESET(0x2c0, BIT(29)),
	[RST_AHB1_OHCI1]	= RESET(0x2c0, BIT(30)),
	[RST_AHB1_OHCI2]	= RESET(0x2c0, BIT(31)),
};

static const struct ccu_desc a31_ccu_desc = {
	.gates = a31_gates,
	.resets = a31_resets,
};

static int a31_clk_bind(struct udevice *dev)
{
	return sunxi_reset_bind(dev, ARRAY_SIZE(a31_resets));
}

static const struct udevice_id a31_clk_ids[] = {
	{ .compatible = "allwinner,sun6i-a31-ccu",
	  .data = (ulong)&a31_ccu_desc },
	{ }
};

U_BOOT_DRIVER(clk_sun6i_a31) = {
	.name		= "sun6i_a31_ccu",
	.id		= UCLASS_CLK,
	.of_match	= a31_clk_ids,
	.priv_auto_alloc_size	= sizeof(struct ccu_priv),
	.ops		= &sunxi_clk_ops,
	.probe		= sunxi_clk_probe,
	.bind		= a31_clk_bind,
};

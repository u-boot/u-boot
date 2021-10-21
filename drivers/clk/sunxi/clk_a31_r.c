// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) Samuel Holland <samuel@sholland.org>
 */

#include <clk-uclass.h>
#include <dm.h>
#include <clk/sunxi.h>
#include <dt-bindings/clock/sun8i-r-ccu.h>
#include <dt-bindings/reset/sun8i-r-ccu.h>
#include <linux/bitops.h>

static struct ccu_clk_gate a31_r_gates[] = {
	[CLK_APB0_PIO]		= GATE(0x028, BIT(0)),
	[CLK_APB0_IR]		= GATE(0x028, BIT(1)),
	[CLK_APB0_TIMER]	= GATE(0x028, BIT(2)),
	[CLK_APB0_RSB]		= GATE(0x028, BIT(3)),
	[CLK_APB0_UART]		= GATE(0x028, BIT(4)),
	[CLK_APB0_I2C]		= GATE(0x028, BIT(6)),
	[CLK_APB0_TWD]		= GATE(0x028, BIT(7)),
};

static struct ccu_reset a31_r_resets[] = {
	[RST_APB0_IR]		= RESET(0x0b0, BIT(1)),
	[RST_APB0_TIMER]	= RESET(0x0b0, BIT(2)),
	[RST_APB0_RSB]		= RESET(0x0b0, BIT(3)),
	[RST_APB0_UART]		= RESET(0x0b0, BIT(4)),
	[RST_APB0_I2C]		= RESET(0x0b0, BIT(6)),
};

static const struct ccu_desc a31_r_ccu_desc = {
	.gates = a31_r_gates,
	.resets = a31_r_resets,
};

static int a31_r_clk_bind(struct udevice *dev)
{
	return sunxi_reset_bind(dev, ARRAY_SIZE(a31_r_resets));
}

static const struct udevice_id a31_r_clk_ids[] = {
	{ .compatible = "allwinner,sun8i-a83t-r-ccu",
	  .data = (ulong)&a31_r_ccu_desc },
	{ .compatible = "allwinner,sun8i-h3-r-ccu",
	  .data = (ulong)&a31_r_ccu_desc },
	{ .compatible = "allwinner,sun50i-a64-r-ccu",
	  .data = (ulong)&a31_r_ccu_desc },
	{ }
};

U_BOOT_DRIVER(clk_sun6i_a31_r) = {
	.name		= "sun6i_a31_r_ccu",
	.id		= UCLASS_CLK,
	.of_match	= a31_r_clk_ids,
	.priv_auto	= sizeof(struct ccu_priv),
	.ops		= &sunxi_clk_ops,
	.probe		= sunxi_clk_probe,
	.bind		= a31_r_clk_bind,
};

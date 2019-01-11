// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Amarula Solutions.
 * Author: Jagan Teki <jagan@amarulasolutions.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/arch/ccu.h>
#include <dt-bindings/clock/sun9i-a80-ccu.h>
#include <dt-bindings/reset/sun9i-a80-ccu.h>

static const struct ccu_clk_gate a80_gates[] = {
	[CLK_BUS_UART0]		= GATE(0x594, BIT(16)),
	[CLK_BUS_UART1]		= GATE(0x594, BIT(17)),
	[CLK_BUS_UART2]		= GATE(0x594, BIT(18)),
	[CLK_BUS_UART3]		= GATE(0x594, BIT(19)),
	[CLK_BUS_UART4]		= GATE(0x594, BIT(20)),
	[CLK_BUS_UART5]		= GATE(0x594, BIT(21)),
};

static const struct ccu_reset a80_resets[] = {
	[RST_BUS_UART0]		= RESET(0x5b4, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x5b4, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x5b4, BIT(18)),
	[RST_BUS_UART3]		= RESET(0x5b4, BIT(19)),
	[RST_BUS_UART4]		= RESET(0x5b4, BIT(20)),
	[RST_BUS_UART5]		= RESET(0x5b4, BIT(21)),
};

static const struct ccu_desc a80_ccu_desc = {
	.gates = a80_gates,
	.resets = a80_resets,
};

static int a80_clk_bind(struct udevice *dev)
{
	return sunxi_reset_bind(dev, ARRAY_SIZE(a80_resets));
}

static const struct udevice_id a80_ccu_ids[] = {
	{ .compatible = "allwinner,sun9i-a80-ccu",
	  .data = (ulong)&a80_ccu_desc },
	{ }
};

U_BOOT_DRIVER(clk_sun9i_a80) = {
	.name		= "sun9i_a80_ccu",
	.id		= UCLASS_CLK,
	.of_match	= a80_ccu_ids,
	.priv_auto_alloc_size	= sizeof(struct ccu_priv),
	.ops		= &sunxi_clk_ops,
	.probe		= sunxi_clk_probe,
	.bind		= a80_clk_bind,
};

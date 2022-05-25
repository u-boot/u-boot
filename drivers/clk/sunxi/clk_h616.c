// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2021 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <clk/sunxi.h>
#include <dt-bindings/clock/sun50i-h616-ccu.h>
#include <dt-bindings/reset/sun50i-h616-ccu.h>
#include <linux/bitops.h>

static struct ccu_clk_gate h616_gates[] = {
	[CLK_PLL_PERIPH0]	= GATE(0x020, BIT(31) | BIT(27)),

	[CLK_APB1]		= GATE_DUMMY,

	[CLK_BUS_MMC0]		= GATE(0x84c, BIT(0)),
	[CLK_BUS_MMC1]		= GATE(0x84c, BIT(1)),
	[CLK_BUS_MMC2]		= GATE(0x84c, BIT(2)),

	[CLK_BUS_UART0]		= GATE(0x90c, BIT(0)),
	[CLK_BUS_UART1]		= GATE(0x90c, BIT(1)),
	[CLK_BUS_UART2]		= GATE(0x90c, BIT(2)),
	[CLK_BUS_UART3]		= GATE(0x90c, BIT(3)),
	[CLK_BUS_UART4]		= GATE(0x90c, BIT(4)),
	[CLK_BUS_UART5]		= GATE(0x90c, BIT(5)),

	[CLK_BUS_I2C0]		= GATE(0x91c, BIT(0)),
	[CLK_BUS_I2C1]		= GATE(0x91c, BIT(1)),
	[CLK_BUS_I2C2]		= GATE(0x91c, BIT(2)),
	[CLK_BUS_I2C3]		= GATE(0x91c, BIT(3)),
	[CLK_BUS_I2C4]		= GATE(0x91c, BIT(4)),

	[CLK_SPI0]		= GATE(0x940, BIT(31)),
	[CLK_SPI1]		= GATE(0x944, BIT(31)),

	[CLK_BUS_SPI0]		= GATE(0x96c, BIT(0)),
	[CLK_BUS_SPI1]		= GATE(0x96c, BIT(1)),

	[CLK_BUS_EMAC0]		= GATE(0x97c, BIT(0)),
	[CLK_BUS_EMAC1]		= GATE(0x97c, BIT(1)),

	[CLK_USB_PHY0]		= GATE(0xa70, BIT(29)),
	[CLK_USB_OHCI0]		= GATE(0xa70, BIT(31)),

	[CLK_USB_PHY1]		= GATE(0xa74, BIT(29)),
	[CLK_USB_OHCI1]		= GATE(0xa74, BIT(31)),

	[CLK_USB_PHY2]		= GATE(0xa78, BIT(29)),
	[CLK_USB_OHCI2]		= GATE(0xa78, BIT(31)),

	[CLK_USB_PHY3]		= GATE(0xa7c, BIT(29)),
	[CLK_USB_OHCI3]		= GATE(0xa7c, BIT(31)),

	[CLK_BUS_OHCI0]		= GATE(0xa8c, BIT(0)),
	[CLK_BUS_OHCI1]		= GATE(0xa8c, BIT(1)),
	[CLK_BUS_OHCI2]		= GATE(0xa8c, BIT(2)),
	[CLK_BUS_OHCI3]		= GATE(0xa8c, BIT(3)),
	[CLK_BUS_EHCI0]		= GATE(0xa8c, BIT(4)),
	[CLK_BUS_EHCI1]		= GATE(0xa8c, BIT(5)),
	[CLK_BUS_EHCI2]		= GATE(0xa8c, BIT(6)),
	[CLK_BUS_EHCI3]		= GATE(0xa8c, BIT(7)),
	[CLK_BUS_OTG]		= GATE(0xa8c, BIT(8)),
};

static struct ccu_reset h616_resets[] = {
	[RST_BUS_MMC0]		= RESET(0x84c, BIT(16)),
	[RST_BUS_MMC1]		= RESET(0x84c, BIT(17)),
	[RST_BUS_MMC2]		= RESET(0x84c, BIT(18)),

	[RST_BUS_UART0]		= RESET(0x90c, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x90c, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x90c, BIT(18)),
	[RST_BUS_UART3]		= RESET(0x90c, BIT(19)),
	[RST_BUS_UART4]		= RESET(0x90c, BIT(20)),
	[RST_BUS_UART5]		= RESET(0x90c, BIT(21)),

	[RST_BUS_I2C0]		= RESET(0x91c, BIT(16)),
	[RST_BUS_I2C1]		= RESET(0x91c, BIT(17)),
	[RST_BUS_I2C2]		= RESET(0x91c, BIT(18)),
	[RST_BUS_I2C3]		= RESET(0x91c, BIT(19)),
	[RST_BUS_I2C4]		= RESET(0x91c, BIT(20)),

	[RST_BUS_SPI0]		= RESET(0x96c, BIT(16)),
	[RST_BUS_SPI1]		= RESET(0x96c, BIT(17)),

	[RST_BUS_EMAC0]		= RESET(0x97c, BIT(16)),
	[RST_BUS_EMAC1]		= RESET(0x97c, BIT(17)),

	[RST_USB_PHY0]		= RESET(0xa70, BIT(30)),

	[RST_USB_PHY1]		= RESET(0xa74, BIT(30)),

	[RST_USB_PHY2]		= RESET(0xa78, BIT(30)),

	[RST_USB_PHY3]		= RESET(0xa7c, BIT(30)),

	[RST_BUS_OHCI0]		= RESET(0xa8c, BIT(16)),
	[RST_BUS_OHCI1]		= RESET(0xa8c, BIT(17)),
	[RST_BUS_OHCI2]		= RESET(0xa8c, BIT(18)),
	[RST_BUS_OHCI3]		= RESET(0xa8c, BIT(19)),
	[RST_BUS_EHCI0]		= RESET(0xa8c, BIT(20)),
	[RST_BUS_EHCI1]		= RESET(0xa8c, BIT(21)),
	[RST_BUS_EHCI2]		= RESET(0xa8c, BIT(22)),
	[RST_BUS_EHCI3]		= RESET(0xa8c, BIT(23)),
	[RST_BUS_OTG]		= RESET(0xa8c, BIT(24)),
};

static const struct ccu_desc h616_ccu_desc = {
	.gates = h616_gates,
	.resets = h616_resets,
};

static int h616_clk_bind(struct udevice *dev)
{
	return sunxi_reset_bind(dev, ARRAY_SIZE(h616_resets));
}

static const struct udevice_id h616_ccu_ids[] = {
	{ .compatible = "allwinner,sun50i-h616-ccu",
	  .data = (ulong)&h616_ccu_desc },
	{ }
};

U_BOOT_DRIVER(clk_sun50i_h616) = {
	.name		= "sun50i_h616_ccu",
	.id		= UCLASS_CLK,
	.of_match	= h616_ccu_ids,
	.priv_auto	= sizeof(struct ccu_priv),
	.ops		= &sunxi_clk_ops,
	.probe		= sunxi_clk_probe,
	.bind		= h616_clk_bind,
};

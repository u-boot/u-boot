// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2024 Arm Ltd.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <clk/sunxi.h>
#include <linux/bitops.h>

#include <dt-bindings/clock/sun55i-a523-ccu.h>
#include <dt-bindings/reset/sun55i-a523-ccu.h>

static struct ccu_clk_gate a523_gates[] = {
	[CLK_PLL_PERIPH0_200M]	= GATE_DUMMY,
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
	[CLK_SPI0]		= GATE(0x940, BIT(31)),
	[CLK_SPI1]		= GATE(0x944, BIT(31)),
	[CLK_BUS_SPI0]		= GATE(0x96c, BIT(0)),
	[CLK_BUS_SPI1]		= GATE(0x96c, BIT(1)),

	[CLK_EMAC0_25M]		= GATE(0x970, BIT(30) | BIT(31)),
	[CLK_EMAC1_25M]		= GATE(0x974, BIT(30) | BIT(31)),
	[CLK_BUS_EMAC0]		= GATE(0x97c, BIT(0)),
	[CLK_BUS_EMAC1]		= GATE(0x98c, BIT(0)),

	[CLK_USB_OHCI0]		= GATE(0xa70, BIT(31)),
	[CLK_USB_OHCI1]		= GATE(0xa74, BIT(31)),
	[CLK_BUS_OHCI0]		= GATE(0xa8c, BIT(0)),
	[CLK_BUS_OHCI1]		= GATE(0xa8c, BIT(1)),
	[CLK_BUS_EHCI0]		= GATE(0xa8c, BIT(4)),
	[CLK_BUS_EHCI1]		= GATE(0xa8c, BIT(5)),
	[CLK_BUS_OTG]		= GATE(0xa8c, BIT(8)),
};

static struct ccu_reset a523_resets[] = {
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
	[RST_BUS_SPI0]		= RESET(0x96c, BIT(16)),
	[RST_BUS_SPI1]		= RESET(0x96c, BIT(17)),

	[RST_BUS_EMAC0]		= RESET(0x97c, BIT(16)),
	[RST_BUS_EMAC1]		= RESET(0x98c, BIT(16) | BIT(17)),

	[RST_USB_PHY0]		= RESET(0xa70, BIT(30)),
	[RST_USB_PHY1]		= RESET(0xa74, BIT(30)),
	[RST_BUS_OHCI0]		= RESET(0xa8c, BIT(16)),
	[RST_BUS_OHCI1]		= RESET(0xa8c, BIT(17)),
	[RST_BUS_EHCI0]		= RESET(0xa8c, BIT(20)),
	[RST_BUS_EHCI1]		= RESET(0xa8c, BIT(21)),
	[RST_BUS_OTG]		= RESET(0xa8c, BIT(24)),
};

const struct ccu_desc a523_ccu_desc = {
	.gates	= a523_gates,
	.resets	= a523_resets,
	.num_gates = ARRAY_SIZE(a523_gates),
	.num_resets = ARRAY_SIZE(a523_resets),
};

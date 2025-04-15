// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2023-2024 Arm Ltd.
 */

#include <clk/sunxi.h>
#include <dt-bindings/clock/sun50i-a100-ccu.h>
#include <dt-bindings/reset/sun50i-a100-ccu.h>
#include <linux/bitops.h>

static struct ccu_clk_gate a100_gates[] = {
	[CLK_PLL_PERIPH0]	= GATE(0x020, BIT(31) | BIT(27)),

	[CLK_APB1]		= GATE_DUMMY,

	[CLK_DE]		= GATE(0x600, BIT(31)),
	[CLK_BUS_DE]		= GATE(0x60c, BIT(0)),

	[CLK_BUS_MMC0]		= GATE(0x84c, BIT(0)),
	[CLK_BUS_MMC1]		= GATE(0x84c, BIT(1)),
	[CLK_BUS_MMC2]		= GATE(0x84c, BIT(2)),

	[CLK_BUS_UART0]		= GATE(0x90c, BIT(0)),
	[CLK_BUS_UART1]		= GATE(0x90c, BIT(1)),
	[CLK_BUS_UART2]		= GATE(0x90c, BIT(2)),
	[CLK_BUS_UART3]		= GATE(0x90c, BIT(3)),
	[CLK_BUS_UART4]		= GATE(0x90c, BIT(4)),

	[CLK_BUS_I2C0]		= GATE(0x91c, BIT(0)),
	[CLK_BUS_I2C1]		= GATE(0x91c, BIT(1)),
	[CLK_BUS_I2C2]		= GATE(0x91c, BIT(2)),
	[CLK_BUS_I2C3]		= GATE(0x91c, BIT(3)),

	[CLK_SPI0]		= GATE(0x940, BIT(31)),
	[CLK_SPI1]		= GATE(0x944, BIT(31)),
	[CLK_SPI2]		= GATE(0x948, BIT(31)),

	[CLK_BUS_SPI0]		= GATE(0x96c, BIT(0)),
	[CLK_BUS_SPI1]		= GATE(0x96c, BIT(1)),
	[CLK_BUS_SPI2]		= GATE(0x96c, BIT(2)),

	[CLK_BUS_EMAC]		= GATE(0x97c, BIT(0)),

	[CLK_USB_PHY0]		= GATE(0xa70, BIT(29)),
	[CLK_USB_OHCI0]		= GATE(0xa70, BIT(31)),

	[CLK_USB_PHY1]		= GATE(0xa74, BIT(29)),
	[CLK_USB_OHCI1]		= GATE(0xa74, BIT(31)),

	[CLK_BUS_OHCI0]		= GATE(0xa8c, BIT(0)),
	[CLK_BUS_OHCI1]		= GATE(0xa8c, BIT(1)),
	[CLK_BUS_EHCI0]		= GATE(0xa8c, BIT(4)),
	[CLK_BUS_EHCI1]		= GATE(0xa8c, BIT(5)),
	[CLK_BUS_OTG]		= GATE(0xa8c, BIT(8)),

	[CLK_TCON_LCD]		= GATE(0xb60, BIT(31)),
	[CLK_BUS_TCON_LCD]	= GATE(0xb7c, BIT(0)),
};

static struct ccu_reset a100_resets[] = {
	[RST_BUS_DE]		= RESET(0x60c, BIT(16)),

	[RST_BUS_MMC0]		= RESET(0x84c, BIT(16)),
	[RST_BUS_MMC1]		= RESET(0x84c, BIT(17)),
	[RST_BUS_MMC2]		= RESET(0x84c, BIT(18)),

	[RST_BUS_UART0]		= RESET(0x90c, BIT(16)),
	[RST_BUS_UART1]		= RESET(0x90c, BIT(17)),
	[RST_BUS_UART2]		= RESET(0x90c, BIT(18)),
	[RST_BUS_UART3]		= RESET(0x90c, BIT(19)),
	[RST_BUS_UART4]		= RESET(0x90c, BIT(20)),

	[RST_BUS_I2C0]		= RESET(0x91c, BIT(16)),
	[RST_BUS_I2C1]		= RESET(0x91c, BIT(17)),
	[RST_BUS_I2C2]		= RESET(0x91c, BIT(18)),
	[RST_BUS_I2C3]		= RESET(0x91c, BIT(19)),

	[RST_BUS_SPI0]		= RESET(0x96c, BIT(16)),
	[RST_BUS_SPI1]		= RESET(0x96c, BIT(17)),
	[RST_BUS_SPI2]		= RESET(0x96c, BIT(18)),

	[RST_BUS_EMAC]		= RESET(0x97c, BIT(16)),

	[RST_USB_PHY0]		= RESET(0xa70, BIT(30)),

	[RST_USB_PHY1]		= RESET(0xa74, BIT(30)),

	[RST_BUS_OHCI0]		= RESET(0xa8c, BIT(16)),
	[RST_BUS_OHCI1]		= RESET(0xa8c, BIT(17)),
	[RST_BUS_EHCI0]		= RESET(0xa8c, BIT(20)),
	[RST_BUS_EHCI1]		= RESET(0xa8c, BIT(21)),
	[RST_BUS_OTG]		= RESET(0xa8c, BIT(24)),

	[RST_BUS_TCON_LCD]	= RESET(0xb7c, BIT(16)),
};

const struct ccu_desc a100_ccu_desc = {
	.gates = a100_gates,
	.resets = a100_resets,
	.num_gates = ARRAY_SIZE(a100_gates),
	.num_resets = ARRAY_SIZE(a100_resets),
};

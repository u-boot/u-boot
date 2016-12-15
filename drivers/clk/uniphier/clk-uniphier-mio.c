/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm/device.h>

#include "clk-uniphier.h"

#define UNIPHIER_MIO_CLK_GATE_SD(ch, idx)	\
	{					\
		.index = (idx),			\
		.reg = 0x20 + 0x200 * (ch),	\
		.mask = 0x00000100,		\
		.data = 0x00000100,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x110 + 0x200 * (ch),	\
		.mask = 0x00000001,		\
		.data = 0x00000001,		\
	}

#define UNIPHIER_MIO_CLK_RATE_SD(ch, idx)	\
	{					\
		.index = (idx),			\
		.reg = 0x30 + 0x200 * (ch),	\
		.mask = 0x00031300,		\
		.data = 0x00000000,		\
		.rate = 44444444,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x30 + 0x200 * (ch),	\
		.mask = 0x00031300,		\
		.data = 0x00010000,		\
		.rate = 33333333,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x30 + 0x200 * (ch),	\
		.mask = 0x00031300,		\
		.data = 0x00020000,		\
		.rate = 50000000,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x30 + 0x200 * (ch),	\
		.mask = 0x00031300,		\
		.data = 0x00020000,		\
		.rate = 66666666,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x30 + 0x200 * (ch),	\
		.mask = 0x00031300,		\
		.data = 0x00001000,		\
		.rate = 100000000,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x30 + 0x200 * (ch),	\
		.mask = 0x00031300,		\
		.data = 0x00001100,		\
		.rate = 40000000,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x30 + 0x200 * (ch),	\
		.mask = 0x00031300,		\
		.data = 0x00001200,		\
		.rate = 25000000,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x30 + 0x200 * (ch),	\
		.mask = 0x00031300,		\
		.data = 0x00001300,		\
		.rate = 22222222,		\
	}

#define UNIPHIER_MIO_CLK_GATE_USB(ch, idx)	\
	{					\
		.index = (idx),			\
		.reg = 0x20 + 0x200 * (ch),	\
		.mask = 0x30000000,		\
		.data = 0x30000000,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x110 + 0x200 * (ch),	\
		.mask = 0x01000000,		\
		.data = 0x01000000,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x114 + 0x200 * (ch),	\
		.mask = 0x00000001,		\
		.data = 0x00000001,		\
	}

#define UNIPHIER_MIO_CLK_GATE_DMAC(idx)		\
	{					\
		.index = (idx),			\
		.reg = 0x20,			\
		.mask = 0x02000000,		\
		.data = 0x02000000,		\
	},					\
	{					\
		.index = (idx),			\
		.reg = 0x110,			\
		.mask = 0x00020000,		\
		.data = 0x00020000,		\
	}

static struct uniphier_clk_gate_data uniphier_mio_clk_gate[] = {
	UNIPHIER_MIO_CLK_GATE_SD(0, 0),
	UNIPHIER_MIO_CLK_GATE_SD(1, 1),
	UNIPHIER_MIO_CLK_GATE_SD(2, 2),		/* for PH1-Pro4 only */
	UNIPHIER_MIO_CLK_GATE_USB(0, 3),
	UNIPHIER_MIO_CLK_GATE_USB(1, 4),
	UNIPHIER_MIO_CLK_GATE_USB(2, 5),
	UNIPHIER_MIO_CLK_GATE_DMAC(6),
	UNIPHIER_MIO_CLK_GATE_USB(3, 7),	/* for PH1-sLD3 only */
};

static struct uniphier_clk_rate_data uniphier_mio_clk_rate[] = {
	UNIPHIER_MIO_CLK_RATE_SD(0, 0),
	UNIPHIER_MIO_CLK_RATE_SD(1, 1),
	UNIPHIER_MIO_CLK_RATE_SD(2, 2),		/* for PH1-Pro4 only */
};

static struct uniphier_clk_soc_data uniphier_mio_clk_data = {
	.gate = uniphier_mio_clk_gate,
	.nr_gate = ARRAY_SIZE(uniphier_mio_clk_gate),
	.rate = uniphier_mio_clk_rate,
	.nr_rate = ARRAY_SIZE(uniphier_mio_clk_rate),
};

static const struct udevice_id uniphier_mio_clk_match[] = {
	{
		.compatible = "socionext,ph1-sld3-mioctrl",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,ph1-ld4-mioctrl",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,ph1-pro4-mioctrl",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,ph1-sld8-mioctrl",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,ph1-pro5-mioctrl",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,proxstream2-mioctrl",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,ph1-ld11-mioctrl",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{
		.compatible = "socionext,ph1-ld20-mioctrl",
		.data = (ulong)&uniphier_mio_clk_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(uniphier_mio_clk) = {
	.name = "uniphier-mio-clk",
	.id = UCLASS_CLK,
	.of_match = uniphier_mio_clk_match,
	.probe = uniphier_clk_probe,
	.priv_auto_alloc_size = sizeof(struct uniphier_clk_priv),
	.ops = &uniphier_clk_ops,
};

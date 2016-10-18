/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "clk-uniphier.h"

#define UNIPHIER_MIO_CLK_SD_GATE(id, ch)				\
	UNIPHIER_CLK_GATE((id), 0x20 + 0x200 * (ch), 8)

#define UNIPHIER_MIO_CLK_USB2(id, ch)					\
	UNIPHIER_CLK_GATE((id), 0x20 + 0x200 * (ch), 28)

#define UNIPHIER_MIO_CLK_USB2_PHY(id, ch)				\
	UNIPHIER_CLK_GATE((id), 0x20 + 0x200 * (ch), 29)

#define UNIPHIER_MIO_CLK_DMAC(id)					\
	UNIPHIER_CLK_GATE((id), 0x20, 25)

#define UNIPHIER_MIO_CLK_SD_MUX(_id, ch)				\
	{								\
		.id = (_id),						\
		.nr_muxs = 8,						\
		.reg = 0x30 + 0x200 * (ch),				\
		.masks = {						\
			0x00031000,					\
			0x00031000,					\
			0x00031000,					\
			0x00031000,					\
			0x00001300,					\
			0x00001300,					\
			0x00001300,					\
			0x00001300,					\
		},							\
		.vals = {						\
			0x00000000,					\
			0x00010000,					\
			0x00020000,					\
			0x00030000,					\
			0x00001000,					\
			0x00001100,					\
			0x00001200,					\
			0x00001300,					\
		},							\
		.rates = {						\
			44444444,					\
			33333333,					\
			50000000,					\
			66666666,					\
			100000000,					\
			40000000,					\
			25000000,					\
			22222222,					\
		},							\
	}

static const struct uniphier_clk_gate_data uniphier_mio_clk_gate[] = {
	UNIPHIER_MIO_CLK_SD_GATE(0, 0),
	UNIPHIER_MIO_CLK_SD_GATE(1, 1),
	UNIPHIER_MIO_CLK_SD_GATE(2, 2),		/* for PH1-Pro4 only */
	UNIPHIER_MIO_CLK_DMAC(7),
	UNIPHIER_MIO_CLK_USB2(8, 0),
	UNIPHIER_MIO_CLK_USB2(9, 1),
	UNIPHIER_MIO_CLK_USB2(10, 2),
	UNIPHIER_MIO_CLK_USB2(11, 3),		/* for PH1-sLD3 only */
	UNIPHIER_MIO_CLK_USB2_PHY(12, 0),
	UNIPHIER_MIO_CLK_USB2_PHY(13, 1),
	UNIPHIER_MIO_CLK_USB2_PHY(14, 2),
	UNIPHIER_MIO_CLK_USB2_PHY(15, 3),	/* for PH1-sLD3 only */
	UNIPHIER_CLK_END
};

static const struct uniphier_clk_mux_data uniphier_mio_clk_mux[] = {
	UNIPHIER_MIO_CLK_SD_MUX(0, 0),
	UNIPHIER_MIO_CLK_SD_MUX(1, 1),
	UNIPHIER_MIO_CLK_SD_MUX(2, 2),		/* for PH1-Pro4 only */
	UNIPHIER_CLK_END
};

const struct uniphier_clk_data uniphier_mio_clk_data = {
	.gate = uniphier_mio_clk_gate,
	.mux = uniphier_mio_clk_mux,
};

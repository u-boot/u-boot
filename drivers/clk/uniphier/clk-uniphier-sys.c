/*
 * Copyright (C) 2016-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "clk-uniphier.h"

const struct uniphier_clk_gate_data uniphier_pxs2_sys_clk_gate[] = {
	UNIPHIER_CLK_GATE(8, 0x2104, 10),	/* stdmac */
	UNIPHIER_CLK_GATE(12, 0x2104, 6),	/* gio (Pro4, Pro5) */
	UNIPHIER_CLK_GATE(14, 0x2104, 16),	/* usb30 (Pro4, Pro5, PXs2) */
	UNIPHIER_CLK_GATE(15, 0x2104, 17),	/* usb31 (Pro4, Pro5, PXs2) */
	UNIPHIER_CLK_GATE(16, 0x2104, 19),	/* usb30-phy (PXs2) */
	UNIPHIER_CLK_GATE(20, 0x2104, 20),	/* usb31-phy (PXs2) */
	UNIPHIER_CLK_END
};

const struct uniphier_clk_data uniphier_pxs2_sys_clk_data = {
	.gate = uniphier_pxs2_sys_clk_gate,
};

const struct uniphier_clk_gate_data uniphier_ld20_sys_clk_gate[] = {
	UNIPHIER_CLK_GATE(8, 0x210c, 8),	/* stdmac */
	UNIPHIER_CLK_GATE(14, 0x210c, 14),	/* usb30 (LD20) */
	UNIPHIER_CLK_GATE(16, 0x210c, 12),	/* usb30-phy0 (LD20) */
	UNIPHIER_CLK_GATE(17, 0x210c, 13),	/* usb30-phy1 (LD20) */
	UNIPHIER_CLK_END
};

const struct uniphier_clk_data uniphier_ld20_sys_clk_data = {
	.gate = uniphier_ld20_sys_clk_gate,
};

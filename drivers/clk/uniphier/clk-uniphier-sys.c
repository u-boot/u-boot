/*
 * Copyright (C) 2016-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "clk-uniphier.h"

const struct uniphier_clk_data uniphier_pxs2_sys_clk_data[] = {
#if defined(CONFIG_ARCH_UNIPHIER_LD4) || defined(CONFIG_ARCH_UNIPHIER_SLD8) ||\
    defined(CONFIG_ARCH_UNIPHIER_PRO4) || defined(CONFIG_ARCH_UNIPHIER_PRO5) ||\
    defined(CONFIG_ARCH_UNIPHIER_PXS2) || defined(CONFIG_ARCH_UNIPHIER_LD6B)
	UNIPHIER_CLK_GATE_SIMPLE(8, 0x2104, 10),	/* stdmac */
	UNIPHIER_CLK_GATE_SIMPLE(12, 0x2104, 6),	/* gio (Pro4, Pro5) */
	UNIPHIER_CLK_GATE_SIMPLE(14, 0x2104, 16),	/* usb30 (Pro4, Pro5, PXs2) */
	UNIPHIER_CLK_GATE_SIMPLE(15, 0x2104, 17),	/* usb31 (Pro4, Pro5, PXs2) */
	UNIPHIER_CLK_GATE_SIMPLE(16, 0x2104, 19),	/* usb30-phy (PXs2) */
	UNIPHIER_CLK_GATE_SIMPLE(20, 0x2104, 20),	/* usb31-phy (PXs2) */
	{ /* sentinel */ }
#endif
};

const struct uniphier_clk_data uniphier_ld20_sys_clk_data[] = {
#if defined(CONFIG_ARCH_UNIPHIER_LD11) || defined(CONFIG_ARCH_UNIPHIER_LD20)
	UNIPHIER_CLK_GATE_SIMPLE(8, 0x210c, 8),		/* stdmac */
	UNIPHIER_CLK_GATE_SIMPLE(14, 0x210c, 14),	/* usb30 (LD20) */
	UNIPHIER_CLK_GATE_SIMPLE(16, 0x210c, 12),	/* usb30-phy0 (LD20) */
	UNIPHIER_CLK_GATE_SIMPLE(17, 0x210c, 13),	/* usb30-phy1 (LD20) */
	{ /* sentinel */ }
#endif
};

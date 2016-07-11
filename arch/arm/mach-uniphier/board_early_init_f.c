/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "init.h"
#include "micro-support-card.h"
#include "soc-info.h"

int board_early_init_f(void)
{
	led_puts("U0");

	switch (uniphier_get_soc_type()) {
#if defined(CONFIG_ARCH_UNIPHIER_SLD3)
	case SOC_UNIPHIER_SLD3:
		uniphier_sld3_pin_init();
		led_puts("U1");
		uniphier_ld4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD4)
	case SOC_UNIPHIER_LD4:
		uniphier_ld4_pin_init();
		led_puts("U1");
		uniphier_ld4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO4)
	case SOC_UNIPHIER_PRO4:
		uniphier_pro4_pin_init();
		led_puts("U1");
		uniphier_pro4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_SLD8)
	case SOC_UNIPHIER_SLD8:
		uniphier_sld8_pin_init();
		led_puts("U1");
		uniphier_ld4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PRO5)
	case SOC_UNIPHIER_PRO5:
		uniphier_pro5_pin_init();
		led_puts("U1");
		uniphier_pro5_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PXS2)
	case SOC_UNIPHIER_PXS2:
		uniphier_pxs2_pin_init();
		led_puts("U1");
		uniphier_pxs2_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD6B)
	case SOC_UNIPHIER_LD6B:
		uniphier_ld6b_pin_init();
		led_puts("U1");
		uniphier_pxs2_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_LD20)
	case SOC_UNIPHIER_LD20:
		uniphier_ld20_pin_init();
		led_puts("U1");
		uniphier_ld20_clk_init();
		cci500_init(2);
		break;
#endif
	default:
		break;
	}

	led_puts("U2");

	return 0;
}

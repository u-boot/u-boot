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
#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD3)
	case SOC_UNIPHIER_PH1_SLD3:
		ph1_sld3_pin_init();
		led_puts("U1");
		ph1_ld4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_LD4)
	case SOC_UNIPHIER_PH1_LD4:
		ph1_ld4_pin_init();
		led_puts("U1");
		ph1_ld4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_PRO4)
	case SOC_UNIPHIER_PH1_PRO4:
		ph1_pro4_pin_init();
		led_puts("U1");
		ph1_pro4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_SLD8)
	case SOC_UNIPHIER_PH1_SLD8:
		ph1_sld8_pin_init();
		led_puts("U1");
		ph1_ld4_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_PRO5)
	case SOC_UNIPHIER_PH1_PRO5:
		ph1_pro5_pin_init();
		led_puts("U1");
		ph1_pro5_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PROXSTREAM2)
	case SOC_UNIPHIER_PROXSTREAM2:
		proxstream2_pin_init();
		led_puts("U1");
		proxstream2_clk_init();
		break;
#endif
#if defined(CONFIG_ARCH_UNIPHIER_PH1_LD6B)
	case SOC_UNIPHIER_PH1_LD6B:
		ph1_ld6b_pin_init();
		led_puts("U1");
		proxstream2_clk_init();
		break;
#endif
	default:
		break;
	}

	led_puts("U2");

	return 0;
}

/*
 * Copyright (C) 2013-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>

#include "../init.h"
#include "../micro-support-card.h"

int uniphier_sld3_init(const struct uniphier_board_data *bd)
{
	uniphier_sld3_bcu_init(bd);

	uniphier_sbc_init_admulti(bd);

	uniphier_sld3_early_pin_init(bd);

	support_card_reset();

	uniphier_sld3_pll_init(bd);

	support_card_init();

	led_puts("L0");

	memconf_init(bd);
	uniphier_sld3_memconf_init(bd);

	led_puts("L1");

	uniphier_ld4_early_clk_init(bd);

	led_puts("L2");

	led_puts("L3");

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif

	led_puts("L4");

	led_puts("L5");

	uniphier_sld3_enable_dpll_ssc(bd);

	led_puts("L6");

	return 0;
}

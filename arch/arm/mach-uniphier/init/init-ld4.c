/*
 * Copyright (C) 2013-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>

#include "../init.h"
#include "../micro-support-card.h"

int ph1_ld4_init(const struct uniphier_board_data *bd)
{
	ph1_ld4_bcu_init(bd);

	ph1_ld4_sbc_init(bd);

	support_card_reset();

	ph1_ld4_pll_init(bd);

	support_card_init();

	led_puts("L0");

	memconf_init(bd);

	led_puts("L1");

	ph1_ld4_early_clk_init(bd);

	led_puts("L2");

	led_puts("L3");

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif

	led_puts("L4");

	{
		int res;

		res = ph1_ld4_umc_init(bd);
		if (res < 0) {
			while (1)
				;
		}
	}

	led_puts("L5");

	ph1_ld4_enable_dpll_ssc(bd);

	led_puts("L6");

	return 0;
}

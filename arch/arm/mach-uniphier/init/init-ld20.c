/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>

#include "../init.h"
#include "../micro-support-card.h"

int uniphier_ld20_init(const struct uniphier_board_data *bd)
{
	uniphier_ld11_sbc_init();

	support_card_init();

	led_puts("L0");

	uniphier_memconf_3ch_init(bd);

	led_puts("L1");

	uniphier_ld11_early_clk_init();
	uniphier_ld20_dram_clk_init();

	led_puts("L2");

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif

	led_puts("L3");

	uniphier_ld20_dpll_init(bd);

	led_puts("L4");

	{
		int res;

		res = uniphier_ld20_umc_init(bd);
		if (res < 0) {
			while (1)
				;
		}
	}

	led_puts("L5");

	dcache_disable();

	return 0;
}

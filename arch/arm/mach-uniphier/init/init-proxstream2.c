/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <linux/compiler.h>
#include <mach/init.h>
#include <mach/micro-support-card.h>

int proxstream2_init(const struct uniphier_board_data *bd)
{
	proxstream2_sbc_init(bd);

	support_card_reset();

	support_card_init();

	led_puts("L0");

	memconf_init(bd);
	proxstream2_memconf_init(bd);

	led_puts("L1");

	proxstream2_early_clk_init(bd);

	led_puts("L2");

	led_puts("L3");

#ifdef CONFIG_SPL_SERIAL_SUPPORT
	preloader_console_init();
#endif

	led_puts("L4");

	return 0;
}

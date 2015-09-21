/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <mach/micro-support-card.h>

void pin_init(void);
void clkrst_init(void);

int board_early_init_f(void)
{
	led_puts("U0");

	pin_init();

	led_puts("U1");

	clkrst_init();

	led_puts("U2");

	return 0;
}

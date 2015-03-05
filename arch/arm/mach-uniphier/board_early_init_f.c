/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <mach/led.h>
#include <mach/board.h>

void pin_init(void);
void clkrst_init(void);

int board_early_init_f(void)
{
	led_write(U, 0, , );

	pin_init();

	led_write(U, 1, , );

	clkrst_init();

	led_write(U, 2, , );

	return 0;
}

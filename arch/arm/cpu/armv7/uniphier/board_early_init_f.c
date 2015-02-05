/*
 * Copyright (C) 2012-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/led.h>
#include <asm/arch/board.h>

void pin_init(void);

int board_early_init_f(void)
{
	led_write(U, 0, , );

	pin_init();

	led_write(U, 1, , );

	return 0;
}

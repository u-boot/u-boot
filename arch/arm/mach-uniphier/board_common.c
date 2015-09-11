/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mach/led.h>

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	led_write(U, B, O, O);

	return 0;
}

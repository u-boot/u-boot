/*
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mach/board.h>

int board_early_init_r(void)
{
	uniphier_board_late_init();
	return 0;
}

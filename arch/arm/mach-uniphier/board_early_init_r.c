/*
 * Copyright (C) 2014-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mach/board.h>

int board_early_init_r(void)
{
	support_card_late_init();
	return 0;
}

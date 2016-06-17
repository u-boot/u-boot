/*
 * Copyright (C) 2012-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#include "micro-support-card.h"

int board_init(void)
{
	led_puts("Uboo");

	return 0;
}

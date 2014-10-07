/*
 * Copyright (C) 2012-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/board.h>

int checkboard(void)
{
	puts("Board: PH1-LD4 Board\n");

	return check_support_card();
}

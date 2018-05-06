// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <asm/processor.h>

int checkboard(void)
{
	puts("BOARD: SH7750/SH7750S/SH7750R Solution Engine\n");
	return 0;
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	return 0;
}

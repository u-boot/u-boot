// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007
 * Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 *
 * Copyright (C) 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * Copyright (C) 2007
 * Kenati Technologies, Inc.
 *
 * board/ms7720se/ms7720se.c
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>

#define LED_BASE	0xB0800000

int checkboard(void)
{
	puts("BOARD: Hitachi UL MS7720SE\n");
	return 0;
}

int board_init(void)
{
	return 0;
}

void led_set_state(unsigned short value)
{
	outw(value & 0xFF, LED_BASE);
}

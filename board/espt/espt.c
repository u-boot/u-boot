/*
 * Copyright (C) 2009 Renesas Solutions Corp.
 * Copyright (C) 2009 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 *
 * board/espt/espt.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>

int checkboard(void)
{
	puts("BOARD: ESPT-GIGA\n");
	return 0;
}

int board_init(void)
{
	return 0;
}

void led_set_state(unsigned short value)
{
}

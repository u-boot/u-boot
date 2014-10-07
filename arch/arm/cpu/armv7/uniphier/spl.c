/*
 * Copyright (C) 2013-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>

void spl_board_init(void)
{
#if defined(CONFIG_BOARD_POSTCLK_INIT)
	board_postclk_init();
#endif
	dram_init();
}

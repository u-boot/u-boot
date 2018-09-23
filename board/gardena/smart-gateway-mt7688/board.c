// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <asm/io.h>

int board_early_init_f(void)
{
	/*
	 * Nothing to be done here for this board (no UART setup etc)
	 * right now. We might need some pin muxing, so lets keep this
	 * function for now.
	 */
	return 0;
}

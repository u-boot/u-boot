// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <asm/io.h>

#define MT76XX_GPIO1_MODE	0xb0000060

void board_debug_uart_init(void)
{
	/* Select UART2 mode instead of GPIO mode (default) */
	clrbits_le32((void __iomem *)MT76XX_GPIO1_MODE, GENMASK(27, 26));
}

int board_early_init_f(void)
{
	/*
	 * The pin muxing of UART2 also needs to be done, if debug uart
	 * is not enabled. So we need to call this function here as well.
	 */
	board_debug_uart_init();

	return 0;
}

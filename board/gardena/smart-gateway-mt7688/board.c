// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <linux/io.h>

#define MT76XX_AGPIO_CFG	0x1000003c

int board_early_init_f(void)
{
	void __iomem *gpio_mode;

	/* Configure digital vs analog GPIOs */
	gpio_mode = ioremap_nocache(MT76XX_AGPIO_CFG, 0x100);
	iowrite32(0x00fe01ff, gpio_mode);

	return 0;
}

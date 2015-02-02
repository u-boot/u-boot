/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	return 0;
}

void setup_pch_gpios(u16 gpiobase, const struct pch_gpio_map *gpio)
{
	return;
}

/*
 * Copyright (C) 2015, Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/gpio.h>

int arch_early_init_r(void)
{
	/* do the pin-muxing */
	gpio_ich6_pinctrl_init();

	return 0;
}

void setup_pch_gpios(u16 gpiobase, const struct pch_gpio_map *gpio)
{
	return;
}

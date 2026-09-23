// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017-2018 Freebox SA
 * Copyright (C) 2026 Free Mobile, Vincent Jardin <vjardin@free.fr>
 *
 * Freebox Nodebox 10G board support
 */

#include <init.h>
#include <asm/gpio.h>
#include <linux/delay.h>

/* Management PHY reset GPIO */
#define NBX_PHY_RESET_GPIO	83

/* Nodebox 10G ASCII art logo */
static const char * const nbx_logo =
	"  _   _           _      _                 __  ___   _____\n"
	" | \\ | |         | |    | |               /_ |/ _ \\ / ____|\n"
	" |  \\| | ___   __| | ___| |__   _____  __  | | | | | |  __\n"
	" | . ` |/ _ \\ / _` |/ _ \\ '_ \\ / _ \\ \\/ /  | | | | | | |_ |\n"
	" | |\\  | (_) | (_| |  __/ |_) | (_) >  <   | | |_| | |__| |\n"
	" |_| \\_|\\___/ \\__,_|\\___|_.__/ \\___/_/\\_\\  |_|\\___/ \\_____|\n";

int checkboard(void)
{
	printf("%s\n", nbx_logo);
	return 0;
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	int ret;

	/* Reset the management PHY */
	ret = gpio_request(NBX_PHY_RESET_GPIO, "phy-reset");
	if (ret) {
		printf("Failed to request PHY reset GPIO: %d\n", ret);
		return 0;
	}

	gpio_direction_output(NBX_PHY_RESET_GPIO, 0);
	mdelay(100);
	gpio_set_value(NBX_PHY_RESET_GPIO, 1);
	mdelay(100);

	return 0;
}

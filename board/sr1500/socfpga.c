/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <miiphy.h>
#include <asm/arch/reset_manager.h>
#include <asm/gpio.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

void s_init(void) {}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	/* Address of boot parameters for ATAG (if ATAG is used) */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int board_early_init_f(void)
{
	int ret;

	/* Reset the Marvell PHY 88E1510 */
	ret = gpio_request(63, "PHY reset");
	if (ret)
		return ret;

	gpio_direction_output(63, 0);
	mdelay(1);
	gpio_set_value(63, 1);
	mdelay(10);

	return 0;
}

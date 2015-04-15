/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include <power/as3722.h>
#include <power/pmic.h>
#include "pinmux-config-nyan-big.h"

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	gpio_config_table(nyan_big_gpio_inits,
			  ARRAY_SIZE(nyan_big_gpio_inits));

	pinmux_config_pingrp_table(nyan_big_pingrps,
				   ARRAY_SIZE(nyan_big_pingrps));

	pinmux_config_drvgrp_table(nyan_big_drvgrps,
				   ARRAY_SIZE(nyan_big_drvgrps));
}

int tegra_board_id(void)
{
	static const int vector[] = {GPIO_PQ3, GPIO_PT1, GPIO_PX1,
					GPIO_PX4, -1};

	gpio_claim_vector(vector, "board_id%d");
	return gpio_get_values_as_int(vector);
}

int tegra_lcd_pmic_init(int board_id)
{
	struct udevice *pmic;
	int ret;

	ret = as3722_get(&pmic);
	if (ret)
		return -ENOENT;

	if (board_id == 0)
		as3722_write(pmic, 0x00, 0x3c);
	else
		as3722_write(pmic, 0x00, 0x50);
	as3722_write(pmic, 0x12, 0x10);
	as3722_write(pmic, 0x0c, 0x07);
	as3722_write(pmic, 0x20, 0x10);

	return 0;
}

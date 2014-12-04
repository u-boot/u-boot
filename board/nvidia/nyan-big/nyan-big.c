/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
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

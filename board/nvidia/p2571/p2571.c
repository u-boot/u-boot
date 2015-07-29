/*
 * (C) Copyright 2013-2015
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include "pinmux-config-p2571.h"

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_clear_tristate_input_clamping();

	gpio_config_table(p2571_gpio_inits,
			  ARRAY_SIZE(p2571_gpio_inits));

	pinmux_config_pingrp_table(p2571_pingrps,
				   ARRAY_SIZE(p2571_pingrps));

	pinmux_config_drvgrp_table(p2571_drvgrps,
				   ARRAY_SIZE(p2571_drvgrps));
}

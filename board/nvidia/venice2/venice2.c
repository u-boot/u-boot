/*
 * (C) Copyright 2013-2014
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm-generic/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch/pinmux.h>
#include "pinmux-config-venice2.h"
#include <i2c.h>

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_config_pingrp_table(tegra124_pinmux_set_nontristate,
		ARRAY_SIZE(tegra124_pinmux_set_nontristate));

	pinmux_config_pingrp_table(tegra124_pinmux_common,
		ARRAY_SIZE(tegra124_pinmux_common));

	pinmux_config_pingrp_table(unused_pins_lowpower,
		ARRAY_SIZE(unused_pins_lowpower));

	/* Initialize any non-default pad configs (APB_MISC_GP regs) */
	pinmux_config_drvgrp_table(venice2_padctrl,
		ARRAY_SIZE(venice2_padctrl));
}

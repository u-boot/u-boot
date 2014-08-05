/*
 *  (C) Copyright 2014
 *  Stefan Agner <stefan@agner.ch>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/gp_padctrl.h>
#include "pinmux-config-colibri_t30.h"
#include <i2c.h>
#include <asm/gpio.h>

/*
 * Routine: pinmux_init
 * Description: Do individual peripheral pinmux configs
 */
void pinmux_init(void)
{
	pinmux_config_pingrp_table(tegra3_pinmux_common,
				   ARRAY_SIZE(tegra3_pinmux_common));

	pinmux_config_pingrp_table(unused_pins_lowpower,
				   ARRAY_SIZE(unused_pins_lowpower));

	/* Initialize any non-default pad configs (APB_MISC_GP regs) */
	pinmux_config_drvgrp_table(colibri_t30_padctrl,
				   ARRAY_SIZE(colibri_t30_padctrl));
}

/*
 * Enable AX88772B USB to LAN controller
 */
void pin_mux_usb(void)
{
	/* Reset ASIX using LAN_RESET */
	gpio_request(GPIO_PDD0, NULL);
	gpio_direction_output(GPIO_PDD0, 0);
	udelay(5);
	gpio_set_value(GPIO_PDD0, 1);
}

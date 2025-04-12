// SPDX-License-Identifier: GPL-2.0+
/*
 *  T20 Motorola Atrix 4G and Droid X2 SPL stage configuration
 *
 *  (C) Copyright 2025
 *  Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/pmc.h>
#include <spl_gpio.h>
#include <linux/delay.h>

/*
 * Unlike all other supported Tegra devices and most known Tegra devices, the
 * both Atrix 4G and Droid X2 have no hardware way to enter APX/RCM mode, which
 * may lead to a dangerous situation when, if BCT is set correctly and the
 * bootloader is faulty, the device will hang in a permanent brick state.
 * Exiting from this state can be done only by disassembling the device and
 * shortening testpad to the ground.
 *
 * To prevent this or to minimize the probability of such an accident, it was
 * proposed to add the RCM rebooting hook as early into SPL as possible since
 * SPL is much more robust and has minimal changes that can break bootflow.
 *
 * gpio_early_init_uart() function was chosen as it is the earliest function
 * exposed for setup by the device. Hook performs a check for volume up
 * button state and triggers RCM if it is pressed.
 */
void gpio_early_init_uart(void)
{
	int value;

	/* Configure pinmux for PR0 */
	pinmux_set_func(PMUX_PINGRP_KBCA, PMUX_FUNC_KBC);
	pinmux_set_pullupdown(PMUX_PINGRP_KBCA, PMUX_PULL_UP);
	pinmux_tristate_disable(PMUX_PINGRP_KBCA);

	/* Configure pinmux for PQ0 */
	pinmux_set_func(PMUX_PINGRP_KBCC, PMUX_FUNC_KBC);
	pinmux_set_pullupdown(PMUX_PINGRP_KBCC, PMUX_PULL_UP);
	pinmux_tristate_disable(PMUX_PINGRP_KBCC);

	/* Hog column 0 (PQ0) low - active */
	spl_gpio_output(NULL, TEGRA_GPIO(Q, 0), 0);
	udelay(500);

	spl_gpio_input(NULL, TEGRA_GPIO(R, 0));
	value = spl_gpio_get_value(NULL, TEGRA_GPIO(R, 0));

	/* Enter RCM if button is pressed */
	if (!value) {
		tegra_pmc_writel(2, PMC_SCRATCH0);
		tegra_pmc_writel(PMC_CNTRL_MAIN_RST, PMC_CNTRL);
	}
}

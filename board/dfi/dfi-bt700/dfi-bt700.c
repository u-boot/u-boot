/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <nuvoton_nct6102d.h>
#include <asm/gpio.h>
#include <asm/ibmpc.h>
#include <asm/pnp_def.h>

int board_early_init_f(void)
{
#ifdef CONFIG_INTERNAL_UART
	/* Disable the legacy UART which is enabled per default */
	nct6102d_uarta_disable();
#else
	/*
	 * The FSP enables the BayTrail internal legacy UART (again).
	 * Disable it again, so that the Nuvoton one can be used.
	 */
	setup_internal_uart(0);
#endif

	/* Disable the watchdog which is enabled per default */
	nct6102d_wdt_disable();

	return 0;
}

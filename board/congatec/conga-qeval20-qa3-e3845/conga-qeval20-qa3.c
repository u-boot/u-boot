/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <winbond_w83627.h>
#include <asm/gpio.h>
#include <asm/ibmpc.h>
#include <asm/pnp_def.h>

int board_early_init_f(void)
{
#ifndef CONFIG_INTERNAL_UART
	/*
	 * The FSP enables the BayTrail internal legacy UART (again).
	 * Disable it again, so that the Winbond one can be used.
	 */
	setup_internal_uart(0);

	/* Enable the legacy UART in the Winbond W83627 Super IO chip */
	winbond_enable_serial(PNP_DEV(WINBOND_IO_PORT, W83627DHG_SP1),
			      UART0_BASE, UART0_IRQ);
#endif

	return 0;
}

int arch_early_init_r(void)
{
	return 0;
}

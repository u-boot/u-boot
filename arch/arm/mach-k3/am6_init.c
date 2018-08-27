// SPDX-License-Identifier: GPL-2.0+
/*
 * K3: Architecture initialization
 *
 * Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <spl.h>

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	/* Init DM early in-order to invoke system controller */
	spl_early_init();

	/* Prepare console output */
	preloader_console_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_RAM;
}
#endif

#ifndef CONFIG_SYSRESET
void reset_cpu(ulong ignored)
{
}
#endif

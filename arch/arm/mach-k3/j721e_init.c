// SPDX-License-Identifier: GPL-2.0+
/*
 * J721E: SoC specific initialization
 *
 * Copyright (C) 2018-2019 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/armv7_mpu.h>
#include "common.h"

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	/*
	 * ToDo:
	 * - Store boot rom index.
	 * - unlock mmr.
	 */

#ifdef CONFIG_CPU_V7R
	setup_k3_mpu_regions();
#endif

	/* Init DM early */
	spl_early_init();

	/* Prepare console output */
	preloader_console_init();
}
#endif

// SPDX-License-Identifier: GPL-2.0
/*
 * AM642: SoC specific initialization
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Keerthy <j-keerthy@ti.com>
 *	Dave Gerlach <d-gerlach@ti.com>
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include "common.h"

#if defined(CONFIG_SPL_BUILD)

void board_init_f(ulong dummy)
{
#if defined(CONFIG_CPU_V7R)
	setup_k3_mpu_regions();
#endif

	/* Init DM early */
	spl_early_init();

	preloader_console_init();
}
#endif

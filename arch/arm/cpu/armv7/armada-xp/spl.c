/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

u32 spl_boot_device(void)
{
	/* Right now only booting via SPI NOR flash is supported */
	return BOOT_DEVICE_SPI;
}

void board_init_f(ulong dummy)
{
	/* Set global data pointer */
	gd = &gdata;

	/* Linux expects the internal registers to be at 0xf1000000 */
	arch_cpu_init();

	preloader_console_init();

	/* First init the serdes PHY's */
	serdes_phy_config();

	/* Setup DDR */
	ddr3_init();

	board_init_r(NULL, 0);
}

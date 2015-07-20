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
#if defined(CONFIG_SPL_SPI_FLASH_SUPPORT)
	return BOOT_DEVICE_SPI;
#endif
#if defined(CONFIG_SPL_MMC_SUPPORT)
	return BOOT_DEVICE_MMC1;
#endif
}

#ifdef CONFIG_SPL_MMC_SUPPORT
u32 spl_boot_mode(void)
{
	return MMCSD_MODE_RAW;
}
#endif

void board_init_f(ulong dummy)
{
	/* Set global data pointer */
	gd = &gdata;

	/* Linux expects the internal registers to be at 0xf1000000 */
	arch_cpu_init();

	/*
	 * Pin muxing needs to be done before UART output, since
	 * on A38x the UART pins need some re-muxing for output
	 * to work.
	 */
	board_early_init_f();

	preloader_console_init();

	timer_init();

	/* First init the serdes PHY's */
	serdes_phy_config();

	/* Setup DDR */
	ddr3_init();

	board_init_r(NULL, 0);
}

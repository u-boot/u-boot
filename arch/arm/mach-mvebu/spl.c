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
#ifndef CONFIG_MVEBU_BOOTROM_UARTBOOT
	/*
	 * Only call arch_cpu_init() when not returning to the
	 * Marvell BootROM, which is done when booting via
	 * the xmodem protocol (kwboot tool). Otherwise the
	 * internal register will get remapped and the BootROM
	 * can't continue to run correctly.
	 */

	/* Linux expects the internal registers to be at 0xf1000000 */
	arch_cpu_init();
#endif

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

#ifdef CONFIG_MVEBU_BOOTROM_UARTBOOT
	/*
	 * Return to the BootROM to continue the Marvell xmodem
	 * UART boot protocol. As initiated by the kwboot tool.
	 *
	 * This can only be done by the BootROM and not by the
	 * U-Boot SPL infrastructure, since the beginning of the
	 * image is already read and interpreted by the BootROM.
	 * SPL has no chance to receive this information. So we
	 * need to return to the BootROM to enable this xmodem
	 * UART download.
	 */
	return_to_bootrom();
#endif
}

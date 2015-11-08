/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	_ASM_SPL_H_
#define	_ASM_SPL_H_

#if defined(CONFIG_OMAP) \
	|| defined(CONFIG_EXYNOS4) || defined(CONFIG_EXYNOS5) \
	|| defined(CONFIG_EXYNOS4210)
/* Platform-specific defines */
#include <asm/arch/spl.h>

#else
enum {
	BOOT_DEVICE_RAM,
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_MMC2_2,
	BOOT_DEVICE_NAND,
	BOOT_DEVICE_ONENAND,
	BOOT_DEVICE_NOR,
	BOOT_DEVICE_UART,
	BOOT_DEVICE_SPI,
	BOOT_DEVICE_SATA,
	BOOT_DEVICE_I2C,
	BOOT_DEVICE_BOARD,
	BOOT_DEVICE_NONE
};
#endif

/**
 * Board specific load method for boards that have a special way of loading
 * U-Boot, which does not fit with the existing SPL code.
 *
 * @return 0 on success, negative errno value on failure.
 */

int spl_board_load_image(void);

/* Linker symbols. */
extern char __bss_start[], __bss_end[];

#ifndef CONFIG_DM
extern gd_t gdata;
#endif

#endif

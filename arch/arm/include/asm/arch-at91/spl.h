/*
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	_ASM_ARCH_SPL_H_
#define	_ASM_ARCH_SPL_H_

enum {
	BOOT_DEVICE_NONE,
#ifdef CONFIG_SYS_USE_MMC
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_MMC2_2,
#elif CONFIG_SYS_USE_NANDFLASH
	BOOT_DEVICE_NAND,
#elif CONFIG_SYS_USE_SERIALFLASH
	BOOT_DEVICE_SPI,
#endif
};

#endif

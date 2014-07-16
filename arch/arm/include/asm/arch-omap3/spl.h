/*
 * (C) Copyright 2012
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	_ASM_ARCH_SPL_H_
#define	_ASM_ARCH_SPL_H_

#define BOOT_DEVICE_NONE	0
#define BOOT_DEVICE_XIP		1
#define BOOT_DEVICE_NAND	2
#define BOOT_DEVICE_ONENAND	3
#define BOOT_DEVICE_MMC2	5 /*emmc*/
#define BOOT_DEVICE_MMC1	6
#define BOOT_DEVICE_XIPWAIT	7
#define BOOT_DEVICE_MMC2_2      0xFF

#define MMC_BOOT_DEVICES_START	BOOT_DEVICE_MMC2
#define MMC_BOOT_DEVICES_END	BOOT_DEVICE_MMC1
#endif

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Gateworks Corporation
 */

#ifndef __IMX8MM_VENICE_H
#define __IMX8MM_VENICE_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CONFIG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#ifdef CONFIG_SPL_BUILD
/* malloc f used before GD_FLG_FULL_MALLOC_INIT set */
#define CONFIG_MALLOC_F_ADDR		0x930000
#endif

/* Enable Distro Boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(USB, usb, 0) \
	func(USB, usb, 1) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#define CONFIG_EXTRA_ENV_SETTINGS \
	"splblk=0x42\0" \
	BOOTENV

#define CONFIG_SYS_INIT_RAM_ADDR        0x40000000
#define CONFIG_SYS_INIT_RAM_SIZE        SZ_2M

#define CONFIG_SYS_SDRAM_BASE           0x40000000

/* SDRAM configuration */
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			SZ_4G

#endif

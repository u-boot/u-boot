/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Gateworks Corporation
 */

#ifndef __IMX8MN_VENICE_H
#define __IMX8MN_VENICE_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

/* Enable Distro Boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 2) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#define CFG_EXTRA_ENV_SETTINGS \
	"splblk=0x40\0" \
	BOOTENV

#define CFG_SYS_INIT_RAM_ADDR        0x40000000
#define CFG_SYS_INIT_RAM_SIZE        SZ_2M

#define CFG_SYS_SDRAM_BASE           0x40000000

/* SDRAM configuration */
#define PHYS_SDRAM                      0x40000000
#define PHYS_SDRAM_SIZE			SZ_4G

#endif

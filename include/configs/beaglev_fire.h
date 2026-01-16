/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023 Microchip Technology Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define CFG_SYS_SDRAM_BASE       0x80000000

/* Environment options */

#if defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_DHCP(func)	func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#if defined(CONFIG_CMD_MMC)
#define BOOT_TARGET_DEVICES_MMC(func)	func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_MMC(func)\
	BOOT_TARGET_DEVICES_DHCP(func)

#define BOOTENV_DESIGN_OVERLAYS \
	"design_overlays=" \
	"if test -n ${no_of_overlays}; then " \
		"setenv inc 1; " \
		"setenv idx 0; " \
		"fdt resize ${dtbo_size}; " \
		"while test $idx -ne ${no_of_overlays}; do " \
			"setenv dtbo_name dtbo_image${idx}; " \
			"setenv fdt_cmd \"fdt apply $\"$dtbo_name; " \
			"run fdt_cmd; " \
			"setexpr idx $inc + $idx; " \
		"done; " \
	"fi;\0 " \

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"kernel_addr_r=0x80200000\0" \
	"fdt_addr_r=0x8a000000\0" \
	"fdtoverlay_addr_r=0x8a080000\0" \
	"ramdisk_addr_r=0x8aa00000\0" \
	"scriptaddr=0x8e000000\0" \
	BOOTENV_DESIGN_OVERLAYS \
	BOOTENV \

#endif /* __CONFIG_H */

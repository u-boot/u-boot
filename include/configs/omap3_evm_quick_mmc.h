/*
 * Configuration settings for quick boot from MMC on OMAP3 EVM.
 *
 * Copyright (C) 2006-2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Author :
 *     Sanjeev Premi <premi@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __OMAP3_EVM_QUICK_MMC_H
#define __OMAP3_EVM_QUICK_MMC_H

#include <asm/arch/cpu.h>
#include <asm/arch/omap3.h>

/* ----------------------------------------------------------------------------
 * Supported U-boot commands
 * ----------------------------------------------------------------------------
 */
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT

/*
 * Board revision is detected by probing the Ethernet chip.
 *
 * When revision is statically configured via CONFIG_STATIC_BOARD_REV,
 * this option can be removed. Generated binary is leaner by ~16Kbytes.
 */
#define CONFIG_CMD_NET

/* ----------------------------------------------------------------------------
 * Supported U-boot features
 * ----------------------------------------------------------------------------
 */
#define CONFIG_SILENT_CONSOLE
#define CONFIG_ENV_IS_NOWHERE

/* ----------------------------------------------------------------------------
 * Supported hardware
 * ----------------------------------------------------------------------------
 */

/* MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_DOS_PARTITION

/* -----------------------------------------------------------------------------
 * Include common board configuration
 * -----------------------------------------------------------------------------
 */
#include "omap3_evm_common.h"

/* -----------------------------------------------------------------------------
 * Default environment
 * -----------------------------------------------------------------------------
 */
#define CONFIG_BOOTDELAY	0

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"verify=no\0"			\
	"silent=1"

#define CONFIG_BOOTCOMMAND			\
	"mmc rescan 0; "			\
	"fatload mmc 0 0x82000000 uImage; "	\
	"bootm 0x82000000;"

/*
 * Update the bootargs as necessary e.g. size of memory, partition and fstype
 */
#define CONFIG_BOOTARGS			\
	"quiet "			\
	"console=ttyO0,115200n8 "	\
	"mem=128M "			\
	"noinitrd "			\
	"root=/dev/mmcblk0p2 rw "	\
	"rootfstype=ext3 rootwait"

/*
 * SPL
 */
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"

#endif /* __OMAP3_EVM_QUICK_MMC_H */

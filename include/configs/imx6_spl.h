/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef __IMX6_SPL_CONFIG_H
#define __IMX6_SPL_CONFIG_H

#ifdef CONFIG_SPL

#define CONFIG_SPL_FRAMEWORK

/*
 * see Figure 8-3 in IMX6DQ/IMX6SDL Reference manuals:
 *  - IMX6SDL OCRAM (IRAM) is from 0x00907000 to 0x0091FFFF
 *  - IMX6DQ has 2x IRAM of IMX6SDL but we intend to support IMX6SDL as well
 *  - BOOT ROM stack is at 0x0091FFB8
 *  - if icache/dcache is enabled (eFuse/strapping controlled) then the
 *    IMX BOOT ROM will setup MMU table at 0x00918000, therefore we need to
 *    fit between 0x00907000 and 0x00918000.
 *  - Additionally the BOOT ROM loads what they consider the firmware image
 *    which consists of a 4K header in front of us that contains the IVT, DCD
 *    and some padding thus 'our' max size is really 0x00908000 - 0x00918000
 *    or 64KB
 */
#define CONFIG_SYS_THUMB_BUILD
#define CONFIG_SPL_LDSCRIPT	"arch/arm/cpu/armv7/omap-common/u-boot-spl.lds"
#define CONFIG_SPL_TEXT_BASE		0x00908000
#define CONFIG_SPL_MAX_SIZE		0x10000
#define CONFIG_SPL_STACK		0x0091FFB8
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT

/* NAND support */
#if defined(CONFIG_SPL_NAND_SUPPORT)
#define CONFIG_SPL_NAND_MXS
#define CONFIG_SPL_DMA_SUPPORT
#endif

/* MMC support */
#if defined(CONFIG_SPL_MMC_SUPPORT)
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	138 /* offset 69KB */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	800 /* 400 KB */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SYS_MONITOR_LEN  (CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS/2*1024)
#endif

/* SATA support */
#if defined(CONFIG_SPL_SATA_SUPPORT)
#define CONFIG_SPL_SATA_BOOT_DEVICE		0
#define CONFIG_SYS_SATA_FAT_BOOT_PARTITION	1
#endif

/* Define the payload for FAT/EXT support */
#if defined(CONFIG_SPL_FAT_SUPPORT) || defined(CONFIG_SPL_EXT_SUPPORT)
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME  "u-boot.img"
#define CONFIG_SPL_LIBDISK_SUPPORT
#endif

#define CONFIG_SPL_BSS_START_ADDR	0x18200000
#define CONFIG_SPL_BSS_MAX_SIZE		0x100000	/* 1 MB */
#define CONFIG_SYS_SPL_MALLOC_START	0x18300000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x3200000	/* 50 MB */
#define CONFIG_SYS_TEXT_BASE		0x17800000
#endif

#endif

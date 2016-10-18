/*
 * Common configuration settings for IGEP technology based boards
 *
 * (C) Copyright 2012
 * ISEE 2007 SL, <www.iseebcn.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __IGEP00X0_H
#define __IGEP00X0_H

#define CONFIG_NR_DRAM_BANKS            2
#define CONFIG_NAND

#include <configs/ti_omap3_common.h>
#include <asm/mach-types.h>

/*
 * We are only ever GP parts and will utilize all of the "downloaded image"
 * area in SRAM which starts at 0x40200000 and ends at 0x4020FFFF (64KB).
 */
#undef CONFIG_SPL_TEXT_BASE
#define CONFIG_SPL_TEXT_BASE		0x40200000

#define CONFIG_MISC_INIT_R

#define CONFIG_REVISION_TAG		1

/* Status LED available for IGEP0020 and IGEP0030 but not IGEP0032 */
#if (CONFIG_MACH_TYPE != MACH_TYPE_IGEP0032)
#define CONFIG_STATUS_LED
#define CONFIG_BOARD_SPECIFIC_LED
#define CONFIG_GPIO_LED
#if (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0020)
#define RED_LED_GPIO 27
#elif (CONFIG_MACH_TYPE == MACH_TYPE_IGEP0030)
#define RED_LED_GPIO 16
#else
#error "status LED not defined for this machine."
#endif
#define RED_LED_DEV			0
#define STATUS_LED_BIT			RED_LED_GPIO
#define STATUS_LED_STATE		STATUS_LED_ON
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BOOT			RED_LED_DEV
#endif

/* GPIO banks */
#define CONFIG_OMAP3_GPIO_3		/* GPIO64 .. 95 is in GPIO bank 3 */
#define CONFIG_OMAP3_GPIO_5		/* GPIO128..159 is in GPIO bank 5 */
#define CONFIG_OMAP3_GPIO_6		/* GPIO160..191 is in GPIO bank 6 */

/* USB */
#define CONFIG_USB_MUSB_UDC		1
#define CONFIG_USB_OMAP3		1
#define CONFIG_TWL4030_USB		1

/* USB device configuration */
#define CONFIG_USB_DEVICE		1
#define CONFIG_USB_TTY			1

/* Change these to suit your needs */
#define CONFIG_USBD_VENDORID		0x0451
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Texas Instruments"
#define CONFIG_USBD_PRODUCT_NAME	"IGEP"

#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_ONENAND

#ifndef CONFIG_SPL_BUILD

/* Environment */
#define ENV_DEVICE_SETTINGS \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0"

#define MEM_LAYOUT_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"scriptaddr=0x87E00000\0" \
	"pxefile_addr_r=0x87F00000\0"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	ENV_DEVICE_SETTINGS \
	MEM_LAYOUT_SETTINGS \
	BOOTENV

#endif

/*
 * SMSC911x Ethernet
 */
#if defined(CONFIG_CMD_NET)
#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CONFIG_SMC911X_BASE		0x2C000000
#endif /* (CONFIG_CMD_NET) */

#define CONFIG_RBTREE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_SYS_MTDPARTS_RUNTIME

/* OneNAND config */
#define CONFIG_USE_ONENAND_BOARD_INIT
#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP
#define CONFIG_SYS_ONENAND_BLOCK_SIZE	(128*1024)

/* NAND config */
#define CONFIG_SPL_OMAP3_ID_NAND
#define CONFIG_SYS_NAND_BUSWIDTH_16BIT
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2,  3,  4,  5,  6,  7,  8,  9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW_DETECTION_SW
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_BCH

/* UBI configuration */
#define CONFIG_SPL_UBI			1
#define CONFIG_SPL_UBI_MAX_VOL_LEBS	256
#define CONFIG_SPL_UBI_MAX_PEB_SIZE	(256*1024)
#define CONFIG_SPL_UBI_MAX_PEBS		4096
#define CONFIG_SPL_UBI_VOL_IDS		8
#define CONFIG_SPL_UBI_LOAD_MONITOR_ID	0
#define CONFIG_SPL_UBI_LOAD_KERNEL_ID	3
#define CONFIG_SPL_UBI_LOAD_ARGS_ID	4
#define CONFIG_SPL_UBI_PEB_OFFSET	4
#define CONFIG_SPL_UBI_VID_OFFSET	512
#define CONFIG_SPL_UBI_LEB_START	2048
#define CONFIG_SPL_UBI_INFO_ADDR	0x88080000

/* environment organization */
#define CONFIG_ENV_IS_IN_UBI		1
#define CONFIG_ENV_UBI_PART		"UBI"
#define CONFIG_ENV_UBI_VOLUME		"config"
#define CONFIG_ENV_UBI_VOLUME_REDUND	"config_r"
#define CONFIG_UBI_SILENCE_MSG		1
#define CONFIG_UBIFS_SILENCE_MSG	1
#define CONFIG_ENV_SIZE			(32*1024)

#endif /* __IGEP00X0_H */

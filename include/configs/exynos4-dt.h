/*
 * Copyright (C) 2014 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_SAMSUNG			/* in a SAMSUNG core */
#define CONFIG_S5P			/* S5P Family */
#define CONFIG_EXYNOS4			/* which is in a Exynos4 Family */

#include <asm/arch/cpu.h>		/* get chip and board defs */

#define CONFIG_ARCH_CPU_INIT
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO
#define CONFIG_BOARD_COMMON
#define CONFIG_SYS_GENERIC_BOARD

/* Enable fdt support */
#define CONFIG_OF_CONTROL
#define CONFIG_OF_SEPARATE

#define CONFIG_SYS_CACHELINE_SIZE	32

/* input clock of PLL: EXYNOS4 boards have 24MHz input clock */
#define CONFIG_SYS_CLK_FREQ		24000000

#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_CMDLINE_EDITING

#include <linux/sizes.h>

/* SD/MMC configuration */
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_S5P_SDHCI
#define CONFIG_SDHCI
#define CONFIG_MMC_SDMA
#define CONFIG_DWMMC
#define CONFIG_EXYNOS_DWMMC
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_MMC_DEFAULT_DEV	0

/* PWM */
#define CONFIG_PWM

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_SKIP_LOWLEVEL_INIT

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Command definition*/
#include <config_cmd_default.h>

#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_MISC
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_XIMG
#undef CONFIG_CMD_CACHE
#undef CONFIG_CMD_ONENAND
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_MMC
#define CONFIG_CMD_DFU
#define CONFIG_CMD_GPT
#define CONFIG_CMD_PMIC
#define CONFIG_CMD_SETEXPR

#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK

/* FAT */
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE

/* EXT4 */
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE

/* USB Composite download gadget - g_dnl */
#define CONFIG_USBDOWNLOAD_GADGET

/* TIZEN THOR downloader support */
#define CONFIG_CMD_THOR_DOWNLOAD
#define CONFIG_THOR_FUNCTION

#define CONFIG_DFU_FUNCTION
#define CONFIG_DFU_MMC
#define CONFIG_SYS_DFU_DATA_BUF_SIZE SZ_32M
#define DFU_DEFAULT_POLL_TIMEOUT 300

/* USB Samsung's IDs */
#define CONFIG_G_DNL_VENDOR_NUM 0x04E8
#define CONFIG_G_DNL_PRODUCT_NUM 0x6601
#define CONFIG_G_DNL_THOR_VENDOR_NUM CONFIG_G_DNL_VENDOR_NUM
#define CONFIG_G_DNL_THOR_PRODUCT_NUM 0x685D
#define CONFIG_G_DNL_MANUFACTURER "Samsung"

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser	*/
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		384	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH
#undef CONFIG_CMD_IMLS

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_CMD_PART
#define CONFIG_PARTITION_UUIDS

#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_S3C_UDC_OTG
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET_VBUS_DRAW	2

#define CONFIG_CMD_USB_MASS_STORAGE
#define CONFIG_USB_GADGET_MASS_STORAGE

/* Enable devicetree support */
#define CONFIG_OF_LIBFDT

#endif	/* __CONFIG_H */

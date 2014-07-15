/*
 * kwb.h
 *
 * specific parts for B&R KWB Motherboard
 *
 * Copyright (C) 2013 Hannes Petermaier <oe5hpm@oevsv.at> -
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:        GPL-2.0+
 */

#ifndef __CONFIG_KWB_H__
#define __CONFIG_KWB_H__

#include <configs/bur_am335x_common.h>
/* ------------------------------------------------------------------------- */
/* Clock Defines */
#define V_OSCK				26000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_POWER_TPS65217

#define CONFIG_MACH_TYPE		3589
/* I2C IP block */
#define CONFIG_SYS_OMAP24_I2C_SPEED_PSOC	20000

/* GPIO */
#define CONFIG_SPL_GPIO_SUPPORT

/* MMC/SD IP block */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_CMD_MMC
#define CONFIG_SUPPORT_EMMC_BOOT
/* RAW SD card / eMMC locations. */
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /*addr. 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS		0x200 /* 256 KB */
#define CONFIG_SPL_MMC_SUPPORT

#undef CONFIG_SPL_OS_BOOT
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_SPL_ARGS_ADDR		0x80F80000

/* RAW SD card / eMMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x900	/* address 0x120000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x80	/* address 0x10000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0x80	/* 64KiB */

#endif /* CONFIG_SPL_OS_BOOT */

/* Always 128 KiB env size */
#define CONFIG_ENV_SIZE			(128 << 10)

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
	"autoload=0\0" \
	"loadaddr=0x80100000\0" \
	"bootfile=arimg\0" \
	"usbboot=echo Booting from USB-Stick ...; " \
		"usb start; " \
		"fatload usb 0 ${loadaddr} ${bootfile}; " \
		"usb stop; " \
		"go ${loadaddr};\0" \
	"netboot=echo Booting from network ...; " \
		"setenv autoload 0; " \
		"dhcp; " \
		"tftp ${loadaddr} arimg; " \
		"go ${loadaddr}\0" \
	"usbupdate=echo Updating UBOOT from USB-Stick ...; " \
		"usb start; " \
		"fatload usb 0 0x80000000 updateubootusb.img; " \
		"source;\0" \
	"netupdate=echo Updating UBOOT from Network (TFTP) ...; " \
		"setenv autoload 0; " \
		"dhcp;" \
		"tftp 0x80000000 updateUBOOT.img;" \
		"source;\0"
#endif /* !CONFIG_SPL_BUILD*/

#define CONFIG_BOOTCOMMAND \
	"run usbupdate;"
#define CONFIG_BOOTDELAY		1 /* TODO: für release auf 0 setzen */

/* undefine command which we not need here */
#undef	CONFIG_BOOTM_LINUX
#undef	CONFIG_BOOTM_NETBSD
#undef	CONFIG_BOOTM_PLAN9
#undef	CONFIG_BOOTM_RTEMS
#undef	CONFIG_GZIP
#undef	CONFIG_ZLIB
#undef CONFIG_CMD_CRC32

/* USB configuration */
#define CONFIG_USB_MUSB_DSPS
#define CONFIG_ARCH_MISC_INIT
#define CONFIG_MUSB_PIO_ONLY
#define CONFIG_MUSB_DISABLE_BULK_COMBINE_SPLIT
/* attention! not only for gadget, enables also highspeed in hostmode */
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_MUSB_HOST
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE	MUSB_HOST

#ifdef CONFIG_MUSB_HOST
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#endif /* CONFIG_MUSB_HOST */

#undef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		0x40000	/* TODO: Adresse definieren */
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
/*
 * Common filesystems support.  When we have removable storage we
 * enabled a number of useful commands and support.
 */
#if defined(CONFIG_MMC) || defined(CONFIG_USB_STORAGE)
#define CONFIG_DOS_PARTITION
#define CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_CMD_FS_GENERIC
#endif /* CONFIG_MMC, ... */

#endif	/* ! __CONFIG_TSERIES_H__ */

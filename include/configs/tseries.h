/*
 * tseries.h
 *
 * specific parts for B&R T-Series Motherboard
 *
 * Copyright (C) 2013 Hannes Petermaier <oe5hpm@oevsv.at> -
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:        GPL-2.0+
 */

#ifndef __CONFIG_TSERIES_H__
#define __CONFIG_TSERIES_H__

#include <configs/bur_am335x_common.h>
/* ------------------------------------------------------------------------- */
/* Clock Defines */
#define V_OSCK				26000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_POWER_TPS65217

/* Support both device trees and ATAGs. */
#define CONFIG_OF_LIBFDT
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_CMD_BOOTZ
/*#define CONFIG_MACH_TYPE		3589*/
#define CONFIG_MACH_TYPE		0xFFFFFFFF /* TODO: check with kernel*/

/* MMC/SD IP block */
#if defined(CONFIG_EMMC_BOOT)
 #define CONFIG_MMC
 #define CONFIG_GENERIC_MMC
 #define CONFIG_OMAP_HSMMC
 #define CONFIG_CMD_MMC
 #define CONFIG_SUPPORT_EMMC_BOOT
/* RAW SD card / eMMC locations. */
 #define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /*addr. 0x60000 */
 #define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS		0x200 /* 256 KB */
 #define CONFIG_SPL_MMC_SUPPORT
#endif /* CONFIG_EMMC_BOOT */

/*
 * When we have SPI or NAND flash we expect to be making use of mtdparts,
 * both for ease of use in U-Boot and for passing information on to
 * the Linux kernel.
 */
#if defined(CONFIG_SPI_BOOT) || defined(CONFIG_NAND)
#define CONFIG_MTD_DEVICE		/* Required for mtdparts */
#define CONFIG_CMD_MTDPARTS
#endif /* CONFIG_SPI_BOOT, ... */

#undef CONFIG_SPL_OS_BOOT
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_SPL_ARGS_ADDR		0x80F80000

/* RAW SD card / eMMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x900	/* address 0x120000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x80	/* address 0x10000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0x80	/* 64KiB */

/* NAND */
#ifdef CONFIG_NAND
#define CONFIG_CMD_SPL_NAND_OFS			0x080000 /* end of u-boot */
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS		0x140000
#define CONFIG_CMD_SPL_WRITE_SIZE		0x2000
#endif /* CONFIG_NAND */
#endif /* CONFIG_SPL_OS_BOOT */

#ifdef CONFIG_NAND
#define CONFIG_SPL_NAND_AM33XX_BCH	/* OMAP4 and later ELM support */
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#endif /* CONFIG_NAND */

/* Always 128 KiB env size */
#define CONFIG_ENV_SIZE			(128 << 10)

#ifdef CONFIG_NAND
#define NANDARGS \
	"mtdids=" MTDIDS_DEFAULT "\0" \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
	"nandroot=ubi0:rootfs rw ubi.mtd=8,2048\0" \
	"nandrootfstype=ubifs rootwait=1\0" \
	"nandimgsize=0x500000\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} kernel ${nandimgsize}; " \
		"bootz ${loadaddr}\0"
#else
#define NANDARGS ""
#endif /* CONFIG_NAND */

#ifdef CONFIG_MMC
#define MMCARGS \
	"silent=1\0"
#else
#define MMCARGS ""
#endif /* CONFIG_MMC */

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
	"autoload=0\0" \
	"loadaddr=0x80200000\0" \
	"bootfile=zImage\0" \
	"console=ttyO0,115200n8\0" \
	"optargs=\0" \
	"rootpath=/tftpboot/tseries/rootfs-small\0" \
	"nfsopts=nolock\0" \
	"netargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} rw " \
		"ip=dhcp\0" \
	"netboot=echo Booting from network ...; " \
		"setenv autoload no; " \
		"dhcp; " \
		"tftp ${loadaddr} ${bootfile}; " \
		"run netargs; " \
		"bootm ${loadaddr}\0" \
	"usbupdate=echo Updating UBOOT from USB-Stick ...; " \
		"usb start; " \
		"fatload usb 0 0x80000000 updateubootusb.img; " \
		"source;\0" \
	"netupdate=echo Updating UBOOT from Network (TFTP) ...; " \
		"setenv autoload 0; " \
		"dhcp;" \
		"tftp 0x80000000 updateUBOOT.img;" \
		"source;\0" \
	NANDARGS \
	MMCARGS
#endif /* !CONFIG_SPL_BUILD*/

#define CONFIG_BOOTCOMMAND \
	"run mmcboot1;"
#define CONFIG_BOOTDELAY		1 /* TODO: für release auf 0 setzen */

#ifdef CONFIG_NAND
/*
 * GPMC  block.  We support 1 device and the physical address to
 * access CS0 at is 0x8000000.
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x8000000
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_CMD_NAND
/* don't change OMAP_ELM, ECCSCHEME. ROM code only supports this */
#define CONFIG_NAND_OMAP_ELM
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9, \
					10, 11, 12, 13, 14, 15, 16, 17, \
					18, 19, 20, 21, 22, 23, 24, 25, \
					26, 27, 28, 29, 30, 31, 32, 33, \
					34, 35, 36, 37, 38, 39, 40, 41, \
					42, 43, 44, 45, 46, 47, 48, 49, \
					50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14

#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

#define MTDIDS_DEFAULT			"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT		"mtdparts=omap2-nand.0:" \
					"128k(SPL)," \
					"128k(SPL.backup1)," \
					"128k(SPL.backup2)," \
					"128k(SPL.backup3)," \
					"512k(u-boot)," \
					"128k(u-boot-spl-os)," \
					"128k(u-boot-env)," \
					"5m(kernel),"\
					"-(rootfs)"
#endif /* CONFIG_NAND */

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
#define CONFIG_AM335X_USB1
#define CONFIG_AM335X_USB1_MODE MUSB_HOST

#ifdef CONFIG_MUSB_HOST
#define CONFIG_CMD_USB
#define CONFIG_USB_STORAGE
#endif /* CONFIG_MUSB_HOST */

#if defined(CONFIG_SPI_BOOT)
/* McSPI IP block */
#define CONFIG_SPI
#define CONFIG_OMAP3_SPI
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_SF_DEFAULT_SPEED		24000000

#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SPL_SPI_BUS		0
#define CONFIG_SPL_SPI_CS		0
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#undef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#define CONFIG_ENV_SECT_SIZE		(4 << 10) /* 4 KB sectors */
#define CONFIG_ENV_OFFSET		(768 << 10) /* 768 KiB in */
#define CONFIG_ENV_OFFSET_REDUND	(896 << 10) /* 896 KiB in */

#elif defined(CONFIG_EMMC_BOOT)
#undef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		0x40000	/* TODO: Adresse definieren */
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT

#elif defined(CONFIG_NAND)
#undef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x120000 /* TODO: Adresse definieren */
#define CONFIG_SYS_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#else
#error "no storage for Environment defined!"
#endif
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

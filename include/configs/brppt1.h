/*
 * brtpp1.h
 *
 * specific parts for B&R T-Series Motherboard
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:        GPL-2.0+
 */

#ifndef __CONFIG_BRPPT1_H__
#define __CONFIG_BRPPT1_H__

#include <configs/bur_cfg_common.h>
#include <configs/bur_am335x_common.h>
/* ------------------------------------------------------------------------- */
#define CONFIG_AM335X_LCD
#define CONFIG_LCD_ROTATION
#define CONFIG_LCD_DT_SIMPLEFB
#define LCD_BPP				LCD_COLOR32

/* Bootcount using the RTC block */
#define CONFIG_SYS_BOOTCOUNT_ADDR	0x44E3E000
#define CONFIG_BOOTCOUNT_LIMIT
#define CONFIG_BOOTCOUNT_AM33XX

/* memory */
#define CONFIG_SYS_MALLOC_LEN		(5 * 1024 * 1024)

/* Clock Defines */
#define V_OSCK				26000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_POWER_TPS65217

/* Support both device trees and ATAGs. */
#define CONFIG_USE_FDT			/* use fdt within board code */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
/*#define CONFIG_MACH_TYPE		3589*/
#define CONFIG_MACH_TYPE		0xFFFFFFFF /* TODO: check with kernel*/

/* MMC/SD IP block */
#if defined(CONFIG_EMMC_BOOT)
 #define CONFIG_SUPPORT_EMMC_BOOT
#endif /* CONFIG_EMMC_BOOT */

/*
 * When we have SPI or NAND flash we expect to be making use of mtdparts,
 * both for ease of use in U-Boot and for passing information on to
 * the Linux kernel.
 */
#if defined(CONFIG_SPI_BOOT) || defined(CONFIG_NAND)
#define CONFIG_MTD_DEVICE		/* Required for mtdparts */
#endif /* CONFIG_SPI_BOOT, ... */

#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_SPL_ARGS_ADDR		0x80F80000

/* RAW SD card / eMMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x900	/* address 0x120000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x80	/* address 0x10000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0x80	/* 64KiB */

/* NAND */
#ifdef CONFIG_NAND
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS		0x140000
#endif /* CONFIG_NAND */
#endif /* CONFIG_SPL_OS_BOOT */

#ifdef CONFIG_NAND
#define CONFIG_SPL_NAND_AM33XX_BCH	/* OMAP4 and later ELM support */
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#endif /* CONFIG_NAND */

/* Always 64 KiB env size */
#define CONFIG_ENV_SIZE			(64 << 10)

#ifdef CONFIG_NAND
#define NANDARGS \
	"mtdids=" MTDIDS_DEFAULT "\0" \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"${optargs_rot} " \
		"root=mtd6 " \
		"rootfstype=jffs2\0" \
	"kernelsize=0x400000\0" \
	"nandboot=echo booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} kernel ${kernelsize}; " \
		"bootz ${loadaddr} - ${dtbaddr}\0" \
	"defboot=run nandboot\0" \
	"bootlimit=1\0" \
	"simplefb=1\0 " \
	"altbootcmd=run usbscript\0"
#else
#define NANDARGS ""
#endif /* CONFIG_NAND */

#ifdef CONFIG_MMC
#define MMCARGS \
"dtbdev=mmc\0" \
"dtbpart=1:1\0" \
"mmcroot0=setenv bootargs ${optargs_rot} ${optargs} console=${console}\0" \
"mmcroot1=setenv bootargs ${optargs_rot} ${optargs} console=${console} " \
	"root=/dev/mmcblk0p2 rootfstype=ext4\0" \
"mmcboot0=echo booting Updatesystem from mmc (ext4-fs) ...; " \
	"setenv simplefb 1; " \
	"ext4load mmc 1:1 ${loadaddr} /${kernel}; " \
	"ext4load mmc 1:1 ${ramaddr} /${ramdisk}; " \
	"run mmcroot0; bootz ${loadaddr} ${ramaddr} ${dtbaddr};\0" \
"mmcboot1=echo booting PPT-OS from mmc (ext4-fs) ...; " \
	"setenv simplefb 0; " \
	"ext4load mmc 1:2 ${loadaddr} /boot/${kernel}; " \
	"run mmcroot1; bootz ${loadaddr} - ${dtbaddr};\0" \
"defboot=ext4load mmc 1:2 ${loadaddr} /boot/PPTImage.md5 && run mmcboot1; " \
	"ext4load mmc 1:1 ${dtbaddr} /$dtb && run mmcboot0; " \
	"run ramboot; run usbscript;\0" \
"bootlimit=1\0" \
"altbootcmd=mmc dev 1; run mmcboot0;\0" \
"upduboot=dhcp; " \
	"tftp ${loadaddr} MLO && mmc write ${loadaddr} 100 100; " \
	"tftp ${loadaddr} u-boot.img && mmc write ${loadaddr} 300 400;\0"
#else
#define MMCARGS ""
#endif /* CONFIG_MMC */

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
BUR_COMMON_ENV \
"verify=no\0" \
"autoload=0\0" \
"dtb=bur-ppt-ts30.dtb\0" \
"dtbaddr=0x80100000\0" \
"loadaddr=0x80200000\0" \
"ramaddr=0x80A00000\0" \
"kernel=zImage\0" \
"ramdisk=rootfs.cpio.uboot\0" \
"console=ttyO0,115200n8\0" \
"optargs=consoleblank=0 quiet panic=2\0" \
"nfsroot=/tftpboot/tseries/rootfs-small\0" \
"nfsopts=nolock\0" \
"ramargs=setenv bootargs ${optargs} console=${console} root=/dev/ram0\0" \
"netargs=setenv bootargs console=${console} " \
	"${optargs} " \
	"root=/dev/nfs " \
	"nfsroot=${serverip}:${nfsroot},${nfsopts} rw " \
	"ip=dhcp\0" \
"netboot=echo Booting from network ...; " \
	"dhcp; " \
	"tftp ${loadaddr} ${kernel}; " \
	"tftp ${dtbaddr} ${dtb}; " \
	"run netargs; " \
	"bootz ${loadaddr} - ${dtbaddr}\0" \
"ramboot=echo Booting from network into RAM ...; "\
	"if dhcp; then; " \
	"tftp ${loadaddr} ${kernel}; " \
	"tftp ${ramaddr} ${ramdisk}; " \
	"if ext4load ${dtbdev} ${dtbpart} ${dtbaddr} /${dtb}; " \
	"then; else tftp ${dtbaddr} ${dtb}; fi;" \
	"run mmcroot0; " \
	"bootz ${loadaddr} ${ramaddr} ${dtbaddr}; fi;\0" \
"netupdate=echo Updating UBOOT from Network (TFTP) ...; " \
	"setenv autoload 0; " \
	"dhcp && tftp 0x80000000 updateUBOOT.img && source;\0" \
NANDARGS \
MMCARGS
#endif /* !CONFIG_SPL_BUILD*/

#define CONFIG_BOOTCOMMAND \
	"mmc dev 1; run defboot;"

#ifdef CONFIG_NAND
/*
 * GPMC  block.  We support 1 device and the physical address to
 * access CS0 at is 0x8000000.
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x8000000
#define CONFIG_NAND_OMAP_GPMC
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
					"128k(MLO)," \
					"128k(MLO.backup)," \
					"128k(dtb)," \
					"128k(u-boot-env)," \
					"512k(u-boot)," \
					"4m(kernel),"\
					"128m(rootfs),"\
					"-(user)"
#define CONFIG_NAND_OMAP_GPMC_WSCFG	1
#endif /* CONFIG_NAND */

/* USB configuration */
#define CONFIG_USB_MUSB_DSPS
#define CONFIG_USB_MUSB_PIO_ONLY
#define CONFIG_USB_MUSB_DISABLE_BULK_COMBINE_SPLIT
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE	MUSB_HOST
#define CONFIG_AM335X_USB1
#define CONFIG_AM335X_USB1_MODE MUSB_HOST

#if defined(CONFIG_SPI_BOOT)
/* McSPI IP block */
#define CONFIG_SPI
#define CONFIG_SF_DEFAULT_SPEED		24000000

#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#define CONFIG_ENV_SECT_SIZE		(4 << 10) /* 4 KB sectors */
#define CONFIG_ENV_OFFSET		(768 << 10) /* 768 KiB in */
#define CONFIG_ENV_OFFSET_REDUND	(896 << 10) /* 896 KiB in */

#elif defined(CONFIG_EMMC_BOOT)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		0x40000	/* TODO: Adresse definieren */
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT

#elif defined(CONFIG_NAND)
/* No NAND env support in SPL */
#define CONFIG_ENV_OFFSET		0x60000
#define CONFIG_SYS_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#else
#error "no storage for Environment defined!"
#endif
/*
 * Common filesystems support.  When we have removable storage we
 * enabled a number of useful commands and support.
 */
#if defined(CONFIG_MMC) || defined(CONFIG_USB_STORAGE)
#define CONFIG_FS_EXT4
#define CONFIG_EXT4_WRITE
#endif /* CONFIG_MMC, ... */

#endif	/* ! __CONFIG_BRPPT1_H__ */

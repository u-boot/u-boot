/*
 * kwb.h
 *
 * specific parts for B&R KWB Motherboard
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:        GPL-2.0+
 */

#ifndef __CONFIG_KWB_H__
#define __CONFIG_KWB_H__

#include <configs/bur_cfg_common.h>
#include <configs/bur_am335x_common.h>
/* ------------------------------------------------------------------------- */
#define CONFIG_AM335X_LCD
#define CONFIG_LCD
#define CONFIG_LCD_NOSTDOUT
#define CONFIG_SYS_WHITE_ON_BLACK
#define LCD_BPP				LCD_COLOR32

#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE	(1366*767*4)
#define CONFIG_CMD_UNZIP
#define CONFIG_CMD_BMP
#define CONFIG_BMP_24BMP
#define CONFIG_BMP_32BPP

/* memory */
#define CONFIG_SYS_MALLOC_LEN		(5 * 1024 * 1024)

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
#define CONFIG_SUPPORT_EMMC_BOOT
/* RAW SD card / eMMC locations. */
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /*addr. 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS		0x200 /* 256 KB */
#define CONFIG_SPL_MMC_SUPPORT

/* Always 64 KiB env size */
#define CONFIG_ENV_SIZE			(64 << 10)

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
BUR_COMMON_ENV \
"bootaddr=0x80001100\0" \
"bootdev=cpsw(0,0)\0" \
"vx_romfsbase=0x800E0000\0" \
"vx_romfssize=0x20000\0" \
"vx_memtop=0x8FBEF000\0" \
"loadromfs=mmc read ${vx_romfsbase} 700 100\0" \
"autoload=0\0" \
"loadaddr=0x80100000\0" \
"logoaddr=0x82000000\0" \
"defaultARlen=0x8000\0" \
"loaddefaultAR=mmc read ${loadaddr} 800 ${defaultARlen}\0" \
"defaultAR=run loadromfs; run loaddefaultAR; bootvx ${loadaddr}\0" \
"logo0=fatload mmc 0:1 ${logoaddr} SYSTEM/ADDON/Bootlogo/Bootlogo.bmp.gz && " \
	"bmp display ${logoaddr} 0 0\0" \
"logo1=fatload mmc 0:1 ${logoaddr} SYSTEM/BASE/Bootlogo/Bootlogo.bmp.gz && " \
	"bmp display ${logoaddr} 0 0\0" \
"mmcboot=echo booting AR from eMMC-flash ...; "\
	"run logo0 || run logo1; " \
	"run loadromfs; " \
	"fatload mmc 0:1 ${loadaddr} arimg && bootvx ${loadaddr}; " \
	"run defaultAR;\0" \
"netboot=echo booting AR from network ...; " \
	"run loadromfs; " \
	"tftp ${loadaddr} arimg && bootvx ${loadaddr}; " \
	"puts 'networkboot failed!';\0" \
"netscript=echo running script from network (tftp) ...; " \
	"tftp 0x80000000 netscript.img && source; " \
	"puts 'netscript load failed!'\0" \
"netupdate=tftp ${loadddr} MLO && mmc write ${loadaddr} 100 100; " \
	"tftp ${loadaddr} u-boot.img && mmc write ${loadaddr} 300 300\0" \
"netupdatedefaultAR=echo updating defaultAR from network (tftp) ...; " \
	"if tftp 0x80100000 arimg.bin; " \
	"then mmc write 0x80100000 800 ${defaultARlen}; " \
	"else setcurs 1 8; puts 'defAR update failed (tftp)!'; fi;\0" \
"netupdateROMFS=echo updating romfs from network (tftp) ...; " \
	"if tftp 0x80100000 romfs.bin; " \
	"then mmc write 0x80100000 700 100; " \
	"else setcurs 1 8; puts 'romfs update failed (tftp)!'; fi;\0"

#endif /* !CONFIG_SPL_BUILD*/

#define CONFIG_BOOTCOMMAND \
	"run usbscript;"
#define CONFIG_BOOTDELAY		0

/* undefine command which we not need here */
#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_PLAN9
#undef CONFIG_BOOTM_RTEMS

/* Support both device trees and ATAGs. */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* USB configuration */
#define CONFIG_USB_MUSB_DSPS
#define CONFIG_ARCH_MISC_INIT
#define CONFIG_USB_MUSB_PIO_ONLY
#define CONFIG_USB_MUSB_DISABLE_BULK_COMBINE_SPLIT
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE	MUSB_HOST
#define CONFIG_AM335X_USB1
#define CONFIG_AM335X_USB1_MODE	MUSB_HOST

#undef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
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
#define CONFIG_FAT_WRITE
#endif /* CONFIG_MMC, ... */

#endif	/* __CONFIG_KWB_H__ */

/*
 * brxre1.h
 *
 * specific parts for B&R KWB Motherboard
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 * SPDX-License-Identifier:        GPL-2.0+
 */

#ifndef __CONFIG_BRXRE1_H__
#define __CONFIG_BRXRE1_H__

#include <configs/bur_cfg_common.h>
#include <configs/bur_am335x_common.h>
/* ------------------------------------------------------------------------- */
#define CONFIG_AM335X_LCD
#define LCD_BPP				LCD_COLOR32

/* memory */
#define CONFIG_SYS_MALLOC_LEN		(5 * 1024 * 1024)

/* Clock Defines */
#define V_OSCK				26000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_POWER_TPS65217

#define CONFIG_MACH_TYPE		3589
/* I2C IP block */
#define CONFIG_SYS_OMAP24_I2C_SPEED_PSOC	20000

/* MMC/SD IP block */
#define CONFIG_SUPPORT_EMMC_BOOT

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
"loadromfs=mmc dev 1; mmc read ${vx_romfsbase} 700 100\0" \
"autoload=0\0" \
"loadaddr=0x80100000\0" \
"defaultARlen=0x8000\0" \
"loaddefaultAR=mmc dev 1; mmc read ${loadaddr} 800 ${defaultARlen}\0" \
"defaultAR=run loadromfs; run loaddefaultAR; bootvx ${loadaddr}\0" \
"mmcboot=echo booting AR from eMMC-flash ...; "\
	"run loadromfs; " \
	"fatload mmc 1:1 ${loadaddr} arimg && bootvx ${loadaddr}; " \
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
#define CONFIG_USB_MUSB_PIO_ONLY
#define CONFIG_USB_MUSB_DISABLE_BULK_COMBINE_SPLIT
#define CONFIG_AM335X_USB0
#define CONFIG_AM335X_USB0_MODE	MUSB_HOST
#define CONFIG_AM335X_USB1
#define CONFIG_AM335X_USB1_MODE	MUSB_HOST

#undef CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		0x40000	/* TODO: Adresse definieren */
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT

#endif	/* __CONFIG_BRXRE1_H__ */

/*
 * Configuration settings for the TI OMAP3 EVM board.
 *
 * Copyright (C) 2006-2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Author :
 *	Manikandan Pillai <mani.pillai@ti.com>
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 * Manikandan Pillai <mani.pillai@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __OMAP3EVM_CONFIG_H
#define __OMAP3EVM_CONFIG_H

#include <asm/arch/cpu.h>
#include <asm/arch/omap3.h>

/* ----------------------------------------------------------------------------
 * Supported U-boot commands
 * ----------------------------------------------------------------------------
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV

#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_JFFS2

#define CONFIG_CMD_I2C
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NAND
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING

#undef CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

/* ----------------------------------------------------------------------------
 * Supported U-boot features
 * ----------------------------------------------------------------------------
 */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER

/* Display CPU and Board information */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* Add auto-completion support */
#define CONFIG_AUTO_COMPLETE

/* ----------------------------------------------------------------------------
 * Supported hardware
 * ----------------------------------------------------------------------------
 */

/* MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_OMAP_HSMMC

/* SPL */
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMC_SD_FAT_BOOT_PARTITION	1
#define CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME	"u-boot.img"

/* Partition tables */
#define CONFIG_EFI_PARTITION
#define CONFIG_DOS_PARTITION

/* USB
 *
 * Enable CONFIG_MUSB_HCD for Host functionalities MSC, keyboard
 * Enable CONFIG_MUSB_UDD for Device functionalities.
 */
#define CONFIG_USB_OMAP3
#define CONFIG_MUSB_HCD
/* #define CONFIG_MUSB_UDC */

/* NAND SPL */
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9,\
						10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_HW
#define CONFIG_SYS_NAND_U_BOOT_START   CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

/* -----------------------------------------------------------------------------
 * Include common board configuration
 * -----------------------------------------------------------------------------
 */
#include "omap3_evm_common.h"

/* -----------------------------------------------------------------------------
 * Default environment
 * -----------------------------------------------------------------------------
 */
#define CONFIG_BOOTDELAY	3

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"usbtty=cdc_acm\0" \
	"mmcdev=0\0" \
	"console=ttyO0,115200n8\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=/dev/mmcblk0p2 rw " \
		"rootfstype=ext3 rootwait\0" \
	"nandargs=setenv bootargs console=${console} " \
		"root=/dev/mtdblock4 rw " \
		"rootfstype=jffs2\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"onenand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run nandboot; " \
			"fi; " \
		"fi; " \
	"else run nandboot; fi"

#endif /* __OMAP3EVM_CONFIG_H */

/*
 * Copyright (C) 2015, Savoir-faire Linux Inc.
 *
 * Derived from MX51EVK code by
 *   Guennadi Liakhovetski <lg@denx.de>
 *   Freescale Semiconductor, Inc.
 *
 * Configuration settings for the TS4800 Board
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* High Level Configuration Options */
#define CONFIG_MX51

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#define CONFIG_SYS_NO_FLASH		/* No NOR Flash */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is a 2nd stage bootloader */

#define CONFIG_HW_WATCHDOG

#define CONFIG_OF_LIBFDT

#define CONFIG_MACH_TYPE	MACH_TYPE_TS48XX

/* text base address used when linking */
#define CONFIG_SYS_TEXT_BASE	0x90008000

#include <asm/arch/imx-regs.h>

/* enable passing of ATAGs */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(10 * 1024 * 1024)

/*
 * Hardware drivers
 */

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE	UART1_BASE
#define CONFIG_MXC_GPIO

/*
 * SPI Configs
 * */
#define CONFIG_HARD_SPI /* puts SPI: ready */
#define CONFIG_MXC_SPI /* driver for the SPI controllers*/
#define CONFIG_CMD_SPI /* SPI serial bus support */

/*
 * MMC Configs
 * */
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	MMC_SDHC1_BASE_ADDR

#define CONFIG_MMC

#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

/*
 * Eth Configs
 */
#define CONFIG_MII
#define CONFIG_PHYLIB
#define CONFIG_PHY_SMSC

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE	        FEC_BASE_ADDR
#define CONFIG_ETHPRIME		"FEC"
#define CONFIG_FEC_MXC_PHYADDR	0

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE		/* disable vendor parameters protection (serial#, ethaddr) */
#define CONFIG_CONS_INDEX		1 /* use UART0 : used by serial driver */
#define CONFIG_BAUDRATE			115200

/***********************************************************
 * Command definition
 ***********************************************************/

#define CONFIG_CMD_BOOTZ
#undef CONFIG_CMD_IMLS

/* Environment variables */

#define CONFIG_BOOTDELAY	1

#define CONFIG_LOADADDR		0x91000000	/* loadaddr env var */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=uImage\0" \
	"mmcdev=0\0" \
	"mmcpart=1\0" \
	"mmcargs=setenv bootargs root=/dev/mmcblk0p2 rootwait rw\0" \
	"addtty=setenv bootargs ${bootargs} console=ttymxc0,${baudrate}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image};\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs addtty; " \
                "bootm; "

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loadimage; then " \
				"run mmcboot; " \
			"fi; " \
		"fi; " \
	"fi; "

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS	16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size */

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM_1		CSD0_BASE_ADDR
#define PHYS_SDRAM_1_SIZE	(256 * 1024 * 1024)

#define CONFIG_SYS_SDRAM_BASE		(PHYS_SDRAM_1)
#define CONFIG_SYS_INIT_RAM_ADDR	(IRAM_BASE_ADDR)
#define CONFIG_SYS_INIT_RAM_SIZE	(IRAM_SIZE)

#define CONFIG_BOARD_EARLY_INIT_F

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Low level init */
#define CONFIG_SYS_DDR_CLKSEL	0
#define CONFIG_SYS_CLKTL_CBCDR	0x59E35100
#define CONFIG_SYS_MAIN_PWR_ON

/*-----------------------------------------------------------------------
 * Environment organization
 */

#define CONFIG_ENV_OFFSET      (6 * 64 * 1024)
#define CONFIG_ENV_SIZE        (8 * 1024)
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_SYS_MMC_ENV_DEV 0

#endif

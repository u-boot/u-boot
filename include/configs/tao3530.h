/*
 * Configuration settings for the TechNexion TAO-3530 SOM
 * equipped on Thunder baseboard.
 *
 * Edward Lin <linuxfae@technexion.com>
 * Tapani Utriainen <linuxfae@technexion.com>
 *
 * Copyright (C) 2013 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_OMAP			/* in a TI OMAP core */

#define CONFIG_OMAP_GPIO
#define CONFIG_OMAP_COMMON
#define CONFIG_SYS_GENERIC_BOARD
/* Common ARM Erratas */
#define CONFIG_ARM_ERRATA_454179
#define CONFIG_ARM_ERRATA_430973
#define CONFIG_ARM_ERRATA_621766

#define MACH_TYPE_OMAP3_TAO3530		2836

#define CONFIG_SDRC			/* Has an SDRC controller */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_MISC_INIT_R

#define CONFIG_OF_LIBFDT

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(4 << 20)
#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB sector */

/*
 * Hardware drivers
 */

/*
 * NS16550 Configuration
 */
#define V_NS16550_CLK			48000000	/* 48MHz (APLL96/2) */

#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		V_NS16550_CLK

/*
 * select serial console configuration
 */
#define CONFIG_CONS_INDEX		3
#define CONFIG_SYS_NS16550_COM3		OMAP34XX_UART3

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_DOS_PARTITION

/* GPIO banks */
#define CONFIG_OMAP3_GPIO_2		/* GPIO32 ..63  is in GPIO bank 2 */
#define CONFIG_OMAP3_GPIO_3		/* GPIO64 ..95  is in GPIO bank 3 */
#define CONFIG_OMAP3_GPIO_4		/* GPIO96 ..127 is in GPIO bank 4 */
#define CONFIG_OMAP3_GPIO_5		/* GPIO128..159 is in GPIO bank 5 */
#define CONFIG_OMAP3_GPIO_6		/* GPIO160..191 is in GPIO bank 6 */

/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_MTDPARTS	/* Enable MTD parts commands */
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define MTDIDS_DEFAULT			"nand0=nand"
#define MTDPARTS_DEFAULT		"mtdparts=nand:512k(x-loader),"\
					"1920k(u-boot),128k(u-boot-env),"\
					"4m(kernel),-(fs)"

#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_CMD_NAND		/* NAND support			*/
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING

#undef CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMI		/* iminfo			*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_OMAP34XX
#define CONFIG_SYS_OMAP24_I2C_SPEED	100000
#define CONFIG_SYS_OMAP24_I2C_SLAVE	1
#define CONFIG_I2C_MULTI_BUS

/*
 * TWL4030
 */
#define CONFIG_TWL4030_POWER
#define CONFIG_TWL4030_LED

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_NAND_QUIET_TEST
#define CONFIG_NAND_OMAP_GPMC
#define CONFIG_SYS_NAND_ADDR		NAND_BASE	/* physical address */
							/* to access nand */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access nand at */
							/* CS0 */

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
							/* devices */
#define CONFIG_SYS_NAND_BUSWIDTH_16BIT	16
/* Environment information */
#define CONFIG_BOOTDELAY		3

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyO2,115200n8\0" \
	"mpurate=600\0" \
	"dvi_mode=omapfb.mode=dvi:1280x720-24@60\0" \
	"tv_mode=omapfb.mode=tv:ntsc\0" \
	"video_mode=omapdss.def_disp=lcd vram=6M omapfb.vram=0:2M,1:2M,2:2M\0" \
	"lcd_mode=omapfb.mode=lcd:800x480@60 \0" \
	"extra_options= \0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"nandroot=ubi0:rootfs ubi.mtd=4\0" \
	"nandrootfstype=ubifs\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"mpurate=${mpurate} " \
		"${video_mode} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype} " \
		"${extra_options}\0" \
	"nandargs=setenv bootargs console=${console} " \
		"mpurate=${mpurate} " \
		"${video_mode} " \
		"${network_setting} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype} "\
		"${extra_options}\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"if mmc rescan ${mmcdev}; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"else run nandboot; " \
			"fi; " \
		"fi; " \
	"else run nandboot; fi"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"TAO-3530 # "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */

/* turn on command-line edit/hist/auto */
#define CONFIG_CMDLINE_EDITING
#define CONFIG_COMMAND_HISTORY
#define CONFIG_AUTO_COMPLETE

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_SYS_ALT_MEMTEST		1
#define CONFIG_SYS_MEMTEST_START	(0x82000000)		/* memtest */
								/* defaults */
#define CONFIG_SYS_MEMTEST_END		(0x83FFFFFF)		/* 64MB */
#define CONFIG_SYS_MEMTEST_SCRATCH	(0x81000000)	/* dummy address */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0)	/* default */
							/* load address */
#define CONFIG_SYS_TEXT_BASE		0x80008000

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128 << 10)	/* regular stack 128 KiB */

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	2	/* CS1 may or may not be populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	(32 << 20)	/* at least 32 MiB */
#define PHYS_SDRAM_2		OMAP34XX_SDRC_CS1

/*
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */
#define CONFIG_SYS_FLASH_BASE		NAND_BASE

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP

#define CONFIG_ENV_IS_IN_NAND		1
#define ONENAND_ENV_OFFSET		0x260000 /* environment starts here */
#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - \
					 GENERATED_GBL_DATA_SIZE)

#define CONFIG_OMAP3_SPI

/*
 * USB
 *
 * Currently only EHCI is enabled, the MUSB OTG controller
 * is not enabled.
 */

/* USB EHCI */
#define CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_OMAP
#define CONFIG_OMAP_EHCI_PHY1_RESET_GPIO	162

#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS 3
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_SMSC95XX

#define CONFIG_USB_ETHER
#define CONFIG_USB_ETHER_RNDIS
#define CONFIG_USB_STORAGE
#define CONGIG_CMD_STORAGE

/* Defines for SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_NAND_SIMPLE

#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_OMAP3_ID_NAND
#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/omap-common/u-boot-spl.lds"

/* NAND boot config */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128 * 1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
/*
 * Use the ECC/OOB layout from omap_gpmc.h that matches your chip:
 * SP vs LP, 8bit vs 16bit: GPMC_NAND_HW_ECC_LAYOUT
 */
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13 }
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_HW

#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

#define CONFIG_SPL_TEXT_BASE		0x40200800
#define CONFIG_SPL_MAX_SIZE		(54 * 1024)	/* 8 KB for stack */

/*
 * Use 0x80008000 as TEXT_BASE here for compatibility reasons with the
 * older x-loader implementations. And move the BSS area so that it
 * doesn't overlap with TEXT_BASE.
 */
#define CONFIG_SYS_TEXT_BASE		0x80008000
#define CONFIG_SPL_BSS_START_ADDR	0x80100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */

#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

#endif /* __CONFIG_H */

/*
 * (C) Copyright 2011 CompuLab, Ltd.
 * Mike Rapoport <mike@compulab.co.il>
 * Igor Grinberg <grinberg@compulab.co.il>
 *
 * Based on omap3_beagle.h
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * Configuration settings for the CompuLab CM-T35 and CM-T3730 boards
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */
#define CONFIG_OMAP	/* in a TI OMAP core */
#define CONFIG_OMAP34XX	/* which is a 34XX */
#define CONFIG_OMAP_GPIO
#define CONFIG_CM_T3X	/* working with CM-T35 and CM-T3730 */

#define CONFIG_SYS_TEXT_BASE	0x80008000

#define CONFIG_SDRC	/* The chip has SDRC controller */

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap3.h>

/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Clock Defines */
#define V_OSCK			26000000	/* Clock output from T2 */
#define V_SCLK			(V_OSCK >> 1)

#define CONFIG_MISC_INIT_R

#define CONFIG_OF_LIBFDT		1
/*
 * The early kernel mapping on ARM currently only maps from the base of DRAM
 * to the end of the kernel image.  The kernel is loaded at DRAM base + 0x8000.
 * The early kernel pagetable uses DRAM base + 0x4000 to DRAM base + 0x8000,
 * so that leaves DRAM base to DRAM base + 0x4000 available.
 */
#define CONFIG_SYS_BOOTMAPSZ	        0x4000

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_SERIAL_TAG

/*
 * Size of malloc() pool
 */
#define CONFIG_ENV_SIZE		(16 << 10)	/* 16 KiB */
					/* Sector */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + (128 << 10))

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
#define CONFIG_SERIAL3			3	/* UART3 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{4800, 9600, 19200, 38400, 57600,\
					115200}

#define CONFIG_GENERIC_MMC
#define CONFIG_MMC
#define CONFIG_OMAP_HSMMC
#define CONFIG_DOS_PARTITION

/* USB */
#define CONFIG_USB_OMAP3
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_OMAP
#define CONFIG_USB_ULPI
#define CONFIG_USB_ULPI_VIEWPORT_OMAP
#define CONFIG_USB_STORAGE
#define CONFIG_MUSB_UDC
#define CONFIG_TWL4030_USB
#define CONFIG_CMD_USB

/* USB device configuration */
#define CONFIG_USB_DEVICE
#define CONFIG_USB_TTY
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/* commands to include */
#include <config_cmd_default.h>

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EXT2		/* EXT2 Support			*/
#define CONFIG_CMD_FAT		/* FAT support			*/
#define CONFIG_CMD_JFFS2	/* JFFS2 Support		*/
#define CONFIG_CMD_MTDPARTS	/* Enable MTD parts commands */
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define MTDIDS_DEFAULT		"nand0=nand"
#define MTDPARTS_DEFAULT	"mtdparts=nand:512k(x-loader),"\
				"1920k(u-boot),128k(u-boot-env),"\
				"4m(kernel),-(fs)"

#define CONFIG_CMD_I2C		/* I2C serial bus support	*/
#define CONFIG_CMD_MMC		/* MMC support			*/
#define CONFIG_CMD_NAND		/* NAND support			*/
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING

#undef CONFIG_CMD_FLASH		/* flinfo, erase, protect	*/
#undef CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#undef CONFIG_CMD_IMLS		/* List all found images	*/

#define CONFIG_SYS_NO_FLASH
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_I2C_SLAVE		1
#define CONFIG_SYS_I2C_BUS		0
#define CONFIG_SYS_I2C_BUS_SELECT	1
#define CONFIG_DRIVER_OMAP34XX_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
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
#define GPMC_NAND_ECC_LP_x8_LAYOUT

#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
							/* devices */
#define CONFIG_JFFS2_NAND
/* nand device jffs2 lives on */
#define CONFIG_JFFS2_DEV		"nand0"
/* start of jffs2 partition */
#define CONFIG_JFFS2_PART_OFFSET	0x680000
#define CONFIG_JFFS2_PART_SIZE		0xf980000	/* size of jffs2 */
							/* partition */

/* Environment information */
#define CONFIG_BOOTDELAY		10

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"usbtty=cdc_acm\0" \
	"console=ttyS2,115200n8\0" \
	"mpurate=500\0" \
	"vram=12M\0" \
	"dvimode=1024x768MR-16@60\0" \
	"defaultdisplay=dvi\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"nandroot=/dev/mtdblock4 rw\0" \
	"nandrootfstype=jffs2\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"mpurate=${mpurate} " \
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapfb.debug=y " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"nandargs=setenv bootargs console=${console} " \
		"mpurate=${mpurate} " \
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapfb.debug=y " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype}\0" \
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

/*
 * Miscellaneous configurable options
 */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_TIMESTAMP
#define CONFIG_SYS_AUTOLOAD		"no"
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT		"CM-T3x # "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		(CONFIG_SYS_CBSIZE)

#define CONFIG_SYS_MEMTEST_START	(OMAP34XX_SDRC_CS0)	/* memtest */
								/* works on */
#define CONFIG_SYS_MEMTEST_END		(OMAP34XX_SDRC_CS0 + \
					0x01F00000) /* 31MB */

#define CONFIG_SYS_LOAD_ADDR		(OMAP34XX_SDRC_CS0)	/* default */
							/* load address */

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_TIMERBASE		(OMAP34XX_GPT2)
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */
#define CONFIG_SYS_HZ			1000

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	/* CS1 is never populated */
#define PHYS_SDRAM_1		OMAP34XX_SDRC_CS0
#define PHYS_SDRAM_1_SIZE	(32 << 20)	/* at least 32 MiB */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */
/* Configure the PISMO */
#define PISMO1_NAND_SIZE		GPMC_SIZE_128M

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 2 sectors */

#define CONFIG_ENV_IS_IN_NAND
#define SMNAND_ENV_OFFSET		0x260000 /* environment starts here */
#define CONFIG_ENV_OFFSET		SMNAND_ENV_OFFSET
#define CONFIG_ENV_ADDR			SMNAND_ENV_OFFSET

#if defined(CONFIG_CMD_NET)
#define CONFIG_SMC911X
#define CONFIG_SMC911X_32_BIT
#define CM_T3X_SMC911X_BASE	0x2C000000
#define SB_T35_SMC911X_BASE	(CM_T3X_SMC911X_BASE + (16 << 20))
#define CONFIG_SMC911X_BASE	CM_T3X_SMC911X_BASE
#endif /* (CONFIG_CMD_NET) */

/* additions for new relocation code, must be added to all boards */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_ADDR	0x4020f800
#define CONFIG_SYS_INIT_RAM_SIZE	0x800
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR +	\
					 CONFIG_SYS_INIT_RAM_SIZE -	\
					 GENERATED_GBL_DATA_SIZE)

/* Status LED */
#define CONFIG_STATUS_LED		/* Status LED enabled */
#define CONFIG_BOARD_SPECIFIC_LED
#define STATUS_LED_GREEN		0
#define STATUS_LED_BIT			STATUS_LED_GREEN
#define STATUS_LED_STATE		STATUS_LED_ON
#define STATUS_LED_PERIOD		(CONFIG_SYS_HZ / 2)
#define STATUS_LED_BOOT			STATUS_LED_BIT
#define GREEN_LED_GPIO			186 /* CM-T35 Green LED is GPIO186 */

/* GPIO banks */
#ifdef CONFIG_STATUS_LED
#define CONFIG_OMAP3_GPIO_6	/* GPIO186 is in GPIO bank 6  */
#endif

#endif /* __CONFIG_H */

/*
 * (C) Copyright 2002, 2003
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Gary Jennejohn <garyj@denx.de>
 * David Mueller <d.mueller@elsoft.ch>
 *
 * Configuation settings for the MPL VCMA9 board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H


#define MACH_TYPE_MPL_VCMA9	227

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_THUMB_BUILD

#define CONFIG_S3C24X0		/* This is a SAMSUNG S3C24x0-type SoC */
#define CONFIG_S3C2410		/* specifically a SAMSUNG S3C2410 SoC */
#define CONFIG_VCMA9		/* on a MPL VCMA9 Board  */
#define CONFIG_MACH_TYPE	MACH_TYPE_MPL_VCMA9 /* Machine type */

#define CONFIG_SYS_TEXT_BASE	0x0


#define CONFIG_SYS_ARM_CACHE_WRITETHROUGH

/* input clock of PLL (VCMA9 has 12MHz input clock) */
#define CONFIG_SYS_CLK_FREQ	12000000

#define CONFIG_CMDLINE_TAG	/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/*
 * Command line configuration.
 */
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_USB
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_BSP
#define CONFIG_CMD_NAND

#define CONFIG_BOARD_LATE_INIT

#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_CMDLINE_EDITING

/*
 * I2C stuff:
 * the MPL VCMA9 is equipped with an ATMEL 24C256 EEPROM at
 * address 0x50 with 16bit addressing
 */
#define CONFIG_SYS_I2C

/* we use the built-in I2C controller */
#define CONFIG_SYS_I2C_S3C24X0
#define CONFIG_SYS_I2C_S3C24X0_SPEED    100000	/* I2C speed */
#define CONFIG_SYS_I2C_S3C24X0_SLAVE    0x7F	/* I2C slave addr */

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
/* use EEPROM for environment vars */
#define CONFIG_ENV_IS_IN_EEPROM		1
/* environment starts at offset 0 */
#define CONFIG_ENV_OFFSET		0x000
/* 2KB should be more than enough */
#define CONFIG_ENV_SIZE			0x800

#undef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
/* 64 bytes page write mode on 24C256 */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 6
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

/*
 * Hardware drivers
 */
#define CONFIG_CS8900			/* we have a CS8900 on-board */
#define CONFIG_CS8900_BASE		0x20000300
#define CONFIG_CS8900_BUS16

/*
 * select serial console configuration
 */
#define CONFIG_S3C24X0_SERIAL
#define CONFIG_SERIAL1		1	/* we use SERIAL 1 on VCMA9 */

/* USB support (currently only works with D-cache off) */
#define CONFIG_USB_OHCI
#define CONFIG_USB_OHCI_S3C24XX
#define CONFIG_USB_KEYBOARD
#define CONFIG_USB_STORAGE
#define CONFIG_DOS_PARTITION

/* Enable needed helper functions */
#define CONFIG_SYS_STDIO_DEREGISTER	/* needs stdio_deregister */

/* RTC */
#define CONFIG_RTC_S3C24X0


/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE			9600

#define CONFIG_BOOTDELAY		5
#define CONFIG_BOOT_RETRY_TIME		-1
#define CONFIG_RESET_TO_RETRY
#define CONFIG_ZERO_BOOTDELAY_CHECK

#define CONFIG_NETMASK			255.255.255.0
#define CONFIG_IPADDR			10.0.0.110
#define CONFIG_SERVERIP			10.0.0.1

#if defined(CONFIG_CMD_KGDB)
/* speed to run kgdb serial port */
#define CONFIG_KGDB_BAUDRATE		115200
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_CBSIZE		256
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS		16
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_DISPLAY_CPUINFO				/* Display cpu info */
#define CONFIG_DISPLAY_BOARDINFO			/* Display board info */

#define CONFIG_SYS_MEMTEST_START	0x30000000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x31FFFFFF	/* 32 MB in DRAM */

#define CONFIG_SYS_ALT_MEMTEST
#define CONFIG_SYS_LOAD_ADDR		0x30800000

/* we configure PWM Timer 4 to 1ms 1000Hz  */

/* support additional compression methods */
#define CONFIG_BZIP2
#define CONFIG_LZO
#define CONFIG_LZMA

/* Ident */
/*#define VERSION_TAG "released"*/
#define VERSION_TAG "unstable"
#define CONFIG_IDENT_STRING "\n(c) 2003 - 2011 by MPL AG Switzerland, " \
			    "MEV-10080-001 " VERSION_TAG

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x30000000	/* SDRAM Bank #1 */
#define PHYS_FLASH_1		0x00000000	/* Flash Bank #1 */

#define CONFIG_SYS_FLASH_BASE	PHYS_FLASH_1

/* FLASH and environment organization */

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_FLASH_CFI_LEGACY
#define CONFIG_SYS_FLASH_LEGACY_512Kx16
#define CONFIG_FLASH_SHOW_PROGRESS	45
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }
#define CONFIG_SYS_MAX_FLASH_SECT	(19)

/*
 * Size of malloc() pool
 * BZIP2 / LZO / LZMA need a lot of RAM
 */
#define CONFIG_SYS_MALLOC_LEN		(4 * 1024 * 1024)
#define CONFIG_SYS_MONITOR_LEN		(512 * 1024)
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE

/* NAND configuration */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_S3C2410
#define CONFIG_SYS_S3C2410_NAND_HWECC
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x4E000000
#define CONFIG_S3C24XX_CUSTOM_NAND_TIMING
#define CONFIG_S3C24XX_TACLS		1
#define CONFIG_S3C24XX_TWRPH0		5
#define CONFIG_S3C24XX_TWRPH1		3
#endif

#define MULTI_PURPOSE_SOCKET_ADDR	0x08000000

/* File system */
#define CONFIG_CMD_FAT
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_JFFS2
#define CONFIG_YAFFS2
#define CONFIG_RBTREE
#define CONFIG_MTD_DEVICE               /* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_LZO

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x1000 - \
					GENERATED_GBL_DATA_SIZE)

#define CONFIG_BOARD_EARLY_INIT_F

#endif /* __CONFIG_H */

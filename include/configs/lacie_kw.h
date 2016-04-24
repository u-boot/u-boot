/*
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_LACIE_KW_H
#define _CONFIG_LACIE_KW_H

/*
 * Machine number definition
 */
#if defined(CONFIG_INETSPACE_V2)
#define CONFIG_MACH_TYPE		MACH_TYPE_INETSPACE_V2
#define CONFIG_IDENT_STRING		" IS v2"
#elif defined(CONFIG_NETSPACE_V2)
#define CONFIG_MACH_TYPE		MACH_TYPE_NETSPACE_V2
#define CONFIG_IDENT_STRING		" NS v2"
#elif defined(CONFIG_NETSPACE_LITE_V2)
#define MACH_TYPE_NETSPACE_LITE_V2	2983 /* missing in mach-types.h */
#define CONFIG_MACH_TYPE		MACH_TYPE_NETSPACE_LITE_V2
#define CONFIG_IDENT_STRING		" NS v2 Lite"
#elif defined(CONFIG_NETSPACE_MINI_V2)
#define MACH_TYPE_NETSPACE_MINI_V2	2831 /* missing in mach-types.h */
#define CONFIG_MACH_TYPE		MACH_TYPE_NETSPACE_MINI_V2
#define CONFIG_IDENT_STRING		" NS v2 Mini"
#elif defined(CONFIG_NETSPACE_MAX_V2)
#define CONFIG_MACH_TYPE		MACH_TYPE_NETSPACE_MAX_V2
#define CONFIG_IDENT_STRING		" NS Max v2"
#elif defined(CONFIG_D2NET_V2)
#define CONFIG_MACH_TYPE		MACH_TYPE_D2NET_V2
#define CONFIG_IDENT_STRING		" D2 v2"
#elif defined(CONFIG_NET2BIG_V2)
#define CONFIG_MACH_TYPE		MACH_TYPE_NET2BIG_V2
#define CONFIG_IDENT_STRING		" 2Big v2"
#else
#error "Unknown board"
#endif

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_FEROCEON_88FR131		/* CPU Core subversion */
/* SoC name */
#if defined(CONFIG_NETSPACE_LITE_V2) || defined(CONFIG_NETSPACE_MINI_V2)
#define CONFIG_KW88F6192
#else
#define CONFIG_KW88F6281
#endif
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#define CONFIG_CMD_ENV
#define CONFIG_CMD_IDE
#ifndef CONFIG_NETSPACE_MINI_V2 /* No USB ports on Network Space v2 Mini */
#endif

/*
 * Core clock definition
 */
#define CONFIG_SYS_TCLK			166000000 /* 166MHz */

/*
 * SDRAM configuration
 */
#define CONFIG_NR_DRAM_BANKS		1

/*
 * Different SDRAM configuration and size for some of the boards derived
 * from the Network Space v2
 */
#if defined(CONFIG_INETSPACE_V2)
#define CONFIG_SYS_KWD_CONFIG $(CONFIG_BOARDDIR)/kwbimage-is2.cfg
#elif defined(CONFIG_NETSPACE_LITE_V2) || defined(CONFIG_NETSPACE_MINI_V2)
#define CONFIG_SYS_KWD_CONFIG $(CONFIG_BOARDDIR)/kwbimage-ns2l.cfg
#endif

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* Remove or override few declarations from mv-common.h */
#undef CONFIG_RBTREE
#undef CONFIG_ENV_SPI_MAX_HZ
#undef CONFIG_SYS_IDE_MAXBUS
#undef CONFIG_SYS_IDE_MAXDEVICE
#define CONFIG_ENV_SPI_MAX_HZ           20000000 /* 20Mhz */

/*
 * Enable platform initialisation via misc_init_r() function
 */
#define CONFIG_MISC_INIT_R

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS		{1, 0} /* enable port 0 only */
#define CONFIG_NETCONSOLE
#endif

/*
 * SATA Driver configuration
 */
#ifdef CONFIG_MVSATA_IDE
#define CONFIG_SYS_ATA_IDE0_OFFSET      MV_SATA_PORT0_OFFSET
#if defined(CONFIG_NETSPACE_MAX_V2) || defined(CONFIG_D2NET_V2) || \
	defined(CONFIG_NET2BIG_V2)
#define CONFIG_SYS_ATA_IDE1_OFFSET      MV_SATA_PORT1_OFFSET
#define CONFIG_SYS_IDE_MAXBUS           2
#define CONFIG_SYS_IDE_MAXDEVICE        2
#else
#define CONFIG_SYS_IDE_MAXBUS           1
#define CONFIG_SYS_IDE_MAXDEVICE        1
#endif
#endif /* CONFIG_MVSATA_IDE */

/*
 * Enable GPI0 support
 */
#define CONFIG_KIRKWOOD_GPIO

/*
 * Enable I2C support
 */
#ifdef CONFIG_CMD_I2C
/* I2C EEPROM HT24LC04 (512B - 32 pages of 16 Bytes) */
#define CONFIG_CMD_EEPROM
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4 /* 16-byte page size */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1 /* 8-bit device address */
#if defined(CONFIG_NET2BIG_V2)
#define CONFIG_SYS_I2C_G762_ADDR		0x3e
#endif
#endif /* CONFIG_CMD_I2C */

/*
 * Partition support
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

/*
 * File systems support
 */

/*
 * Console configuration
 */
#define CONFIG_CONSOLE_MUX
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/*
 * Environment variables configurations
 */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE		0x10000	/* 64KB */
#define CONFIG_ENV_SIZE			0x1000	/* 4KB */
#define CONFIG_ENV_ADDR			0x70000
#define CONFIG_ENV_OFFSET		0x70000	/* env starts here */

/*
 * Default environment variables
 */
#define CONFIG_BOOTARGS "console=ttyS0,115200"

#define CONFIG_BOOTCOMMAND					\
	"dhcp && run netconsole; "				\
	"if run usbload || run diskload; then bootm; fi"

#define CONFIG_EXTRA_ENV_SETTINGS				\
	"stdin=serial\0"					\
	"stdout=serial\0"					\
	"stderr=serial\0"					\
	"bootfile=uImage\0"					\
	"loadaddr=0x800000\0"					\
	"autoload=no\0"						\
	"netconsole="						\
		"set stdin $stdin,nc; "				\
		"set stdout $stdout,nc; "			\
		"set stderr $stderr,nc;\0"			\
	"diskload=ide reset && "				\
		"ext2load ide 0:1 $loadaddr /boot/$bootfile\0"	\
	"usbload=usb start && "					\
		"fatload usb 0:1 $loadaddr /boot/$bootfile\0"

#endif /* _CONFIG_LACIE_KW_H */

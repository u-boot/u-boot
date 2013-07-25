/*
 * Copyright (C) 2011 Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * Based on the netspace_v2 code which is
 * Copyright (C) 2011 Simon Guinot <sguinot@lacie.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_WIRELESS_SPACE_H
#define _CONFIG_WIRELESS_SPACE_H

/*
 * Machine number definition
 */
#define MACH_TYPE_WIRELESS_SPACE	2500 /* is missing in mach-types.h */
#define CONFIG_MACH_TYPE		MACH_TYPE_WIRELESS_SPACE
#define CONFIG_IDENT_STRING		" Wireless Space"

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_FEROCEON_88FR131		/* CPU Core subversion */
#define CONFIG_KIRKWOOD			/* SoC Family Name */
/* SoC name */
#define CONFIG_KW88F6281
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* no NOR or SPI flash */
#include <config_cmd_default.h>
#define CONFIG_CMD_ENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_NAND
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_USB

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

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* Remove or override few declarations from mv-common.h */
#undef CONFIG_RBTREE
#undef CONFIG_SYS_IDE_MAXBUS
#undef CONFIG_SYS_IDE_MAXDEVICE
#define CONFIG_SYS_IDE_MAXBUS           1
#define CONFIG_SYS_IDE_MAXDEVICE        1
#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"ws> "

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MISC_INIT_R /* misc_init_r() initializes MAC address */
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable only egiga0... */
#define PORT_SERIAL_CONTROL_VALUE 0x00A4260E /* ... tied to the switch... */
#define CONFIG_PHY_BASE_ADR 0xa		/* ... through a 'fake' PHY */
#define CONFIG_MII
#undef CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#define CONFIG_NETCONSOLE
#define CONFIG_MV88E61XX_SWITCH
#define CONFIG_MV88E61XX_MULTICHIP_ADRMODE
#define CONFIG_MV88E61XX_CMD
#define CONFIG_CMD_TFTPPUT
#endif /* CONFIG_CMD_NET */

/*
 * SATA Driver configuration
 */
#ifdef CONFIG_MVSATA_IDE
#define CONFIG_SYS_ATA_IDE0_OFFSET      MV_SATA_PORT0_OFFSET
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
#endif /* CONFIG_CMD_I2C */

/*
 * Partition support
 */
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

/*
 * File systems support
 */
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT

/*
 * Use the HUSH parser
 */
#define CONFIG_SYS_HUSH_PARSER

/*
 * Console configuration
 */
#define CONFIG_CONSOLE_MUX
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/*
 * Enable device tree support
 */
#define CONFIG_OF_LIBFDT

/*
 * Environment variables configurations
 */

#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SECT_SIZE		0x20000	/* 128KB */
#define CONFIG_ENV_SIZE			0x20000	/* 128KB */
#define CONFIG_ENV_OFFSET		0x80000	/* env starts here */

/*
 * Board-specific command to make using buttons etc easier
 */

#define CONFIG_WIRELESS_SPACE_CMD

/*
 * Default environment variables
 */
#define CONFIG_PREBOOT

#define CONFIG_BOOTARGS "console=ttyS0,115200"

#define CONFIG_BOOTCOMMAND					\
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
		"fatload usb 0:1 $loadaddr /boot/$bootfile\0"	\
	"preboot="						\
		"dhcp && run netconsole\0"

#endif /* _CONFIG_WIRELESS_SPACE_H */

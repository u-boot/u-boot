/*
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _CONFIG_LSXL_H
#define _CONFIG_LSXL_H

/*
 * Version number information
 */
#if defined(CONFIG_LSCHLV2)
#define CONFIG_IDENT_STRING " LS-CHLv2"
#define CONFIG_SYS_KWD_CONFIG $(SRCTREE)/$(CONFIG_BOARDDIR)/kwbimage-lschl.cfg
#define CONFIG_MACH_TYPE 3006
#define CONFIG_SYS_TCLK 166666667 /* 166 MHz */
#elif defined(CONFIG_LSXHL)
#define CONFIG_IDENT_STRING " LS-XHL"
#define CONFIG_SYS_KWD_CONFIG $(SRCTREE)/$(CONFIG_BOARDDIR)/kwbimage-lsxhl.cfg
#define CONFIG_MACH_TYPE 2663
/* CONFIG_SYS_TCLK is 200000000 by default */
#else
#error "unknown board"
#endif

/*
 * General configuration options
 */
#define CONFIG_FEROCEON_88FR131		/* CPU Core subversion */
#define CONFIG_KIRKWOOD			/* SOC Family Name */
#define CONFIG_KW88F6281		/* SOC Name */

#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_MISC_INIT_R
#define CONFIG_SHOW_BOOT_PROGRESS

#define CONFIG_RANDOM_MACADDR
#define CONFIG_KIRKWOOD_GPIO
#define CONFIG_OF_LIBFDT

#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_CONSOLE_INFO_QUIET

/*
 * Enable u-boot API for standalone programs.
 */
#define CONFIG_API

/*
 * Commands configuration
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_ENV
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_IDE
#define CONFIG_CMD_PING
#define CONFIG_CMD_PING
#define CONFIG_CMD_SF
#define CONFIG_CMD_SPI
#define CONFIG_CMD_USB

#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* ST M25P40 */
#undef CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_STMICRO
#undef CONFIG_ENV_SPI_MAX_HZ
#define CONFIG_ENV_SPI_MAX_HZ		25000000
#undef CONFIG_SF_DEFAULT_SPEED
#define CONFIG_SF_DEFAULT_SPEED		25000000


#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_SPI_FLASH
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	8
#define CONFIG_ENV_IS_IN_SPI_FLASH	1
#define CONFIG_ENV_SECT_SIZE		0x10000 /* 64K */
#else
#define CONFIG_ENV_IS_NOWHERE
#endif

#define CONFIG_ENV_SIZE			0x10000 /* 64k */
#define CONFIG_ENV_OFFSET		0x70000 /* env starts here */

/*
 * Default environment variables
 */
#define CONFIG_LOADADDR		0x00800000
#define CONFIG_BOOTCOMMAND	"run bootcmd_${bootsource}"
#define CONFIG_BOOTARGS		"console=ttyS0,115200 root=/dev/sda2"
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"bootsource=hdd\0"						\
	"hdpart=0:1\0"							\
	"bootcmd_net=bootp 0x00100000 uImage "				\
		"&& tftpboot 0x00800000 uInitrd "			\
		"&& bootm 0x00100000 0x00800000\0"			\
	"bootcmd_hdd=ide reset "					\
		"&& ext2load ide ${hdpart} 0x00100000 /uImage "		\
		"&& ext2load ide ${hdpart} 0x00800000 /uInitrd "	\
		"&& bootm 0x00100000 0x00800000\0"			\
	"bootcmd_usb=usb start "					\
		"&& fatload usb 0:1 0x00100000 /uImage "		\
		"&& fatload usb 0:1 0x00800000 /uInitrd "		\
		"&& bootm 0x00100000 0x00800000\0"			\
	"bootcmd_rescue=run config_nc_dhcp; run nc\0"			\
	"eraseenv=sf probe 0 "						\
		"&& sf erase " __stringify(CONFIG_ENV_OFFSET)		\
			" +" __stringify(CONFIG_ENV_SIZE) "\0"		\
	"config_nc_dhcp=setenv autoload_old ${autoload}; "		\
		"setenv autoload no "					\
		"&& bootp "						\
		"&& setenv ncip "					\
		"&& setenv autoload ${autoload_old}; "			\
		"setenv autoload_old\0"					\
	"standard_env=setenv ipaddr; setenv netmask; setenv serverip; "	\
		"setenv ncip; setenv gatewayip; setenv ethact; "	\
		"setenv bootfile; setenv dnsip; "			\
		"setenv bootsource hdd; run ser\0"			\
	"restore_env=run standard_env; saveenv; reset\0"		\
	"ser=setenv stdin serial; setenv stdout serial; "		\
		"setenv stderr serial\0"				\
	"nc=setenv stdin nc; setenv stdout nc; setenv stderr nc\0"	\
	"stdin=serial\0"						\
	"stdout=serial\0"						\
	"stderr=serial\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS		{0, 1} /* enable port 1 only */
#define CONFIG_PHY_BASE_ADR		7
#undef CONFIG_RESET_PHY_R
#endif /* CONFIG_CMD_NET */

#ifdef CONFIG_CMD_IDE
#undef CONFIG_IDE_LED
#undef CONFIG_SYS_IDE_MAXBUS
#define CONFIG_SYS_IDE_MAXBUS		1
#undef CONFIG_SYS_IDE_MAXDEVICE
#define CONFIG_SYS_IDE_MAXDEVICE	1
#define CONFIG_SYS_ATA_IDE0_OFFSET	MV_SATA_PORT0_OFFSET
#endif

#endif /* _CONFIG_LSXL_H */

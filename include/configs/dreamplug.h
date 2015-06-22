/*
 * (C) Copyright 2011
 * Jason Cooper <u-boot@lakedaemon.net>
 *
 * Based on work by:
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_DREAMPLUG_H
#define _CONFIG_DREAMPLUG_H

/*
 * FIXME: This belongs in mach-types.h.  However, we only pull mach-types
 * from Linus' kernel.org tree.  This hasn't been updated primarily due to
 * the recent arch/arm reshuffling.  So, in the meantime, we'll place it
 * here.
 */
#include <asm/mach-types.h>
#ifdef MACH_TYPE_DREAMPLUG
#error "MACH_TYPE_DREAMPLUG has been defined properly, please remove this."
#else
#define MACH_TYPE_DREAMPLUG            3550
#endif

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nMarvell-DreamPlug"

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SHEEVA_88SV131	1	/* CPU Core subversion */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_MACH_TYPE	MACH_TYPE_DREAMPLUG
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_SYS_GENERIC_BOARD

/* Add target to build it automatically upon "make" */
#define CONFIG_BUILD_TARGET     "u-boot.kwb"

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FAT
#define CONFIG_CMD_SF
#define CONFIG_CMD_PING
#define CONFIG_CMD_USB
#define CONFIG_CMD_IDE
#define CONFIG_CMD_DATE

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_SPI_FLASH
#define CONFIG_ENV_IS_IN_SPI_FLASH	1
#define CONFIG_ENV_SECT_SIZE		0x10000	/* 64k */
#else
#define CONFIG_ENV_IS_NOWHERE		1	/* if env in SDRAM */
#endif

#ifdef CONFIG_CMD_SF
#define CONFIG_HARD_SPI			1
#define CONFIG_KIRKWOOD_SPI		1
#define CONFIG_SPI_FLASH_MACRONIX	1
#define CONFIG_ENV_SPI_BUS		0
#define CONFIG_ENV_SPI_CS		0
#define CONFIG_ENV_SPI_MAX_HZ		50000000 /* 50 MHz */
#endif

/*
 * max 4k env size is enough, but in case of nand
 * it has to be rounded to sector size
 */
#define CONFIG_ENV_SIZE			0x1000  /* 4k */
#define CONFIG_ENV_ADDR			0x100000
#define CONFIG_ENV_OFFSET		0x100000 /* env starts here */

/*
 * Default environment variables
 */
#define CONFIG_BOOTCOMMAND		"setenv ethact egiga0; " \
	"${x_bootcmd_ethernet}; setenv ethact egiga1; " \
	"${x_bootcmd_ethernet}; ${x_bootcmd_usb}; ${x_bootcmd_kernel}; "\
	"setenv bootargs ${x_bootargs} ${x_bootargs_root}; "	\
	"bootm 0x6400000;"

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"x_bootcmd_ethernet=ping 192.168.2.1\0"	\
	"x_bootcmd_usb=usb start\0"	\
	"x_bootcmd_kernel=fatload usb 0 0x6400000 uImage\0" \
	"x_bootargs=console=ttyS0,115200\0"	\
	"x_bootargs_root=root=/dev/sda2 rootdelay=10\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 1}	/* enable both ports */
#define CONFIG_PHY_BASE_ADR	0
#endif /* CONFIG_CMD_NET */

/*
 * SATA Driver configuration
 */
#ifdef CONFIG_MVSATA_IDE
#define CONFIG_SYS_ATA_IDE0_OFFSET	MV_SATA_PORT0_OFFSET
#endif /*CONFIG_MVSATA_IDE*/

/*
 * RTC driver configuration
 */
#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_MV
#endif /* CONFIG_CMD_DATE */

#define CONFIG_SYS_ALT_MEMTEST

/*
 * display enhanced info about the cpu at boot.
 */
#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_OF_LIBFDT

#endif /* _CONFIG_DREAMPLUG_H */

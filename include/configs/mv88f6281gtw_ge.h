/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_MV88F6281GTW_GE_H
#define _CONFIG_MV88F6281GTW_GE_H

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nMarvell-MV88F6281GTW_GE"

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_FEROCEON_88FR131	1	/* CPU Core subversion */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_MACH_MV88F6281GTW_GE	/* Machine type */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_ENV
#define CONFIG_CMD_FAT
#define CONFIG_CMD_PING
#define CONFIG_CMD_SF
#define CONFIG_CMD_USB

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* Unwanted stuffs from mv-common.h */
#undef	CONFIG_CMD_EXT2
#undef	CONFIG_CMD_JFFS2
#undef	CONFIG_CMD_FAT
#undef	CONFIG_CMD_UBI
#undef	CONFIG_CMD_UBIFS
#undef	CONFIG_RBTREE

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_SPI_FLASH
#define CONFIG_ENV_IS_IN_SPI_FLASH	1
#define CONFIG_ENV_SECT_SIZE		0x10000	/* 64K */
#else
#define CONFIG_ENV_IS_NOWHERE		1	/* if env in SDRAM */
#endif
#define CONFIG_ENV_SIZE			0x1000	/* 4k */
#define CONFIG_ENV_ADDR			0x30000
#define CONFIG_ENV_OFFSET		0x30000	/* env starts here */

/*
 * Default environment variables
 */
#define CONFIG_BOOTCOMMAND		"${x_bootcmd_kernel}; "	\
	"setenv bootargs ${x_bootargs} ${x_bootargs_root}; "	\
	"${x_bootcmd_usb}; bootm 0x6400000;"

#define CONFIG_MTDPARTS			"spi0.0:512k(uboot),"	\
	"512k@512k(psm),2m@1m(kernel),13m@3m(rootfs)\0"

#define CONFIG_EXTRA_ENV_SETTINGS	"x_bootargs=console"	\
	"=ttyS0,115200 mtdparts="CONFIG_MTDPARTS	\
	"x_bootcmd_kernel=cp.b 0xE8100000 0x6400000 0x200000\0" \
	"x_bootcmd_usb=usb start\0" \
	"x_bootargs_root=root=/dev/mtdblock3 ro rootfstype=squashfs\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
#define CONFIG_MV88E61XX_SWITCH	/* Enable mv88e61xx switch driver */
#endif /* CONFIG_CMD_NET */

#endif /* _CONFIG_MV88F6281GTW_GE_H */

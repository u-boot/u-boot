/*
 * (C) Copyright 2009-2014
 * Gerald Kerma <dreagle@doukki.net>
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_SHEEVAPLUG_H
#define _CONFIG_SHEEVAPLUG_H

#define CONFIG_SYS_GENERIC_BOARD

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nMarvell-Sheevaplug"

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_FEROCEON_88FR131	1	/* CPU Core subversion */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_MACH_SHEEVAPLUG	/* Machine type */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * Compression configuration
 */
#define CONFIG_BZIP2
#define CONFIG_LZMA
#define CONFIG_LZO

/*
 * Enable device tree support
 */
#define CONFIG_OF_LIBFDT

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#include <config_cmd_default.h>
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ENV
#define CONFIG_CMD_IDE
#define CONFIG_CMD_MII
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PING
#define CONFIG_CMD_USB

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_ENV_IS_IN_NAND		1
#define CONFIG_ENV_SECT_SIZE		0x20000	/* 128K */
#else
#define CONFIG_ENV_IS_NOWHERE		1	/* if env in SDRAM */
#endif
/*
 * max 4k env size is enough, but in case of nand
 * it has to be rounded to sector size
 */
#define CONFIG_ENV_SIZE			0x20000	/* 128k */
#define CONFIG_ENV_ADDR			0x80000
#define CONFIG_ENV_OFFSET		0x80000	/* env starts here */

/*
 * Default environment variables
 */
#define CONFIG_BOOTCOMMAND		"${x_bootcmd_kernel}; "	\
	"setenv bootargs ${x_bootargs} ${x_bootargs_root}; "	\
	"${x_bootcmd_usb}; bootm 0x6400000;"

#define CONFIG_MTDPARTS		\
	"mtdparts=orion_nand:512K(uboot),"				\
	"512K(env),1M(script),6M(kernel),"				\
	"12M(ramdisk),4M(spare),-(rootfs)"

#define CONFIG_EXTRA_ENV_SETTINGS	"x_bootargs=console"	\
	"=ttyS0,115200 mtdparts="CONFIG_MTDPARTS	\
	"x_bootcmd_kernel=nand read 0x6400000 0x100000 0x300000\0" \
	"x_bootcmd_usb=usb start\0" \
	"x_bootargs_root=root=/dev/mtdblock3 rw rootfstype=jffs2\0"

#define MTDIDS_DEFAULT	"nand0=orion_nand"

#define MTDPARTS_DEFAULT	\
	"mtdparts="CONFIG_MTDPARTS

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
#define CONFIG_PHY_BASE_ADR	0
#endif /* CONFIG_CMD_NET */

/*
 * SDIO/MMC Card Configuration
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_MVEBU_MMC
#define CONFIG_SYS_MMC_BASE KW_SDIO_BASE
#endif /* CONFIG_CMD_MMC */

/*
 * SATA driver configuration
 */
#ifdef CONFIG_CMD_IDE
#define __io
#define CONFIG_IDE_PREINIT
#define CONFIG_DOS_PARTITION
#define CONFIG_MVSATA_IDE_USE_PORT0
#define CONFIG_MVSATA_IDE_USE_PORT1
#define CONFIG_SYS_ATA_IDE0_OFFSET	MV_SATA_PORT0_OFFSET
#define CONFIG_SYS_ATA_IDE1_OFFSET	MV_SATA_PORT1_OFFSET
#endif /* CONFIG_CMD_IDE */

/*
 * File system
 */
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_FAT
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_RBTREE
#define CONFIG_MTD_DEVICE               /* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS

#endif /* _CONFIG_SHEEVAPLUG_H */

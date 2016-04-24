/*
 * (C) Copyright 2009
 * Net Insight <www.netinsight.net>
 * Written-by: Simon Kagstrom <simon.kagstrom@netinsight.net>
 *
 * Based on sheevaplug.h:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_OPENRD_H
#define _CONFIG_OPENRD_H

/*
 * Version number information
 */
#ifdef CONFIG_BOARD_IS_OPENRD_ULTIMATE
# define CONFIG_IDENT_STRING	"\nOpenRD-Ultimate"
#else
# ifdef CONFIG_BOARD_IS_OPENRD_CLIENT
#  define CONFIG_IDENT_STRING	"\nOpenRD-Client"
# else
#  ifdef CONFIG_BOARD_IS_OPENRD_BASE
#   define CONFIG_IDENT_STRING	"\nOpenRD-Base"
#  else
#   error Unknown OpenRD board specified
#  endif
# endif
#endif

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SHEEVA_88SV131	1	/* CPU Core subversion */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_MACH_OPENRD_BASE	/* Machine type */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_SYS_THUMB_BUILD

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#define CONFIG_SYS_MVFS
#define CONFIG_CMD_ENV
#define CONFIG_CMD_NAND
#define CONFIG_CMD_IDE

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
#define CONFIG_ENV_ADDR			0x60000
#define CONFIG_ENV_OFFSET		0x60000	/* env starts here */
/*
 * Environment is right behind U-Boot in flash. Make sure U-Boot
 * doesn't grow into the environment area.
 */
#define CONFIG_BOARD_SIZE_LIMIT		CONFIG_ENV_OFFSET

/*
 * Default environment variables
 */
#define CONFIG_BOOTCOMMAND		"${x_bootcmd_kernel}; "	\
	"setenv bootargs ${x_bootargs} ${x_bootargs_root}; "	\
	"${x_bootcmd_usb}; bootm 0x6400000;"

#define MTDIDS_DEFAULT		"nand0=nand_mtd"
#define MTDPARTS_DEFAULT	"mtdparts=nand_mtd:0x100000@0x000000(uboot),"\
	"0x400000@0x100000(uImage),"\
	"0x1fb00000@0x500000(rootfs)"

#define CONFIG_EXTRA_ENV_SETTINGS	"x_bootargs=console"		\
	"=ttyS0,115200 "MTDPARTS_DEFAULT " rw ubi.mtd=2,2048\0"		\
	"x_bootcmd_kernel=nand read 0x6400000 0x100000 0x300000\0"	\
	"x_bootcmd_usb=usb start\0"					\
	"x_bootargs_root=root=ubi0:rootfs rootfstype=ubifs\0"		\
	"mtdids="MTDIDS_DEFAULT"\0"					\
	"mtdparts="MTDPARTS_DEFAULT"\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
# ifdef CONFIG_BOARD_IS_OPENRD_BASE
#  define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
# else
#  define CONFIG_MVGBE_PORTS	{1, 1}	/* enable both ports */
# endif
# ifdef CONFIG_BOARD_IS_OPENRD_ULTIMATE
#  define CONFIG_PHY_BASE_ADR	0x0
#  define PHY_NO		"88E1121"
# else
#  define CONFIG_PHY_BASE_ADR	0x8
#  define PHY_NO		"88E1116"
# endif
#endif /* CONFIG_CMD_NET */

/*
 * SATA Driver configuration
 */
#ifdef CONFIG_MVSATA_IDE
#define CONFIG_SYS_ATA_IDE0_OFFSET	MV_SATA_PORT0_OFFSET
#define CONFIG_SYS_ATA_IDE1_OFFSET	MV_SATA_PORT1_OFFSET
#endif /*CONFIG_MVSATA_IDE*/

#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_MVEBU_MMC
#define CONFIG_SYS_MMC_BASE KW_SDIO_BASE
#endif /* CONFIG_CMD_MMC */

#endif /* _CONFIG_OPENRD_BASE_H */

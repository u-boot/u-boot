/*
 * Copyright (C) 2011-2012
 * Gerald Kerma <dreagle@doukki.net>
 * Luka Perkov <luka@openwrt.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_IB62x0_H
#define _CONFIG_IB62x0_H

#define CONFIG_SYS_GENERIC_BOARD

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	" RaidSonic ICY BOX IB-NAS62x0"

/*
 * High level configuration options
 */
#define CONFIG_FEROCEON_88FR131		/* CPU Core subversion */
#define CONFIG_KW88F6281		/* SOC Name */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * Machine type
 */
#define CONFIG_MACH_TYPE	MACH_TYPE_NAS6210

/*
 * Enable device tree support
 */
#define CONFIG_OF_LIBFDT

/*
 * Compression configuration
 */
#define CONFIG_BZIP2
#define CONFIG_LZMA
#define CONFIG_LZO

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* declare no flash (NOR/SPI) */
#define CONFIG_SYS_MVFS
#define CONFIG_CMD_ENV
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_IDE
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PING
#define CONFIG_CMD_USB

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 * Environment variables configuration
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SECT_SIZE	0x20000
#else
#define CONFIG_ENV_IS_NOWHERE
#endif
#define CONFIG_ENV_SIZE		0x20000
#define CONFIG_ENV_OFFSET	0xe0000

/*
 * Default environment variables
 */
#define CONFIG_BOOTCOMMAND \
	"setenv bootargs ${console} ${mtdparts} ${bootargs_root}; "	\
	"ubi part root; "						\
	"ubifsmount ubi:rootfs; "					\
	"ubifsload 0x800000 ${kernel}; "				\
	"ubifsload 0x700000 ${fdt}; "					\
	"ubifsumount; "							\
	"fdt addr 0x700000; fdt resize; fdt chosen; "			\
	"bootz 0x800000 - 0x700000"

#define CONFIG_MTDPARTS \
	"mtdparts=orion_nand:"						\
	"0xe0000@0x0(uboot),"						\
	"0x20000@0xe0000(uboot_env),"					\
	"-@0x100000(root)\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=console=ttyS0,115200\0"				\
	"mtdids=nand0=orion_nand\0"					\
	"mtdparts="CONFIG_MTDPARTS					\
	"kernel=/boot/zImage\0"						\
	"fdt=/boot/ib62x0.dtb\0"					\
	"bootargs_root=ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs rw\0"

/*
 * Ethernet driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
#define CONFIG_PHY_BASE_ADR	0
#undef CONFIG_RESET_PHY_R
#endif /* CONFIG_CMD_NET */

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
 * RTC driver configuration
 */
#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_MV
#endif /* CONFIG_CMD_DATE */

/*
 * File system
 */
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_RBTREE
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS

#endif /* _CONFIG_IB62x0_H */

/*
 * (C) Copyright 2009-2014
 * Gerald Kerma <dreagle@doukki.net>
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Siddarth Gore <gores@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CONFIG_GURUPLUG_H
#define _CONFIG_GURUPLUG_H


/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nMarvell-GuruPlug"

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_SHEEVA_88SV131	1	/* CPU Core subversion */
#define CONFIG_MACH_GURUPLUG	/* Machine type */

/*
 * Standard filesystems
 */
#define CONFIG_SYS_MVFS

/*
 * mv-plug-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-plug-common.h"

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
#define CONFIG_ENV_OFFSET		0xE0000	/* env starts here */

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

#define CONFIG_MTDPARTS	\
	"mtdparts=orion_nand:"						\
	"896K(uboot),128K(uboot_env),"					\
	"-@1M(root)\0"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=console=ttyS0,115200\0"				\
	"mtdids=nand0=orion_nand\0"					\
	"mtdparts="CONFIG_MTDPARTS					\
	"kernel=/boot/zImage\0"						\
	"fdt=/boot/guruplug-server-plus.dtb\0"				\
	"bootargs_root=ubi.mtd=2 root=ubi0:rootfs rootfstype=ubifs rw\0"

#define MTDIDS_DEFAULT	"nand0=orion_nand"

#define MTDPARTS_DEFAULT	\
	"mtdparts="CONFIG_MTDPARTS

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

#endif /* _CONFIG_GURUPLUG_H */

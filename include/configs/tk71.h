/*
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 * on behalf of DENX Software Engineering GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_TK71_H__
#define __CONFIG_TK71_H__

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nKa-Ro TK71"

/*
 * High Level Configuration Options (easy to change)
 */
#define CONFIG_FEROCEON_88FR131	1	/* CPU Core subversion */
#define CONFIG_KIRKWOOD		1	/* SOC Family Name */
#define CONFIG_KW88F6281	1	/* SOC Name */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */
#define CONFIG_NR_DRAM_BANKS	1

#define MACH_TYPE_TK71		2399
#define CONFIG_MACH_TYPE	MACH_TYPE_TK71

/*
 * Commands configuration
 */
#define	CONFIG_SYS_HUSH_PARSER

#define CONFIG_SYS_NO_FLASH
#define CONFIG_DOS_PARTITION
#define CONFIG_SUPPORT_VFAT

#include <config_cmd_default.h>
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ENV
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_NAND
#define CONFIG_CMD_PING
#define CONFIG_CMD_USB

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/*
 * NAND flash
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_JFFS2_NAND
#define CONFIG_JFFS2_DEV		"nand0,3"
#endif

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS	{1, 0}
#define CONFIG_PHY_BASE_ADR	0x08
#endif

/*
 * USB/EHCI
 */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_KIRKWOOD
#define CONFIG_EHCI_IS_TDI
#define CONFIG_USB_STORAGE
#endif

/*
 *  Environment variables configurations
 */
#ifdef CONFIG_CMD_NAND
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SECT_SIZE		0x20000
#else
#define CONFIG_ENV_IS_NOWHERE
#endif

#define CONFIG_ENV_SIZE			0x20000
#define CONFIG_ENV_ADDR			0x80000
#define CONFIG_ENV_OFFSET		0x80000

/*
 * Default environment variables
 */
#define CONFIG_BOOTCOMMAND "nand read 0x800000 kernel 0x300000; bootm;"
#define CONFIG_MTDPARTS 	"512K(u-boot),512K(u-boot-env),3M(kernel),-(root)"
#define CONFIG_EXTRA_ENV_SETTINGS \
	"update_uboot=dhcp u-boot.kwb; nand erase.part u-boot; nand write ${fileaddr} u-boot ${filesize}\0" \
	"update_kernel=dhcp uImage-tk71; nand erase.part kernel; nand write ${fileaddr} kernel ${filesize} \0" \
	"update_rootfs=dhcp rootfs-tk71; nand erase.part root; nand write ${fileaddr} root ${filesize}\0" \
	"update_all=run update_uboot; run update_kernel; run update_rootfs; reset\0" \
	"mtdids=nand0=orion_nand\0" \
	"mtdparts=mtdparts=orion_nand:"CONFIG_MTDPARTS"\0" \
	"bootargs=console=ttyS0,115200 mtdparts=orion_nand:"CONFIG_MTDPARTS" rootfstype=jffs2 root=/dev/mtdblock3 rw\0"
#define MTDIDS_DEFAULT			"nand0=orion_nand"
#define MTDPARTS_DEFAULT		"mtdparts=orion_nand:"CONFIG_MTDPARTS

#define PHYS_SDRAM_1		0x00000000	/* Base address */
#define PHYS_SDRAM_1_SIZE	0x20000000	/* Max 512 MB RAM */

#endif	/* __CONFIG_TK71_H__ */

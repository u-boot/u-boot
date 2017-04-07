/*
 * Copyright (C) 2013, ISEE 2007 SL - http://www.isee.biz/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CONFIG_IGEP0033_H
#define __CONFIG_IGEP0033_H

#define CONFIG_NAND
#include <configs/ti_am335x_common.h>

/* Mach type */
#define CONFIG_MACH_TYPE		MACH_TYPE_IGEP0033

/* Clock defines */
#define V_OSCK				24000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */

/* Make the verbose messages from UBI stop printing */
#define CONFIG_UBI_SILENCE_MSG
#define CONFIG_UBIFS_SILENCE_MSG

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV \
	"bootdir=/boot\0" \
	"bootfile=zImage\0" \
	"dtbfile=am335x-base0033.dtb\0" \
	"console=ttyO0,115200n8\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext4 rootwait\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
		"bootenv=uEnv.txt\0" \
	"loadbootenv=load mmc ${mmcdev} ${loadaddr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from mmc ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"mmcload=load mmc ${mmcdev}:2 ${loadaddr} ${bootdir}/${bootfile}; " \
		"load mmc ${mmcdev}:2 ${fdtaddr} ${bootdir}/${dtbfile}\0" \
	"mmcboot=mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev};" \
			"if run loadbootenv; then " \
				"echo Loaded environment from ${bootenv};" \
				"run importbootenv;" \
			"fi;" \
			"if test -n $uenvcmd; then " \
				"echo Running uenvcmd ...;" \
				"run uenvcmd;" \
			"fi;" \
			"if run mmcload; then " \
				"run mmcargs; " \
				"bootz ${loadaddr} - ${fdtaddr};" \
			"fi;" \
		"fi;\0" \
	"mtdids=" MTDIDS_DEFAULT "\0" \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"nandroot=ubi0:filesystem rw ubi.mtd=3,2048\0" \
	"nandrootfstype=ubifs rootwait\0" \
	"nandload=ubi part filesystem 2048; ubifsmount ubi0; " \
		"ubifsload ${loadaddr} ${bootdir}/${bootfile}; " \
		"ubifsload ${fdtaddr} ${bootdir}/${dtbfile} \0" \
	"nandargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype} \0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"run nandload; " \
		"bootz ${loadaddr} - ${fdtaddr} \0"
#endif

#define CONFIG_BOOTCOMMAND \
	"run mmcboot;" \
	"run nandboot;"

/* NS16550 Configuration */
#define CONFIG_SYS_NS16550_COM1		0x44e09000	/* UART0 */
#define CONFIG_CONS_INDEX		1

/* Ethernet support */
#define CONFIG_PHYLIB
#define CONFIG_PHY_SMSC

/* NAND support */
#define CONFIG_NAND_OMAP_ELM
#define CONFIG_SYS_NAND_ONFI_DETECTION	1
#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x180000 /* environment starts here */
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_ENV_OFFSET + CONFIG_SYS_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		(CONFIG_ENV_SIZE)

#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_RBTREE
#define CONFIG_LZO

#define MTDIDS_DEFAULT			"nand0=omap2-nand.0"
#define MTDPARTS_DEFAULT		"mtdparts=omap2-nand.0:512k(spl),"\
					"1m(uboot),256k(environment),"\
					"-(filesystem)"

/* SPL */
#define CONFIG_SPL_LDSCRIPT		"arch/arm/mach-omap2/am33xx/u-boot-spl.lds"

#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					 CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW

#define	CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE

#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

#endif	/* ! __CONFIG_IGEP0033_H */

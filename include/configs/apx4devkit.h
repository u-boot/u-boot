/*
 * Copyright (C) 2012 Bluegiga Technologies Oy
 *
 * Authors:
 * Veli-Pekka Peltola <veli-pekka.peltola@bluegiga.com>
 * Lauri Hintsala <lauri.hintsala@bluegiga.com>
 *
 * Based on m28evk.h:
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __CONFIGS_APX4DEVKIT_H__
#define __CONFIGS_APX4DEVKIT_H__

/* System configurations */
#define CONFIG_MX28				/* i.MX28 SoC */
#define MACH_TYPE_APX4DEVKIT	3712
#define CONFIG_MACH_TYPE	MACH_TYPE_APX4DEVKIT

/* U-Boot Commands */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DOS_PARTITION

#define CONFIG_CMD_DATE
#define CONFIG_CMD_NAND

/* Memory configuration */
#define CONFIG_NR_DRAM_BANKS		1		/* 1 bank of DRAM */
#define PHYS_SDRAM_1			0x40000000	/* Base address */
#define PHYS_SDRAM_1_SIZE		0x20000000	/* Max 512 MB RAM */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Environment */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_IS_IN_NAND

/* Environment is in MMC */
#if defined(CONFIG_CMD_MMC) && defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_ENV_OFFSET		(256 * 1024)
#define CONFIG_ENV_SIZE			(16 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0
#endif

/* Environment is in NAND */
#if defined(CONFIG_CMD_NAND) && defined(CONFIG_ENV_IS_IN_NAND)
#define CONFIG_ENV_SECT_SIZE		(128 * 1024)
#define CONFIG_ENV_SIZE			(128 * 1024)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_RANGE		(384 * 1024)
#define CONFIG_ENV_OFFSET		0x120000
#define CONFIG_ENV_OFFSET_REDUND	\
		(CONFIG_ENV_OFFSET + CONFIG_ENV_RANGE)
#endif

/* UBI and NAND partitioning */
#ifdef CONFIG_CMD_NAND
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_RBTREE
#define CONFIG_LZO
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define MTDIDS_DEFAULT			"nand0=gpmi-nand"
#define MTDPARTS_DEFAULT \
	"mtdparts=gpmi-nand:128k(bootstrap),1024k(boot),768k(env),-(root)"
#else
#define MTDPARTS_DEFAULT		""
#endif

/* FEC Ethernet on SoC */
#ifdef CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0
#define IMX_FEC_BASE			MXS_ENET0_BASE
#endif

/* USB */
#ifdef CONFIG_CMD_USB
#define CONFIG_EHCI_MXS_PORT1
#define CONFIG_USB_MAX_CONTROLLER_COUNT	1
#endif

/* RTC */
#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR		0x51
#endif

/* Boot Linux */
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_BOOTCOMMAND		"run bootcmd_nand"
#define CONFIG_LOADADDR			0x41000000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SERIAL_TAG
#define CONFIG_REVISION_TAG

/* Extra Environments */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"mtdparts=" MTDPARTS_DEFAULT "\0" \
	"verify=no\0" \
	"bootcmd=run bootcmd_nand\0" \
	"kernelargs=console=tty0 console=ttyAMA0,115200 consoleblank=0\0" \
	"bootargs_nand=" \
		"setenv bootargs ${kernelargs} ubi.mtd=3,2048 " \
		"root=ubi0:rootfs rootfstype=ubifs ${mtdparts} rw\0" \
	"bootcmd_nand=" \
		"run bootargs_nand && ubi part root 2048 && " \
		"ubifsmount ubi:rootfs && ubifsload 41000000 boot/uImage && " \
		"bootm 41000000\0" \
	"bootargs_mmc=" \
		"setenv bootargs ${kernelargs} " \
		"root=/dev/mmcblk0p2 rootwait ${mtdparts} rw\0" \
	"bootcmd_mmc=" \
		"run bootargs_mmc && mmc rescan && " \
		"ext2load mmc 0:2 41000000 boot/uImage && bootm 41000000\0" \
""

/* The rest of the configuration is shared */
#include <configs/mxs.h>

#endif /* __CONFIGS_APX4DEVKIT_H__ */

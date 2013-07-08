/*
 * Copyright (C) 2009 Ilya Yanok <yanok@emcraft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* include common defines/options for all imx27lite related boards */
#include "imx27lite-common.h"
/*
 * SoC Configuration
 */
#define CONFIG_IMX27LITE
#define CONFIG_HOSTNAME		imx27
#define CONFIG_BOARDNAME	"LogicPD imx27lite\n"

/*
 * Flash & Environment
 */
#define CONFIG_SYS_FLASH_SECT_SZ	0x2000	/* 8KB sect size Intel Flash */
#define CONFIG_ENV_OFFSET		(PHYS_FLASH_SIZE - 0x20000)
#define PHYS_FLASH_SIZE			0x200000
#define CONFIG_ENV_SECT_SIZE		0x10000		/* Env sector Size */

/*
 * SD/MMC
 */
#define CONFIG_MXC_MCI_REGS_BASE	0x10014000

/*
 * MTD partitions
 */
#define MTDIDS_DEFAULT		"nor0=physmap-flash.0,nand0=mxc_nand.0"
#define MTDPARTS_DEFAULT			\
	"mtdparts="				\
		"physmap-flash.0:"		\
			"256k(U-Boot),"		\
			"1664k(user),"		\
			"64k(env1),"		\
			"64k(env2);"		\
		"mxc_nand.0:"			\
			"128k(IPL-SPL),"	\
			"4m(kernel),"		\
			"22m(rootfs),"		\
			"-(userfs)"

#endif /* __CONFIG_H */

/*
 * Copyright (C) 2010 Heiko Schocher <hs@denx.de>
 *
 * based on:
 * Copyright (C) 2009 Ilya Yanok <yanok@emcraft.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* include common defines/options for all imx27lite related boards */
#include "imx27lite-common.h"

/*
 * SoC Configuration
 */
#define CONFIG_MAGNESIUM
#define CONFIG_HOSTNAME		magnesium
#define CONFIG_BOARDNAME	"Projectiondesign magnesium\n"

/*
 * Flash & Environment
 */
#define CONFIG_SYS_FLASH_SECT_SZ	0x8000	/* 64KB sect size */
#define CONFIG_ENV_OFFSET		(PHYS_FLASH_SIZE - 0x40000)
#define PHYS_FLASH_SIZE			0x800000
#define CONFIG_ENV_SECT_SIZE		0x20000		/* Env sector Size */

/*
 * NAND
 */
#define CONFIG_SYS_NAND_LARGEPAGE

/*
 * SD/MMC
 */
#define CONFIG_MXC_MCI_REGS_BASE	0x10013000

/*
 * MTD partitions
 */
#define MTDIDS_DEFAULT		"nor0=physmap-flash.0,nand0=mxc_nand.0"
#define MTDPARTS_DEFAULT			\
	"mtdparts="				\
		"physmap-flash.0:"		\
			"256k(U-Boot),"		\
			"7680k(user),"		\
			"128k(env1),"		\
			"128k(env2);"		\
		"mxc_nand.0:"			\
			"128k(IPL-SPL),"	\
			"4m(kernel),"		\
			"22m(rootfs),"		\
			"-(userfs)"

#endif /* __CONFIG_H */

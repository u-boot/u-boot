/*
 * (C) Copyright 2011, Stefano Babic <sbabic@denx.de>
 *
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * Configuration for the woodburn board.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>
#include "woodburn_common.h"

/* Set TEXT in RAM */
#define CONFIG_SYS_TEXT_BASE	0x82000000

#define CONFIG_BOOT_INTERNAL

/*
 * SPL
 */
#define	CONFIG_SPL
#define CONFIG_SPL_FRAMEWORK
#define	CONFIG_SPL_LDSCRIPT	"arch/arm/cpu/arm1136/u-boot-spl.lds"
#define	CONFIG_SPL_LIBCOMMON_SUPPORT
#define	CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x100 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x400 /* 512 KB */
#define	CONFIG_SPL_GPIO_SUPPORT

#define CONFIG_SPL_TEXT_BASE		0x10002300
#define CONFIG_SPL_MAX_SIZE		(64 * 1024)	/* 8 KB for stack */
#define CONFIG_SPL_STACK		LOW_LEVEL_SRAM_STACK

#define CONFIG_SYS_SPL_MALLOC_START	0x8f000000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000
#define CONFIG_SPL_BSS_START_ADDR	0x8f080000 /* end of RAM */
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000

#endif				/* __CONFIG_H */

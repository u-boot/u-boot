/*
 * Configuration settings for quick boot from NAND on OMAP3 EVM.
 *
 * Copyright (C) 2006-2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Author :
 *     Sanjeev Premi <premi@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __OMAP3_EVM_QUICK_NAND_H
#define __OMAP3_EVM_QUICK_NAND_H

#include <asm/arch/cpu.h>
#include <asm/arch/omap.h>

/* ----------------------------------------------------------------------------
 * Supported U-boot commands
 * ----------------------------------------------------------------------------
 */
#define CONFIG_CMD_NAND

/*
 * Board revision is detected by probing the Ethernet chip.
 *
 * When revision is statically configured via CONFIG_STATIC_BOARD_REV,
 * this option can be removed. Generated binary is leaner by ~16Kbytes.
 */
#define CONFIG_CMD_NET

/* ----------------------------------------------------------------------------
 * Supported U-boot features
 * ----------------------------------------------------------------------------
 */
#define CONFIG_SILENT_CONSOLE
#define CONFIG_ENV_IS_NOWHERE

/* -----------------------------------------------------------------------------
 * Include common board configuration
 * -----------------------------------------------------------------------------
 */
#include "omap3_evm_common.h"

/* -----------------------------------------------------------------------------
 * Default environment
 * -----------------------------------------------------------------------------
 */
#define CONFIG_BOOTDELAY	0

#define CONFIG_EXTRA_ENV_SETTINGS	\
	"verify=no\0"			\
	"silent=1"

#define CONFIG_BOOTCOMMAND				\
	"nandecc hw; "	\
	"nand read.i 0x80000000 280000 300000; "	\
	"bootm 0x80000000;"

/*
 * Update the bootargs as necessary e.g. size of memory, partition and fstype
 */
#define CONFIG_BOOTARGS				\
	"quiet "			\
	"console=ttyO0,115200n8 "	\
	"mem=128M "			\
	"noinitrd "			\
	"root=/dev/mtdblock4 rw "	\
	"rootfstype=jffs2 "

/*
 * SPL
 */
#define CONFIG_SPL_NAND_SIMPLE
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9,\
						10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_HW
#define CONFIG_SYS_NAND_U_BOOT_START   CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000

#endif /* __OMAP3_EVM_QUICK_NAND_H */

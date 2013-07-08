/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_DKB_H
#define __CONFIG_DKB_H

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nMarvell-TTC DKB"

/*
 * High Level Configuration Options
 */
#define CONFIG_SHEEVA_88SV331xV5	1	/* CPU Core subversion */
#define CONFIG_PANTHEON			1	/* SOC Family Name */
#define CONFIG_MACH_TTC_DKB		1	/* Machine type */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE - 0x00200000)
#define CONFIG_NR_DRAM_BANKS_MAX	2

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#include <config_cmd_default.h>
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MMC
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS
/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

#undef CONFIG_ARCH_MISC_INIT

/*
 * Environment variables configurations
 */
#define CONFIG_ENV_IS_NOWHERE	1	/* if env in SDRAM */
#define CONFIG_ENV_SIZE	0x20000	/* 64k */

#endif	/* __CONFIG_DKB_H */

/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ASPENITE_H
#define __CONFIG_ASPENITE_H

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nMarvell-Aspenite DB"

/*
 * High Level Configuration Options
 */
#define CONFIG_SHEEVA_88SV331xV5	1	/* CPU Core subversion */
#define CONFIG_ARMADA100		1	/* SOC Family Name */
#define CONFIG_ARMADA168		1	/* SOC Used on this Board */
#define CONFIG_MACH_ASPENITE			/* Machine type */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * There is no internal RAM in ARMADA100, using DRAM
 * TBD: dcache to be used for this
 */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE - 0x00200000)
#define CONFIG_NR_DRAM_BANKS_MAX	2

/*
 * Commands configuration
 */
#define CONFIG_SYS_NO_FLASH		/* Declare no flash (NOR/SPI) */
#include <config_cmd_default.h>
#define CONFIG_CMD_I2C
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

#endif	/* __CONFIG_ASPENITE_H */

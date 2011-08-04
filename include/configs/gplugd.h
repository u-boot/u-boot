/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * Based on Aspenite:
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __CONFIG_GPLUGD_H
#define __CONFIG_GPLUGD_H

/*
 * Version number information
 */
#define CONFIG_IDENT_STRING	"\nMarvell-gplugD"

/*
 * High Level Configuration Options
 */
#define CONFIG_SHEEVA_88SV331xV5	1	/* CPU Core subversion */
#define CONFIG_ARMADA100		1	/* SOC Family Name */
#define CONFIG_ARMADA168		1	/* SOC Used on this Board */
#define CONFIG_MACH_SHEEVAD			/* Machine type */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

#define	CONFIG_SYS_TEXT_BASE	0x00f00000

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
#define CONFIG_CMD_AUTOSCRIPT
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"
#undef CONFIG_ARCH_MISC_INIT

#ifdef CONFIG_SYS_NS16550_COM1
#undef CONFIG_SYS_NS16550_COM1
#endif /* CONFIG_SYS_NS16550_COM1 */

#define CONFIG_SYS_NS16550_COM1 ARMD1_UART3_BASE

/*
 * Environment variables configurations
 */
#define CONFIG_ENV_IS_NOWHERE	1	/* if env in SDRAM */
#define CONFIG_ENV_SIZE	0x20000		/* 64k */

#endif	/* __CONFIG_GPLUGD_H */

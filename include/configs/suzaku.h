/*
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Yasushi SHOJI <yashi@atmark-techno.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MICROBLAZE	1	/* This is an MicroBlaze CPU	*/
#define CONFIG_SUZAKU		1	/* on an SUZAKU Board		*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_SDRAM_SIZE		0x01000000
#define CONFIG_SYS_FLASH_BASE		0xfff00000
#define CONFIG_SYS_FLASH_SIZE		0x00400000
#define CONFIG_SYS_RESET_ADDRESS	0xfff00100
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_SDRAM_SIZE - (1024 * 1024))
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc */
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_MONITOR_BASE - (1024 * 1024))

#define CONFIG_XILINX_UARTLITE
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 115200 }

/* System Register (GPIO) */
#define MICROBLAZE_SYSREG_BASE_ADDR 0xFFFFA000
#define MICROBLAZE_SYSREG_RECONFIGURE (1 << 0)

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_SAVEENV
#undef CONFIG_CMD_MEMORY
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_MISC

#define CONFIG_SYS_UART1_BASE		(0xFFFF2000)
#define CONFIG_SERIAL_BASE	CONFIG_SYS_UART1_BASE

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define CONFIG_SYS_PROMPT		"SUZAKU> "	/* Monitor Command Prompt	*/
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size	*/
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/

#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_SDRAM_BASE	/* default load address		*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	1	/* max number of sectors on one chip	*/

/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CONFIG_ENV_IS_NOWHERE	1
#define	CONFIG_ENV_SIZE		0x10000	/* Total Size of Environment Sector	*/
#define CONFIG_ENV_SECT_SIZE	0x10000	/* see README - env sector total size	*/

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */

#define CONFIG_SYS_INIT_RAM_ADDR	0x80000000	/* inside of SDRAM */
#define CONFIG_SYS_INIT_RAM_END	0x2000		/* End of used area in RAM */
#define CONFIG_SYS_GBL_DATA_SIZE	128		/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET    (CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define XILINX_CLOCK_FREQ	50000000
#define CONFIG_XILINX_CLOCK_FREQ	XILINX_CLOCK_FREQ

#endif	/* __CONFIG_H */

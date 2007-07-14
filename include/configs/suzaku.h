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
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x80000000
#define CFG_SDRAM_SIZE		0x01000000
#define CFG_FLASH_BASE		0xfff00000
#define CFG_FLASH_SIZE		0x00400000
#define CFG_RESET_ADDRESS	0xfff00100
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CFG_MONITOR_BASE	(CFG_SDRAM_BASE + CFG_SDRAM_SIZE - (1024 * 1024))
#define CFG_MALLOC_LEN		(256 << 10)	/* Reserve 256 kB for malloc */
#define CFG_MALLOC_BASE		(CFG_MONITOR_BASE - (1024 * 1024))

#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 115200 }

/* System Register (GPIO) */
#define MICROBLAZE_SYSREG_BASE_ADDR 0xFFFFA000
#define MICROBLAZE_SYSREG_RECONFIGURE (1 << 0)

#define CONFIG_COMMANDS		(CONFIG__CMD_DFL)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CFG_UART1_BASE		(0xFFFF2000)
#define CONFIG_SERIAL_BASE	CFG_UART1_BASE

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define CFG_PROMPT		"SUZAKU> "	/* Monitor Command Prompt	*/
#define CFG_CBSIZE		256
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size	*/
#define CFG_MAXARGS		16		/* max number of command args	*/

#define CFG_LOAD_ADDR		CFG_SDRAM_BASE	/* default load address		*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	1	/* max number of sectors on one chip	*/

/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CFG_ENV_IS_NOWHERE	1
#define	CFG_ENV_SIZE		0x10000	/* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x10000	/* see README - env sector total size	*/

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */

#define CFG_INIT_RAM_ADDR	0x80000000	/* inside of SDRAM */
#define CFG_INIT_RAM_END	0x2000		/* End of used area in RAM */
#define CFG_GBL_DATA_SIZE	128		/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define XILINX_CLOCK_FREQ	50000000

#endif	/* __CONFIG_H */

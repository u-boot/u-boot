/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * This file contains the configuration parameters for the INCA-IP board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MIPS32		1	/* MIPS 4Kc CPU core	*/
#define CONFIG_INCA_IP		1	/* on a INCA-IP Board	*/

/* allowed values: 100000000 and 150000000 */
#define CPU_CLOCK_RATE	150000000   /* 150 MHz clock for the MIPS core */

#define CONFIG_BAUDRATE		115200

#define CFG_SDRAM_BASE		0x80000000

#define CFG_MALLOC_LEN		128*1024

#define CFG_BOOTPARAMS_LEN	128*1024

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL | CFG_CMD_ELF)
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory      */
#define	CFG_PROMPT		"INCA-IP # "	/* Monitor Command Prompt    */
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define CFG_HZ			(CPU_CLOCK_RATE/2)
#define	CFG_MAXARGS		16		/* max number of command args*/

#define	CFG_LOAD_ADDR		0x80100000	/* default load address	*/

#define CFG_MEMTEST_START	0x80200000
#define CFG_MEMTEST_END		0x80800000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(128)	/* max number of sectors on one chip */

#define PHYS_FLASH_1		0xb0000000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0xb0800000 /* Flash Bank #2 */

/* The following #defines are needed to get flash environment right */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#define CFG_INIT_SP_OFFSET	0x400000

#define CFG_FLASH_BASE		PHYS_FLASH_1

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_IN_FLASH	1

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		0xB0030000
#define CFG_ENV_SIZE		0x10000

#define CONFIG_FLASH_16BIT

#define CONFIG_NR_DRAM_BANKS	1

#define CONFIG_INCA_IP_SWITCH
#define CONFIG_NET_MULTI

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		4096
#define CFG_ICACHE_SIZE		4096
#define CFG_CACHELINE_SIZE	16

#endif	/* __CONFIG_H */

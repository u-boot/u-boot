/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Compact Integrator board.
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

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM926EJS	1	/* This is an arm926ejs CPU core  */
#define CONFIG_INTEGRATOR	1	/* in an Integrator board	*/
#define CONFIG_ARCH_CINTEGRATOR 1	/* Specifically, a CP		*/


#define CFG_MEMTEST_START       0x100000
#define CFG_MEMTEST_END         0x10000000
#define CFG_HZ                  (1000000 / 256)	/* Timer 1 is clocked at 1Mhz, with 256 divider */
#define CFG_TIMERBASE           0x13000100

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r during start up */
/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_SMC91111
#define CONFIG_SMC_USE_32_BIT
#define CONFIG_SMC91111_BASE    0xC8000000
#undef CONFIG_SMC91111_EXT_PHY

/*
 * NS16550 Configuration
 */
#define CFG_PL011_SERIAL
#define CONFIG_PL011_CLOCK	14745600
#define CONFIG_PL01x_PORTS	{ (void *)CFG_SERIAL0, (void *)CFG_SERIAL1 }
#define CONFIG_CONS_INDEX	0
#define CONFIG_BAUDRATE	38400
#define CFG_BAUDRATE_TABLE      { 9600, 19200, 38400, 57600, 115200 }
#define CFG_SERIAL0		0x16000000
#define CFG_SERIAL1		0x17000000

#define CONFIG_COMMANDS	(CFG_CMD_DHCP | CFG_CMD_IMI | CFG_CMD_NET | CFG_CMD_PING | CFG_CMD_BDI | CFG_CMD_MEMORY)
#define CONFIG_BOOTP_MASK	CONFIG_BOOTP_DEFAULT

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTARGS	"root=/dev/nfs mem=128M ip=dhcp netdev=27,0,0xfc800000,0xfc800010,eth0"
#define CONFIG_BOOTCOMMAND "bootp ; bootm"

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP	/* undef to save memory     */
#define CFG_PROMPT	"Integrator-CP # "	/* Monitor Command Prompt   */
#define CFG_CBSIZE	256		/* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CFG_PBSIZE	(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS	16		/* max number of command args   */
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size    */

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#define CFG_LOAD_ADDR	0x7fc0	/* default load address */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS    1	/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1            0x00000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE       0x02000000	/* 32 MB */

#define CFG_FLASH_BASE          0x24000000
#define PHYS_FLASH_1		(CFG_FLASH_BASE)

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_ENV_IS_NOWHERE
#define CFG_MAX_FLASH_BANKS	1		/* max number of memory banks */
#define PHYS_FLASH_SIZE         0x01000000	/* 16MB */
/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(20*CFG_HZ)	/* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(20*CFG_HZ)	/* Timeout for Flash Write */
#define CFG_MAX_FLASH_SECT 128
#define CFG_ENV_SIZE 32768

#endif							/* __CONFIG_H */

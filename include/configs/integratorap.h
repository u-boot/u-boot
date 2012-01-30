/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Integrator AP board.
 *.
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

#define CONFIG_INTEGRATOR
#define CONFIG_ARCH_INTEGRATOR
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SYS_TEXT_BASE		0x01000000
#define CONFIG_SYS_MEMTEST_START	0x100000
#define CONFIG_SYS_MEMTEST_END		0x10000000
#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_HZ_CLOCK		24000000	/* Timer 1 is clocked at 24Mhz */
#define CONFIG_SYS_TIMERBASE		0x13000100	/* Timer1		       */

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r during start up */

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_CM_INIT		1
#define CONFIG_CM_REMAP		1
#define CONFIG_CM_SPD_DETECT

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 128*1024)

/*
 * PL010 Configuration
 */
#define CONFIG_PL010_SERIAL
#define CONFIG_CONS_INDEX	0
#define CONFIG_BAUDRATE		38400
#define CONFIG_PL01x_PORTS	{ (void *) (CONFIG_SYS_SERIAL0), (void *) (CONFIG_SYS_SERIAL1) }
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_SYS_SERIAL0		0x16000000
#define CONFIG_SYS_SERIAL1		0x17000000


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTARGS		"root=/dev/mtdblock0 console=ttyAM0 console=tty"
#define CONFIG_BOOTCOMMAND	""

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	/* undef to save memory	    */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT	"Integrator-AP # "	/* Monitor Command Prompt   */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"# "
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size  */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_LOAD_ADDR	0x7fc0	/* default load address */

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
#define CONFIG_NR_DRAM_BANKS	1	/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x00000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000	/* 32 MB */
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define CONFIG_SYS_INIT_RAM_SIZE PHYS_SDRAM_1_SIZE
#define CONFIG_SYS_GBL_DATA_OFFSET (CONFIG_SYS_SDRAM_BASE + \
				    CONFIG_SYS_INIT_RAM_SIZE - \
				    GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_FLASH_BASE	0x24000000

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_FLASH_CFI		1
#define CONFIG_FLASH_CFI_DRIVER		1
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* max number of memory banks */
/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ)	/* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ)	/* Timeout for Flash Write */
#define CONFIG_SYS_MAX_FLASH_SECT	128
#define CONFIG_ENV_SIZE			32768


/*-----------------------------------------------------------------------
 * PCI definitions
 */

#define CONFIG_PCI
#define CONFIG_CMD_PCI
#define CONFIG_PCI_PNP

#define CONFIG_NET_MULTI
#define CONFIG_TULIP
#define CONFIG_EEPRO100
#define CONFIG_SYS_RX_ETH_BUFFER	8	/* use 8 rx buffer on eepro100	*/


/*-----------------------------------------------------------------------
 * There are various dependencies on the core module (CM) fitted
 * Users should refer to their CM user guide
 * - when porting adjust u-boot/Makefile accordingly
 *   to define the necessary CONFIG_ s for the CM involved
 * see e.g. integratorcp_CM926EJ-S_config
 */
#include "armcoremodule.h"

#endif	/* __CONFIG_H */

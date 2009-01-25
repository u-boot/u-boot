/*
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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

#ifndef __CONFIG_KEYMILE_H
#define __CONFIG_KEYMILE_H

/* Do boardspecific init for all boards */
#define CONFIG_BOARD_EARLY_INIT_R       1

#if defined(CONFIG_MGCOGE) || defined(CONFIG_MGSUVD)
#define CONFIG_BOOTCOUNT_LIMIT
#endif

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING

/* should go away, if kmeter I2C support is enabled */
#if defined(CONFIG_MGCOGE) || defined(CONFIG_MGSUVD)
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#endif

#undef	CONFIG_WATCHDOG			/* disable platform specific watchdog */

#define CONFIG_BOOTCOMMAND	"run net_nfs"
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#undef	CONFIG_BOOTARGS			/* the boot command will set bootargs */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size  */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size  */
#define CONFIG_CMDLINE_EDITING		1	/* add command line history     */

/* should go away, if kmeter I2C support is enabled */
#if defined(CONFIG_MGCOGE) || defined(CONFIG_MGSUVD)
#define CONFIG_HUSH_INIT_VAR	1
#endif

#define CONFIG_SYS_ALT_MEMTEST		/* memory test, takes time */
#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change */

/*
 * How to get access to the slot ID.  Put this here to make it easy
 * to modify in a centralized location.  This is used in the HDLC
 * driver to set the MAC.
*/
#define CONFIG_CHECK_ETHERNET_PRESENT	1
#define CONFIG_SYS_SLOT_ID_BASE		CONFIG_SYS_PIGGY_BASE
#define CONFIG_SYS_SLOT_ID_OFF		(0x07)	/* register offset */
#define CONFIG_SYS_SLOT_ID_MASK		(0x3f)	/* mask for slot ID bits */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

#endif /* __CONFIG_KEYMILE_H */

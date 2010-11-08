/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * 2004 (c) MontaVista Software, Inc.
 *
 * Configuation settings for the Intel Assabet board.
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
#define CONFIG_SA1110		1	/* This is an SA1100 CPU        */
#define CONFIG_ASSABET		1	/* on an Intel Assabet Board    */

#undef CONFIG_USE_IRQ
/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_NO_DCACHE

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs      */
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN          (CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE       128	/* size rsrvd for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_NET_MULTI
#define CONFIG_LAN91C96	/* we have an SMC9194 on-board */
#define CONFIG_LAN91C96_BASE	0x18000000

/*
 * select serial console configuration
 */
#define CONFIG_SA1100_SERIAL
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on Intel Assabet */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH


#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"console=ttySA0,115200n8 root=/dev/nfs ip=bootp"
#define CONFIG_BOOTCOMMAND	"bootp;tftp;bootm"
#define CONFIG_SYS_AUTOLOAD            "n"	/* No autoload */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP		/* undef to save memory         */
#define CONFIG_SYS_PROMPT		"Intel Assabet # "	/* Monitor Command Prompt       */
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size      */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args   */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

#define CONFIG_SYS_MEMTEST_START	0xc0400000	/* memtest works on     */
#define CONFIG_SYS_MEMTEST_END		0xc0800000	/* 4 ... 8 MB in DRAM   */

#define CONFIG_SYS_LOAD_ADDR		0xc0000000	/* default load address */

#define CONFIG_SYS_HZ			3686400	/* incrementer freq: 3.6864 MHz */
#define CONFIG_SYS_CPUSPEED		0x0a	/* set core clock to 206MHz */

						/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

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
#define CONFIG_NR_DRAM_BANKS	1	/* we have 1 bank of SDRAM */
#define PHYS_SDRAM_1		0xc0000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000	/* 32 MB */

#define PHYS_FLASH_1		0x00000000	/* Flash Bank #1 */
#define PHYS_FLASH_SIZE		0x02000000	/* 32 MB */
#define PHYS_FLASH_BANK_SIZE    0x01000000	/* 16 MB Banks */
#define PHYS_FLASH_SECT_SIZE    0x00040000	/* 256 KB sectors (x2) */

#define CONFIG_SYS_MONITOR_BASE        TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN         (256 * 1024)	/* Reserve 256 KB for Monitor */

#if CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_RAMSTART
#endif

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_FLASH_SIZE          PHYS_FLASH_SIZE
#define CONFIG_SYS_FLASH_CFI           1	/* flash is CFI conformant      */
#define CONFIG_FLASH_CFI_DRIVER    1	/* use common cfi driver        */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster) */
#define CONFIG_SYS_MAX_FLASH_BANKS     1	/* max # of memory banks        */
#define CONFIG_SYS_FLASH_INCREMENT     0	/* there is only one bank       */
#define CONFIG_SYS_MAX_FLASH_SECT      128	/* max # of sectors on one chip */
#undef CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_FLASH_BANKS_LIST    { CONFIG_SYS_FLASH_BASE }

#define CONFIG_ENV_IS_IN_FLASH	1

#if defined(CONFIG_ENV_IS_IN_FLASH)
#define CONFIG_ENV_IN_OWN_SECTOR	1
#define CONFIG_ENV_ADDR		(PHYS_FLASH_1 + PHYS_FLASH_SECT_SIZE)
#define CONFIG_ENV_SIZE		PHYS_FLASH_SECT_SIZE
#define CONFIG_ENV_SECT_SIZE	PHYS_FLASH_SECT_SIZE
#endif

#endif /* __CONFIG_H */

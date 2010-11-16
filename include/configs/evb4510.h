/*
 * Copyright (c) 2004	Cucy Systems (http://www.cucy.com)
 * Curt Brune <curt@cucy.com>
 *
 * Configuation settings for evb4510 board.
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
 * If we are developing, we might want to start u-boot from ram
 * so we MUST NOT initialize critical regs like mem-timing ...
 *
 * Also swap the flash1 and flash2 addresses during debug.
 *
 * #define CONFIG_SKIP_LOWLEVEL_INIT
 */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM7		1	/* This is a ARM7 CPU	 */
#define CONFIG_ARM_THUMB	1	/* this is an ARM7TDMI	 */
#define CONFIG_S3C4510B		1	/* it's a S3C4510B chip	 */
#define CONFIG_EVB4510		1	/* on an EVB4510 Board	 */
#define CONFIG_SYS_NO_CP15_CACHE

#define CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ    (4*1024)
#define CONFIG_STACKSIZE_FIQ    (4*1024)

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128*1024)

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_S3C4510_ETH   1
#define CONFIG_DRIVER_S3C4510_I2C   1
#define CONFIG_DRIVER_S3C4510_UART  1
#define CONFIG_DRIVER_S3C4510_FLASH 1

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1		1	/* we use Serial line 1, could also use 2 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		19200

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_PING


#define CONFIG_ETHADDR		00:40:95:36:35:33
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		10.0.0.11
#define CONFIG_SERVERIP		10.0.0.1
#define CONFIG_CMDLINE_TAG	/* submit bootargs to kernel */

#define CONFIG_BOOTDELAY	2
#define CONFIG_BOOTCOMMAND	"tftp 100000 uImage"
/* #define CONFIG_BOOTARGS	"console=ttyS0,19200 initrd=0x100a0040,530K root=/dev/ram keepinitrd" */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	19200		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP				/* undef to save memory */
#define CONFIG_SYS_PROMPT		"evb4510 # "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_CMDLINE_TAG                      /* allow passing of command line args to linux */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x00780000	/* 4 ... 8 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x00000000	/* default load address */

#define CONFIG_SYS_SYS_CLK_FREQ	50000000	/* CPU freq: 50 MHz */
#define CONFIG_SYS_HZ			1000		/* decrementer freq: 1 KHz */

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
 * Physical Memory Map after relocation
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 banks of DRAM */
#define PHYS_SDRAM_1		0x00000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x00800000 /* 8 MB */

#define PHYS_FLASH_1		0x01000000 /* Flash Bank #1 */
#define PHYS_FLASH_1_SIZE	0x00200000 /* 2 MB (one chip, 8bit access) */

#define PHYS_FLASH_2		0x02000000 /* Flash Bank #2 */
#define PHYS_FLASH_2_SIZE	0x00080000 /* 512KB (one chip, 8bit access) */

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_FLASH_SIZE		PHYS_FLASH_1_SIZE

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT	35	/* max number of sectors on one chip */
#define CONFIG_SYS_MAIN_SECT_SIZE	0x00010000  /* main size of sectors on one chip */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(4*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* environment settings */
#define CONFIG_ENV_IS_IN_FLASH
#undef CONFIG_ENV_IS_NOWHERE

#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x20000) /* environment start address */
#define CONFIG_ENV_SECT_SIZE	0x10000	   /* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE		0x1000	   /* max size for environment */

#endif	/* __CONFIG_H */

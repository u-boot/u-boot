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

#define CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ    (4*1024)
#define CONFIG_STACKSIZE_FIQ    (4*1024)

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128

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

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT|CONFIG_BOOTP_BOOTFILESIZE)


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
/* #define CONFIG_BOOTARGS    	"console=ttyS0,19200 initrd=0x100a0040,530K root=/dev/ram keepinitrd" */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	19200		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory */
#define CFG_PROMPT		"evb4510 # "	/* Monitor Command Prompt */
#define CFG_CBSIZE		256		/* Console I/O Buffer Size */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_CMDLINE_TAG                      /* allow passing of command line args to linux */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#define CFG_MEMTEST_START	0x00000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00780000	/* 4 ... 8 MB in DRAM	*/

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		0x00000000	/* default load address */

#define CFG_SYS_CLK_FREQ	50000000	/* CPU freq: 50 MHz */
#define CFG_HZ			1000		/* decrementer freq: 1 KHz */

						/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

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

#define CFG_FLASH_BASE		PHYS_FLASH_1
#define CFG_FLASH_SIZE		PHYS_FLASH_1_SIZE

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	35	/* max number of sectors on one chip */
#define CFG_MAIN_SECT_SIZE	0x00010000  /* main size of sectors on one chip */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(4*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ) /* Timeout for Flash Write */

/* environment settings */
#define CFG_ENV_IS_IN_FLASH
#undef CFG_ENV_IS_NOWHERE

#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x20000) /* environment start address */
#define CFG_ENV_SECT_SIZE	0x10000	   /* Total Size of Environment Sector */
#define CFG_ENV_SIZE		0x1000	   /* max size for environment */

#endif	/* __CONFIG_H */

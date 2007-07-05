/*
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Configuation settings for the EP7312 board.
 *
 * Modified to work on Armadillo HT1070 ARM720T board
 * (C) Copyright 2005 Rowel Atienza rowel@diwalabs.com
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
 * If we are developing, we might want to start armboot from ram
 * so we MUST NOT initialize critical regs like mem-timing ...
 */
/*#define	CONFIG_INIT_CRITICAL*/		/* undef for developing */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM7		1	/* This is a ARM7 CPU	*/
#define CONFIG_ARMADILLO 	1	/* on an Armadillo Board      */
#define CONFIG_ARM_THUMB	1	/* this is an ARM720TDMI */
#undef  CONFIG_ARM7_REVD	 	/* disable ARM720 REV.D Workarounds */

#undef CONFIG_USE_IRQ			/* don't need them anymore */

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#define CS8900_BASE		0x20000300 /* armadillo board */
#define CS8900_BUS16		1
#undef  CS8900_BUS32

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1		1	/* we use Serial line 1 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

#define CONFIG_BOOTP_MASK       (CONFIG_BOOTP_DEFAULT|CONFIG_BOOTP_BOOTFILESIZE)


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>


#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS    	"root=/dev/ram0 rootfstype=ext2 console=ttyAM0,115200"

#define CONFIG_BOOTCOMMAND	"bootm 40000 180000"

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"ARMADILLO # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0xc0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0xc0800000	/* 4 ... 8 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x00040000	/* default load address	for armadillo: kernel img is here*/

#define	CFG_HZ			2000		/* decrementer freq: 2 kHz */

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
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0xc0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB armadillo SDRAM */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_SIZE		0x00400000 /* 4 MB */

#define CFG_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + 0x20000)	/* Addr of Environment Sector	*/
#define CFG_ENV_SIZE		0x20000	/* Total Size of Environment Sector	*/

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

#endif	/* __CONFIG_H */

/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * 2003-2004 (c) MontaVista Software, Inc.
 *
 * Configuation settings for the ADS GraphicsClient+ board.
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

#undef	 DEBUG

/*
 * The ADS GCPlus Linux boot ROM loads U-Boot into RAM at 0xc0200000.
 * We don't actually init RAM in this case since we're using U-Boot as
 * an secondary boot loader during Linux kernel development and testing,
 * e.g. bootp/tftp download of the kernel is a far more convenient
 * when testing new kernels on this target. However the ADS GCPlus Linux
 * boot ROM leaves the MMU enabled when it passes control to U-Boot. So
 * we use lowlevel_init (CONFIG_INIT_CRITICAL) to remedy that problem.
 */
#undef  CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT	1

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SA1110		1	/* This is an SA1100 CPU	*/
#define CONFIG_GCPLUS		1	/* on an ADS GCPlus Board      */

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN          (CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE       128     /* size rsrvd for initial data */


/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_LAN91C96	/* we have an SMC9194 on-board */
#define CONFIG_LAN91C96_BASE	0x100e0000

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL3          1	/* we use SERIAL 3 on ADS GCPlus */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		38400

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL | CFG_CMD_DHCP)
#define CONFIG_BOOTP_MASK	CONFIG_BOOTP_DEFAULT

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"console=ttySA0,38400n8 mtdparts=sa1100-flash:1m@0(zImage),3m@1m(ramdisk.gz),12m@4m(userfs) root=/dev/nfs ip=bootp"
#define CONFIG_BOOTCOMMAND	"bootp;tftp;bootm"
#define CFG_AUTOLOAD            "n"             /* No autoload */

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	38400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"ADS GCPlus # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0xc0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0xc0800000	/* 4 ... 8 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0xc0000000	/* default load address	*/

#define	CFG_HZ			3686400		/* incrementer freq: 3.6864 MHz */
#define CFG_CPUSPEED		0x0a		/* set core clock to 206MHz */

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
#define CONFIG_NR_DRAM_BANKS	2	   /* we have 2 banks of DRAM */
#define PHYS_SDRAM_1		0xc0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x01000000 /* 16 MB */
#define PHYS_SDRAM_2		0xc8000000 /* SDRAM Bank #2 */
#define PHYS_SDRAM_2_SIZE	0x01000000 /* 16 MB */


#define PHYS_FLASH_1		0x08000000 /* Flash Bank #1 */
#define PHYS_FLASH_SIZE		0x00800000 /* 8 MB */
#define PHYS_FLASH_BANK_SIZE    0x01000000 /* 16 MB Banks */
#define PHYS_FLASH_SECT_SIZE    0x00040000 /* 256 KB sectors (x2) */

#define CFG_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#if	1
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ) /* Timeout for Flash Write */
#else
/* REVISIT: This doesn't work on ADS GCPlus just yet: */
#define CFG_FLASH_CFI           1       /* flash is CFI conformant      */
#define CFG_FLASH_CFI_DRIVER    1       /* use common cfi driver        */
#define CFG_FLASH_USE_BUFFER_WRITE 1    /* use buffered writes (20x faster) */
#define CFG_MAX_FLASH_BANKS     1       /* max # of memory banks        */
#define CFG_FLASH_INCREMENT     0       /* there is only one bank       */
#define CFG_MAX_FLASH_SECT      128     /* max # of sectors on one chip */
/*#define CFG_FLASH_PROTECTION    1       /--* hardware flash protection    */
#define CFG_FLASH_BANKS_LIST    { CFG_FLASH_BASE }
#endif

#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + PHYS_FLASH_SECT_SIZE)	/* Addr of Environment Sector	*/
#define CFG_ENV_SIZE		PHYS_FLASH_SECT_SIZE

#endif	/* __CONFIG_H */

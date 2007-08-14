/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Configuation settings for the Shannon/TuxScreen/IS2630 WebPhone Board.
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
 * Since we use the Inferno-Loader to bring us to live,
 * we skip the lowlevel init stuff.
 * But U-Boot still relocates itself into RAM
 */
#define CONFIG_INFERNO			/* we are using the inferno bootldr */
#define CONFIG_SKIP_LOWLEVEL_INIT	1
#undef  CONFIG_SKIP_RELOCATE_UBOOT

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_SA1100		1	/* This is an SA1100 CPU	*/
#define CONFIG_SHANNON		1	/* on an SHANNON/TuxScreen Board      */

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_3C589	1

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL3          1	/* we use SERIAL 3  */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200


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


#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS    	"root=ramfs devfs=mount console=ttySA0,115200"
#define CONFIG_NETMASK          255.255.0.0
#define CONFIG_BOOTCOMMAND	"help"

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"TuxScreen # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0xc0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0xc0800000	/* 4 ... 8 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0xd0000000	/* default load address	*/

#define	CFG_HZ			3686400		/* incrementer freq: 3.6864 MHz */
#define CFG_CPUSPEED		0x09		/* 190 MHz for Shannon */

						/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_DOS_PARTITION	1		/* DOS partitiion support */

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
/* BE CAREFUL */
#define CONFIG_NR_DRAM_BANKS	4	   /* we have 4 banks of EDORAM */
#define PHYS_SDRAM_1		0xc0000000 /* RAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x00400000 /* 4 MB */
#define PHYS_SDRAM_2		0xc8000000 /* RAM Bank #2 */
#define PHYS_SDRAM_2_SIZE	0x00400000 /* 4 MB */
#define PHYS_SDRAM_3            0xd0000000 /* RAM Bank #3 */
#define PHYS_SDRAM_3_SIZE       0x00400000 /* 4 MB */
#define PHYS_SDRAM_4            0xd8000000 /* RAM Bank #4 */
#define PHYS_SDRAM_4_SIZE       0x00400000 /* 4 MB */


#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_SIZE		0x00400000 /* 4 MB */

#define CFG_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	(31+4)	/* max number of sectors on one chip	*/

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_IN_FLASH	1
#ifdef CONFIG_INFERNO
/* we take the last sector, 128 KB in size, but we only use 4 KB of it for stack reasons */
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + 0x003E0000)	/* Addr of Environment Sector	*/
#define CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/
#else
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + 0x1C000)	/* Addr of Environment Sector	*/
#define CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/
#endif

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */

/* we pick the upper one */

#define CONFIG_PCMCIA_SLOT_A

#define CFG_PCMCIA_IO_ADDR	(0x20000000)
#define CFG_PCMCIA_IO_SIZE	( 64 << 20 )
#define CFG_PCMCIA_DMA_ADDR	(0x24000000)
#define CFG_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CFG_PCMCIA_ATTRB_ADDR	(0x2C000000)
#define CFG_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CFG_PCMCIA_MEM_ADDR	(0x28000000)
#define CFG_PCMCIA_MEM_SIZE	( 64 << 20 )

/* in fact, MEM and ATTRB are swapped - has to be corrected soon in cmd_pcmcia or so */

/*-----------------------------------------------------------------------
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define	CONFIG_IDE_PCCARD	1	/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_PCMCIA		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CFG_ATA_IDE0_OFFSET	0x0000

/* it's simple, all regs are in I/O space */
#define CFG_ATA_BASE_ADDR	CFG_PCMCIA_ATTRB_ADDR

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	0

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	0

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	0

/*-----------------------------------------------------------------------
 */

#endif	/* __CONFIG_H */

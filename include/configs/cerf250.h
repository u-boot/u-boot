/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Configuation settings for the CERF250 board.
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
#define CONFIG_PXA250		1	/* This is an PXA250 CPU    */
#define CONFIG_CERF250		1	/* on Cerf PXA Board	    */
#define BOARD_LATE_INIT		1
#define CONFIG_BAUDRATE		38400

#undef 	CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	    (CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_SMC91111
#define CONFIG_SMC91111_BASE 0x04000300
#define CONFIG_SMC_USE_32_BIT

/*
 * select serial console configuration
 */
#define CONFIG_FFUART	     1	/* we use FFUART on CERF PXA */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_COMMANDS		(CONFIG_CMD_DFL)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
#define CONFIG_ETHADDR		00:D0:CA:F1:3C:D2
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.0.5
#define CONFIG_SERVERIP		192.168.0.2
#define CONFIG_BOOTCOMMAND	"bootm 0xC0000"
#define CONFIG_BOOTARGS		"root=/dev/mtdblock3 rootfstype=jffs2 console=ttyS0,38400"
#define CONFIG_CMDLINE_TAG

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER		1
#define CFG_PROMPT_HUSH_PS2	"> "

#define CFG_LONGHELP					/* undef to save memory		*/
#ifdef CFG_HUSH_PARSER
#define CFG_PROMPT			"uboot$ "	/* Monitor Command Prompt */
#else
#define CFG_PROMPT			"=> "		/* Monitor Command Prompt */
#endif
#define CFG_CBSIZE			256			/* Console I/O Buffer Size	*/
#define CFG_PBSIZE 			(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
										/* Print Buffer Size */
#define CFG_MAXARGS			16			/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_DEVICE_NULLDEV	1

#define CFG_MEMTEST_START	0xa0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM	*/

#undef	CFG_CLKS_IN_HZ

#define CFG_LOAD_ADDR		0xa2000000	/* default load address */

#define CFG_HZ				3686400		/* incrementer freq: 3.6864 MHz */
#define CFG_CPUSPEED		0x141		/* set core clock to 400/200/100 MHz */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }


/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE		(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	4	   		/* we have 2 banks of DRAM */
#define PHYS_SDRAM_1			0xa0000000 	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE		0x04000000 	/* 64 MB */
#define PHYS_SDRAM_2			0xa4000000 	/* SDRAM Bank #2 */
#define PHYS_SDRAM_2_SIZE		0x00000000 	/* 0 MB */
#define PHYS_SDRAM_3			0xa8000000 	/* SDRAM Bank #3 */
#define PHYS_SDRAM_3_SIZE		0x00000000 	/* 0 MB */
#define PHYS_SDRAM_4			0xac000000 	/* SDRAM Bank #4 */
#define PHYS_SDRAM_4_SIZE		0x00000000 	/* 0 MB */

#define PHYS_FLASH_1			0x00000000 	/* Flash Bank #1 */
#define PHYS_FLASH_2			0x04000000 	/* Flash Bank #2 */
#define PHYS_FLASH_SIZE			0x02000000 	/* 32 MB */
#define PHYS_FLASH_BANK_SIZE		0x02000000 	/* 32 MB Banks */
#define PHYS_FLASH_SECT_SIZE		0x00040000 	/* 256 KB sectors (x2) */

#define CFG_DRAM_BASE			0xa0000000
#define CFG_DRAM_SIZE			0x04000000

#define CFG_FLASH_BASE			PHYS_FLASH_1

/*
 * GPIO settings
 */


#define CFG_GPSR0_VAL		0x00408030
#define CFG_GPSR1_VAL		0x00BFA882
#define CFG_GPSR2_VAL		0x0001C000
#define CFG_GPCR0_VAL		0xC0031100
#define CFG_GPCR1_VAL		0xFC400300
#define CFG_GPCR2_VAL		0x00003FFF
#define CFG_GPDR0_VAL		0xC0439330
#define CFG_GPDR1_VAL		0xFCFFAB82
#define CFG_GPDR2_VAL		0x0001FFFF
#define CFG_GAFR0_L_VAL		0x80000000
#define CFG_GAFR0_U_VAL		0xA5000010
#define CFG_GAFR1_L_VAL		0x60008018
#define CFG_GAFR1_U_VAL		0xAAA5AAAA
#define CFG_GAFR2_L_VAL		0xAAA0000A
#define CFG_GAFR2_U_VAL		0x00000002

#define CFG_PSSR_VAL		0x20

/*
 * Memory settings
 */
#define CFG_MSC0_VAL		0x12447FF0
#define CFG_MSC1_VAL		0x12BC5554
#define CFG_MSC2_VAL		0x7FF97FF1
#define CFG_MDCNFG_VAL		0x00001AC9
#define CFG_MDREFR_VAL		0x03CDC017
#define CFG_MDMRS_VAL		0x00000000

/*
 * PCMCIA and CF Interfaces
 */
#define CFG_MECR_VAL		0x00000000
#define CFG_MCMEM0_VAL		0x00010504
#define CFG_MCMEM1_VAL		0x00010504
#define CFG_MCATT0_VAL		0x00010504
#define CFG_MCATT1_VAL		0x00010504
#define CFG_MCIO0_VAL		0x00004715
#define CFG_MCIO1_VAL		0x00004715

#define _LED			0x08000010	/*check this */
#define LED_BLANK		0x08000040
#define LED_GPIO		0x10

/*
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	128  	/* max number of sectors on one chip    */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(25*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(25*CFG_HZ) /* Timeout for Flash Write */

#define CFG_MONITOR_LEN		0x40000		/* 256 KiB */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + CFG_MONITOR_LEN)
#define CFG_ENV_SIZE		0x40000	/* Total Size of Environment Sector	*/


#endif	/* __CONFIG_H */

/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Configuration settings for the PLEB 2 board.
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
#define CONFIG_PXA250		1	/* This is an PXA255 CPU    */
#define CONFIG_PLEB2		1	/* on an PLEB2 Board	    */
#undef CONFIG_LCD
#undef CONFIG_MMC
#define BOARD_LATE_INIT		1

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	    (CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */

/* None - PLEB 2 doesn't have any of this.
   #define CONFIG_DRIVER_LAN91C96
   #define CONFIG_LAN91C96_BASE 0x0C000000 */

/*
 * select serial console configuration
 */
#define CONFIG_FFUART	       1       /* we use FFUART on PLEB 2 */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL & ~CFG_CMD_NET)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_BOOTDELAY	3
#define CONFIG_ETHADDR		08:00:3e:26:0a:5b
#define CONFIG_NETMASK		255.255.0.0
#define CONFIG_IPADDR		192.168.0.21
#define CONFIG_SERVERIP		192.168.0.250
#define CONFIG_BOOTCOMMAND	"bootm 40000"
#define CONFIG_BOOTARGS		"root=/dev/mtdblock2 prompt_ramdisk=0 load_ramdisk=1 console=ttyS0,115200"

#define CONFIG_CMDLINE_TAG
#define CONFIG_INITRD_TAG
#define CONFIG_SETUP_MEMORY_TAGS

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER		1
#define CFG_PROMPT_HUSH_PS2	"> "

#define CFG_LONGHELP				/* undef to save memory		*/
#ifdef CFG_HUSH_PARSER
#define CFG_PROMPT		"$ "		/* Monitor Command Prompt */
#else
#define CFG_PROMPT		"=> "		/* Monitor Command Prompt */
#endif
#define CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_DEVICE_NULLDEV	1

#define CFG_MEMTEST_START	0xa0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM	*/

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		0xa2000000	/* default load address */

#define CFG_HZ			3686400		/* incrementer freq: 3.6864 MHz */
#define CFG_CPUSPEED		0x141		/* set core clock to 200/200/100 MHz */

						/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	4	   /* we have 2 banks of DRAM */
#define PHYS_SDRAM_1		0xa0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB */
#define PHYS_SDRAM_2		0xa4000000 /* SDRAM Bank #2 */
#define PHYS_SDRAM_2_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_3		0xa8000000 /* SDRAM Bank #3 */
#define PHYS_SDRAM_3_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_4		0xac000000 /* SDRAM Bank #4 */
#define PHYS_SDRAM_4_SIZE	0x00000000 /* 0 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0x04000000 /* Flash Bank #2 */
#define PHYS_FLASH_SIZE		0x00800000 /* 4 MB */

/* Not entirely sure about this - DS/CHC */
#define PHYS_FLASH_BANK_SIZE	0x02000000 /* 32 MB Banks */
#define PHYS_FLASH_SECT_SIZE	0x00010000 /* 64 KB sectors (x2) */

#define CFG_DRAM_BASE		PHYS_SDRAM_1
#define CFG_DRAM_SIZE		PHYS_SDRAM_1_SIZE

#define CFG_FLASH_BASE		PHYS_FLASH_1
#define CFG_MONITOR_BASE	CFG_FLASH_BASE

/*
 * GPIO settings
 */
#define CFG_GPSR0_VAL		0x00000000  /* Don't set anything */
#define CFG_GPSR1_VAL		0x00000080
#define CFG_GPSR2_VAL		0x00000000

#define CFG_GPCR0_VAL		0x00000000  /* Don't clear anything */
#define CFG_GPCR1_VAL		0x00000000
#define CFG_GPCR2_VAL		0x00000000

#define CFG_GPDR0_VAL		0x00000000
#define CFG_GPDR1_VAL		0x000007C3
#define CFG_GPDR2_VAL		0x00000000

/* Edge detect registers (these are set by the kernel) */
#define CFG_GRER0_VAL	    0x00000000
#define CFG_GRER1_VAL	    0x00000000
#define CFG_GRER2_VAL	    0x00000000
#define CFG_GFER0_VAL	    0x00000000
#define CFG_GFER1_VAL	    0x00000000
#define CFG_GFER2_VAL	    0x00000000

#define CFG_GAFR0_L_VAL		0x00000000
#define CFG_GAFR0_U_VAL		0x00000000
#define CFG_GAFR1_L_VAL		0x00008010  /* Use FF UART Send and Receive */
#define CFG_GAFR1_U_VAL		0x00000000
#define CFG_GAFR2_L_VAL		0x00000000
#define CFG_GAFR2_U_VAL		0x00000000

#define CFG_PSSR_VAL		0x20
#define CFG_CCCR_VAL	    0x00000141	/* 100 MHz memory, 200 MHz CPU	*/
#define CFG_CKEN_VAL	    0x00000060	/* FFUART and STUART enabled	*/
#define CFG_ICMR_VAL	    0x00000000	/* No interrupts enabled	*/

/*
 * Memory settings
 */
#define CFG_MSC0_VAL		0x00007FF0 /* Not properly calculated - FIXME (DS) */
#define CFG_MSC1_VAL		0x00000000
#define CFG_MSC2_VAL		0x00000000

#define CFG_MDCNFG_VAL		0x00000aC9 /* Memory timings for the SDRAM.
					      tRP=2, CL=2, tRCD=2, tRAS=5, tRC=8 */

#define CFG_MDREFR_VAL		0x00403018 /* Initial setting, individual	*/
					   /* bits set in lowlevel_init.S	*/
#define CFG_MDMRS_VAL		0x00000000

/*
 * PCMCIA and CF Interfaces
 */
#define CFG_MECR_VAL		0x00000000  /* Hangover from Lubbock.
					       Needs calculating. (DS/CHC) */
#define CFG_MCMEM0_VAL		0x00010504
#define CFG_MCMEM1_VAL		0x00010504
#define CFG_MCATT0_VAL		0x00010504
#define CFG_MCATT1_VAL		0x00010504
#define CFG_MCIO0_VAL		0x00004715
#define CFG_MCIO1_VAL		0x00004715

/*
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/

/* timeout values are in ticks */
/* FIXME */
#define CFG_FLASH_ERASE_TOUT	(25*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(25*CFG_HZ) /* Timeout for Flash Write */

/* Flash protection */
#define CFG_FLASH_PROTECTION	1

/* FIXME */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + 0x3C000)	/* Addr of Environment Sector	*/
#define CFG_ENV_SIZE		0x4000	/* Total Size of Environment */
#define CFG_ENV_SECT_SIZE	0x20000

/* Option added to get around byte ordering issues in the flash driver */
#define CFG_LITTLE_ENDIAN	1

#endif	/* __CONFIG_H */

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
#define	CONFIG_SYS_TEXT_BASE	0x0

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_DCACHE_OFF

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 128*1024)

/*
 * Hardware drivers
 */

/* None - PLEB 2 doesn't have any of this.
	#define CONFIG_NET_MULTI
	#define CONFIG_LAN91C96
	#define CONFIG_LAN91C96_BASE 0x0C000000
 */

/*
 * select serial console configuration
 */
#define CONFIG_PXA_SERIAL
#define CONFIG_FFUART	       1       /* we use FFUART on PLEB 2 */

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

#undef CONFIG_CMD_NET
#undef CONFIG_CMD_NFS


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

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_HUSH_PARSER		1
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

#define CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT		"$ "		/* Monitor Command Prompt */
#else
#define CONFIG_SYS_PROMPT		"=> "		/* Monitor Command Prompt */
#endif
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_DEVICE_NULLDEV	1

#define CONFIG_SYS_MEMTEST_START	0xa0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0xa2000000	/* default load address */

#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_CPUSPEED		0x141		/* set core clock to 200/200/100 MHz */

						/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#ifdef CONFIG_MMC
#define CONFIG_PXA_MMC
#define CONFIG_CMD_MMC
#endif

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
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0xa0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x02000000 /* 32 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0x04000000 /* Flash Bank #2 */
#define PHYS_FLASH_SIZE		0x00800000 /* 4 MB */

/* Not entirely sure about this - DS/CHC */
#define PHYS_FLASH_BANK_SIZE	0x02000000 /* 32 MB Banks */
#define PHYS_FLASH_SECT_SIZE	0x00010000 /* 64 KB sectors (x2) */

#define CONFIG_SYS_DRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_DRAM_SIZE		PHYS_SDRAM_1_SIZE

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		(GENERATED_GBL_DATA_SIZE + PHYS_SDRAM_1)

/*
 * GPIO settings
 */
#define CONFIG_SYS_GPSR0_VAL		0x00000000  /* Don't set anything */
#define CONFIG_SYS_GPSR1_VAL		0x00000080
#define CONFIG_SYS_GPSR2_VAL		0x00000000

#define CONFIG_SYS_GPCR0_VAL		0x00000000  /* Don't clear anything */
#define CONFIG_SYS_GPCR1_VAL		0x00000000
#define CONFIG_SYS_GPCR2_VAL		0x00000000

#define CONFIG_SYS_GPDR0_VAL		0x00000000
#define CONFIG_SYS_GPDR1_VAL		0x000007C3
#define CONFIG_SYS_GPDR2_VAL		0x00000000

/* Edge detect registers (these are set by the kernel) */
#define CONFIG_SYS_GRER0_VAL	    0x00000000
#define CONFIG_SYS_GRER1_VAL	    0x00000000
#define CONFIG_SYS_GRER2_VAL	    0x00000000
#define CONFIG_SYS_GFER0_VAL	    0x00000000
#define CONFIG_SYS_GFER1_VAL	    0x00000000
#define CONFIG_SYS_GFER2_VAL	    0x00000000

#define CONFIG_SYS_GAFR0_L_VAL		0x00000000
#define CONFIG_SYS_GAFR0_U_VAL		0x00000000
#define CONFIG_SYS_GAFR1_L_VAL		0x00008010  /* Use FF UART Send and Receive */
#define CONFIG_SYS_GAFR1_U_VAL		0x00000000
#define CONFIG_SYS_GAFR2_L_VAL		0x00000000
#define CONFIG_SYS_GAFR2_U_VAL		0x00000000

#define CONFIG_SYS_PSSR_VAL		0x20
#define CONFIG_SYS_CCCR		    0x00000141	/* 100 MHz memory, 200 MHz CPU	*/
#define CONFIG_SYS_CKEN		    0x00000060	/* FFUART and STUART enabled	*/
#define CONFIG_SYS_ICMR		    0x00000000	/* No interrupts enabled	*/

/*
 * Memory settings
 */
#define CONFIG_SYS_MSC0_VAL		0x00007FF0 /* Not properly calculated - FIXME (DS) */
#define CONFIG_SYS_MSC1_VAL		0x00000000
#define CONFIG_SYS_MSC2_VAL		0x00000000

#define CONFIG_SYS_MDCNFG_VAL		0x00000aC9 /* Memory timings for the SDRAM.
					      tRP=2, CL=2, tRCD=2, tRAS=5, tRC=8 */

#define CONFIG_SYS_MDREFR_VAL		0x00403018 /* Initial setting, individual	*/
					   /* bits set in lowlevel_init.S	*/
#define CONFIG_SYS_MDMRS_VAL		0x00000000

#define	CONFIG_SYS_FLYCNFG_VAL		0x00000000
#define	CONFIG_SYS_SXCNFG_VAL		0x00000000

/*
 * PCMCIA and CF Interfaces
 */
#define CONFIG_SYS_MECR_VAL		0x00000000  /* Hangover from Lubbock.
					       Needs calculating. (DS/CHC) */
#define CONFIG_SYS_MCMEM0_VAL		0x00010504
#define CONFIG_SYS_MCMEM1_VAL		0x00010504
#define CONFIG_SYS_MCATT0_VAL		0x00010504
#define CONFIG_SYS_MCATT1_VAL		0x00010504
#define CONFIG_SYS_MCIO0_VAL		0x00004715
#define CONFIG_SYS_MCIO1_VAL		0x00004715

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/

/* timeout values are in ticks */
/* FIXME */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* Flash protection */
#define CONFIG_SYS_FLASH_PROTECTION	1

/* FIXME */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(PHYS_FLASH_1 + 0x3C000)	/* Addr of Environment Sector	*/
#define CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment */
#define CONFIG_ENV_SECT_SIZE	0x20000

/* Option added to get around byte ordering issues in the flash driver */
#define CONFIG_SYS_LITTLE_ENDIAN	1

#endif	/* __CONFIG_H */

/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Configuation settings for the LUBBOCK board.
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
#define CONFIG_LUBBOCK		1	/* on an LUBBOCK Board	    */
#define CONFIG_LCD		1
#ifdef CONFIG_LCD
#define CONFIG_SHARP_LM8V31
#endif
#define CONFIG_MMC
#define BOARD_LATE_INIT		1
#define CONFIG_DOS_PARTITION

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_NO_DCACHE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 128*1024)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Hardware drivers
 */
#define CONFIG_NET_MULTI
#define CONFIG_LAN91C96
#define CONFIG_LAN91C96_BASE 0x0C000000

/*
 * select serial console configuration
 */
#define CONFIG_PXA_SERIAL
#define CONFIG_FFUART	       1       /* we use FFUART on LUBBOCK */

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

#define CONFIG_CMD_FAT


#define CONFIG_BOOTDELAY	3
#define CONFIG_ETHADDR		08:00:3e:26:0a:5b
#define CONFIG_NETMASK		255.255.0.0
#define CONFIG_IPADDR		192.168.0.21
#define CONFIG_SERVERIP		192.168.0.250
#define CONFIG_BOOTCOMMAND	"bootm 80000"
#define CONFIG_BOOTARGS		"root=/dev/mtdblock2 rootfstype=cramfs console=ttyS0,115200"
#define CONFIG_CMDLINE_TAG
#define CONFIG_TIMESTAMP

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

#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_DRAM_BASE + 0x8000) /* default load address */

#define CONFIG_SYS_HZ			1000
#define CONFIG_SYS_CPUSPEED		0x161		/* set core clock to 400/200/100 MHz */

						/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#ifdef CONFIG_MMC
#define CONFIG_PXA_MMC
#define CONFIG_CMD_MMC
#define CONFIG_SYS_MMC_BASE		0xF0000000
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
#define CONFIG_NR_DRAM_BANKS	4	   /* we have 2 banks of DRAM */
#define PHYS_SDRAM_1		0xa0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */
#define PHYS_SDRAM_2		0xa4000000 /* SDRAM Bank #2 */
#define PHYS_SDRAM_2_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_3		0xa8000000 /* SDRAM Bank #3 */
#define PHYS_SDRAM_3_SIZE	0x00000000 /* 0 MB */
#define PHYS_SDRAM_4		0xac000000 /* SDRAM Bank #4 */
#define PHYS_SDRAM_4_SIZE	0x00000000 /* 0 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0x04000000 /* Flash Bank #2 */
#define PHYS_FLASH_SIZE		0x02000000 /* 32 MB */
#define PHYS_FLASH_BANK_SIZE	0x02000000 /* 32 MB Banks */
#define PHYS_FLASH_SECT_SIZE	0x00040000 /* 256 KB sectors (x2) */

#define CONFIG_SYS_DRAM_BASE		0xa0000000
#define CONFIG_SYS_DRAM_SIZE		0x04000000

#define CONFIG_SYS_FLASH_BASE		PHYS_FLASH_1

#define FPGA_REGS_BASE_PHYSICAL 0x08000000

/*
 * GPIO settings
 */
#define CONFIG_SYS_GPSR0_VAL		0x00008000
#define CONFIG_SYS_GPSR1_VAL		0x00FC0382
#define CONFIG_SYS_GPSR2_VAL		0x0001FFFF
#define CONFIG_SYS_GPCR0_VAL		0x00000000
#define CONFIG_SYS_GPCR1_VAL		0x00000000
#define CONFIG_SYS_GPCR2_VAL		0x00000000
#define CONFIG_SYS_GPDR0_VAL		0x0060A800
#define CONFIG_SYS_GPDR1_VAL		0x00FF0382
#define CONFIG_SYS_GPDR2_VAL		0x0001C000
#define CONFIG_SYS_GAFR0_L_VAL		0x98400000
#define CONFIG_SYS_GAFR0_U_VAL		0x00002950
#define CONFIG_SYS_GAFR1_L_VAL		0x000A9558
#define CONFIG_SYS_GAFR1_U_VAL		0x0005AAAA
#define CONFIG_SYS_GAFR2_L_VAL		0xA0000000
#define CONFIG_SYS_GAFR2_U_VAL		0x00000002

#define CONFIG_SYS_PSSR_VAL		0x20

/*
 * Memory settings
 */
#define CONFIG_SYS_MSC0_VAL		0x23F223F2
#define CONFIG_SYS_MSC1_VAL		0x3FF1A441
#define CONFIG_SYS_MSC2_VAL		0x7FF97FF1
#define CONFIG_SYS_MDCNFG_VAL		0x00001AC9
#define CONFIG_SYS_MDREFR_VAL		0x00018018
#define CONFIG_SYS_MDMRS_VAL		0x00000000

/*
 * PCMCIA and CF Interfaces
 */
#define CONFIG_SYS_MECR_VAL		0x00000000
#define CONFIG_SYS_MCMEM0_VAL		0x00010504
#define CONFIG_SYS_MCMEM1_VAL		0x00010504
#define CONFIG_SYS_MCATT0_VAL		0x00010504
#define CONFIG_SYS_MCATT1_VAL		0x00010504
#define CONFIG_SYS_MCIO0_VAL		0x00004715
#define CONFIG_SYS_MCIO1_VAL		0x00004715

#define _LED			0x08000010
#define LED_BLANK		0x08000040

/*
 * FLASH and environment organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	128  /* max number of sectors on one chip    */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(25*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* NOTE: many default partitioning schemes assume the kernel starts at the
 * second sector, not an environment.  You have been warned!
 */
#define	CONFIG_SYS_MONITOR_LEN		PHYS_FLASH_SECT_SIZE
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_ADDR		(PHYS_FLASH_1 + PHYS_FLASH_SECT_SIZE)
#define CONFIG_ENV_SECT_SIZE	PHYS_FLASH_SECT_SIZE
#define CONFIG_ENV_SIZE		(PHYS_FLASH_SECT_SIZE / 16)


/*
 * FPGA Offsets
 */
#define WHOAMI_OFFSET		0x00
#define HEXLED_OFFSET		0x10
#define BLANKLED_OFFSET		0x40
#define DISCRETELED_OFFSET	0x40
#define CNFG_SWITCHES_OFFSET	0x50
#define USER_SWITCHES_OFFSET	0x60
#define MISC_WR_OFFSET		0x80
#define MISC_RD_OFFSET		0x90
#define INT_MASK_OFFSET		0xC0
#define INT_CLEAR_OFFSET	0xD0
#define GP_OFFSET		0x100

#endif	/* __CONFIG_H */

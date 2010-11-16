/*
 * (C) Copyright 2002
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Configuation settings for the Zylonite board.
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
#define CONFIG_CPU_MONAHANS	1	/* Intel Monahan CPU    */
#define	CONFIG_CPU_PXA320
#define CONFIG_ZYLONITE		1	/* Zylonite board       */

/* #define CONFIG_LCD		1 */
#ifdef CONFIG_LCD
#define CONFIG_SHARP_LM8V31
#endif
#undef CONFIG_MMC
#define BOARD_LATE_INIT		1

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

/* we will never enable dcache, because we have to setup MMU first */
#define CONFIG_SYS_NO_DCACHE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 128*1024)

/*
 * Hardware drivers
 */

#undef TURN_ON_ETHERNET
#ifdef TURN_ON_ETHERNET
# define CONFIG_SMC91111 1
# define CONFIG_SMC91111_BASE   0x14000300
# define CONFIG_SMC91111_EXT_PHY
# define CONFIG_SMC_USE_32_BIT
# undef CONFIG_SMC_USE_IOFUNCS          /* just for use with the kernel */
#endif

/*
 * select serial console configuration
 */
#define CONFIG_PXA_SERIAL
#define CONFIG_FFUART	       1

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

#ifdef TURN_ON_ETHERNET
    #define CONFIG_CMD_PING
#else
    #define CONFIG_CMD_SAVEENV
    #define CONFIG_CMD_NAND

    #undef CONFIG_CMD_NET
    #undef CONFIG_CMD_FLASH
    #undef CONFIG_CMD_IMLS
#endif


#define CONFIG_BOOTDELAY	-1
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

#define CONFIG_SYS_MEMTEST_START	0x9c000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x9c400000	/* 4 ... 8 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR	(CONFIG_SYS_DRAM_BASE + 0x8000) /* default load address */

#define CONFIG_SYS_HZ			1000

/* Monahans Core Frequency */
#define CONFIG_SYS_MONAHANS_RUN_MODE_OSC_RATIO		16 /* valid values: 8, 16, 24, 31 */
#define CONFIG_SYS_MONAHANS_TURBO_RUN_MODE_RATIO	1  /* valid values: 1, 2 */

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

#define CONFIG_SYS_DRAM_BASE		0x80000000 /* at CS0 */
#define CONFIG_SYS_DRAM_SIZE		0x04000000 /* 64 MB Ram */

#undef CONFIG_SYS_SKIP_DRAM_SCRUB

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define	CONFIG_SYS_INIT_SP_ADDR		(GENERATED_GBL_DATA_SIZE + PHYS_SDRAM_1)

/*
 * NAND Flash
 */
#define CONFIG_SYS_NAND0_BASE		0x0
#undef CONFIG_SYS_NAND1_BASE

#define CONFIG_SYS_NAND_BASE_LIST	{ CONFIG_SYS_NAND0_BASE }
#define CONFIG_SYS_MAX_NAND_DEVICE	1	/* Max number of NAND devices */

/* nand timeout values */
#define CONFIG_SYS_NAND_PROG_ERASE_TO	3000
#define CONFIG_SYS_NAND_OTHER_TO	100
#define CONFIG_SYS_NAND_SENDCMD_RETRY	3
#undef NAND_ALLOW_ERASE_ALL	/* Allow erasing bad blocks - don't use */

/* NAND Timing Parameters (in ns) */
#define NAND_TIMING_tCH		10
#define NAND_TIMING_tCS		0
#define NAND_TIMING_tWH		20
#define NAND_TIMING_tWP		40

#define NAND_TIMING_tRH		20
#define NAND_TIMING_tRP		40

#define NAND_TIMING_tR		11123
#define NAND_TIMING_tWHR	100
#define NAND_TIMING_tAR		10

/* NAND debugging */
#define CONFIG_SYS_DFC_DEBUG1 /* usefull */
#undef CONFIG_SYS_DFC_DEBUG2  /* noisy */
#undef CONFIG_SYS_DFC_DEBUG3  /* extremly noisy  */

#define CONFIG_MTD_DEBUG
#define CONFIG_MTD_DEBUG_VERBOSE 1

#define CONFIG_SYS_NO_FLASH		1

#define CONFIG_ENV_IS_IN_NAND	1
#define CONFIG_ENV_OFFSET		0x40000
#define CONFIG_ENV_OFFSET_REDUND	0x44000
#define CONFIG_ENV_SIZE		0x4000


#endif	/* __CONFIG_H */

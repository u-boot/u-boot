/*
 * (C) Copyright 2003
 * MuLogic B.V.
 *
 * (C) Copyright 2002
 * Simple Network Magic Corporation
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* various debug settings */
#undef CFG_DEVICE_NULLDEV		/* null device */
#undef CONFIG_SILENT_CONSOLE		/* silent console */
#undef CFG_CONSOLE_INFO_QUIET		/* silent console ? */
#undef DEBUG				/* debug output code */
#undef DEBUG_FLASH			/* debug flash code */
#undef FLASH_DEBUG			/* debug fash code */
#undef DEBUG_ENV			/* debug environment code */

#define CFG_DIRECT_FLASH_TFTP	1	/* allow direct tftp to flash */
#define CONFIG_ENV_OVERWRITE	1	/* allow overwrite MAC address */


/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC860		1	/* This is a MPC860 CPU */
#define CONFIG_QS860T		1	/* ...on a QS860T module */

#define CONFIG_FEC_ENET		1	/* FEC 10/100BaseT ethernet */
#define CONFIG_MII
#define FEC_INTERRUPT		SIU_LEVEL1
#undef CONFIG_SCC1_ENET			/* SCC1 10BaseT ethernet */
#define CFG_DISCOVER_PHY

#undef CONFIG_8xx_CONS_SMC1
#define CONFIG_8xx_CONS_SMC2	1	/* Console is on SMC */
#undef CONFIG_8xx_CONS_NONE

#define CONFIG_BAUDRATE		38400	/* console baudrate = 38.4kbps */

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

/* Pass clocks to Linux 2.4.18 in Hz */
#undef CONFIG_CLOCKS_IN_MHZ		/* clocks passsed to Linux in MHz */

#define CONFIG_PREBOOT		"echo;" \
	"echo 'Type \"run flash_nfs\" to mount root filesystem over NFS';" \
	"echo"

#undef CONFIG_BOOTARGS
/* TODO compare against CADM860 */
#define CONFIG_BOOTCOMMAND	"bootp; " \
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; " \
	"bootm"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download */
#undef CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change */

#undef CONFIG_WATCHDOG			/* watchdog disabled */

#undef CONFIG_STATUS_LED		/* Status LED disabled */

#undef CONFIG_CAN_DRIVER		/* CAN Driver support disabled */

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

#define CONFIG_RTC_MPC8xx	/* use internal RTC of MPC8xx */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DATE


/* TODO */
#if 0
/* Look at these */
CONFIG_IPADDR
CONFIG_SERVERIP
CONFIG_I2C
CONFIG_SPI
#endif

/*
 * Environment variable storage is in NVRAM
 */
#define CFG_ENV_IS_IN_NVRAM	1
#define CFG_ENV_SIZE		0x00001000	/* We use only the last 4K for PPCBoot */
#define CFG_ENV_ADDR		0xD100E000

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory */
#define CFG_PROMPT		"=> "		/* Monitor Command Prompt */

#define CFG_HUSH_PARSER		1		/* use "hush" command parser */
#define CFG_PROMPT_HUSH_PS2	"> "

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE		256		/* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */

/* TODO - size? */
#define CFG_MEMTEST_START	0x0400000	/* memtest works */
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM */

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000		/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xF0000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2F00		/* End of used area in DPRAM */
#define CFG_GBL_DATA_SIZE	64		/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFF00000

#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor */
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc() */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/* TODO flash parameters */
/*-----------------------------------------------------------------------
 * FLASH organization for Intel Strataflash
 */
#define CFG_FLASH_16BIT		1		/* 16-bit wide flash memory */
#define CFG_MAX_FLASH_BANKS	1		/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	64		/* max number of sectors on one chip */

#define CFG_FLASH_ERASE_TOUT	120000		/* Timeout for Flash Erase (in ms) */
#define CFG_FLASH_WRITE_TOUT	500		/* Timeout for Flash Write (in ms) */

#undef	CFG_ENV_IS_IN_FLASH

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16		/* For all MPC8xx CPUs */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4		/* log base 2 of the above value */
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control 11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(0xFFFFFF88 | SYPCR_SWE | SYPCR_SWRI)
#else
#define CFG_SYPCR	0xFFFFFF88
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration 11-6
 *-----------------------------------------------------------------------
 */
#define CFG_SIUMCR	0x00620000

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control 11-26
 *-----------------------------------------------------------------------
 */
#define CFG_TBSCR	0x00C3

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register 11-27
 *-----------------------------------------------------------------------
 */
#define CFG_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control 11-31
 *-----------------------------------------------------------------------
 */
#define CFG_PISCR	0x0082

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register 15-30
 *-----------------------------------------------------------------------
 */
#define CFG_PLPRCR	0x0090D000

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 */
#define SCCR_MASK	SCCR_EBDF11
#define CFG_SCCR	0x02000000


/*-----------------------------------------------------------------------
 * Debug Enable Register
 * 0x73E67C0F - All interrupts handled by BDM
 * 0x00824001 - Only interrupts needed by MWDebug.exe handled by BDM
 *-----------------------------------------------------------------------
#define CFG_DER			0x73E67C0F
*/
#define CFG_DER			0x0082400F


/*-----------------------------------------------------------------------
 * Memory Controller Initialization Constants
 *-----------------------------------------------------------------------
 */

/*
 * BR0 and OR0 (AMD 512K Socketed FLASH)
 * Base address = 0xFFF0_0000 - 0xFFF7_FFFF (After relocation)
 */
#define CFG_PRELIM_OR_AM
#define CFG_OR_TIMING_FLASH

#define FLASH_BASE0_PRELIM	0xFFF00001
#define CFG_OR0_PRELIM		0xFFF80D42
#define CFG_BR0_PRELIM		0xFFF00401


/*
 * BR1 and OR1 (Intel 8M StrataFLASH)
 * Base address = 0xD000_0000 - 0xD07F_FFFF
 */

#define FLASH_BASE1_PRELIM	0xD0000000
#define CFG_OR1_PRELIM		0xFF800D42
#define CFG_BR1_PRELIM		0xD0000801
/* #define CFG_OR1		0xFF800D42 */
/* #define CFG_BR1		0xD0000801 */


/*
 * BR2 and OR2 (SDRAM)
 * Base Address = 0x00000000 - 0x00FF_FFFF (16M After relocation)
 * Base Address = 0x00000000 - 0x03FF_FFFF (64M After relocation)
 * Base Address = 0x00000000 - 0x07FF_FFFF (128M After relocation)
 *
 */
#define SDRAM_BASE		0x00000000	/* SDRAM bank */
#define SDRAM_PRELIM_OR_AM	0xF8000000	/* map max. 128 MB */

/* SDRAM timing */
#define SDRAM_TIMING		0x00000A00

/* For boards with 16M of SDRAM */
#define SDRAM_16M_MAX_SIZE	0x01000000	/* max 16MB SDRAM */
#define CFG_16M_MBMR		0x18802114	/* Mem Periodic Timer Prescaler */

/* For boards with 64M of SDRAM */
#define SDRAM_64M_MAX_SIZE	0x04000000	/* max 64MB SDRAM */
/* TODO - determine real value */
#define CFG_64M_MBMR		0x18802114	/* Mem Period Timer Prescaler */

#define CFG_OR2			(SDRAM_PRELIM_OR_AM | SDRAM_TIMING)
#define CFG_BR2			(SDRAM_BASE | 0x000000C1)


/*
 * BR3 and OR3 (NVRAM, Sipex, NAND Flash)
 * Base address = 0xD100_0000 - 0xD100_FFFF (64K NVRAM)
 * Base address = 0xD108_0000 - 0xD108_0000 (Sipex chip ctl register)
 * Base address = 0xD110_0000 - 0xD110_0000 (NAND ctl register)
 * Base address = 0xD138_0000 - 0xD138_0000 (LED ctl register)
 *
 */

#define CFG_OR3_PRELIM		0xFFC00DF6
#define CFG_BR3_PRELIM		0xD1000401
/* #define CFG_OR3		0xFFC00DF6 */
/* #define CFG_BR3		0xD1000401 */


/*
 * BR4 and OR4 (Unused)
 * Base address = 0xE000_0000 - 0xE3FF_FFFF
 *
 */

#define CFG_OR4_PRELIM		0xFF000000
#define CFG_BR4_PRELIM		0xE0000000
/* #define CFG_OR4		0xFF000000 */
/* #define CFG_BR4		0xE0000000 */


/*
 * BR5 and OR5 (Expansion bus)
 * Base address = 0xE400_0000 - 0xE7FF_FFFF
 *
 */

#define CFG_OR5_PRELIM		0xFF000000
#define CFG_BR5_PRELIM		0xE4000000
/* #define CFG_OR5		0xFF000000 */
/* #define CFG_BR5		0xE4000000 */


/*
 * BR6 and OR6 (Expansion bus)
 * Base address = 0xE800_0000 - 0xEBFF_FFFF
 *
 */

#define CFG_OR6_PRELIM		0xFF000000
#define CFG_BR6_PRELIM		0xE8000000
/* #define CFG_OR6		0xFF000000 */
/* #define CFG_BR6		0xE8000000 */


/*
 * BR7 and OR7 (Expansion bus)
 * Base address = 0xEC00_0000 - 0xEFFF_FFFF
 *
 */

#define CFG_OR7_PRELIM		0xFF000000
#define CFG_BR7_PRELIM		0xE8000000
/* #define CFG_OR7		0xFF000000 */
/* #define CFG_BR7		0xE8000000 */


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot */

/*
 * Sanity checks
 */
#if defined(CONFIG_SCC1_ENET) && defined(CONFIG_FEC_ENET)
#error Both CONFIG_SCC1_ENET and CONFIG_FEC_ENET configured
#endif

#endif /* __CONFIG_H */

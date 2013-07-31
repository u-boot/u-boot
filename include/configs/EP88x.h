/*
 * Copyright (C) 2005 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Support for Embedded Planet EP88x boards.
 * Tested on EP88xC with MPC885 CPU, 64MB SDRAM and 16MB flash.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */
#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MPC885

#define CONFIG_EP88X				/* Embedded Planet EP88x board	*/

#define	CONFIG_SYS_TEXT_BASE	0xFC000000

#define CONFIG_BOARD_EARLY_INIT_F		/* Call board_early_init_f	*/

/* Allow serial number (serial#) and MAC address (ethaddr) to be overwritten */
#define CONFIG_ENV_OVERWRITE

#define	CONFIG_8xx_CONS_SMC1	1		/* Console is on SMC1		*/
#define CONFIG_BAUDRATE		38400

#define	CONFIG_ETHER_ON_FEC1			/* Enable Ethernet on FEC1	*/
#define	CONFIG_ETHER_ON_FEC2			/* Enable Ethernet on FEC2	*/
#if defined(CONFIG_ETHER_ON_FEC1) || defined(CONFIG_ETHER_ON_FEC2)
#define CONFIG_SYS_DISCOVER_PHY
#define CONFIG_MII_INIT		1
#define FEC_ENET
#endif /* CONFIG_FEC_ENET */

#define CONFIG_8xx_OSCLK		10000000 /* 10 MHz oscillator on EXTCLK */
#define CONFIG_8xx_CPUCLK_DEFAULT	100000000
#define CONFIG_SYS_8xx_CPUCLK_MIN		40000000
#define CONFIG_SYS_8xx_CPUCLK_MAX		133000000

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

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING


#define CONFIG_BOOTDELAY	5		/* Autoboot after 5 seconds	*/
#define CONFIG_BOOTCOMMAND	"bootm fe060000"	/* Autoboot command	*/
#define CONFIG_BOOTARGS		"root=/dev/mtdblock1 rw mtdparts=phys:2M(ROM)ro,-(root)"

#define CONFIG_BZIP2		/* Include support for bzip2 compressed images  */
#undef	CONFIG_WATCHDOG		/* Disable platform specific watchdog		*/

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_PROMPT		"=> "		/* Monitor Command Prompt	*/
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_LONGHELP				/* #undef to save memory	*/
#define CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)  /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* Max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_LOAD_ADDR		0x400000	/* Default load address		*/

#define CONFIG_SYS_HZ			1000		/* Decrementer freq: 1 ms ticks	*/

/*-----------------------------------------------------------------------
 * RAM configuration (note that CONFIG_SYS_SDRAM_BASE must be zero)
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_MAX_SIZE	0x08000000	/* Up to 128 Mbyte		*/

#define CONFIG_SYS_MAMR		0x00805000

/*
 * 4096	Up to 4096 SDRAM rows
 * 1000	factor s -> ms
 * 32	PTP (pre-divider from MPTPR)
 * 4	Number of refresh cycles per period
 * 64	Refresh cycle in ms per number of rows
 */
#define CONFIG_SYS_PTA_PER_CLK		((4096 * 32 * 1000) / (4 * 64))

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x00500000	/* 1 ... 5 MB in SDRAM		*/

#define CONFIG_SYS_RESET_ADDRESS	0x09900000

/*-----------------------------------------------------------------------
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 KB for Monitor   */
#ifdef CONFIG_BZIP2
#define CONFIG_SYS_MALLOC_LEN		(4096 << 10)	/* Reserve ~4 MB for malloc()   */
#else
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 KB for malloc()  */
#endif /* CONFIG_BZIP2 */

/*-----------------------------------------------------------------------
 * Flash organisation
 */
#define CONFIG_SYS_FLASH_BASE		0xFC000000
#define CONFIG_SYS_FLASH_CFI				/* The flash is CFI compatible  */
#define CONFIG_FLASH_CFI_DRIVER			/* Use common CFI driver        */
#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* Max number of flash banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	512		/* Max num of sects on one chip */

/* Environment is in flash */
#define CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000		/* We use one complete sector	*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)

#define CONFIG_SYS_OR0_PRELIM		0xFC000160
#define CONFIG_SYS_BR0_PRELIM		(CONFIG_SYS_FLASH_BASE | BR_PS_32 | BR_MS_GPCM | BR_V)

#define	CONFIG_SYS_DIRECT_FLASH_TFTP

/*-----------------------------------------------------------------------
 * BCSR
 */
#define CONFIG_SYS_OR3_PRELIM		0xFF0005B0
#define CONFIG_SYS_BR3_PRELIM		(0xFA000000 |BR_PS_16 | BR_MS_GPCM | BR_V)

#define CONFIG_SYS_BCSR		0xFA400000

/*-----------------------------------------------------------------------
 * Internal Memory Map Register
 */
#define CONFIG_SYS_IMMR		0xF0000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x2F00		/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Configuration registers
 */
#ifdef CONFIG_WATCHDOG
#define CONFIG_SYS_SYPCR		(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME  | \
				 SYPCR_SWF  | SYPCR_SWE | SYPCR_SWRI | \
				 SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR		(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME  | \
				 SYPCR_SWF  | SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

#define CONFIG_SYS_SIUMCR		(SIUMCR_MLRC01 | SIUMCR_DBGC11)

/* TBSCR - Time Base Status and Control Register */
#define CONFIG_SYS_TBSCR		(TBSCR_TBF | TBSCR_TBE)

/* PISCR - Periodic Interrupt Status and Control */
#define CONFIG_SYS_PISCR		PISCR_PS

/* SCCR - System Clock and reset Control Register */
#define SCCR_MASK		SCCR_EBDF11
#define CONFIG_SYS_SCCR		SCCR_RTSEL

#define CONFIG_SYS_DER			0

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx chips			*/

#endif /* __CONFIG_H */

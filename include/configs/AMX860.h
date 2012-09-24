/*
 * (C) Copyright 2001-2005
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

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC860		1
#define CONFIG_AMX860		1

#define	CONFIG_SYS_TEXT_BASE	0x40000000

#undef	CONFIG_8xx_CONS_SMC1		/* Console is on SCC2		*/
#undef	CONFIG_8xx_CONS_SMC2
#define	CONFIG_8xx_CONS_SCC2	1
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/

#define MPC8XX_FACT		10		/* Multiply by 10	*/
#define MPC8XX_XIN		5000000			/* 5 MHz in	*/
#define MPC8XX_HZ ((MPC8XX_XIN) * (MPC8XX_FACT))

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_BOOTCOMMAND							\
	"bootp;"								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;"	\
	"bootm"				/* autoboot command */

#undef CONFIG_BOOTARGS

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_SCC1_ENET	1	/* use SCC1 ethernet */

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP


#if defined(CONFIG_CMD_KGDB)
#undef	CONFIG_KGDB_ON_SMC		/* define if kgdb on SMC */
#define	CONFIG_KGDB_ON_SCC		/* define if kgdb on SCC */
#undef	CONFIG_KGDB_NONE		/* define if kgdb on something else */
#define CONFIG_KGDB_INDEX	1	/* which serial channel for kgdb */
#define CONFIG_KGDB_BAUDRATE	9600	/* speed to run kgdb serial port at */
#endif


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0200000	/* 1 ... 4 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x00100000

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR			0xFF000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_SIZE	0x2F00	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0x40000000
#if defined(DEBUG)
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#endif
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define	CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * U-Boot for AMX board supports two types of memory extension
 * modules: one that provides 4 MB flash memory, and another one with
 * 16 MB EDO DRAM.
 *
 * The flash module swaps the CS0 and CS1 signals: if the module is
 * installed, CS0 is connected to Flash on the module and CS1 is
 * connected to the on-board Flash. This means that you must intall
 * U-Boot when the Flash module is plugged in, if you plan to use
 * it.
 *
 * To enable support for the DRAM extension card, CONFIG_AMX_RAM_EXT
 * must be defined. The DRAM module uses CS1.
 *
 * Only one of these modules may be installed at a time. If U-Boot
 * is compiled with the CONFIG_AMX_RAM_EXT option set, it will not
 * work if the Flash extension module is installed instead of the
 * DRAM module.
 */
#define CONFIG_AMX_RAM_EXT	/* 16Mb Ext. DRAM module support */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 *
 * Use 4 MB for without and 8 MB with 16 MB DRAM extension module
 * (CONFIG_AMX_RAM_EXT)
 */
#ifdef CONFIG_AMX_RAM_EXT
# define	CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux	*/
#else
# define	CONFIG_SYS_BOOTMAPSZ	(4 << 20)	/* Initial Memory map for Linux	*/
#endif
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	35	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_ENV_OFFSET		0x8000	/*   Offset   of Environment Sector	*/
#define	CONFIG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control					11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration					11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control					11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	15-30
 *-----------------------------------------------------------------------
 * set the PLL, the low-power modes and the reset control (15-29)
 */
#define CONFIG_SYS_PLPRCR	(((MPC8XX_FACT-1) << PLPRCR_MF_SHIFT) |	\
				PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR	(SCCR_TBS|SCCR_COM00|SCCR_DFSYNC00|SCCR_DFBRG00|SCCR_DFNL000|SCCR_DFNH000|SCCR_DFLCD000|SCCR_DFALCD00)

#define CONFIG_SYS_DER		0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/
#ifndef CONFIG_AMX_RAM_EXT
#define FLASH_BASE1_PRELIM	0x60000000	/* FLASH bank #1	*/
#endif

#define CONFIG_SYS_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xFFC00000	/* OR addr mask */

/* FLASH timing: ACS = 10, TRLX = 1, CSNT = 1, SCY = 3, EHTR = 0	*/
/*				 0x00000800	0x00000400 0x00000100 0x00000030     0x00000004 */
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV4 | OR_BI | OR_SCY_5_CLK | OR_TRLX)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)

#define CONFIG_SYS_OR0_PRELIM	0xFFC00954	/* Real values for the board */
#define CONFIG_SYS_BR0_PRELIM	0x40000001	/* Real values for the board */

#ifndef CONFIG_AMX_RAM_EXT
#define CONFIG_SYS_OR1_REMAP	CONFIG_SYS_OR0_REMAP
#define CONFIG_SYS_OR1_PRELIM	0xFFC00954	/* Real values for the board */
#define CONFIG_SYS_BR1_PRELIM	0x60000001	/* Real values for the board */
#endif

/* DSP ("Glue") Xilinx */
#define CONFIG_SYS_OR6_PRELIM	0xFFFF8000	/* 32kB, 15 waits, cs after addr, no bursts */
#define CONFIG_SYS_BR6_PRELIM	0x60000401	/* use GPCM for CS generation, 8 bit port */

#endif	/* __CONFIG_H */

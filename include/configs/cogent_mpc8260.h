/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@cmst.csiro.au>
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
 * Config header file for Cogent platform using an MPC8xx CPU module
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8260		1	/* This is an MPC8260 CPU	*/
#define CONFIG_COGENT		1	/* using Cogent Modular Architecture */
#define CONFIG_CPM2		1	/* Has a CPM2 */

#define	CONFIG_MISC_INIT_F	1	/* Use misc_init_f()		*/

/* Cogent Modular Architecture options */
#define CONFIG_CMA282		1	/* ...on a CMA282 CPU module	*/
#define CONFIG_CMA111		1	/* ...on a CMA111 motherboard	*/

/*
 * select serial console configuration
 *
 * if either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 *
 * if CONFIG_CONS_NONE is defined, then the serial console routines must
 * defined elsewhere (for example, on the cogent platform, there are serial
 * ports on the motherboard which are used for the serial console - see
 * cogent/cma101/serial.[ch]).
 */
#define	CONFIG_CONS_ON_SMC		/* define if console on SMC */
#undef	CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef	CONFIG_CONS_NONE		/* define if console on something else*/
#define CONFIG_CONS_INDEX	1	/* which serial channel for console */
#undef	CONFIG_CONS_USE_EXTC		/* SMC/SCC use ext clock not brg_clk */
#define	CONFIG_CONS_EXTC_RATE	3686400	/* SMC/SCC ext clk rate in Hz */
#define	CONFIG_CONS_EXTC_PINSEL	0	/* pin select 0=CLK3/CLK9,1=CLK5/CLK15*/

/*
 * select ethernet configuration
 *
 * if either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected, then
 * CONFIG_ETHER_INDEX must be set to the channel number (1-4 for SCC, 1-3
 * for FCC)
 *
 * if CONFIG_ETHER_NONE is defined, then either the ethernet routines must be
 * defined elsewhere (as for the console), or CFG_CMD_NET must be removed
 * from CONFIG_COMMANDS to remove support for networking.
 */
#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC	*/
#undef	CONFIG_ETHER_ON_FCC		/* define if ether on FCC	*/
#define	CONFIG_ETHER_NONE		/* define if ether on something else */
#define CONFIG_ETHER_INDEX	1	/* which channel for ether	*/

/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#define CONFIG_8260_CLKIN	66666666	/* in Hz */

#if defined(CONFIG_CONS_NONE) || defined(CONFIG_CONS_USE_EXTC)
#define CONFIG_BAUDRATE		230400
#else
#define CONFIG_BAUDRATE		9600
#endif


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_KGDB

#undef CONFIG_CMD_NET


#ifdef DEBUG
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif
#define CONFIG_BOOTCOMMAND	"bootm 04080000 04200000" /* autoboot command*/

#define CONFIG_BOOTARGS		"root=/dev/ram rw"

#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_KGDB_ON_SMC		/* define if kgdb on SMC */
#undef	CONFIG_KGDB_ON_SCC		/* define if kgdb on SCC */
#undef	CONFIG_KGDB_NONE		/* define if kgdb on something else */
#define CONFIG_KGDB_INDEX	2	/* which serial channel for kgdb */
#define	CONFIG_KGDB_USE_EXTC		/* SMC/SCC use ext clock not brg_clk */
#define	CONFIG_KGDB_EXTC_RATE	3686400	/* serial ext clk rate in Hz */
#define	CONFIG_KGDB_EXTC_PINSEL	0	/* pin select 0=CLK3/CLK9,1=CLK5/CLK15*/
# if defined(CONFIG_KGDB_NONE) || defined(CONFIG_KGDB_USE_EXTC)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port at */
# else
#define CONFIG_KGDB_BAUDRATE	9600	/* speed to run kgdb serial port at */
# endif
#endif

#undef	CONFIG_WATCHDOG			/* disable platform specific watchdog */

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x01c00000	/* 4 ... 28 MB in DRAM	*/

#define	CFG_LOAD_ADDR		0x100000	/* default load address	*/

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Low Level Cogent settings
 * if CFG_CMA_CONS_SERIAL is defined, make sure the 8260 CPM serial is not.
 * also, make sure CONFIG_CONS_INDEX is still defined - the index will be
 * 1 for serialA, 2 for serialB, 3 for ser2A, 4 for ser2B
 * (second 2 for CMA120 only)
 */
#define CFG_CMA_MB_BASE		0x00000000	/* base of m/b address space */

#include <configs/cogent_common.h>

#ifdef CONFIG_CONS_NONE
#define CFG_CMA_CONS_SERIAL	/* use Cogent motherboard serial for console */
#endif
#define CFG_CMA_LCD_HEARTBEAT	/* define for sec rotator in lcd corner */
#define CONFIG_SHOW_ACTIVITY

#if (CMA_MB_CAPS & CMA_MB_CAP_FLASH)
/*
 * flash exists on the motherboard
 * set these four according to TOP dipsw:
 * TOP on  => ..._FLLOW_...	(boot EPROM space is high so FLASH is low )
 * TOP off => ..._FLHIGH_...	(boot EPROM space is low  so FLASH is high)
 */
#define CMA_MB_FLASH_EXEC_BASE	CMA_MB_FLLOW_EXEC_BASE
#define CMA_MB_FLASH_EXEC_SIZE	CMA_MB_FLLOW_EXEC_SIZE
#define CMA_MB_FLASH_RDWR_BASE	CMA_MB_FLLOW_RDWR_BASE
#define CMA_MB_FLASH_RDWR_SIZE	CMA_MB_FLLOW_RDWR_SIZE
#endif
#define CMA_MB_FLASH_BASE	CMA_MB_FLASH_EXEC_BASE
#define CMA_MB_FLASH_SIZE	CMA_MB_FLASH_EXEC_SIZE

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * if you change bits in the HRCW, you must also change the CFG_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CFG_SIUMCR.
 */
#define CFG_HRCW_MASTER	(HRCW_EBM|HRCW_BPS10|HRCW_L2CPC10|HRCW_DPPC11|\
			 HRCW_ISB100|HRCW_MMR11|HRCW_MODCK_H0101)
/* no slaves so just duplicate the master hrcw */
#define CFG_HRCW_SLAVE1	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE2	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE3	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE4	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE5	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE6	CFG_HRCW_MASTER
#define CFG_HRCW_SLAVE7	CFG_HRCW_MASTER

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xF0000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define	CFG_INIT_RAM_END	0x4000	/* End of used area in DPRAM	*/
#define	CFG_GBL_DATA_SIZE	128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define	CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define	CFG_SDRAM_BASE		CMA_MB_RAM_BASE
#ifdef CONFIG_CMA302
#define CFG_FLASH_BASE		CMA_MB_SLOT2_BASE	/* cma302 in slot 2 */
#else
#define CFG_FLASH_BASE		CMA_MB_FLASH_BASE	/* flash on m/b */
#endif
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define	CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Mem map for Linux*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max num of memory banks	*/
#define CFG_MAX_FLASH_SECT	67	/* max num of sects on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Flash Erase Timeout (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		CFG_FLASH_BASE /* Addr of Environment Sector */
#ifdef CONFIG_CMA302
#define	CFG_ENV_SIZE		0x1000	/* Total Size of Environment Sector */
#define CFG_ENV_SECT_SIZE	(512*1024) /* see README - env sect real size */
#else
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector */
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPU		*/
#if defined(CONFIG_CMD_KGDB)
# define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value*/
#endif

/*-----------------------------------------------------------------------
 * HIDx - Hardware Implementation-dependent Registers			 2-11
 *-----------------------------------------------------------------------
 * HID0 also contains cache control - initially enable both caches and
 * invalidate contents, then the final state leaves only the instruction
 * cache enabled. Note that Power-On and Hard reset invalidate the caches,
 * but Soft reset does not.
 *
 * HID1 has only read-only information - nothing to set.
 */
#define CFG_HID0_INIT	(HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI|\
				HID0_IFEM|HID0_ABE)
#define CFG_HID0_FINAL	(HID0_ICE|HID0_IFEM|HID0_ABE)
#define CFG_HID2	0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register					 5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#define CFG_RMR		RMR_CSRE

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration					 4-25
 *-----------------------------------------------------------------------
 */
#define CFG_BCR		BCR_EBM

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				 4-31
 *-----------------------------------------------------------------------
 */
#define CFG_SIUMCR	(SIUMCR_DPPC11|SIUMCR_L2CPC10|SIUMCR_MMR11)

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				 4-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x Bus Monitor enable
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP|SYPCR_SWE)
#else
#define CFG_SYPCR	(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control			 4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CFG_TMCNTSC	(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CFG_PISCR	(PISCR_PS|PISCR_PTF|PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control					 9-8
 *-----------------------------------------------------------------------
 * Ensure DFBRG is Divide by 16
 */
#define CFG_SCCR	(SCCR_DFBRG01)

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration				13-7
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR	0

#if defined(CONFIG_CMA282)

/*
 * Init Memory Controller:
 *
 * According to the Cogent manual, only CS0 and CS2 are used - CS0 for EPROM
 * and CS2 for (optional) local bus RAM on the CPU module.
 *
 * Note the motherboard address space (256 Mbyte in size) is connected
 * to the 60x Bus and is located starting at address 0. The Hard Reset
 * Configuration Word should put the 60x Bus into External Bus Mode, since
 * we dont set up any memory controller maps for it (see BCR[EBM], 4-26).
 *
 * (the *_SIZE vars must be a power of 2)
 */

#define CFG_CMA_CS0_BASE	TEXT_BASE	/* EPROM */
#define CFG_CMA_CS0_SIZE	(1 << 20)
#if 0
#define CFG_CMA_CS2_BASE	0x10000000	/* Local Bus SDRAM */
#define CFG_CMA_CS2_SIZE	(16 << 20)
#endif

/*
 * CS0 maps the EPROM on the cpu module
 * Set it for 10 wait states, address CFG_MONITOR_BASE and size 1M
 *
 * Note: We must have already transferred control to the final location
 * of the EPROM before these are used, because when BR0/OR0 are set, the
 * mirror of the eprom at any other addresses will disappear.
 */

/* base address = CFG_CMA_CS0_BASE, 16-bit, no parity, r/o, gpcm (60x bus) */
#define CFG_BR0_PRELIM	((CFG_CMA_CS0_BASE&BRx_BA_MSK)|BRx_PS_16|BRx_WP|BRx_V)
/* mask size CFG_CMA_CS0_SIZE, csneg 1/4 early, adr-to-cs 1/2, 10-wait states */
#define CFG_OR0_PRELIM	(P2SZ_TO_AM(CFG_CMA_CS0_SIZE)|\
				ORxG_CSNT|ORxG_ACS_DIV2|ORxG_SCY_10_CLK)

/*
 * CS2 enables the Local Bus SDRAM on the CPU Module
 *
 * Will leave this unset for the moment, because a) my CPU module has no
 * SDRAM installed (it is optional); and b) it will require programming
 * one of the UPMs in SDRAM mode - not a trivial job, and hard to get right
 * if you can't test it.
 */

#if 0
/* base address = CFG_CMA_CS2_BASE, 32-bit, no parity, ??? */
#define CFG_BR0_PRELIM	((CFG_CMA_CS2_BASE&BRx_BA_MSK)|BRx_PS_32|/*???*/|BRx_V)
/* mask size CFG_CMA_CS2_SIZE, CS time normal, ??? */
#define CFG_OR2_PRELIM	((~(CFG_CMA_CS2_SIZE-1)&ORx_AM_MSK)|/*???*/)
#endif

#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH*/
#define BOOTFLAG_WARM	0x02		/* Software reboot		*/

#endif	/* __CONFIG_H */

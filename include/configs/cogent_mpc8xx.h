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

#define CONFIG_MPC860		1	/* This is an MPC860 CPU	*/
#define CONFIG_COGENT		1	/* using Cogent Modular Architecture */

#define	CONFIG_MISC_INIT_F	1	/* Use misc_init_f()		*/

/* Cogent Modular Architecture options */
#define CONFIG_CMA286_60_OLD	1	/* ...on an old CMA286-60 CPU module */
#define CONFIG_CMA102		1	/* ...on a CMA102 motherboard	*/
#define CONFIG_CMA302		1	/* ...with a CMA302 flash I/O module */

/* serial console configuration */
#undef	CONFIG_8xx_CONS_SMC1
#undef	CONFIG_8xx_CONS_SMC2
#define CONFIG_8xx_CONS_NONE	/* not on 8xx serial ports (eg on cogent m/b) */

#if defined(CONFIG_CMA286_60_OLD)
#define CONFIG_8xx_GCLK_FREQ	33333000 /* define if cant use get_gclk_freq */
#endif

#define CONFIG_BAUDRATE		230400

#define CONFIG_HARD_I2C		/* I2C with hardware support */
#define CFG_I2C_SPEED		400000  /* I2C speed and slave address */
#define CFG_I2C_SLAVE		0x7F


#define CONFIG_COMMANDS		((CONFIG_CMD_DFL | CFG_CMD_KGDB | CFG_CMD_I2C) & ~CFG_CMD_NET)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif
#define CONFIG_BOOTCOMMAND	"bootm 04080000 04200000" /* autoboot command*/

#define CONFIG_BOOTARGS		"root=/dev/ram rw"

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#undef	CONFIG_KGDB_ON_SMC		/* define if kgdb on SMC */
#undef	CONFIG_KGDB_ON_SCC		/* define if kgdb on SCC */
#define	CONFIG_KGDB_NONE		/* define if kgdb on something else */
#define CONFIG_KGDB_INDEX	2	/* which SMC/SCC channel for kgdb */
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#endif

#define CONFIG_WATCHDOG			/* turn on platform specific watchdog */

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
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

#define CFG_ALLOC_DPRAM

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Low Level Cogent settings
 * if CFG_CMA_CONS_SERIAL is defined, make sure the 8xx CPM serial is not.
 * also, make sure CONFIG_CONS_INDEX is still defined - the index will be
 * 1 for serialA, 2 for serialB, 3 for ser2A, 4 for ser2B
 * (second 2 for CMA120 only)
 */
#define CFG_CMA_MB_BASE		0x00000000	/* base of m/b address space */

#include <configs/cogent_common.h>

#define CFG_CMA_CONS_SERIAL	/* use Cogent motherboard serial for console */
#define CONFIG_CONS_INDEX	1
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
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xFF000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define	CFG_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define	CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
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
#define	CFG_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor	*/
#define	CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	67	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_ADDR		CFG_FLASH_BASE /* Addr of Environment Sector */
#ifdef CONFIG_CMA302
#define	CFG_ENV_SIZE		0x1000	/* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	(512*1024) /* see README - env sect real size */
#else
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/
#endif
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif


/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif	/* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit - leave PLL multiplication factor unchanged !
 */
#define CFG_PLPRCR	(PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CFG_SCCR	(SCCR_TBS     | SCCR_RTDIV    | SCCR_RTSEL    | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CFG_PCMCIA_MEM_ADDR	(0xE0000000)
#define CFG_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CFG_PCMCIA_DMA_ADDR	(0xE4000000)
#define CFG_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CFG_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CFG_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CFG_PCMCIA_IO_ADDR	(0xEC000000)
#define CFG_PCMCIA_IO_SIZE	( 64 << 20 )

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/*#define	CFG_DER	0x2002000F*/
#define CFG_DER	0

#if defined(CONFIG_CMA286_60_OLD)

/*
 * Init Memory Controller:
 *
 * NOTE: although the names (CFG_xRn_PRELIM) suggest preliminary settings,
 * they are actually the final settings for this cpu/board, because the
 * flash and RAM are on the motherboard, accessed via the CMAbus, and the
 * mappings are pretty much fixed.
 *
 * (the *_SIZE vars must be a power of 2)
 */

#define CFG_CMA_CS0_BASE	TEXT_BASE		/* EPROM */
#define CFG_CMA_CS0_SIZE	(1 << 20)
#define CFG_CMA_CS1_BASE	CMA_MB_RAM_BASE		/* RAM + I/O SLOT 1 */
#define CFG_CMA_CS1_SIZE	(64 << 20)
#define CFG_CMA_CS2_BASE	CMA_MB_SLOT2_BASE	/* I/O SLOTS 2 + 3 */
#define CFG_CMA_CS2_SIZE	(64 << 20)
#define CFG_CMA_CS3_BASE	CMA_MB_ROMLOW_BASE	/* M/B I/O */
#define CFG_CMA_CS3_SIZE	(32 << 20)

/*
 * CS0 maps the EPROM on the cpu module
 * Set it for 4 wait states, address CFG_MONITOR_BASE and size 1M
 *
 * Note: We must have already transferred control to the final location
 * of the EPROM before these are used, because when BR0/OR0 are set, the
 * mirror of the eprom at any other addresses will disappear.
 */

/* base address = CFG_CMA_CS0_BASE, 16-bit, no parity, r/o, gpcm */
#define CFG_BR0_PRELIM	((CFG_CMA_CS0_BASE&BR_BA_MSK)|BR_PS_16|BR_WP|BR_V)
/* mask size CFG_CMA_CS0_SIZE, CS time normal, burst inhibit, 4-wait states */
#define CFG_OR0_PRELIM	((~(CFG_CMA_CS0_SIZE-1)&OR_AM_MSK)|OR_BI|OR_SCY_4_CLK)

/*
 * CS1 maps motherboard DRAM and motherboard I/O slot 1
 * (each 32Mbyte in size)
 */

/* base address = CFG_CMA_CS1_BASE, 32-bit, no parity, r/w, gpcm */
#define CFG_BR1_PRELIM	((CFG_CMA_CS1_BASE&BR_BA_MSK)|BR_V)
/* mask size CFG_CMA_CS1_SIZE, CS time normal, burst ok, ext xfer ack */
#define CFG_OR1_PRELIM	((~(CFG_CMA_CS1_SIZE-1)&OR_AM_MSK)|OR_SETA)

/*
 * CS2 maps motherboard I/O slots 2 and 3
 * (each 32Mbyte in size)
 */

/* base address = CFG_CMA_CS2_BASE, 32-bit, no parity, r/w, gpcm */
#define CFG_BR2_PRELIM	((CFG_CMA_CS2_BASE&BR_BA_MSK)|BR_V)
/* mask size CFG_CMA_CS2_SIZE, CS time normal, burst ok, ext xfer ack */
#define CFG_OR2_PRELIM	((~(CFG_CMA_CS2_SIZE-1)&OR_AM_MSK)|OR_SETA)

/*
 * CS3 maps motherboard I/O
 * (32Mbyte in size)
 */

/* base address = CFG_CMA_CS3_BASE, 32-bit, no parity, r/w, gpcm */
#define CFG_BR3_PRELIM	((CFG_CMA_CS3_BASE&BR_BA_MSK)|BR_V)
/* mask size CFG_CMA_CS3_SIZE, CS time normal, burst inhibit, ext xfer ack */
#define CFG_OR3_PRELIM	((~(CFG_CMA_CS3_SIZE-1)&OR_AM_MSK)|OR_BI|OR_SETA)

#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */

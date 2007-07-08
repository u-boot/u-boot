/*
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

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC850		1	/* This is a MPC850 CPU		*/
#define CONFIG_ESTEEM192E	1	/* ...on a EST ESTEEM192E	*/

#define CONFIG_FLASH_16BIT	1	/* Rom 16 bit data bus		*/

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef  CONFIG_8xx_CONS_NONE

#define MPC8XX_FACT	10		/* Multiply by 10		*/
#define MPC8XX_XIN	4915200	/* 4.915200 MHz in	- ??? - XXX	*/
#define CFG_PLPRCR_MF	((MPC8XX_FACT-1) << 20)
#define MPC8XX_HZ ((MPC8XX_XIN) * (MPC8XX_FACT)) /* 49,152,000 Hz	*/

#define CONFIG_8xx_GCLK_FREQ	MPC8XX_HZ	/* Force it - dont measure it */

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#define CONFIG_BAUDRATE		9600
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif
#define CONFIG_BOOTCOMMAND	"bootm 40030000" /* autoboot command	*/

#define CONFIG_BOOTARGS		"root=/dev/ram rw ramdisk=8192 "			\
				"ip=100.100.100.21:100.100.100.14:100.100.100.1:255.0.0.0 "
/*
 * Miscellaneous configurable options
 */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>


#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"BOOT: "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE	256			/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS	8			/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CFG_LOAD_ADDR		0x100000	/* default load address	*/

#define	CFG_HZ			1000		/* decrementer freq: 1 ms ticks	*/


#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }


/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
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
#define	CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0x40000000
#ifdef	DEBUG
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CFG_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor	*/
#endif
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
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
#define CFG_MAX_FLASH_SECT	71	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CFG_ENV_IS_IN_FLASH	1
#define	CFG_ENV_OFFSET		0x8000	/*   Offset   of Environment Sector	*/
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)

/*-----------------------------------------------------------------------
 * SUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01) /* DBGC00 */

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF | TBSCR_TBE)

/* (TBSCR_REFA | TBSCR_REFB | TBSCR_TBF) */


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
#define CFG_PLPRCR	(CFG_PLPRCR_MF | PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CFG_SCCR	(SCCR_TBS     | \
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

#define CFG_PCMCIA_INTERRUPT	SIU_LEVEL6

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/*#define	CFG_DER	0x2002000F*/
#define CFG_DER	0
/*#define CFG_DER	0x02002000 */


/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0x60000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CFG_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/* FLASH timing: ACS = 11, TRLX = 0, CSNT = 1, SCY = 5, EHTR = 1	*/
#define CFG_OR_TIMING_FLASH	0x00000160
				/*(OR_CSNT_SAM  | OR_ACS_DIV2 | OR_BI | \
				 OR_SCY_5_CLK | OR_EHTR) */

#define CFG_OR0_REMAP	0x80000160     /*(CFG_REMAP_OR_AM  | CFG_OR_TIMING_FLASH)*/
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	( FLASH_BASE0_PRELIM | 0x00000801 )

#define CFG_OR1_REMAP	CFG_OR0_REMAP
#define CFG_OR1_PRELIM	CFG_OR0_PRELIM
#define CFG_BR1_PRELIM	( FLASH_BASE1_PRELIM | 0x00000801 )

/*
 * BR2/3 and OR2/3 (SDRAM)
 *
 */
#define SDRAM_BASE2_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define SDRAM_BASE3_PRELIM	0x04000000	/* SDRAM bank #1	*/
#define	SDRAM_MAX_SIZE		0x02000000	/* max 32 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CFG_OR_TIMING_SDRAM	0x00000A00

#define CFG_OR2_PRELIM	0xFC000E00
#define CFG_BR2_PRELIM	(SDRAM_BASE2_PRELIM | 0x00000081)

#define	CFG_OR3_PRELIM	CFG_OR2_PRELIM
#define CFG_BR3_PRELIM	(SDRAM_BASE3_PRELIM | 0x00000081)


/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CFG_MAMR_PTA	97		/* start with divider for 100 MHz	*/

/* refresh rate 15.6 us (= 64 ms / 4K = 62.4 / quad bursts) for <= 128 MBit	*/
#define CFG_MPTPR_2BK_4K	MPTPR_PTP_DIV16		/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_4K	MPTPR_PTP_DIV32		/* setting for 1 bank	*/

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit		*/
#define CFG_MPTPR_2BK_8K	MPTPR_PTP_DIV8		/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_8K	MPTPR_PTP_DIV16		/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

/* 8 column SDRAM */
#define CFG_MAMR_8COL	0x18803112
#define CFG_MAMR_9COL	0x18803112	/* same as 8 column because its just easier to port with*/


/*
 * Internal Definitions
 *
 * Boot Flags
 */

#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */

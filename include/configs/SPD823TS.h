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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#define CONFIG_MPC823		1	/* This is a MPC823 CPU		*/
#define CONFIG_SPD823TS		1	/* ...on a SPD823TS board	*/

#define CONFIG_RESET_PHY_R	1	/* Call reset_phy()		*/

#define CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		115200
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#define CONFIG_BOOTCOMMAND	"bootp" /* autoboot command		*/

#define CONFIG_BOOTARGS		"root=/dev/nfs rw "			\
				"nfsroot=10.0.0.2:/opt/eldk/ppc_8xx "	\
				"nfsaddrs=10.0.0.99:10.0.0.2"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_IDE

#undef CONFIG_CMD_SAVEENV
#undef CONFIG_CMD_FLASH


#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


/*----------------------------------------------------------------------*/
#define CONFIG_ETHADDR		00:D0:93:00:01:CB
#define CONFIG_IPADDR		10.0.0.98
#define CONFIG_SERVERIP		10.0.0.1
#undef	CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND	"tftp 200000 uImage;bootm 200000"
/*----------------------------------------------------------------------*/

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x00F00000	/* 1 ... 15MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x00100000	/* default load address */

#define CONFIG_SYS_PIO_MODE		0	/* IDE interface in PIO Mode 0	*/

#define CONFIG_SYS_PC_IDE_RESET	((ushort)0x0008)	/* PC 12	*/

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFFF00000 /* was: 0xFF000000 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFF000000
#ifdef	DEBUG
#define CONFIG_SYS_MONITOR_LEN		(512 << 10)	/* Reserve 512 kB for Monitor	*/
#else
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#endif
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	0	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	0	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	0	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	0	/* Timeout for Flash Write (in ms)	*/

#define	CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET		0x8000	/*   Offset   of Environment Sector	*/
#define CONFIG_ENV_SIZE		0x0800	/* Total Size of Environment Sector	*/
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
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
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
/* 0x00000040 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC00 | SIUMCR_GB5E)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit, set PLL multiplication factor !
 */
/* 0x00b0c0c0 */
#define CONFIG_SYS_PLPRCR							\
		(	(11 << PLPRCR_MF_SHIFT) |			\
			PLPRCR_SPLSS | PLPRCR_TEXPS | /*PLPRCR_TMIST|*/ \
			/*PLPRCR_CSRC|*/ PLPRCR_LPM_NORMAL |		\
			PLPRCR_CSR   | PLPRCR_LOLRE /*|PLPRCR_FIOPD*/	\
		)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
/* 0x01800014 */
#define CONFIG_SYS_SCCR	(SCCR_COM00	| /*SCCR_TBS|*/		\
			 SCCR_RTDIV	|   SCCR_RTSEL	  |	\
			 /*SCCR_CRQEN|*/  /*SCCR_PRQEN|*/	\
			 SCCR_EBDF00	|   SCCR_DFSYNC00 |	\
			 SCCR_DFBRG00	|   SCCR_DFNL000  |	\
			 SCCR_DFNH000	|   SCCR_DFLCD101 |	\
			 SCCR_DFALCD00)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register
 *-----------------------------------------------------------------------
 */
/* 0x00C3 */
#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)


/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration Register
 *-----------------------------------------------------------------------
 */
/* TIMEP=2 */
#define CONFIG_SYS_RCCR 0x0200

/*-----------------------------------------------------------------------
 * RMDS - RISC Microcode Development Support Control Register
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RMDS 0

/*-----------------------------------------------------------------------
 * SDSR - SDMA Status Register
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_SDSR ((u_char)0x83)

/*-----------------------------------------------------------------------
 * SDMR - SDMA Mask Register
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_SDMR ((u_char)0x00)

/*-----------------------------------------------------------------------
 *
 * Interrupt Levels
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_CPM_INTERRUPT	13	/* SIU_LEVEL6	*/

/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_PCMCIA_MEM_ADDR	(0xE0000000)
#define CONFIG_SYS_PCMCIA_MEM_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_DMA_ADDR	(0xE4000000)
#define CONFIG_SYS_PCMCIA_DMA_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_ATTRB_ADDR	(0xE8000000)
#define CONFIG_SYS_PCMCIA_ATTRB_SIZE	( 64 << 20 )
#define CONFIG_SYS_PCMCIA_IO_ADDR	(0xEC000000)
#define CONFIG_SYS_PCMCIA_IO_SIZE	( 64 << 20 )

/*-----------------------------------------------------------------------
 * IDE/ATA stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_IDE_8xx_DIRECT	1	/* PCMCIA interface required	*/
#define CONFIG_IDE_LED		1	/* LED   for ide supported	*/
#define CONFIG_IDE_RESET	1	/* reset for ide supported	*/

#define CONFIG_SYS_IDE_MAXBUS		2	/* max. 2 IDE busses		*/
#define CONFIG_SYS_IDE_MAXDEVICE	(CONFIG_SYS_IDE_MAXBUS*2) /* max. 2 drives per IDE bus */

#define CONFIG_SYS_ATA_BASE_ADDR	0xFE100000
#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000
#define CONFIG_SYS_ATA_IDE1_OFFSET	0x0C00

#define CONFIG_SYS_ATA_DATA_OFFSET	0x0000	/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_REG_OFFSET	0x0080	/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	0x0100	/* Offset for alternate registers	*/

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_DER 0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0xFF000000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0xFF080000	/* FLASH bank #1	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
/* EPROMs are 512kb */
#define CONFIG_SYS_REMAP_OR_AM		0xFFF80000	/* OR addr mask */
#define CONFIG_SYS_PRELIM_OR_AM	0xFFF80000	/* OR addr mask */

/* FLASH timing: ACS = 11, TRLX = 0, CSNT = 1, SCY = 5, EHTR = 1	*/
#define CONFIG_SYS_OR_TIMING_FLASH	(/* OR_CSNT_SAM | */ OR_ACS_DIV4 | OR_BI | \
				 OR_SCY_5_CLK | OR_EHTR)

#define CONFIG_SYS_OR0_REMAP	(CONFIG_SYS_REMAP_OR_AM  | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_FLASH)
/* 16 bit, bank valid */
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_16 | BR_V )

#define CONFIG_SYS_OR1_REMAP	CONFIG_SYS_OR0_REMAP
#define CONFIG_SYS_OR1_PRELIM	CONFIG_SYS_OR0_PRELIM
/* 16 bit, bank valid */
#define CONFIG_SYS_BR1_PRELIM	((FLASH_BASE1_PRELIM & BR_BA_MSK) | BR_PS_16 | BR_V )

/*
 * BR2-5 and OR2-5 (SRAM/SDRAM/PER8/SHARC)
 *
 */
#define SRAM_BASE	0xFE200000	/* SRAM bank */
#define SRAM_OR_AM	0xFFE00000	/* SRAM is 2 MB */

#define SDRAM_BASE3_PRELIM	0x00000000	/* SDRAM bank */
#define SDRAM_PRELIM_OR_AM	0xF8000000	/* map max. 128 MB */
#define SDRAM_MAX_SIZE		0x04000000	/* max 64 MB SDRAM */

#define PER8_BASE	0xFE000000	/* PER8 bank */
#define PER8_OR_AM	0xFFF00000	/* PER8 is 1 MB */

#define SHARC_BASE	0xFE400000	/* SHARC bank */
#define SHARC_OR_AM	0xFFC00000	/* SHARC is 4 MB */

/* SRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/

#define CONFIG_SYS_OR_TIMING_SRAM	0x00000D42	/* SRAM-Timing */
#define CONFIG_SYS_OR2 (SRAM_OR_AM | CONFIG_SYS_OR_TIMING_SRAM )
#define CONFIG_SYS_BR2 ((SRAM_BASE & BR_BA_MSK) | BR_PS_16 | BR_V )

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/

#define CONFIG_SYS_OR_TIMING_SDRAM	0x00000A00	/* SDRAM-Timing */
#define CONFIG_SYS_OR3_PRELIM	(SDRAM_PRELIM_OR_AM | CONFIG_SYS_OR_TIMING_SDRAM )
#define CONFIG_SYS_BR3_PRELIM	((SDRAM_BASE3_PRELIM & BR_BA_MSK) | BR_MS_UPMB | BR_V )

#define CONFIG_SYS_OR_TIMING_PER8	0x00000F32	/* PER8-Timing */
#define CONFIG_SYS_OR4 (PER8_OR_AM | CONFIG_SYS_OR_TIMING_PER8 )
#define CONFIG_SYS_BR4 ((PER8_BASE & BR_BA_MSK) | BR_PS_8 | BR_V )

#define CONFIG_SYS_OR_TIMING_SHARC	0x00000700	/* SHARC-Timing */
#define CONFIG_SYS_OR5 (SHARC_OR_AM | CONFIG_SYS_OR_TIMING_SHARC )
#define CONFIG_SYS_BR5 ((SHARC_BASE & BR_BA_MSK) | BR_PS_32 | BR_MS_UPMA | BR_V )
/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MBMR_PTB	204

/* refresh rate 15.6 us (= 64 ms / 4K = 62.4 / quad bursts) for <= 128 MBit	*/
#define CONFIG_SYS_MPTPR_2BK_4K	MPTPR_PTP_DIV16		/* setting for 2 banks	*/
#define CONFIG_SYS_MPTPR_1BK_4K	MPTPR_PTP_DIV32		/* setting for 1 bank	*/

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit		*/
#define CONFIG_SYS_MPTPR_2BK_8K	MPTPR_PTP_DIV8		/* setting for 2 banks	*/
#define CONFIG_SYS_MPTPR_1BK_8K	MPTPR_PTP_DIV16		/* setting for 1 bank	*/

/*
 * MBMR settings for SDRAM
 */

/* 8 column SDRAM */
#define CONFIG_SYS_MBMR_8COL	((CONFIG_SYS_MBMR_PTB << MBMR_PTB_SHIFT)  | \
			 MBMR_AMB_TYPE_0 | MBMR_DSB_1_CYCL | MBMR_G0CLB_A11 |	\
			 MBMR_RLFB_1X	 | MBMR_WLFB_1X	   | MBMR_TLFB_4X)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */

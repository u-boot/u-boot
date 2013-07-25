/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
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
#define CONFIG_FLAGADM		1	/* ...on a FLAGA DM	*/
#define CONFIG_8xx_GCLK_FREQ 48000000	/*48MHz*/

#define	CONFIG_SYS_TEXT_BASE	0x40000000

#undef	CONFIG_8xx_CONS_SMC1		/* Console is on SMC1		*/
#define CONFIG_8xx_CONS_SMC2	1
#undef	CONFIG_8xx_CONS_NONE

#define CONFIG_BAUDRATE		115200	/* console baudrate = 115kbps	*/
#define CONFIG_BOOTDELAY	3	/* autoboot after 5 seconds	*/

#undef	CONFIG_CLOCKS_IN_MHZ

#if 0
#define CONFIG_BOOTARGS		"root=/dev/nfs rw ip=bootp"
#define CONFIG_BOOTCOMMAND							\
   "setenv bootargs root=/dev/ram ip=off panic=1;"     \
   "bootm 40040000 400e0000"
#else
#define CONFIG_BOOTARGS		"root=/dev/nfs rw ip=bootp panic=1"
#define CONFIG_BOOTCOMMAND	"bootp 0x400000; bootm 0x400000"
#endif /* 0|1*/

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CONFIG_SYS_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

/*#define	CONFIG_WATCHDOG*/	/* watchdog enabled		*/
#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


/*
 * Command line configuration.
 */

#define CONFIG_CMD_BDI
#define CONFIG_CMD_IMI
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_NET


/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"EEG> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x40040000	/* default load address */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xFF000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x2F00	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0x40000000
#define CONFIG_SYS_MONITOR_LEN		(128 << 10)	/* Reserve 128 kB for Monitor	*/
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
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	71	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	8000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_ENV_IS_IN_FLASH	1
/* This is a litlebit wasteful, but one sector is 128kb and we have to
 * assigne a whole sector for the environment, so that we can safely
 * erase and write it without disturbing the boot sector
 */
#define CONFIG_ENV_OFFSET		0x20000 /*   Offset   of Environment Sector	*/
#define CONFIG_ENV_SIZE		0x20000 /* Total Size of Environment Sector	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif
#define CONFIG_SYS_DELAYED_ICACHE	1	/* enable ICache not before
						 * running in RAM.
						 */

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#ifdef CONFIG_WATCHDOG
#define CONFIG_SYS_SYPCR (SYPCR_SWTC | SYPCR_SWF | SYPCR_SWE | SYPCR_SWRI | SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CONFIG_SYS_PRE_SIUMCR (SIUMCR_DBGC11 | SIUMCR_MPRE | \
							SIUMCR_MLRC01 | SIUMCR_GB5E)
#define CONFIG_SYS_SIUMCR (CONFIG_SYS_PRE_SIUMCR | SIUMCR_DLK)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)

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
 * interrupt status bit miltiplier of 0x00b i.e. operation clock is
 * 4MHz * (0x00b+1) = 4MHz * 12 =  48MHz
 */
#define CONFIG_SYS_PLPRCR	(0x00b00000 | PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CONFIG_SYS_SCCR	( SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

#define CONFIG_SYS_DER 0

/*
 * In the Flaga DM we have:
 * Flash on BR0/OR0/CS0a at 0x40000000
 * Display on BR1/OR1/CS1 at 0x20000000
 * SDRAM on BR2/OR2/CS2 at 0x00000000
 * Free BR3/OR3/CS3
 * DSP1 on BR4/OR4/CS4 at 0x80000000
 * DSP2 on BR5/OR5/CS5 at 0xa0000000
 *
 * For now we just configure the Flash and the SDRAM and leave the others
 * untouched.
*/

#define CONFIG_SYS_FLASH_PROTECTION 0

#define FLASH_BASE0		0x40000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CONFIG_SYS_OR_AM		0xff000000	/* OR addr mask */
#define CONFIG_SYS_OR_ATM		0x00006000

/* FLASH timing: ACS = 10, TRLX = 1, CSNT = 1, SCY = 3, EHTR = 1	*/
#define CONFIG_SYS_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV4 | OR_BI | \
				 OR_SCY_3_CLK | OR_TRLX | OR_EHTR )

#define CONFIG_SYS_OR0_PRELIM	(CONFIG_SYS_OR_AM | CONFIG_SYS_OR_ATM | CONFIG_SYS_OR_TIMING_FLASH)
#define CONFIG_SYS_BR0_PRELIM	((FLASH_BASE0 & BR_BA_MSK) | BR_PS_16 | BR_V )

/*
 * BR2 and OR2 (SDRAM)
 *
 */
#define SDRAM_BASE2			0x00000000	/* SDRAM bank #0	*/
#define SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CONFIG_SYS_OR_TIMING_SDRAM	( 0x00000800 )

#define CONFIG_SYS_OR2_PRELIM	(CONFIG_SYS_OR_AM | CONFIG_SYS_OR_TIMING_SDRAM)
#define CONFIG_SYS_BR2_PRELIM	((SDRAM_BASE2 & BR_BA_MSK) | BR_MS_UPMA | BR_V )

#define CONFIG_SYS_BR2			CONFIG_SYS_BR2_PRELIM
#define CONFIG_SYS_OR2			CONFIG_SYS_OR2_PRELIM

/*
 * MAMR settings for SDRAM
 */
#define CONFIG_SYS_MAMR_48_SDR (CONFIG_SYS_MAMR_PTA |	 MAMR_WLFA_1X | MAMR_RLFA_1X  \
					| MAMR_G0CLA_A11)

/*
 * Memory Periodic Timer Prescaler
 */

/* periodic timer for refresh */
#define CONFIG_SYS_MAMR_PTA	0x0F000000

/*
   * BR4 and OR4 (DSP1)
   *
   * We do not wan't preliminary setup of the DSP, anyway we need the
   * UPMB setup correctly before we can access the DSP.
   *
*/
#define DSP_BASE 0x80000000

#define CONFIG_SYS_OR4 ( OR_AM_MSK | OR_CSNT_SAM | OR_BI | OR_G5LS)
#define CONFIG_SYS_BR4 ( (DSP_BASE & BR_BA_MSK) | BR_PS_16 | BR_MS_UPMB | BR_V )

#endif	/* __CONFIG_H */

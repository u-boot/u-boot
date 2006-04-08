/*
 * (C) Copyright 2000-2004
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
 * Pantelis Antoniou, Intracom S.A., panto@intracom.gr
 * U-Boot port on NetTA4 board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC885		1	/* This is a MPC885 CPU		*/
#define CONFIG_NETTA		1	/* ...on a NetTA board		*/

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE

#define CONFIG_BAUDRATE		115200	/* console baudrate = 115kbps	*/

/* #define CONFIG_XIN		 10000000 */
#define CONFIG_XIN		 50000000
#define MPC8XX_HZ		120000000
/* #define MPC8XX_HZ		100000000 */
/* #define MPC8XX_HZ		 50000000 */
/* #define MPC8XX_HZ		 80000000 */

#define CONFIG_8xx_GCLK_FREQ	MPC8XX_HZ

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#undef	CONFIG_CLOCKS_IN_MHZ	/* clocks NOT passsed to Linux in MHz */

#define CONFIG_PREBOOT	"echo;echo Type \"run flash_nfs\" to mount root filesystem over NFS;echo"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"tftpboot; " 								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;"	\
	"bootm"

#define CONFIG_LOADS_ECHO	0	/* echo off for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/
#define CONFIG_HW_WATCHDOG

#undef	CONFIG_CAN_DRIVER		/* CAN Driver support disabled	*/

#define CONFIG_BOOTP_MASK		(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE | CONFIG_BOOTP_NISDOMAIN)

#undef CONFIG_MAC_PARTITION
#undef CONFIG_DOS_PARTITION

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/

#define	CONFIG_NET_MULTI	1 	/* the only way to get the FEC in */
#define	FEC_ENET		1	/* eth.c needs it that way... */
#undef  CFG_DISCOVER_PHY		/* do not discover phys */
#define CONFIG_MII		1
#define CONFIG_RMII		1	/* use RMII interface */

#if defined(CONFIG_NETTA_ISDN)
#define CONFIG_ETHER_ON_FEC1	1
#define CONFIG_FEC1_PHY		1   	/* phy address of FEC1 */
#define CONFIG_FEC1_PHY_NORXERR 1
#undef  CONFIG_ETHER_ON_FEC2
#else
#define CONFIG_ETHER_ON_FEC1	1
#define CONFIG_FEC1_PHY		8  	/* phy address of FEC1 */
#define CONFIG_FEC1_PHY_NORXERR 1
#define CONFIG_ETHER_ON_FEC2	1
#define CONFIG_FEC2_PHY		1   	/* phy address of FEC2 */
#define CONFIG_FEC2_PHY_NORXERR 1
#endif

#define CONFIG_ENV_OVERWRITE	1	/* allow modification of vendor params */

/* POST support */
#define CONFIG_POST		(CFG_POST_MEMORY   | \
				 CFG_POST_CODEC	   | \
				 CFG_POST_DSP	   )

#define CONFIG_COMMANDS       ( CONFIG_CMD_DFL	| \
				CFG_CMD_CDP	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_DIAG    | \
				CFG_CMD_FAT	| \
				CFG_CMD_IDE	| \
				CFG_CMD_JFFS2	| \
				CFG_CMD_MII 	| \
				CFG_CMD_NAND	| \
				CFG_CMD_NFS	| \
				CFG_CMD_PCMCIA	| \
				CFG_CMD_PING  	| \
				0)

#define CONFIG_BOARD_EARLY_INIT_F	1
#define CONFIG_MISC_INIT_R

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/

#define CFG_HUSH_PARSER	1
#define CFG_PROMPT_HUSH_PS2	"> "

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0300000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0700000	/* 3 ... 7 MB in DRAM	*/

#define	CFG_LOAD_ADDR		0x100000	/* default load address	*/

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

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
#define	CFG_INIT_RAM_END	0x3000	/* End of used area in DPRAM	*/
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
#if defined(DEBUG)
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
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
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	8	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SECT_SIZE	0x10000

#define	CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x60000)
#define CFG_ENV_OFFSET		0
#define	CFG_ENV_SIZE		0x4000

#define CFG_ENV_ADDR_REDUND	(CFG_FLASH_BASE + 0x70000)
#define CFG_ENV_OFFSET_REDUND	0
#define CFG_ENV_SIZE_REDUND	CFG_ENV_SIZE

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
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#ifndef	CONFIG_CAN_DRIVER
#define CFG_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01 | SIUMCR_FRC)
#else	/* we must activate GPL5 in the SIUMCR for CAN */
#define CFG_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DBPC00 | SIUMCR_MLRC01 | SIUMCR_FRC)
#endif	/* CONFIG_CAN_DRIVER */

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
#define CFG_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)

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
 * interrupt status bit
 *
 */

#if CONFIG_XIN == 10000000

#if MPC8XX_HZ == 120000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (0 << PLPRCR_S_SHIFT) | (12 << PLPRCR_MFI_SHIFT) | (0 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#elif MPC8XX_HZ == 100000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (0 << PLPRCR_S_SHIFT) | (10 << PLPRCR_MFI_SHIFT) | (0 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#elif MPC8XX_HZ == 50000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (1 << PLPRCR_S_SHIFT) | (8 << PLPRCR_MFI_SHIFT) | (3 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#elif MPC8XX_HZ == 25000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (2 << PLPRCR_S_SHIFT) | (8 << PLPRCR_MFI_SHIFT) | (3 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#elif MPC8XX_HZ == 40000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (1 << PLPRCR_S_SHIFT) | (8 << PLPRCR_MFI_SHIFT) | (4 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#elif MPC8XX_HZ == 75000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (1 << PLPRCR_S_SHIFT) | (15 << PLPRCR_MFI_SHIFT) | (0 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#else
#error unsupported CPU freq for XIN = 10MHz
#endif

#elif CONFIG_XIN == 50000000

#if MPC8XX_HZ == 120000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (0 << PLPRCR_S_SHIFT) | (12 << PLPRCR_MFI_SHIFT) | (4 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#elif MPC8XX_HZ == 100000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (0 << PLPRCR_S_SHIFT) | (6 << PLPRCR_MFI_SHIFT) | (2 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#elif MPC8XX_HZ ==  80000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (0 << PLPRCR_S_SHIFT) | (8 << PLPRCR_MFI_SHIFT) | (4 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#elif MPC8XX_HZ ==  50000000
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (1 << PLPRCR_S_SHIFT) | (6 << PLPRCR_MFI_SHIFT) | (2 << PLPRCR_PDF_SHIFT) | \
		 	 PLPRCR_TEXPS)
#else
#error unsupported CPU freq for XIN = 50MHz
#endif

#else

#error unsupported XIN freq
#endif


/*
 *-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 *
 * Note: When TBS == 0 the timebase is independent of current cpu clock.
 */

#define SCCR_MASK	SCCR_EBDF11
#if MPC8XX_HZ > 66666666
#define CFG_SCCR	(/* SCCR_TBS	| */ SCCR_CRQEN | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL111 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00 | SCCR_EBDF01)
#else
#define CFG_SCCR	(/* SCCR_TBS	| */ SCCR_CRQEN | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL111 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)
#endif

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/*#define	CFG_DER	0x2002000F*/
#define CFG_DER	0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x40000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CFG_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/* FLASH timing: ACS = 11, TRLX = 0, CSNT = 1, SCY = 5, EHTR = 1	*/
#define CFG_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_BI | OR_SCY_5_CLK | OR_TRLX)

#define CFG_OR0_REMAP	(CFG_REMAP_OR_AM  | CFG_OR_TIMING_FLASH)
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_8 | BR_V )

/*
 * BR3 and OR3 (SDRAM)
 *
 */
#define SDRAM_BASE3_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define	SDRAM_MAX_SIZE		(256 << 20)	/* max 256MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CFG_OR_TIMING_SDRAM	(OR_CSNT_SAM | OR_G5LS)

#define CFG_OR3_PRELIM	((0xFFFFFFFFLU & ~(SDRAM_MAX_SIZE - 1)) | CFG_OR_TIMING_SDRAM)
#define CFG_BR3_PRELIM	((SDRAM_BASE3_PRELIM & BR_BA_MSK) | BR_MS_UPMB | BR_PS_32 | BR_V)

/*
 * Memory Periodic Timer Prescaler
 */

/*
 * Memory Periodic Timer Prescaler
 *
 * The Divider for PTA (refresh timer) configuration is based on an
 * example SDRAM configuration (64 MBit, one bank). The adjustment to
 * the number of chip selects (NCS) and the actually needed refresh
 * rate is done by setting MPTPR.
 *
 * PTA is calculated from
 *	PTA = (gclk * Trefresh) / ((2 ^ (2 * DFBRG)) * PTP * NCS)
 *
 *	gclk	  CPU clock (not bus clock!)
 *	Trefresh  Refresh cycle * 4 (four word bursts used)
 *
 * 4096  Rows from SDRAM example configuration
 * 1000  factor s -> ms
 *   32  PTP (pre-divider from MPTPR) from SDRAM example configuration
 *    4  Number of refresh cycles per period
 *   64  Refresh cycle in ms per number of rows
 * --------------------------------------------
 * Divider = 4096 * 32 * 1000 / (4 * 64) = 512000
 *
 * 50 MHz => 50.000.000 / Divider =  98
 * 66 Mhz => 66.000.000 / Divider = 129
 * 80 Mhz => 80.000.000 / Divider = 156
 */

#if   MPC8XX_HZ == 120000000
#define CFG_MAMR_PTA		 234
#elif MPC8XX_HZ == 100000000
#define CFG_MAMR_PTA		 195
#elif MPC8XX_HZ ==  80000000
#define CFG_MAMR_PTA		 156
#elif MPC8XX_HZ ==  50000000
#define CFG_MAMR_PTA		  98
#else
#error Unknown frequency
#endif


/*
 * For 16 MBit, refresh rates could be 31.3 us
 * (= 64 ms / 2K = 125 / quad bursts).
 * For a simpler initialization, 15.6 us is used instead.
 *
 * #define CFG_MPTPR_2BK_2K	MPTPR_PTP_DIV32		for 2 banks
 * #define CFG_MPTPR_1BK_2K	MPTPR_PTP_DIV64		for 1 bank
 */
#define CFG_MPTPR_2BK_4K	MPTPR_PTP_DIV16		/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_4K	MPTPR_PTP_DIV32		/* setting for 1 bank	*/

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit		*/
#define CFG_MPTPR_2BK_8K	MPTPR_PTP_DIV8		/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_8K	MPTPR_PTP_DIV16		/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

/* 8 column SDRAM */
#define CFG_MAMR_8COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_0 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A11 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)

/* 9 column SDRAM */
#define CFG_MAMR_9COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#define CONFIG_ARTOS			/* include ARTOS support */

#define CONFIG_LAST_STAGE_INIT		/* needed to reset the damn phys */

/***********************************************************************************************************

   Pin definitions:

 +------+----------------+--------+------------------------------------------------------------
 |  #   | Name           | Type   | Comment
 +------+----------------+--------+------------------------------------------------------------
 | PA3  | OK_ETH_3V      | Input  | CISCO Ethernet power OK
 |      |                |        | (NetRoute: FEC1, TA: FEC2) (0=power OK)
 | PA6  | P_VCCD1        | Output | TPS2211A PCMCIA
 | PA7  | DCL1_3V        | Periph | IDL1 PCM clock
 | PA8  | DSP_DR1        | Periph | IDL1 PCM Data Rx
 | PA9  | L1TXDA         | Periph | IDL1 PCM Data Tx
 | PA10 | P_VCCD0        | Output | TPS2211A PCMCIA
 | PA12 | P_SHDN         | Output | TPS2211A PCMCIA
 | PA13 | ETH_LOOP       | Output | CISCO Loopback remote power
 |      |                |        | (NetRoute: FEC1, TA: FEC2) (1=NORMAL)
 | PA14 | P_VPPD0        | Output | TPS2211A PCMCIA
 | PA15 | P_VPPD1        | Output | TPS2211A PCMCIA
 | PB14 | SPIEN_FXO      | Output | SPI CS for FXO daughter-board
 | PB15 | SPIEN_S1       | Output | SPI CS for S-interface 1 (NetRoute only)
 | PB16 | DREQ1          | Output | D channel request for S-interface chip 1.
 | PB17 | L1ST3          | Periph | IDL1 timeslot enable signal for PPC
 | PB18 | L1ST2          | Periph | IDL1 timeslot enable signal for PPC
 | PB19 | SPIEN_S2       | Output | SPI CS for S-interface 2 (NetRoute only)
 | PB20 | SPIEN_SEEPROM  | Output | SPI CS for serial eeprom
 | PB21 | LEDIO          | Output | Led mode indication for PHY
 | PB22 | UART_CTS       | Input  | UART CTS
 | PB23 | UART_RTS       | Output | UART RTS
 | PB24 | UART_RX        | Periph | UART Data Rx
 | PB25 | UART_TX        | Periph | UART Data Tx
 | PB26 | RMII-MDC       | Periph | Free for future use (MII mgt clock)
 | PB27 | RMII-MDIO      | Periph | Free for future use (MII mgt data)
 | PB28 | SPI_RXD_3V     | Input  | SPI Data Rx
 | PB29 | SPI_TXD        | Output | SPI Data Tx
 | PB30 | SPI_CLK        | Output | SPI Clock
 | PB31 | RMII1-REFCLK   | Periph | RMII reference clock for FEC1
 | PC4  | PHY1_LINK      | Input  | PHY link state FEC1 (interrupt)
 | PC5  | PHY2_LINK      | Input  | PHY link state FEC2 (interrupt)
 | PC6  | RMII1-MDINT    | Input  | PHY prog interrupt FEC1 (interrupt)
 | PC7  | RMII2-MDINT    | Input  | PHY prog interrupt FEC1 (interrupt)
 | PC8  | P_OC           | Input  | TPS2211A PCMCIA overcurrent (interrupt) (1=OK)
 | PC9  | COM_HOOK1      | Input  | Codec interrupt chip #1 (interrupt)
 | PC10 | COM_HOOK2      | Input  | Codec interrupt chip #2 (interrupt)
 | PC11 | COM_HOOK4      | Input  | Codec interrupt chip #4 (interrupt)
 | PC12 | COM_HOOK3      | Input  | Codec interrupt chip #3 (interrupt)
 | PC13 | F_RY_BY        | Input  | NAND ready signal (interrupt)
 | PC14 | FAN_OK         | Input  | Fan status signal (interrupt) (1=OK)
 | PC15 | PC15_DIRECT0   | Periph | PCMCIA DMA request.
 | PD3  | F_ALE          | Output | NAND
 | PD4  | F_CLE          | Output | NAND
 | PD5  | F_CE           | Output | NAND
 | PD6  | DSP_INT        | Output | DSP debug interrupt
 | PD7  | DSP_RESET      | Output | DSP reset
 | PD8  | RMII_MDC       | Periph | MII mgt clock
 | PD9  | SPIEN_C1       | Output | SPI CS for codec #1
 | PD10 | SPIEN_C2       | Output | SPI CS for codec #2
 | PD11 | SPIEN_C3       | Output | SPI CS for codec #3
 | PD12 | FSC2           | Periph | IDL2 frame sync
 | PD13 | DGRANT2        | Input  | D channel grant from S #2
 | PD14 | SPIEN_C4       | Output | SPI CS for codec #4
 | PD15 | TP700          | Output | Testpoint for software debugging
 | PE14 | RMII2-TXD0     | Periph | FEC2 transmit data
 | PE15 | RMII2-TXD1     | Periph | FEC2 transmit data
 | PE16 | RMII2-REFCLK   | Periph | TA: RMII ref clock for
 |      | DCL2           | Periph | NetRoute: PCM clock #2
 | PE17 | TP703          | Output | Testpoint for software debugging
 | PE18 | DGRANT1        | Input  |  D channel grant from S #1
 | PE19 | RMII2-TXEN     | Periph | TA: FEC2 tx enable
 |      | PCM2OUT        | Periph | NetRoute: Tx data for IDL2
 | PE20 | FSC1           | Periph | IDL1 frame sync
 | PE21 | RMII2-RXD0     | Periph | FEC2 receive data
 | PE22 | RMII2-RXD1     | Periph | FEC2 receive data
 | PE23 | L1ST1          | Periph | IDL1 timeslot enable signal for PPC
 | PE24 | U-N1           | Output | Select user/network for S #1 (0=user)
 | PE25 | U-N2           | Output | Select user/network for S #2 (0=user)
 | PE26 | RMII2-RXDV     | Periph | FEC2 valid
 | PE27 | DREQ2          | Output | D channel request for S #2.
 | PE28 | FPGA_DONE      | Input  | FPGA done signal
 | PE29 | FPGA_INIT      | Output | FPGA init signal
 | PE30 | UDOUT2_3V      | Input  | IDL2 PCM input
 | PE31 |                |        | Free
 +------+----------------+--------+---------------------------------------------------

 Chip selects:

 +------+----------------+------------------------------------------------------------
 |  #   | Name           | Comment
 +------+----------------+------------------------------------------------------------
 | CS0  | CS0            | Boot flash
 | CS1  | CS_FLASH       | NAND flash
 | CS2  | CS_DSP         | DSP
 | CS3  | DCS_DRAM       | DRAM
 | CS4  | CS_ER1         | External output register
 +------+----------------+------------------------------------------------------------

 Interrupts:

 +------+----------------+------------------------------------------------------------
 |  #   | Name           | Comment
 +------+----------------+------------------------------------------------------------
 | IRQ1 | UINTER_3V      | S interupt chips interrupt (common)
 | IRQ3 | IRQ_DSP        | DSP interrupt
 | IRQ4 | IRQ_DSP1       | Extra DSP interrupt
 +------+----------------+------------------------------------------------------------

*************************************************************************************************/

#define DSP_SIZE	0x00010000	/* 64K */
#define NAND_SIZE	0x00010000	/* 64K */
#define ER_SIZE		0x00010000	/* 64K */
#define DUMMY_SIZE	0x00010000	/* 64K */

#define DSP_BASE	0xF1000000
#define NAND_BASE	0xF1010000
#define ER_BASE		0xF1020000
#define DUMMY_BASE	0xF1FF0000

/****************************************************************/

/* NAND */
#define CFG_NAND_LEGACY
#define CFG_NAND_BASE			NAND_BASE
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_MTD_NAND_UNSAFE

#define CFG_MAX_NAND_DEVICE		1
/* #define NAND_NO_RB */

#define SECTORSIZE		512
#define ADDR_COLUMN		1
#define ADDR_PAGE		2
#define ADDR_COLUMN_PAGE	3
#define NAND_ChipID_UNKNOWN 	0x00
#define NAND_MAX_FLOORS		1
#define NAND_MAX_CHIPS		1

/* ALE = PD3, CLE = PD4, CE = PD5, F_RY_BY = PC13 */
#define NAND_DISABLE_CE(nand) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) |=  (1 << (15 - 5)); \
	} while(0)

#define NAND_ENABLE_CE(nand) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) &= ~(1 << (15 - 5)); \
	} while(0)

#define NAND_CTL_CLRALE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) &= ~(1 << (15 - 3)); \
	} while(0)

#define NAND_CTL_SETALE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) |=  (1 << (15 - 3)); \
	} while(0)

#define NAND_CTL_CLRCLE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) &= ~(1 << (15 - 4)); \
	} while(0)

#define NAND_CTL_SETCLE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pddat) |=  (1 << (15 - 4)); \
	} while(0)

#ifndef NAND_NO_RB
#define NAND_WAIT_READY(nand) \
	do { \
		while ((((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pcdat & (1 << (15 - 13))) == 0) { \
			WATCHDOG_RESET(); \
		} \
	} while (0)
#else
#define NAND_WAIT_READY(nand) udelay(12)
#endif

#define WRITE_NAND_COMMAND(d, adr) \
	do { \
		*(volatile unsigned char *)((unsigned long)(adr)) = (unsigned char)(d); \
	} while(0)

#define WRITE_NAND_ADDRESS(d, adr) \
	do { \
		*(volatile unsigned char *)((unsigned long)(adr)) = (unsigned char)(d); \
	} while(0)

#define WRITE_NAND(d, adr) \
	do { \
		*(volatile unsigned char *)((unsigned long)(adr)) = (unsigned char)(d); \
	} while(0)

#define READ_NAND(adr) \
	((unsigned char)(*(volatile unsigned char *)(unsigned long)(adr)))

#define CONFIG_JFFS2_NAND	1			/* jffs2 on nand support */
#define NAND_CACHE_PAGES	16			/* size of nand cache in 512 bytes pages */

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nand0"
#define CONFIG_JFFS2_PART_SIZE		0x00100000
#define CONFIG_JFFS2_PART_OFFSET	0x00200000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nand0=netta-nand"
#define MTDPARTS_DEFAULT	"mtdparts=netta-nand:1m@2m(jffs2)"
*/

/*****************************************************************************/

#define CFG_DIRECT_FLASH_TFTP
#define CFG_DIRECT_NAND_TFTP

/*****************************************************************************/

#if 1
/*-----------------------------------------------------------------------
 * PCMCIA stuff
 *-----------------------------------------------------------------------
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
 * IDE/ATA stuff (Supports IDE harddisk on PCMCIA Adapter)
 *-----------------------------------------------------------------------
 */

#define	CONFIG_IDE_8xx_PCCARD	1	/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/
#undef	CONFIG_IDE_RESET		/* reset for ide not supported	*/

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1	/* max. 1 drive per IDE bus	*/

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR	CFG_PCMCIA_MEM_ADDR

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	(CFG_PCMCIA_MEM_SIZE + 0x320)

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	(2 * CFG_PCMCIA_MEM_SIZE + 0x320)

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	0x0100

#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#endif

/*************************************************************************************************/

#define CONFIG_CDP_DEVICE_ID		20
#define CONFIG_CDP_DEVICE_ID_PREFIX	"NT"	/* netta */
#define CONFIG_CDP_PORT_ID		"eth%d"
#define CONFIG_CDP_CAPABILITIES		0x00000010
#define CONFIG_CDP_VERSION		"u-boot 1.0" " " __DATE__ " " __TIME__
#define CONFIG_CDP_PLATFORM		"Intracom NetTA"
#define CONFIG_CDP_TRIGGER		0x20020001
#define CONFIG_CDP_POWER_CONSUMPTION	4300	/* 90 mA @ 48V */
#define CONFIG_CDP_APPLIANCE_VLAN_TYPE	0x01	/* ipphone? */

/*************************************************************************************************/

#define CONFIG_AUTO_COMPLETE	1

/*************************************************************************************************/

#define CONFIG_CRC32_VERIFY	1

/*************************************************************************************************/

#define CONFIG_HUSH_OLD_PARSER_COMPATIBLE	1

/*************************************************************************************************/

#endif	/* __CONFIG_H */

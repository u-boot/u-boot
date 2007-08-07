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

#if !defined(CONFIG_NETTA2_VERSION) || CONFIG_NETTA2_VERSION > 2
#error Unsupported CONFIG_NETTA2 version
#endif

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC870		1	/* This is a MPC885 CPU		*/
#define CONFIG_NETTA2		1	/* ...on a NetTA2 board		*/

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE

#define CONFIG_BAUDRATE		115200	/* console baudrate = 115kbps	*/

/* #define CONFIG_XIN		 10000000 */
#define CONFIG_XIN		 50000000
/* #define MPC8XX_HZ		120000000 */
#define MPC8XX_HZ		 66666666

#define CONFIG_8xx_GCLK_FREQ	MPC8XX_HZ

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#undef	CONFIG_CLOCKS_IN_MHZ	/* clocks NOT passsed to Linux in MHz */

#define CONFIG_PREBOOT	"echo;"

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"tftpboot; " 								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " 	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; " 	\
	"bootm"

#define CONFIG_AUTOSCRIPT
#define CONFIG_LOADS_ECHO	0	/* echo off for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#undef	CONFIG_CAN_DRIVER		/* CAN Driver support disabled	*/

#define	CONFIG_STATUS_LED	1	/* Status LED enabled		*/
#define CONFIG_BOARD_SPECIFIC_LED	/* version has board specific leds */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_NISDOMAIN


#undef CONFIG_MAC_PARTITION
#undef CONFIG_DOS_PARTITION

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/

#define	CONFIG_NET_MULTI	1 	/* the only way to get the FEC in */
#define	FEC_ENET		1	/* eth.c needs it that way... */
#undef CFG_DISCOVER_PHY
#define CONFIG_MII		1
#define CONFIG_RMII		1	/* use RMII interface */

#define CONFIG_ETHER_ON_FEC1	1
#define CONFIG_FEC1_PHY		8 	/* phy address of FEC */
#define CONFIG_FEC1_PHY_NORXERR 1

#define CONFIG_ETHER_ON_FEC2	1
#define CONFIG_FEC2_PHY		4
#define CONFIG_FEC2_PHY_NORXERR 1

#define CONFIG_ENV_OVERWRITE	1	/* allow modification of vendor params */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_NAND
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_CMD_MII
#define CONFIG_CMD_CDP


#define CONFIG_BOARD_EARLY_INIT_F	1
#define CONFIG_MISC_INIT_R

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/

#define CFG_HUSH_PARSER	1
#define CFG_PROMPT_HUSH_PS2	"> "

#if defined(CONFIG_CMD_KGDB)
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
#if CONFIG_NETTA2_VERSION == 2
#define CFG_FLASH_BASE4		0x40080000
#endif

#define CFG_RESET_ADDRESS   0x80000000

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#if CONFIG_NETTA2_VERSION == 1
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#elif CONFIG_NETTA2_VERSION == 2
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#endif
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
#if defined(CONFIG_CMD_KGDB)
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
#elif MPC8XX_HZ ==  66666666
#define CFG_PLPRCR	((0 << PLPRCR_MFN_SHIFT) | (0 << PLPRCR_MFD_SHIFT) | \
			 (1 << PLPRCR_S_SHIFT) | (8 << PLPRCR_MFI_SHIFT) | (2 << PLPRCR_PDF_SHIFT) | \
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
#define CFG_SCCR	(/* SCCR_TBS     | */ SCCR_CRQEN | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00 | SCCR_EBDF01)
#else
#define CFG_SCCR	(/* SCCR_TBS     | */ SCCR_CRQEN | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
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

#if CONFIG_NETTA2_VERSION == 2

#define FLASH_BASE4_PRELIM	0x40080000	/* FLASH bank #1	*/

#define CFG_OR4_REMAP	(CFG_REMAP_OR_AM  | CFG_OR_TIMING_FLASH)
#define CFG_OR4_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR4_PRELIM	((FLASH_BASE4_PRELIM & BR_BA_MSK) | BR_PS_8 | BR_V )

#endif

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

#define CFG_MAMR_PTA		 234

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

/****************************************************************/

#define DSP_SIZE	0x00010000	/* 64K */
#define NAND_SIZE	0x00010000	/* 64K */

#define DSP_BASE	0xF1000000
#define NAND_BASE	0xF1010000

/****************************************************************/

/* NAND */
#define CFG_NAND_LEGACY
#define CFG_NAND_BASE		NAND_BASE
#define CONFIG_MTD_NAND_ECC_JFFS2
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CONFIG_MTD_NAND_UNSAFE

#define CFG_MAX_NAND_DEVICE	1

#define SECTORSIZE		512
#define ADDR_COLUMN		1
#define ADDR_PAGE		2
#define ADDR_COLUMN_PAGE	3
#define NAND_ChipID_UNKNOWN 	0x00
#define NAND_MAX_FLOORS		1
#define NAND_MAX_CHIPS		1

/* ALE = PD17, CLE = PE18, CE = PE20, F_RY_BY = PE31 */
#define NAND_DISABLE_CE(nand) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat) |=  (1 << (31 - 20)); \
	} while(0)

#define NAND_ENABLE_CE(nand) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat) &= ~(1 << (31 - 20)); \
	} while(0)

#define NAND_CTL_CLRALE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat) &= ~(1 << (31 - 17)); \
	} while(0)

#define NAND_CTL_SETALE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat) |=  (1 << (31 - 17)); \
	} while(0)

#define NAND_CTL_CLRCLE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat) &= ~(1 << (31 - 18)); \
	} while(0)

#define NAND_CTL_SETCLE(nandptr) \
	do { \
		(((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat) |=  (1 << (31 - 18)); \
	} while(0)

#if CONFIG_NETTA2_VERSION == 1
#define NAND_WAIT_READY(nand) \
	do { \
		int _tries = 0; \
		while ((((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat & (1 << (31 - 31))) == 0) \
			if (++_tries > 100000) \
				break; \
	} while (0)
#elif CONFIG_NETTA2_VERSION == 2
#define NAND_WAIT_READY(nand) \
	do { \
		int _tries = 0; \
		while ((((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pcdat & (1 << (15 - 15))) == 0) \
			if (++_tries > 100000) \
				break; \
	} while (0)
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

/*****************************************************************************/

#define CFG_DIRECT_FLASH_TFTP
#define CFG_DIRECT_NAND_TFTP

/*****************************************************************************/

#if CONFIG_NETTA2_VERSION == 1
#define STATUS_LED_BIT		0x00000008		/* bit 28 */
#elif CONFIG_NETTA2_VERSION == 2
#define STATUS_LED_BIT		0x00000080		/* bit 24 */
#endif

#define STATUS_LED_PERIOD	(CFG_HZ / 2)
#define STATUS_LED_STATE	STATUS_LED_BLINKING

#define STATUS_LED_ACTIVE	0		/* LED on for bit == 0	*/
#define STATUS_LED_BOOT		0		/* LED 0 used for boot status */

#ifndef __ASSEMBLY__

/* LEDs */

/* led_id_t is unsigned int mask */
typedef unsigned int led_id_t;

#define __led_toggle(_msk) \
	do { \
		((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat ^= (_msk); \
	} while(0)

#define __led_set(_msk, _st) \
	do { \
		if ((_st)) \
			((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat |= (_msk); \
		else \
			((volatile immap_t *)CFG_IMMR)->im_cpm.cp_pedat &= ~(_msk); \
	} while(0)

#define __led_init(msk, st) __led_set(msk, st)

#endif

/***********************************************************************************************************

 ----------------------------------------------------------------------------------------------

   (V1) version 1 of the board
   (V2) version 2 of the board

 ----------------------------------------------------------------------------------------------

   Pin definitions:

 +------+----------------+--------+------------------------------------------------------------
 |  #   | Name           | Type   | Comment
 +------+----------------+--------+------------------------------------------------------------
 | PA3  | SPIEN_MAX      | Output | MAX serial to uart chip select
 | PA7  | DSP_INT        | Output | DSP interrupt
 | PA10 | DSP_RESET      | Output | DSP reset
 | PA14 | USBOE          | Output | USB (1)
 | PA15 | USBRXD         | Output | USB (1)
 | PB19 | BT_RTS         | Output | Bluetooth (0)
 | PB23 | BT_CTS         | Output | Bluetooth (0)
 | PB26 | SPIEN_SEP      | Output | Serial EEPROM chip select
 | PB27 | SPICS_DISP     | Output | Display chip select
 | PB28 | SPI_RXD_3V     | Input  | SPI Data Rx
 | PB29 | SPI_TXD        | Output | SPI Data Tx
 | PB30 | SPI_CLK        | Output | SPI Clock
 | PC10 | DISPA0         | Output | Display A0
 | PC11 | BACKLIGHT      | Output | Display backlit
 | PC12 | SPI2RXD        | Input  | (V1) 2nd SPI RXD
 |      | IO_RESET       | Output | (V2) General I/O reset
 | PC13 | SPI2TXD        | Output | (V1) 2nd SPI TXD (V1)
 |      | HOOK           | Input  | (V2) Hook input interrupt
 | PC15 | SPI2CLK        | Output | (V1) 2nd SPI CLK
 |      | F_RY_BY        | Input  | (V2) NAND F_RY_BY
 | PE17 | F_ALE          | Output | NAND F_ALE
 | PE18 | F_CLE          | Output | NAND F_CLE
 | PE20 | F_CE           | Output | NAND F_CE
 | PE24 | SPICS_SCOUT    | Output | (V1) Codec chip select
 |      | LED            | Output | (V2) LED
 | PE27 | SPICS_ER       | Output | External serial register CS
 | PE28 | LEDIO1         | Output | (V1) LED
 |      | BKBR1          | Input  | (V2) Keyboard input scan
 | PE29 | LEDIO2         | Output | (V1) LED hook for A (TA2)
 |      | BKBR2          | Input  | (V2) Keyboard input scan
 | PE30 | LEDIO3         | Output | (V1) LED hook for A (TA2)
 |      | BKBR3          | Input  | (V2) Keyboard input scan
 | PE31 | F_RY_BY        | Input  | (V1) NAND F_RY_BY
 |      | BKBR4          | Input  | (V2) Keyboard input scan
 +------+----------------+--------+---------------------------------------------------

 ----------------------------------------------------------------------------------------------

   Serial register input:

 +------+----------------+------------------------------------------------------------
 |  #   | Name           | Comment
 +------+----------------+------------------------------------------------------------
 |    4 | HOOK           | Hook switch
 |    5 | BT_LINK        | Bluetooth link status
 |    6 | HOST_WAKE      | Bluetooth host wake up
 |    7 | OK_ETH         | Cisco inline power OK status
 +------+----------------+------------------------------------------------------------

 ----------------------------------------------------------------------------------------------

 Chip selects:

 +------+----------------+------------------------------------------------------------
 |  #   | Name           | Comment
 +------+----------------+------------------------------------------------------------
 | CS0  | CS0            | Boot flash
 | CS1  | CS_FLASH       | NAND flash
 | CS2  | CS_DSP         | DSP
 | CS3  | DCS_DRAM       | DRAM
 | CS4  | CS_FLASH2      | (V2) 2nd flash
 +------+----------------+------------------------------------------------------------

 ----------------------------------------------------------------------------------------------

 Interrupts:

 +------+----------------+------------------------------------------------------------
 |  #   | Name           | Comment
 +------+----------------+------------------------------------------------------------
 | IRQ1 | IRQ_DSP        | DSP interrupt
 | IRQ3 | S_INTER        | DUSLIC ???
 | IRQ4 | F_RY_BY        | NAND
 | IRQ7 | IRQ_MAX        | MAX 3100 interrupt
 +------+----------------+------------------------------------------------------------

 ----------------------------------------------------------------------------------------------

 Interrupts on PCMCIA pins:

 +------+----------------+------------------------------------------------------------
 |  #   | Name           | Comment
 +------+----------------+------------------------------------------------------------
 | IP_A0| PHY1_LINK      | Link status changed for #1 Ethernet interface
 | IP_A1| PHY2_LINK      | Link status changed for #2 Ethernet interface
 | IP_A2| RMII1_MDINT    | PHY interrupt for #1
 | IP_A3| RMII2_MDINT    | PHY interrupt for #2
 | IP_A5| HOST_WAKE      | (V2) Bluetooth host wake
 | IP_A6| OK_ETH         | (V2) Cisco inline power OK
 +------+----------------+------------------------------------------------------------

**************************************************************************************************/

#define CFG_CONSOLE_IS_IN_ENV		1
#define CFG_CONSOLE_OVERWRITE_ROUTINE	1
#define CFG_CONSOLE_ENV_OVERWRITE	1

/*************************************************************************************************/

/* use board specific hardware */
#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/
#define CONFIG_HW_WATCHDOG
#define CONFIG_SHOW_ACTIVITY

/*************************************************************************************************/

#define CONFIG_CDP_DEVICE_ID		20
#define CONFIG_CDP_DEVICE_ID_PREFIX	"NT"	/* netta2 */
#define CONFIG_CDP_PORT_ID		"eth%d"
#define CONFIG_CDP_CAPABILITIES		0x00000010
#define CONFIG_CDP_VERSION		"u-boot" " " __DATE__ " " __TIME__
#define CONFIG_CDP_PLATFORM		"Intracom NetTA2"
#define CONFIG_CDP_TRIGGER		0x20020001
#define CONFIG_CDP_POWER_CONSUMPTION	4300	/* 90 mA @ 48V */
#define CONFIG_CDP_APPLIANCE_VLAN_TYPE	0x01	/* ipphone ? */

/*************************************************************************************************/

#define CONFIG_AUTO_COMPLETE	1

/*************************************************************************************************/

#define CONFIG_CRC32_VERIFY	1

/*************************************************************************************************/

#define CONFIG_HUSH_OLD_PARSER_COMPATIBLE	1

/*************************************************************************************************/
#endif	/* __CONFIG_H */

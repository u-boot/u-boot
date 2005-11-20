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
 * configuration options, board specific, for Siemens Card Controller Module
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#undef	CCM_80MHz			/* define for 80 MHz CPU only */

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC860           1   /* This is a MPC860 CPU ... */
#define CONFIG_CCM              1   /* on a Card Controller Module  */

#define CONFIG_8xx_CONS_SMC1    1   /* Console is on SMC1       */
#undef  CONFIG_8xx_CONS_SMC2
#undef  CONFIG_8xx_CONS_NONE

/*  ENVIRONMENT */

#define CONFIG_BAUDRATE         19200         /* console baudrate in bps    */
#define CONFIG_BOOTDELAY        2             /* autoboot after 2 seconds   */

#define CONFIG_IPADDR           192.168.0.42
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_GATEWAYIP        0.0.0.0
#define CONFIG_SERVERIP         192.168.0.254

#define CONFIG_HOSTNAME         CCM

#define CONFIG_LOADADDR         40180000

#undef	CONFIG_BOOTARGS

#define CONFIG_BOOTCOMMAND      "setenv bootargs " \
				"mem=${mem} " \
				"root=/dev/ram rw ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off " \
				"wt_8xx=timeout:3600; " \
				"bootm"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE	/* don't allow baudrate change	*/

#define	CONFIG_WATCHDOG		1	/* watchdog enabled		*/

#undef	CONFIG_STATUS_LED		/* Status LED disabled		*/

#define	CONFIG_PRAM		512	/* reserve 512kB "protected RAM"*/

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/

#define	CONFIG_SPI			/* enable SPI driver		*/
#define	CONFIG_SPI_X			/* 16 bit EEPROM addressing	*/

/* ----------------------------------------------------------------
 * Offset to initial SPI buffers in DPRAM (used if the environment
 * is in the SPI EEPROM): We need a 520 byte scratch DPRAM area to
 * use at an early stage. It is used between the two initialization
 * calls (spi_init_f() and spi_init_r()). The value 0xB00 makes it
 * far enough from the start of the data area (as well as from the
 * stack pointer).
 * ---------------------------------------------------------------- */
#define CFG_SPI_INIT_OFFSET		0xB00

#define CFG_EEPROM_PAGE_WRITE_BITS	5	/* 32-byte page size	*/


#define CONFIG_MAC_PARTITION		/* nod used yet			*/
#define CONFIG_DOS_PARTITION

#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE)

#define CONFIG_COMMANDS	      ( CONFIG_CMD_DFL	| \
				CFG_CMD_BSP	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_DATE	| \
				CFG_CMD_EEPROM	| \
				CFG_CMD_NFS	| \
				CFG_CMD_SNTP	)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*----------------------------------------------------------------------*/

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

#define CFG_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00F00000	/* 1 ... 15MB in DRAM	*/

#define	CFG_LOAD_ADDR		0x00100000	/* default load address	*/

/* Ethernet hardware configuration done using port pins */
#define CFG_PA_ETH_RESET 	0x0200		/* PA  6	*/
#define CFG_PA_ETH_MDDIS	0x4000		/* PA  1	*/
#define CFG_PB_ETH_POWERDOWN	0x00000800	/* PB 20	*/
#define CFG_PB_ETH_CFG1		0x00000400	/* PB 21	*/
#define CFG_PB_ETH_CFG2		0x00000200	/* PB 22	*/
#define CFG_PB_ETH_CFG3		0x00000100	/* PB 23	*/

/* Ethernet settings:
 * MDIO not disabled, autonegotiation, 10/100Mbps, half/full duplex
 */
#define CFG_ETH_MDDIS_VALUE	0
#define CFG_ETH_CFG1_VALUE	1
#define CFG_ETH_CFG2_VALUE	1
#define CFG_ETH_CFG3_VALUE	1

/* PUMA configuration */
#define CFG_PC_PUMA_PROG	0x0200		/* PC  6        */
#define CFG_PC_PUMA_DONE	0x0008		/* PC 12	*/
#define CFG_PC_PUMA_INIT	0x0004		/* PC 13	*/

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
#define CFG_IMMR		0xF0000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define	CFG_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define	CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define	CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Address accessed to reset the board - must not be mapped/assigned
 */
#define	CFG_RESET_ADDRESS	0xFEFFFFFF

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
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	67	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#if 1
/* Start port with environment in flash; switch to SPI EEPROM later */
#define	CFG_ENV_IS_IN_FLASH	1
#define	CFG_ENV_OFFSET		0x8000	/*   Offset   of Environment Sector	*/
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET+CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#else
/* Final version: environment in EEPROM */
#define CFG_ENV_IS_IN_EEPROM	1
#define CFG_ENV_OFFSET		2048
#define CFG_ENV_SIZE		2048
#endif

/*-----------------------------------------------------------------------
 * Hardware Information Block
 */
#define CFG_HWINFO_OFFSET	0x0003FFC0	/* offset of HW Info block */
#define CFG_HWINFO_SIZE		0x00000040	/* size   of HW Info block */
#define CFG_HWINFO_MAGIC	0x54514D38	/* 'TQM8' */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/

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
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
						  SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * we must activate GPL5 in the SIUMCR for CAN
 */
#define CFG_SIUMCR	(SIUMCR_DBGC11 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

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
 * If this is a 80 MHz CPU, set PLL multiplication factor to 5 (5*16=80)!
 */
#ifdef	CCM_80MHz	/* for 80 MHz, we use a 16 MHz clock * 5 */
#define CFG_PLPRCR							\
		( (5-1)<<PLPRCR_MF_SHIFT | PLPRCR_TEXPS | PLPRCR_TMIST )
#else			/* up to 50 MHz we use a 1:1 clock */
#define CFG_PLPRCR	(PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)
#endif	/* CCM_80MHz */

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#ifdef	CCM_80MHz	/* for 80 MHz, we use a 16 MHz clock * 5 */
#define CFG_SCCR	(/* SCCR_TBS  | */ \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)
#else			/* up to 50 MHz we use a 1:1 clock */
#define CFG_SCCR	(SCCR_TBS     | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)
#endif	/* CCM_80MHz */

/*-----------------------------------------------------------------------
 *
 * Interrupt Levels
 *-----------------------------------------------------------------------
 */
#define CFG_CPM_INTERRUPT	13	/* SIU_LEVEL6	*/

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER	0

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
#define CFG_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV2 | OR_BI | \
				 OR_SCY_5_CLK | OR_EHTR)

#define CFG_OR0_REMAP	(CFG_REMAP_OR_AM  | CFG_OR_TIMING_FLASH)
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V )

#define CFG_OR1_REMAP	CFG_OR0_REMAP
#define CFG_OR1_PRELIM	CFG_OR0_PRELIM
#define CFG_BR1_PRELIM	((FLASH_BASE1_PRELIM & BR_BA_MSK) | BR_V )

/*
 * BR2 and OR2 (SDRAM)
 *
 */
#define SDRAM_BASE2_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define SDRAM_BASE3_PRELIM	0x20000000	/* SDRAM bank #1	*/
#define	SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CFG_OR_TIMING_SDRAM	0x00000A00

#define CFG_OR2_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_SDRAM )
#define CFG_BR2_PRELIM	((SDRAM_BASE2_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/*
 * BR3 and OR3 (CAN Controller)
 */
#define	CFG_CAN_BASE		0xC0000000	/* CAN mapped at 0xC0000000	*/
#define CFG_CAN_OR_AM		0xFFFF8000	/* 32 kB address mask		*/
#define CFG_OR3_CAN		(CFG_CAN_OR_AM | OR_G5LA | OR_BI)
#define CFG_BR3_CAN		((CFG_CAN_BASE & BR_BA_MSK) | \
					BR_PS_8 | BR_MS_UPMB | BR_V )

/*
 * BR4/OR4: PUMA Config
 *
 * Memory controller will be used in 2 modes:
 *
 * - "read" mode:
 *	BR4: 0x10100801		OR4: 0xffff8520
 * - "load" mode (chip select on UPM B):
 *	BR4: 0x101004c1		OR4: 0xffff8600
 *
 * Default initialization is in "read" mode
 */
#define PUMA_CONF_BASE		0x10100000	/* PUMA Config */
#define PUMA_CONF_OR_AM		0xFFFF8000	/* 32 kB */
#define	PUMA_CONF_LOAD_TIMING	(OR_ACS_DIV2	 | OR_SCY_2_CLK)
#define PUMA_CONF_READ_TIMING	(OR_G5LA | OR_BI | OR_SCY_2_CLK)

#define PUMA_CONF_BR_LOAD	((PUMA_CONF_BASE & BR_BA_MSK) | \
					BR_PS_8  | BR_MS_UPMB | BR_V)
#define PUMA_CONF_OR_LOAD	(PUMA_CONF_OR_AM | PUMA_CONF_LOAD_TIMING)

#define PUMA_CONF_BR_READ	((PUMA_CONF_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)
#define PUMA_CONF_OR_READ	(PUMA_CONF_OR_AM | PUMA_CONF_READ_TIMING)

#define CFG_BR4_PRELIM		PUMA_CONF_BR_READ
#define CFG_OR4_PRELIM		PUMA_CONF_OR_READ

/*
 * BR5/OR5: PUMA: SMA Bus 8 Bit
 *	BR5: 0x10200401		OR5: 0xffe0010a
 */
#define PUMA_SMA8_BASE		0x10200000	/* PUMA SMA Bus 8 Bit */
#define PUMA_SMA8_OR_AM		0xFFE00000	/* 2 MB */
#define PUMA_SMA8_TIMING	(OR_BI | OR_SCY_0_CLK | OR_EHTR)

#define CFG_BR5_PRELIM		((PUMA_SMA8_BASE & BR_BA_MSK) | BR_PS_8 | BR_V)
#define CFG_OR5_PRELIM		(PUMA_SMA8_OR_AM | PUMA_SMA8_TIMING | OR_SETA)

/*
 * BR6/OR6: PUMA: SMA Bus 16 Bit
 *	BR6: 0x10600801		OR6: 0xffe0010a
 */
#define PUMA_SMA16_BASE		0x10600000	/* PUMA SMA Bus 16 Bit */
#define PUMA_SMA16_OR_AM	0xFFE00000	/* 2 MB */
#define PUMA_SMA16_TIMING	(OR_BI | OR_SCY_0_CLK | OR_EHTR)

#define CFG_BR6_PRELIM		((PUMA_SMA16_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)
#define CFG_OR6_PRELIM		(PUMA_SMA16_OR_AM | PUMA_SMA16_TIMING | OR_SETA)

/*
 * BR7/OR7: PUMA: external Flash
 *	BR7: 0x10a00801		OR7: 0xfe00010a
 */
#define PUMA_FLASH_BASE		0x10A00000	/* PUMA external Flash */
#define PUMA_FLASH_OR_AM	0xFE000000	/* 32 MB */
#define PUMA_FLASH_TIMING	(OR_BI | OR_SCY_0_CLK | OR_EHTR)

#define CFG_BR7_PRELIM		((PUMA_FLASH_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)
#define CFG_OR7_PRELIM		(PUMA_FLASH_OR_AM | PUMA_FLASH_TIMING | OR_SETA)


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

#endif	/* __CONFIG_H */

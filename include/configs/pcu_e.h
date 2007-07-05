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
 * Workaround for layout bug on prototype board
 */
#define	PCU_E_WITH_SWAPPED_CS	1

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC860		1	/* This is a MPC860T CPU	*/
#define CONFIG_MPC860T		1
#define CONFIG_PCU_E		1	/* ...on a PCU E board		*/

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/

#define CONFIG_BAUDRATE		9600
#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND							\
	"bootp;"								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;"	\
	"bootm"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_STATUS_LED	1	/* Status LED enabled		*/

#define	CONFIG_PRAM		2048	/* reserve 2 MB "protected RAM"	*/

#define	CONFIG_RTC_MPC8xx		/* use internal RTC of MPC8xx	*/

#define	CONFIG_SPI			/* enable SPI driver		*/
#define	CONFIG_SPI_X			/* 16 bit EEPROM addressing	*/

#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F


/* ----------------------------------------------------------------
 * Offset to initial SPI buffers in DPRAM (used if the environment
 * is in the SPI EEPROM): We need a 520 byte scratch DPRAM area to
 * use at an early stage. It is used between the two initialization
 * calls (spi_init_f() and spi_init_r()). The value 0xB00 makes it
 * far enough from the start of the data area (as well as from the
 * stack pointer).
 * ---------------------------------------------------------------- */
#define CFG_SPI_INIT_OFFSET		0xB00


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP


#define CONFIG_BOOTP_MASK \
    ((CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_BOOTFILESIZE) & ~CONFIG_BOOTP_GATEWAY)

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

#define CFG_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00F00000	/* 1 ... 15MB in DRAM	*/

#define	CFG_LOAD_ADDR		0x00100000	/* default load address	*/

#define	CFG_PIO_MODE		0	/* IDE interface in PIO Mode 0	*/

/* Ethernet hardware configuration done using port pins */
#define CFG_PB_ETH_RESET	0x00000020		/* PB 26	*/
#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define CFG_PA_ETH_MDDIS	0x4000			/* PA  1	*/
#define CFG_PB_ETH_POWERDOWN	0x00000800		/* PB 20	*/
#define CFG_PB_ETH_CFG1		0x00000400		/* PB 21	*/
#define CFG_PB_ETH_CFG2		0x00000200		/* PB 22	*/
#define CFG_PB_ETH_CFG3		0x00000100		/* PB 23	*/
#else /* XXX */
#define CFG_PB_ETH_MDDIS	0x00000010		/* PB 27	*/
#define CFG_PB_ETH_POWERDOWN	0x00000100		/* PB 23	*/
#define CFG_PB_ETH_CFG1		0x00000200		/* PB 22	*/
#define CFG_PB_ETH_CFG2		0x00000400		/* PB 21	*/
#define CFG_PB_ETH_CFG3		0x00000800		/* PB 20	*/
#endif /* XXX */

/* Ethernet settings:
 * MDIO enabled, autonegotiation, 10/100Mbps, half/full duplex
 */
#define CFG_ETH_MDDIS_VALUE	0
#define CFG_ETH_CFG1_VALUE	1
#define CFG_ETH_CFG2_VALUE	1
#define CFG_ETH_CFG3_VALUE	1

/* PUMA configuration */
#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define CFG_PB_PUMA_PROG	0x00000010		/* PB 27	*/
#else /* XXX */
#define CFG_PA_PUMA_PROG	0x4000			/* PA  1	*/
#endif /* XXX */
#define CFG_PC_PUMA_DONE	0x0008			/* PC 12	*/
#define CFG_PC_PUMA_INIT	0x0004			/* PC 13	*/

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
#define CFG_IMMR		0xFE000000

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
/* this is an ugly hack needed because of the silly non-constant address map */
#define CFG_FLASH_BASE		(0-flash_info[0].size-flash_info[1].size)

#if defined(DEBUG)
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#else
#define	CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#endif
#define CFG_MONITOR_BASE	TEXT_BASE
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
#define CFG_MAX_FLASH_SECT	160	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	180000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	1000	/* Timeout for Flash Write (in ms)	*/

#if 0
/* Start port with environment in flash; switch to SPI EEPROM later */
#define	CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x2000	/* Total Size of Environment 		*/
#define CFG_ENV_ADDR	    0xFFFFE000	/* Address    of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x2000	/* use the top-most 8k boot sector	*/
#define	CFG_ENV_IS_EMBEDDED	1	/* short-cut compile-time test		*/
#else
/* Final version: environment in EEPROM */
#define CFG_ENV_IS_IN_EEPROM	1
#define CFG_I2C_EEPROM_ADDR	0
#define CFG_I2C_EEPROM_ADDR_LEN	2
#define CFG_ENV_OFFSET		1024
#define CFG_ENV_SIZE		1024
#endif

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
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * External Arbitration max. priority (7),
 * Debug pins configuration '11',
 * Asynchronous external master enable.
 */
/* => 0x70600200 */
#define CFG_SIUMCR	(SIUMCR_EARP7 | SIUMCR_DBGC11 | SIUMCR_AEME)

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
 * interrupt status bit, set PLL multiplication factor !
 */
/* 0x00004080 */
#define	CFG_PLPRCR_MF	0	/* (0+1) * 50 = 50 MHz Clock */
#define CFG_PLPRCR							\
		(	(CFG_PLPRCR_MF << PLPRCR_MF_SHIFT) |		\
			PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST |	\
			/*PLPRCR_CSRC|*/ PLPRCR_LPM_NORMAL |		\
			PLPRCR_CSR    /*| PLPRCR_LOLRE|PLPRCR_FIOPD*/	\
		)

#define	CONFIG_8xx_GCLK_FREQ	((CFG_PLPRCR_MF+1)*50000000)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 *
 * Note: PITRTCLK is 50MHz / 512 = 97'656.25 Hz
 */
#define SCCR_MASK	SCCR_EBDF11
/* 0x01800000 */
#define CFG_SCCR	(SCCR_COM00	| /*SCCR_TBS|*/		\
			 SCCR_RTDIV	|   SCCR_RTSEL	  |	\
			 /*SCCR_CRQEN|*/  /*SCCR_PRQEN|*/ 	\
			 SCCR_EBDF00 |   SCCR_DFSYNC00 |	\
			 SCCR_DFBRG00	|   SCCR_DFNL000  |	\
			 SCCR_DFNH000	|   SCCR_DFLCD100 |	\
			 SCCR_DFALCD01)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 *
 * Note: RTC counts at PITRTCLK / 8'192 = 11.920928 Hz !!!
 *
 * Don't expect the "date" command to work without a 32kHz clock input!
 */
/* 0x00C3 => 0x0003 */
#define CFG_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)


/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration Register		19-4
 *-----------------------------------------------------------------------
 */
#define CFG_RCCR 0x0000

/*-----------------------------------------------------------------------
 * RMDS - RISC Microcode Development Support Control Register
 *-----------------------------------------------------------------------
 */
#define CFG_RMDS 0

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
 * BR0/1 and OR0/1 (FLASH) - second Flash bank optional
 */

#define FLASH_BASE0_PRELIM	0xFF800000	/* FLASH bank #0	*/
#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define FLASH_BASE6_PRELIM	0xFF000000	/* FLASH bank #1	*/
#else /* XXX */
#define FLASH_BASE1_PRELIM	0xFF000000	/* FLASH bank #1	*/
#endif /* XXX */

/*
 * used to re-map FLASH: restrict access enough but not too much to
 * meddle with FLASH accesses
 */
#define CFG_REMAP_OR_AM		0xFF800000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xFF800000	/* OR addr mask */

/* FLASH timing: CSNT = 0, ACS = 00, SCY = 8, EHTR = 1			*/
#define CFG_OR_TIMING_FLASH	(OR_SCY_8_CLK | OR_EHTR)

#define CFG_OR0_REMAP	( CFG_REMAP_OR_AM | OR_ACS_DIV1 | OR_BI | \
				CFG_OR_TIMING_FLASH)
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | OR_ACS_DIV1 | OR_BI | \
				CFG_OR_TIMING_FLASH)
/* 16 bit, bank valid */
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_PS_16 | BR_V )

#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define CFG_OR6_REMAP	CFG_OR0_REMAP
#define CFG_OR6_PRELIM	CFG_OR0_PRELIM
#define CFG_BR6_PRELIM	((FLASH_BASE6_PRELIM & BR_BA_MSK) | BR_PS_16 | BR_V )
#else /* XXX */
#define CFG_OR1_REMAP	CFG_OR0_REMAP
#define CFG_OR1_PRELIM	CFG_OR0_PRELIM
#define CFG_BR1_PRELIM	((FLASH_BASE1_PRELIM & BR_BA_MSK) | BR_PS_16 | BR_V )
#endif /* XXX */

/*
 * BR2/OR2: SDRAM
 *
 * Multiplexed addresses, GPL5 output to GPL5_A (don't care)
 */
#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define SDRAM_BASE5_PRELIM	0x00000000	/* SDRAM bank */
#else /* XXX */
#define SDRAM_BASE2_PRELIM	0x00000000	/* SDRAM bank */
#endif /* XXX */
#define SDRAM_PRELIM_OR_AM	0xF8000000	/* map 128 MB (>SDRAM_MAX_SIZE!) */
#define SDRAM_TIMING		OR_CSNT_SAM	/* SDRAM-Timing */

#define SDRAM_MAX_SIZE		0x04000000	/* max 64 MB SDRAM */

#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define CFG_OR5_PRELIM	(SDRAM_PRELIM_OR_AM | SDRAM_TIMING )
#define CFG_BR5_PRELIM	((SDRAM_BASE5_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )
#else /* XXX */
#define CFG_OR2_PRELIM	(SDRAM_PRELIM_OR_AM | SDRAM_TIMING )
#define CFG_BR2_PRELIM	((SDRAM_BASE2_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )
#endif /* XXX */

/*
 * BR3/OR3: CAN Controller
 *	BR3: 0x10000401		OR3: 0xffff818a
 */
#define CAN_CTRLR_BASE		0x10000000	/* CAN Controller */
#define CAN_CTRLR_OR_AM		0xFFFF8000	/* 32 kB */
#define CAN_CTRLR_TIMING	(OR_BI | OR_SCY_8_CLK | OR_SETA | OR_EHTR)

#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define CFG_BR4_PRELIM		((CAN_CTRLR_BASE & BR_BA_MSK) | BR_PS_8 | BR_V)
#define CFG_OR4_PRELIM		(CAN_CTRLR_OR_AM | CAN_CTRLR_TIMING)
#else /* XXX */
#define CFG_BR3_PRELIM		((CAN_CTRLR_BASE & BR_BA_MSK) | BR_PS_8 | BR_V)
#define CFG_OR3_PRELIM		(CAN_CTRLR_OR_AM | CAN_CTRLR_TIMING)
#endif /* XXX */

/*
 * BR4/OR4: PUMA Config
 *
 * Memory controller will be used in 2 modes:
 *
 * - "read" mode:
 *	BR4: 0x10100801		OR4: 0xffff8530
 * - "load" mode (chip select on UPM B):
 *	BR4: 0x101008c1		OR4: 0xffff8630
 *
 * Default initialization is in "read" mode
 */
#define PUMA_CONF_BASE		0x10100000	/* PUMA Config */
#define PUMA_CONF_OR_AM		0xFFFF8000	/* 32 kB */
#define	PUMA_CONF_LOAD_TIMING	(OR_ACS_DIV2	 | OR_SCY_3_CLK)
#define PUMA_CONF_READ_TIMING	(OR_G5LA | OR_BI | OR_SCY_3_CLK)

#define PUMA_CONF_BR_LOAD	((PUMA_CONF_BASE & BR_BA_MSK) | \
					BR_PS_16 | BR_MS_UPMB | BR_V)
#define PUMA_CONF_OR_LOAD	(PUMA_CONF_OR_AM | PUMA_CONF_LOAD_TIMING)

#define PUMA_CONF_BR_READ	((PUMA_CONF_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)
#define PUMA_CONF_OR_READ	(PUMA_CONF_OR_AM | PUMA_CONF_READ_TIMING)

#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define CFG_BR3_PRELIM		PUMA_CONF_BR_READ
#define CFG_OR3_PRELIM		PUMA_CONF_OR_READ
#else /* XXX */
#define CFG_BR4_PRELIM		PUMA_CONF_BR_READ
#define CFG_OR4_PRELIM		PUMA_CONF_OR_READ
#endif /* XXX */

/*
 * BR5/OR5: PUMA: SMA Bus 8 Bit
 *	BR5: 0x10200401		OR5: 0xffe0010a
 */
#define PUMA_SMA8_BASE		0x10200000	/* PUMA SMA Bus 8 Bit */
#define PUMA_SMA8_OR_AM		0xFFE00000	/* 2 MB */
#define PUMA_SMA8_TIMING	(OR_BI | OR_SCY_0_CLK | OR_EHTR)

#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define CFG_BR2_PRELIM		((PUMA_SMA8_BASE & BR_BA_MSK) | BR_PS_8 | BR_V)
#define CFG_OR2_PRELIM		(PUMA_SMA8_OR_AM | PUMA_SMA8_TIMING | OR_SETA)
#else /* XXX */
#define CFG_BR5_PRELIM		((PUMA_SMA8_BASE & BR_BA_MSK) | BR_PS_8 | BR_V)
#define CFG_OR5_PRELIM		(PUMA_SMA8_OR_AM | PUMA_SMA8_TIMING | OR_SETA)
#endif /* XXX */

/*
 * BR6/OR6: PUMA: SMA Bus 16 Bit
 *	BR6: 0x10600801		OR6: 0xffe0010a
 */
#define PUMA_SMA16_BASE		0x10600000	/* PUMA SMA Bus 16 Bit */
#define PUMA_SMA16_OR_AM	0xFFE00000	/* 2 MB */
#define PUMA_SMA16_TIMING	(OR_BI | OR_SCY_0_CLK | OR_EHTR)

#if PCU_E_WITH_SWAPPED_CS /* XXX */
#define CFG_BR1_PRELIM		((PUMA_SMA16_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)
#define CFG_OR1_PRELIM		(PUMA_SMA16_OR_AM | PUMA_SMA16_TIMING | OR_SETA)
#else /* XXX */
#define CFG_BR6_PRELIM		((PUMA_SMA16_BASE & BR_BA_MSK) | BR_PS_16 | BR_V)
#define CFG_OR6_PRELIM		(PUMA_SMA16_OR_AM | PUMA_SMA16_TIMING | OR_SETA)
#endif /* XXX */

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
#define CFG_MPTPR	0x0200

/*
 * MAMR settings for SDRAM
 * 0x30104118 = Timer A period 0x30, MAMR_AMB_TYPE_1, MAMR_G0CLB_A10,
 *		MAMR_RLFB_1X, MAMR_WLFB_1X, MAMR_TLFB_8X
 * 0x30904114 = - " - | Periodic Timer A Enable, MAMR_TLFB_4X
 */
/* periodic timer for refresh */
#define CFG_MAMR_PTA	0x30	/* = 48 */

#define CFG_MAMR	( (CFG_MAMR_PTA << MAMR_PTA_SHIFT) | \
			  MAMR_AMA_TYPE_1	| \
			  MAMR_G0CLA_A10	| \
			  MAMR_RLFA_1X		| \
			  MAMR_WLFA_1X		| \
			  MAMR_TLFA_8X		)

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#endif	/* __CONFIG_H */

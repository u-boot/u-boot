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
 * Config header file for Hymod board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8260		1	/* This is an MPC8260 CPU	*/
#define CONFIG_HYMOD		1	/* ...on a Hymod board		*/
#define CONFIG_CPM2		1	/* Has a CPM2 */

#define	CONFIG_MISC_INIT_F	1	/* Use misc_init_f()		*/

#define CONFIG_BOARD_POSTCLK_INIT	/* have board_postclk_init() function */

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
#undef	CONFIG_CONS_ON_SMC		/* define if console on SMC */
#define	CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef	CONFIG_CONS_NONE		/* define if console on something else*/
#define	CONFIG_CONS_INDEX	1	/* which serial channel for console */
#define	CONFIG_CONS_USE_EXTC		/* SMC/SCC use ext clock not brg_clk */
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
 * defined elsewhere (as for the console), or CONFIG_CMD_NET must be unset.
 */
#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC	*/
#define	CONFIG_ETHER_ON_FCC		/* define if ether on FCC	*/
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */
#define CONFIG_ETHER_INDEX	1	/* which channel for ether	*/
#define CONFIG_ETHER_LOOPBACK_TEST	/* add ether external loopback test */

#ifdef CONFIG_ETHER_ON_FCC

#if (CONFIG_ETHER_INDEX == 1)

/*
 * - Rx-CLK is CLK10
 * - Tx-CLK is CLK11
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK	(CMXFCR_FC1|CMXFCR_RF1CS_MSK|CMXFCR_TF1CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF1CS_CLK10|CMXFCR_TF1CS_CLK11)
# define CONFIG_SYS_CPMFCR_RAMTYPE	0
# define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)

# define MDIO_PORT		0		/* Port A */
# define MDIO_DATA_PINMASK	0x00040000	/* Pin 13 */
# define MDIO_CLCK_PINMASK	0x00080000	/* Pin 12 */

#elif (CONFIG_ETHER_INDEX == 2)

/*
 * - Rx-CLK is CLK13
 * - Tx-CLK is CLK14
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK	(CMXFCR_FC2|CMXFCR_RF2CS_MSK|CMXFCR_TF2CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13|CMXFCR_TF2CS_CLK14)
# define CONFIG_SYS_CPMFCR_RAMTYPE	0
# define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)

# define MDIO_PORT		0		/* Port A */
# define MDIO_DATA_PINMASK	0x00000040	/* Pin 25 */
# define MDIO_CLCK_PINMASK	0x00000080	/* Pin 24 */

#elif (CONFIG_ETHER_INDEX == 3)

/*
 * - Rx-CLK is CLK15
 * - Tx-CLK is CLK16
 * - RAM for BD/Buffers is on the 60x Bus (see 28-13)
 * - Enable Full Duplex in FSMR
 */
# define CONFIG_SYS_CMXFCR_MASK	(CMXFCR_FC3|CMXFCR_RF3CS_MSK|CMXFCR_TF3CS_MSK)
# define CONFIG_SYS_CMXFCR_VALUE	(CMXFCR_RF3CS_CLK15|CMXFCR_TF3CS_CLK16)
# define CONFIG_SYS_CPMFCR_RAMTYPE	0
# define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE|FCC_PSMR_LPB)

# define MDIO_PORT		0		/* Port A */
# define MDIO_DATA_PINMASK	0x00000100	/* Pin 23 */
# define MDIO_CLCK_PINMASK	0x00000200	/* Pin 22 */

#endif	/* CONFIG_ETHER_INDEX */

#define CONFIG_MII			/* MII PHY management	*/
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/

#define MDIO_ACTIVE	(iop->pdir |=  MDIO_DATA_PINMASK)
#define MDIO_TRISTATE	(iop->pdir &= ~MDIO_DATA_PINMASK)
#define MDIO_READ	((iop->pdat &  MDIO_DATA_PINMASK) != 0)

#define MDIO(bit)	if(bit) iop->pdat |=  MDIO_DATA_PINMASK; \
			else	iop->pdat &= ~MDIO_DATA_PINMASK

#define MDC(bit)	if(bit) iop->pdat |=  MDIO_CLCK_PINMASK; \
			else	iop->pdat &= ~MDIO_CLCK_PINMASK

#define MIIDELAY	udelay(1)

#endif	/* CONFIG_ETHER_ON_FCC */


/* other options */
#define CONFIG_HARD_I2C		1	/* To enable I2C hardware support	*/
#define CONFIG_DTT_ADM1021	1	/* ADM1021 temp sensor support */

/* system clock rate (CLKIN) - equal to the 60x and local bus speed */
#ifdef DEBUG
#define CONFIG_8260_CLKIN	33333333	/* in Hz */
#else
#define CONFIG_8260_CLKIN	66666666	/* in Hz */
#endif

#if defined(CONFIG_CONS_USE_EXTC)
#define CONFIG_BAUDRATE		115200
#else
#define CONFIG_BAUDRATE		9600
#endif

/* default ip addresses - these will be overridden */
#define CONFIG_IPADDR		192.168.1.1	/* hymod "boot" address */
#define CONFIG_SERVERIP		192.168.1.254	/* hymod "server" address */

#define CONFIG_LAST_STAGE_INIT

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BSP
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_CDP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_KGDB
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_PORTIO
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_SNTP

#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_XIMG

#ifdef DEBUG
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#define CONFIG_BOOT_RETRY_TIME 30	/* retry autoboot after 30 secs	*/
#define CONFIG_BOOT_RETRY_MIN	1	/* can go down to 1 second timeout */
/* Be selective on what keys can delay or stop the autoboot process
 *	To stop use: " "
 */
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT		"Autobooting in %d seconds, " \
					"press <SPACE> to stop\n", bootdelay
#define CONFIG_AUTOBOOT_STOP_STR	" "
#undef CONFIG_AUTOBOOT_DELAY_STR
#define DEBUG_BOOTKEYS		0
#endif

#if defined(CONFIG_CMD_KGDB)
#undef	CONFIG_KGDB_ON_SMC		/* define if kgdb on SMC */
#define	CONFIG_KGDB_ON_SCC		/* define if kgdb on SCC */
#undef	CONFIG_KGDB_NONE		/* define if kgdb on something else */
#define CONFIG_KGDB_INDEX	2	/* which serial channel for kgdb */
#define	CONFIG_KGDB_USE_EXTC		/* SMC/SCC use ext clock not brg_clk */
#define	CONFIG_KGDB_EXTC_RATE	3686400	/* serial ext clk rate in Hz */
#define	CONFIG_KGDB_EXTC_PINSEL	0	/* pin select 0=CLK3/CLK9,1=CLK5/CLK15*/
# if defined(CONFIG_KGDB_USE_EXTC)
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port at */
# else
#define CONFIG_KGDB_BAUDRATE	9600	/* speed to run kgdb serial port at */
# endif
#endif

#undef	CONFIG_WATCHDOG			/* disable platform specific watchdog */

#define CONFIG_RTC_PCF8563		/* use Philips PCF8563 RTC	*/

/*
 * Hymod specific configurable options
 */
#undef	CONFIG_SYS_HYMOD_DBLEDS			/* walk mezz board LEDs */

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x03c00000	/* 4 ... 60 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define	CONFIG_SYS_I2C_SPEED		50000
#define	CONFIG_SYS_I2C_SLAVE		0x7e

/* these are for the ST M24C02 2kbit serial i2c eeprom */
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50		/* base address */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1		/* bytes of address */
/* mask of address bits that overflow into the "EEPROM chip address"    */
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07

#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4	/* 16 byte write page size */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10	/* and takes up to 10 msec */

#define CONFIG_SYS_I2C_MULTI_EEPROMS	1		/* hymod has two eeproms */

#define CONFIG_SYS_I2C_RTC_ADDR	0x51	/* philips PCF8563 RTC address */

/*
 * standard dtt sensor configuration - bottom bit will determine local or
 * remote sensor of the ADM1021, the rest determines index into
 * CONFIG_SYS_DTT_ADM1021 array below.
 *
 * On HYMOD board, the remote sensor should be connected to the MPC8260
 * temperature diode thingy, but an errata said this didn't work and
 * should be disabled - so it isn't connected.
 */
#if 0
#define CONFIG_DTT_SENSORS		{ 0, 1 }
#else
#define CONFIG_DTT_SENSORS		{ 0 }
#endif

/*
 * ADM1021 temp sensor configuration (see dtt/adm1021.c for details).
 * there will be one entry in this array for each two (dummy) sensors in
 * CONFIG_DTT_SENSORS.
 *
 * For HYMOD board:
 * - only one ADM1021
 * - i2c addr 0x2a (both ADD0 and ADD1 are N/C)
 * - conversion rate 0x02 = 0.25 conversions/second
 * - ALERT ouput disabled
 * - local temp sensor enabled, min set to 0 deg, max set to 85 deg
 * - remote temp sensor disabled (see comment for CONFIG_DTT_SENSORS above)
 */
#define CONFIG_SYS_DTT_ADM1021		{ { 0x2a, 0x02, 0, 1, 0, 85, 0, } }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Hard Reset Configuration Words
 *
 * if you change bits in the HRCW, you must also change the CONFIG_SYS_*
 * defines for the various registers affected by the HRCW e.g. changing
 * HRCW_DPPCxx requires you to also change CONFIG_SYS_SIUMCR.
 */
#ifdef DEBUG
#define CONFIG_SYS_HRCW_MASTER	(HRCW_BPS11|HRCW_CIP|HRCW_L2CPC01|HRCW_DPPC10|\
			 HRCW_ISB100|HRCW_BMS|HRCW_MMR11|HRCW_APPC10|\
			 HRCW_MODCK_H0010)
#else
#define CONFIG_SYS_HRCW_MASTER	(HRCW_BPS11|HRCW_CIP|HRCW_L2CPC01|HRCW_DPPC10|\
			 HRCW_ISB100|HRCW_BMS|HRCW_MMR11|HRCW_APPC10|\
			 HRCW_MODCK_H0101)
#endif
/* no slaves so just duplicate the master hrcw */
#define CONFIG_SYS_HRCW_SLAVE1	CONFIG_SYS_HRCW_MASTER
#define CONFIG_SYS_HRCW_SLAVE2	CONFIG_SYS_HRCW_MASTER
#define CONFIG_SYS_HRCW_SLAVE3	CONFIG_SYS_HRCW_MASTER
#define CONFIG_SYS_HRCW_SLAVE4	CONFIG_SYS_HRCW_MASTER
#define CONFIG_SYS_HRCW_SLAVE5	CONFIG_SYS_HRCW_MASTER
#define CONFIG_SYS_HRCW_SLAVE6	CONFIG_SYS_HRCW_MASTER
#define CONFIG_SYS_HRCW_SLAVE7	CONFIG_SYS_HRCW_MASTER

/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CONFIG_SYS_IMMR		0xF0000000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define	CONFIG_SYS_INIT_RAM_END	0x4000	/* End of used area in DPRAM	*/
#define	CONFIG_SYS_GBL_DATA_SIZE	128  /* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define	CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		TEXT_BASE
#define	CONFIG_SYS_MONITOR_BASE	TEXT_BASE
#define CONFIG_SYS_FPGA_BASE		0x80000000
/*
 * unfortunately, CONFIG_SYS_MONITOR_LEN must include the
 * (very large i.e. 256kB) environment flash sector
 */
#define	CONFIG_SYS_MONITOR_LEN		(512 << 10)	/* Reserve 512 kB for Monitor*/
#define	CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Mem map for Linux*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max num of memory banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	67	/* max num of sects on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Flash Erase Timeout (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)	*/

#define	CONFIG_ENV_IS_IN_FLASH	1
#define	CONFIG_ENV_SIZE		0x40000	/* Total Size of Environment Sector */
#define CONFIG_ENV_SECT_SIZE	0x40000	/* see README - env sect real size */
#define	CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE+CONFIG_SYS_MONITOR_LEN-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_SYS_USE_PPCENV			/* Environment embedded in sect .ppcenv */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC8260 CPU		*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value*/
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
#define CONFIG_SYS_HID0_INIT	(HID0_ICE|HID0_DCE|HID0_ICFI|HID0_DCI|\
				HID0_IFEM|HID0_ABE)
#ifdef DEBUG
#define CONFIG_SYS_HID0_FINAL	0
#else
#define CONFIG_SYS_HID0_FINAL	(HID0_ICE|HID0_IFEM|HID0_ABE)
#endif
#define CONFIG_SYS_HID2	0

/*-----------------------------------------------------------------------
 * RMR - Reset Mode Register					 5-5
 *-----------------------------------------------------------------------
 * turn on Checkstop Reset Enable
 */
#ifdef DEBUG
#define CONFIG_SYS_RMR		0
#else
#define CONFIG_SYS_RMR		RMR_CSRE
#endif

/*-----------------------------------------------------------------------
 * BCR - Bus Configuration					 4-25
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_BCR		(BCR_ETM)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				 4-31
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DPPC10|SIUMCR_L2CPC01|\
			 SIUMCR_APPC10|SIUMCR_MMR11)

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				 4-35
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Watchdog & Bus Monitor Timer max, 60x & Local Bus Monitor enable
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP|SYPCR_SWE)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC|SYPCR_BMT|SYPCR_PBME|SYPCR_LBME|\
			 SYPCR_SWRI|SYPCR_SWP)
#endif /* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TMCNTSC - Time Counter Status and Control			 4-40
 *-----------------------------------------------------------------------
 * Clear once per Second and Alarm Interrupt Status, Set 32KHz timersclk,
 * and enable Time Counter
 */
#define CONFIG_SYS_TMCNTSC	(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		 4-42
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Set 32KHz timersclk, and enable
 * Periodic timer
 */
#define CONFIG_SYS_PISCR	(PISCR_PS|PISCR_PTF|PISCR_PTE)

/*-----------------------------------------------------------------------
 * SCCR - System Clock Control					 9-8
 *-----------------------------------------------------------------------
 * Ensure DFBRG is Divide by 16
 */
#define CONFIG_SYS_SCCR	(SCCR_DFBRG01)

/*-----------------------------------------------------------------------
 * RCCR - RISC Controller Configuration				13-7
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_RCCR	0

/*
 * Init Memory Controller:
 *
 * Bank	Bus	Machine	PortSz	Device
 * ----	---	-------	------	------
 *  0	60x	GPCM	32 bit	FLASH
 *  1	60x	GPCM	32 bit	FLASH (same as 0 - unused for now)
 *  2	60x	SDRAM	64 bit	SDRAM
 *  3	Local	UPMC	 8 bit	Main Xilinx configuration
 *  4	Local	GPCM	32 bit	Main Xilinx register mode
 *  5	Local	UPMB	32 bit	Main Xilinx port mode
 *  6	Local	UPMC	 8 bit	Mezz Xilinx configuration
 */

/*
 * Bank 0 - FLASH
 *
 * Quotes from the HYMOD IO Board Reference manual:
 *
 * "The flash memory is two Intel StrataFlash chips, each configured for
 *  16 bit operation and connected to give a 32 bit wide port."
 *
 * "The chip select logic is configured to respond to both *CS0 and *CS1.
 *  Therefore the FLASH memory will be mapped to both bank 0 and bank 1.
 *  It is suggested that bank 0 be read-only and bank 1 be read/write. The
 *  FLASH will then appear as ROM during boot."
 *
 * Initially, we are only going to use bank 0 in read/write mode.
 */

/* 32 bit, read-write, GPCM on 60x bus */
#define	CONFIG_SYS_BR0_PRELIM	((CONFIG_SYS_FLASH_BASE&BRx_BA_MSK)|\
				BRx_PS_32|BRx_MS_GPCM_P|BRx_V)
/* up to 32 Mb */
#define	CONFIG_SYS_OR0_PRELIM	(MEG_TO_AM(32)|ORxG_CSNT|ORxG_ACS_DIV2|ORxG_SCY_10_CLK)

/*
 * Bank 2 - SDRAM
 *
 * Quotes from the HYMOD IO Board Reference manual:
 *
 * "The main memory is implemented using TC59SM716FTL-10 SDRAM and has a
 *  fixed size of 64 Mbytes. The Toshiba TC59SM716FTL-10 is a CMOS synchronous
 *  dynamic random access memory organised as 4 banks by 4096 rows by 512
 *  columns by 16 bits. Four chips provide a 64-bit port on the 60x bus."
 *
 * "The locations in SDRAM are accessed using multiplexed address pins to
 *  specify row and column. The pins also act to specify commands. The state
 *  of the inputs *RAS, *CAS and *WE defines the required action. The a10/AP
 *  pin may function as a row address or as the AUTO PRECHARGE control line,
 *  depending on the cycle type. The 60x bus SDRAM machine allows the MPC8260
 *  address lines to be configured to the required multiplexing scheme."
 */

#define CONFIG_SYS_SDRAM_SIZE	64

/* 64 bit, read-write, SDRAM on 60x bus */
#define	CONFIG_SYS_BR2_PRELIM	((CONFIG_SYS_SDRAM_BASE&BRx_BA_MSK)|\
				BRx_PS_64|BRx_MS_SDRAM_P|BRx_V)
/* 64 Mb, 4 int banks per dev, row start addr bit = A6, 12 row addr lines */
#define	CONFIG_SYS_OR2_PRELIM	(MEG_TO_AM(CONFIG_SYS_SDRAM_SIZE)|\
				ORxS_BPD_4|ORxS_ROWST_PBI1_A6|ORxS_NUMR_12)

/*
 * The 60x Bus SDRAM Mode Register (PDSMR) is set as follows:
 *
 * Page Based Interleaving, Refresh Enable, Address Multiplexing where A5
 * is output on A16 pin (A6 on A17, and so on), use address pins A14-A16
 * as bank select, A7 is output on SDA10 during an ACTIVATE command,
 * earliest timing for ACTIVATE command after REFRESH command is 6 clocks,
 * earliest timing for ACTIVATE or REFRESH command after PRECHARGE command
 * is 2 clocks, earliest timing for READ/WRITE command after ACTIVATE
 * command is 2 clocks, earliest timing for PRECHARGE after last data
 * was read is 1 clock, earliest timing for PRECHARGE after last data
 * was written is 1 clock, CAS Latency is 2.
 */

#define CONFIG_SYS_PSDMR	(PSDMR_PBI|PSDMR_SDAM_A16_IS_A5|\
				PSDMR_BSMA_A14_A16|PSDMR_SDA10_PBI1_A7|\
				PSDMR_RFRC_6_CLK|PSDMR_PRETOACT_2W|\
				PSDMR_ACTTORW_2W|PSDMR_LDOTOPRE_1C|\
				PSDMR_WRC_1C|PSDMR_CL_2)

/*
 * The 60x bus-assigned SDRAM Refresh Timer (PSRT) (10-31) and the Refresh
 * Timers Prescale (PTP) value in the Memory Refresh Timer Prescaler Register
 * (MPTPR) (10-32) must also be set up (it used to be called the Periodic Timer
 * Prescaler, hence the P instead of the R). The refresh timer period is given
 * by (note that there was a change in the 8260 UM Errata):
 *
 *	TimerPeriod = (PSRT + 1) / Fmptc
 *
 * where Fmptc is the BusClock divided by PTP. i.e.
 *
 *	TimerPeriod = (PSRT + 1) / (BusClock / PTP)
 *
 * or
 *
 *	TImerPeriod = (PTP * (PSRT + 1)) / BusClock
 *
 * The requirement for the Toshiba TC59SM716FTL-10 is that there must be
 * 4K refresh cycles every 64 ms. i.e. one refresh cycle every 64000/4096
 * = 15.625 usecs.
 *
 * So PTP * (PSRT + 1) <= 15.625 * BusClock. At 66.666MHz, PSRT=31 and PTP=32
 * appear to be reasonable.
 */

#ifdef DEBUG
#define CONFIG_SYS_PSRT	39
#define CONFIG_SYS_MPTPR	MPTPR_PTP_DIV8
#else
#define CONFIG_SYS_PSRT	31
#define CONFIG_SYS_MPTPR	MPTPR_PTP_DIV32
#endif

/*
 * Banks 3,4,5 and 6 - FPGA access
 *
 * Quotes from the HYMOD IO Board Reference manual:
 *
 * "The IO Board is fitted with a Xilinx XCV300E main FPGA. Provision is made
 *  for configuring an optional FPGA on the mezzanine interface.
 *
 *  Access to the FPGAs may be divided into several catagories:
 *
 *  1. Configuration
 *  2. Register mode access
 *  3. Port mode access
 *
 *  The main FPGA is supported for modes 1, 2 and 3. The mezzanine FPGA can be
 *  configured only (mode 1). Consequently there are four access types.
 *
 *  To improve interface performance and simplify software design, the four
 *  possible access types are separately mapped to different memory banks.
 *
 *  All are accessed using the local bus."
 *
 *	 Device		    Mode      Memory Bank Machine Port Size    Access
 *
 *	  Main		Configuration	   3	   UPMC	     8bit	R/W
 *	  Main		  Register	   4	   GPCM	    32bit	R/W
 *	  Main		    Port	   5	   UPMB	    32bit	R/W
 *	Mezzanine	Configuration	   6	   UPMC	     8bit	W/O
 *
 * "Note that mezzanine mode 1 access is write-only."
 */

/* all the bank sizes must be a power of two, greater or equal to 32768 */
#define FPGA_MAIN_CFG_BASE	(CONFIG_SYS_FPGA_BASE)
#define FPGA_MAIN_CFG_SIZE	32768
#define FPGA_MAIN_REG_BASE	(FPGA_MAIN_CFG_BASE + FPGA_MAIN_CFG_SIZE)
#define FPGA_MAIN_REG_SIZE	32768
#define FPGA_MAIN_PORT_BASE	(FPGA_MAIN_REG_BASE + FPGA_MAIN_REG_SIZE)
#define FPGA_MAIN_PORT_SIZE	32768
#define FPGA_MEZZ_CFG_BASE	(FPGA_MAIN_PORT_BASE + FPGA_MAIN_PORT_SIZE)
#define FPGA_MEZZ_CFG_SIZE	32768

/* 8 bit, read-write, UPMC */
#define	CONFIG_SYS_BR3_PRELIM	(FPGA_MAIN_CFG_BASE|BRx_PS_8|BRx_MS_UPMC|BRx_V)
/* up to 32Kbyte, burst inhibit */
#define	CONFIG_SYS_OR3_PRELIM	(P2SZ_TO_AM(FPGA_MAIN_CFG_SIZE)|ORxU_BI)

/* 32 bit, read-write, GPCM */
#define	CONFIG_SYS_BR4_PRELIM	(FPGA_MAIN_REG_BASE|BRx_PS_32|BRx_MS_GPCM_L|BRx_V)
/* up to 32Kbyte */
#define	CONFIG_SYS_OR4_PRELIM	(P2SZ_TO_AM(FPGA_MAIN_REG_SIZE))

/* 32 bit, read-write, UPMB */
#define	CONFIG_SYS_BR5_PRELIM	(FPGA_MAIN_PORT_BASE|BRx_PS_32|BRx_MS_UPMB|BRx_V)
/* up to 32Kbyte */
#define	CONFIG_SYS_OR5_PRELIM	(P2SZ_TO_AM(FPGA_MAIN_PORT_SIZE)|ORxU_BI)

/* 8 bit, write-only, UPMC */
#define	CONFIG_SYS_BR6_PRELIM	(FPGA_MEZZ_CFG_BASE|BRx_PS_8|BRx_MS_UPMC|BRx_V)
/* up to 32Kbyte, burst inhibit */
#define	CONFIG_SYS_OR6_PRELIM	(P2SZ_TO_AM(FPGA_MEZZ_CFG_SIZE)|ORxU_BI)

/*-----------------------------------------------------------------------
 * MBMR - Machine B Mode					10-27
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_MBMR	(MxMR_BSEL|MxMR_OP_NORM)	/* XXX - needs more */

/*-----------------------------------------------------------------------
 * MCMR - Machine C Mode					10-27
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_MCMR	(MxMR_BSEL|MxMR_DSx_2_CYCL)	/* XXX - needs more */

/*
 * FPGA I/O Port/Bit information
 */

#define FPGA_MAIN_PROG_PORT	IOPIN_PORTA
#define FPGA_MAIN_PROG_PIN	4	/* PA4 */
#define FPGA_MAIN_INIT_PORT	IOPIN_PORTA
#define FPGA_MAIN_INIT_PIN	5	/* PA5 */
#define FPGA_MAIN_DONE_PORT	IOPIN_PORTA
#define FPGA_MAIN_DONE_PIN	6	/* PA6 */

#define FPGA_MEZZ_PROG_PORT	IOPIN_PORTA
#define FPGA_MEZZ_PROG_PIN	0	/* PA0 */
#define FPGA_MEZZ_INIT_PORT	IOPIN_PORTA
#define FPGA_MEZZ_INIT_PIN	1	/* PA1 */
#define FPGA_MEZZ_DONE_PORT	IOPIN_PORTA
#define FPGA_MEZZ_DONE_PIN	2	/* PA2 */
#define FPGA_MEZZ_ENABLE_PORT	IOPIN_PORTA
#define FPGA_MEZZ_ENABLE_PIN	3	/* PA3 */

/*
 * FPGA Interrupt configuration
 */
#define FPGA_MAIN_IRQ		SIU_INT_IRQ2

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH*/
#define BOOTFLAG_WARM	0x02		/* Software reboot		*/

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_CMD_MTDPARTS
#define MTDIDS_DEFAULT		""
#define MTDPARTS_DEFAULT	""
*/

#endif	/* __CONFIG_H */

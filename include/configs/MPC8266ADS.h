/*
 * (C) Copyright 2001
 * Stuart Hughes <stuarth@lineo.com>
 * This file is based on similar values for other boards found in other
 * U-Boot config files, and some that I found in the mpc8260ads manual.
 *
 * Note: my board is a PILOT rev.
 * Note: the mpc8260ads doesn't come with a proper Ethernet MAC address.
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
 * Config header file for a MPC8260ADS Pilot 16M Ram Simm, 8Mbytes Flash Simm
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8260		1	/* This is an MPC8260 CPU   */
#define CONFIG_MPC8260ADS	1	/* ...on motorola ads board */

#define CONFIG_BOARD_PRE_INIT	1	/* Call board_pre_init	*/

/* allow serial and ethaddr to be overwritten */
#define CONFIG_ENV_OVERWRITE

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
#define CONFIG_CONS_ON_SCC		/* define if console on SCC */
#undef	CONFIG_CONS_NONE		/* define if console on something else */
#define CONFIG_CONS_INDEX	1	/* which serial channel for console */

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
#undef	CONFIG_ETHER_ON_SCC		/* define if ether on SCC   */
#define CONFIG_ETHER_ON_FCC		/* define if ether on FCC   */
#undef	CONFIG_ETHER_NONE		/* define if ether on something else */
#define CONFIG_ETHER_INDEX	2	/* which channel for ether  */

#if (CONFIG_ETHER_INDEX == 2)

/*
 * - Rx-CLK is CLK13
 * - Tx-CLK is CLK14
 * - Select bus for bd/buffers (see 28-13)
 * - Half duplex
 */
# define CFG_CMXFCR_MASK	(CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)
# define CFG_CMXFCR_VALUE	(CMXFCR_RF2CS_CLK13 | CMXFCR_TF2CS_CLK14)
# define CFG_CPMFCR_RAMTYPE	0
# define CFG_FCC_PSMR		0

#endif	/* CONFIG_ETHER_INDEX */

/* other options */
#define CONFIG_HARD_I2C		1	/* To enable I2C support	*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_EEPROM_ADDR_LEN 1

/*-----------------------------------------------------------------------
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 */
#define SPD_EEPROM_ADDRESS      0x50


#define CONFIG_8260_CLKIN	66666666	/* in Hz */
#define CONFIG_BAUDRATE		115200


#define CONFIG_COMMANDS		(CFG_CMD_ALL & ~( \
				 CFG_CMD_BEDBUG | \
				 CFG_CMD_BSP	| \
				 CFG_CMD_DATE	| \
				 CFG_CMD_DOC	| \
				 CFG_CMD_DTT	| \
				 CFG_CMD_EEPROM | \
				 CFG_CMD_ELF    | \
				 CFG_CMD_FDC	| \
				 CFG_CMD_FDOS	| \
				 CFG_CMD_HWFLOW	| \
				 CFG_CMD_IDE	| \
				 CFG_CMD_JFFS2	| \
				 CFG_CMD_KGDB	| \
				 CFG_CMD_MII	| \
				 CFG_CMD_PCI	| \
				 CFG_CMD_PCMCIA | \
				 CFG_CMD_SCSI	| \
				 CFG_CMD_SPI	| \
				 CFG_CMD_VFD	| \
				 CFG_CMD_USB	) )

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>


#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#define CONFIG_BOOTCOMMAND	"bootm 100000"	/* autoboot command */
#define CONFIG_BOOTARGS		"root=/dev/ram rw"

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#undef	CONFIG_KGDB_ON_SMC		/* define if kgdb on SMC */
#define CONFIG_KGDB_ON_SCC		/* define if kgdb on SCC */
#undef	CONFIG_KGDB_NONE		/* define if kgdb on something else */
#define CONFIG_KGDB_INDEX	2	/* which serial channel for kgdb */
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port at */
#endif

#undef	CONFIG_WATCHDOG			/* disable platform specific watchdog */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt   */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE	256			/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS	16			/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */
					/* for versions < 2.4.5-pre5	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CFG_FLASH_BASE		0xff800000
#define FLASH_BASE		0xff800000
#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks	*/
#define CFG_MAX_FLASH_SECT	32	/* max num of sects on one chip */
#define CFG_FLASH_SIZE		8
#define CFG_FLASH_ERASE_TOUT	8000	/* Timeout for Flash Erase (in ms)    */
#define CFG_FLASH_WRITE_TOUT	5	/* Timeout for Flash Write (in ms)    */

#undef	CFG_FLASH_CHECKSUM

/* this is stuff came out of the Motorola docs */
/* Only change this if you also change the Hardware configuration Word */
#define CFG_DEFAULT_IMMR	0x0F010000

/*
#define CFG_IMMR		0x04700000
#define CFG_BCSR		0x04500000
*/

/* Set IMMR to 0xF0000000 or above to boot Linux  */
#define CFG_IMMR		0xF0000000
#define CFG_BCSR		0x04500000

/* Define CONFIG_VERY_BIG_RAM to allow use of SDRAMs larger than 256MBytes
 */
/*#define CONFIG_VERY_BIG_RAM	1*/

/* What should be the base address of SDRAM DIMM and how big is
 * it (in Mbytes)?  This will normally auto-configure via the SPD.
*/
#define CFG_SDRAM_BASE 0x00000000
#define CFG_SDRAM_SIZE 16

#define SDRAM_SPD_ADDR 0x50


/*-----------------------------------------------------------------------
 * BR2,BR3 - Base Register
 *     Ref: Section 10.3.1 on page 10-14
 * OR2,OR3 - Option Register
 *     Ref: Section 10.3.2 on page 10-16
 *-----------------------------------------------------------------------
 */

/* Bank 2,3 - SDRAM DIMM
 */

/* The BR2 is configured as follows:
 *
 *     - Base address of 0x00000000
 *     - 64 bit port size (60x bus only)
 *     - Data errors checking is disabled
 *     - Read and write access
 *     - SDRAM 60x bus
 *     - Access are handled by the memory controller according to MSEL
 *     - Not used for atomic operations
 *     - No data pipelining is done
 *     - Valid
 */
#define CFG_BR2_PRELIM	((CFG_SDRAM_BASE & BRx_BA_MSK) |\
			 BRx_PS_64			|\
			 BRx_MS_SDRAM_P			|\
			 BRx_V)

#define CFG_BR3_PRELIM	((CFG_SDRAM_BASE & BRx_BA_MSK) |\
			 BRx_PS_64			|\
			 BRx_MS_SDRAM_P			|\
			 BRx_V)

/* With a 64 MB DIMM, the OR2 is configured as follows:
 *
 *     - 64 MB
 *     - 4 internal banks per device
 *     - Row start address bit is A8 with PSDMR[PBI] = 0
 *     - 12 row address lines
 *     - Back-to-back page mode
 *     - Internal bank interleaving within save device enabled
 */
#if (CFG_SDRAM_SIZE == 64)
#define CFG_OR2_PRELIM	(MEG_TO_AM(CFG_SDRAM_SIZE)	|\
			 ORxS_BPD_4			|\
			 ORxS_ROWST_PBI0_A8		|\
			 ORxS_NUMR_12)
#elif (CFG_SDRAM_SIZE == 16)
#define CFG_OR2_PRELIM	(0xFF000CA0)
#else
#error "INVALID SDRAM CONFIGURATION"
#endif

/*-----------------------------------------------------------------------
 * PSDMR - 60x Bus SDRAM Mode Register
 *     Ref: Section 10.3.3 on page 10-21
 *-----------------------------------------------------------------------
 */

#if (CFG_SDRAM_SIZE == 64)
/* With a 64 MB DIMM, the PSDMR is configured as follows:
 *
 *     - Bank Based Interleaving,
 *     - Refresh Enable,
 *     - Address Multiplexing where A5 is output on A14 pin
 *	 (A6 on A15, and so on),
 *     - use address pins A14-A16 as bank select,
 *     - A9 is output on SDA10 during an ACTIVATE command,
 *     - earliest timing for ACTIVATE command after REFRESH command is 7 clocks,
 *     - earliest timing for ACTIVATE or REFRESH command after PRECHARGE command
 *	 is 3 clocks,
 *     - earliest timing for READ/WRITE command after ACTIVATE command is
 *	 2 clocks,
 *     - earliest timing for PRECHARGE after last data was read is 1 clock,
 *     - earliest timing for PRECHARGE after last data was written is 1 clock,
 *     - CAS Latency is 2.
 */
#define CFG_PSDMR	(PSDMR_RFEN	      |\
			 PSDMR_SDAM_A14_IS_A5 |\
			 PSDMR_BSMA_A14_A16   |\
			 PSDMR_SDA10_PBI0_A9  |\
			 PSDMR_RFRC_7_CLK     |\
			 PSDMR_PRETOACT_3W    |\
			 PSDMR_ACTTORW_2W     |\
			 PSDMR_LDOTOPRE_1C    |\
			 PSDMR_WRC_1C	      |\
			 PSDMR_CL_2)
#elif (CFG_SDRAM_SIZE == 16)
/* With a 16 MB DIMM, the PSDMR is configured as follows:
 *
 *   configuration parameters found in Motorola documentation
 */
#define CFG_PSDMR	(0x016EB452)
#else
#error "INVALID SDRAM CONFIGURATION"
#endif


#define RS232EN_1		0x02000002
#define RS232EN_2		0x01000001
#define FETHIEN			0x08000008
#define FETH_RST		0x04000004

#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x4000	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET


/* 0x0EA28205 */
/*#define CFG_HRCW_MASTER (   ( HRCW_BPS11 | HRCW_CIP )			    |\
			    ( HRCW_L2CPC10 | HRCW_DPPC10 | HRCW_ISB010 )    |\
			    ( HRCW_BMS | HRCW_APPC10 )			    |\
			    ( HRCW_MODCK_H0101 )			     \
			)
*/

/* This value should actually be situated in the first 256 bytes of the FLASH
	which on the standard MPC8266ADS board is at address 0xFF800000
	The linker script places it at 0xFFF00000 instead.

	It still works, however, as long as the ADS board jumper JP3 is set to 
	position 2-3 so the board is using the BCSR as Hardware Configuration Word 

	If you want to use the one defined here instead, ust copy the first 256 bytes from 
	0xfff00000 to 0xff800000  (for 8MB flash) 

	- Rune

	*/
#define CFG_HRCW_MASTER 0x0cb23645

/* no slaves */
#define CFG_HRCW_SLAVE1 0
#define CFG_HRCW_SLAVE2 0
#define CFG_HRCW_SLAVE3 0
#define CFG_HRCW_SLAVE4 0
#define CFG_HRCW_SLAVE5 0
#define CFG_HRCW_SLAVE6 0
#define CFG_HRCW_SLAVE7 0

#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM	0x02	/* Software reboot	     */

#define CFG_MONITOR_BASE    TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#ifndef CFG_RAMBOOT
#  define CFG_ENV_IS_IN_FLASH	1
#    define CFG_ENV_ADDR	(CFG_MONITOR_BASE + 0x40000)
#    define CFG_ENV_SECT_SIZE	0x40000
#else
#  define CFG_ENV_IS_IN_NVRAM	1
#  define CFG_ENV_ADDR		(CFG_MONITOR_BASE - 0x1000)
#  define CFG_ENV_SIZE		0x200
#endif /* CFG_RAMBOOT */


#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPU */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif


#define CFG_HID0_INIT		0
#define CFG_HID0_FINAL		(HID0_ICE | HID0_IFEM | HID0_ABE )

#define CFG_HID2		0

#define CFG_SYPCR		0xFFFFFFC3
#define CFG_BCR			0x100C0000
#define CFG_SIUMCR		0x0A200000
#define CFG_SCCR		0x00000000
#define CFG_BR0_PRELIM		0xFF801801
#define CFG_OR0_PRELIM		0xFF800836
#define CFG_BR1_PRELIM		0x04501801
#define CFG_OR1_PRELIM		0xFFFF8010

#define CFG_RMR			0
#define CFG_TMCNTSC		(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)
#define CFG_PISCR		(PISCR_PS|PISCR_PTF|PISCR_PTE)
#define CFG_RCCR		0
/*#define CFG_PSDMR		0x016EB452*/
#define CFG_MPTPR		0x00001900
#define CFG_PSRT		0x00000021

#define CFG_RESET_ADDRESS	0x04400000

#endif /* __CONFIG_H */

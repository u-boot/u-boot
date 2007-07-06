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
 * Config header file for a MPC8266ADS Pilot 16M Ram Simm, 8Mbytes Flash Simm
 */

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   !!								      !!
   !!  This configuration requires JP3 to be in position 1-2 to work  !!
   !!  To make it work for the default, the TEXT_BASE define in	      !!
   !!  board/mpc8266ads/config.mk must be changed from 0xfe000000 to  !!
   !!  0xfff00000						      !!
   !!  The CFG_HRCW_MASTER define below must also be changed to match !!
   !!								      !!
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC8260		1	/* This is an MPC8260 CPU	*/
#define CONFIG_MPC8266ADS	1	/* ...on motorola ADS board	*/
#define CONFIG_CPM2		1	/* Has a CPM2 */

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/

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
#define CONFIG_MII			/* MII PHY management		*/
#define CONFIG_BITBANGMII		/* bit-bang MII PHY management	*/
/*
 * Port pins used for bit-banged MII communictions (if applicable).
 */
#define MDIO_PORT	2	/* Port C */
#define MDIO_ACTIVE	(iop->pdir |=  0x00400000)
#define MDIO_TRISTATE	(iop->pdir &= ~0x00400000)
#define MDIO_READ	((iop->pdat &  0x00400000) != 0)

#define MDIO(bit)	if(bit) iop->pdat |=  0x00400000; \
			else	iop->pdat &= ~0x00400000

#define MDC(bit)	if(bit) iop->pdat |=  0x00200000; \
			else	iop->pdat &= ~0x00200000

#define MIIDELAY	udelay(1)

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
# define CFG_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#endif	/* CONFIG_ETHER_INDEX */

/* other options */
#define CONFIG_HARD_I2C		1	/* To enable I2C support	*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_EEPROM_ADDR_LEN 1

/* PCI */
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_BOOTDELAY 0
#undef CONFIG_PCI_SCAN_SHOW

/*-----------------------------------------------------------------------
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 */
#define SPD_EEPROM_ADDRESS	0x50

#define CONFIG_8260_CLKIN	66000000	/* in Hz */
#define CONFIG_BAUDRATE		115200

/*
 * Command line configuration.
 */
#include <config_cmd_all.h>

#undef CONFIG_CMD_BEDBUG
#undef CONFIG_CMD_BMP
#undef CONFIG_CMD_BSP
#undef CONFIG_CMD_DATE
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_DISPLAY
#undef CONFIG_CMD_DOC
#undef CONFIG_CMD_DTT
#undef CONFIG_CMD_EEPROM
#undef CONFIG_CMD_ELF
#undef CONFIG_CMD_EXT2
#undef CONFIG_CMD_FDC
#undef CONFIG_CMD_FDOS
#undef CONFIG_CMD_HWFLOW
#undef CONFIG_CMD_IDE
#undef CONFIG_CMD_JFFS2
#undef CONFIG_CMD_KGDB
#undef CONFIG_CMD_MMC
#undef CONFIG_CMD_NAND
#undef CONFIG_CMD_PCMCIA
#undef CONFIG_CMD_REISER
#undef CONFIG_CMD_SCSI
#undef CONFIG_CMD_SPI
#undef CONFIG_CMD_SNTP
#undef CONFIG_CMD_VFD
#undef CONFIG_CMD_UNIVERSE
#undef CONFIG_CMD_USB
#undef CONFIG_CMD_XIMG

/* Define a command string that is automatically executed when no character
 * is read on the console interface withing "Boot Delay" after reset.
 */
#undef	CONFIG_BOOT_ROOT_INITRD		/* Use ram disk for the root file system */
#define CONFIG_BOOT_ROOT_NFS		/* Use a NFS mounted root file system */

#ifdef CONFIG_BOOT_ROOT_INITRD
#define CONFIG_BOOTCOMMAND \
	"version;" \
	"echo;" \
	"bootp;" \
	"setenv bootargs root=/dev/ram0 rw " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;" \
	"bootm"
#endif /* CONFIG_BOOT_ROOT_INITRD */

#ifdef CONFIG_BOOT_ROOT_NFS
#define CONFIG_BOOTCOMMAND \
	"version;" \
	"echo;" \
	"bootp;" \
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;" \
	"bootm"
#endif /* CONFIG_BOOT_ROOT_NFS */

/* Add support for a few extra bootp options like:
 *	- File size
 *	- DNS
 */
#define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAULT | \
				 CONFIG_BOOTP_BOOTFILESIZE | \
				 CONFIG_BOOTP_DNS)

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#if defined(CONFIG_CMD_KGDB)
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
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE	256			/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS	16			/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#undef CONFIG_CLOCKS_IN_MHZ		/* clocks passsed to Linux in MHz */
					/* for versions < 2.4.5-pre5	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CFG_FLASH_BASE		0xFE000000
#define FLASH_BASE		0xFE000000
#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks	*/
#define CFG_MAX_FLASH_SECT	32	/* max num of sects on one chip */
#define CFG_FLASH_SIZE		8
#define CFG_FLASH_ERASE_TOUT	8000	/* Timeout for Flash Erase (in ms)    */
#define CFG_FLASH_WRITE_TOUT	5	/* Timeout for Flash Write (in ms)    */

#undef	CFG_FLASH_CHECKSUM

/* this is stuff came out of the Motorola docs */
/* Only change this if you also change the Hardware configuration Word */
#define CFG_DEFAULT_IMMR	0x0F010000

/* Set IMMR to 0xF0000000 or above to boot Linux  */
#define CFG_IMMR		0xF0000000
#define CFG_BCSR		0xF8000000
#define CFG_PCI_INT		0xF8200000	/* PCI interrupt controller */

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
#define CFG_OR2_PRELIM	(0xFF000C80)
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

/* Use this HRCW for booting from address 0xfe00000 (JP3 in setting 1-2)  */
/* 0x0EB2B645 */
#define CFG_HRCW_MASTER (( HRCW_BPS11 | HRCW_CIP )				|\
			 ( HRCW_L2CPC10 | HRCW_DPPC11 | HRCW_ISB010 )		|\
			 ( HRCW_BMS | HRCW_MMR11 | HRCW_LBPC01 | HRCW_APPC10 )	|\
			 ( HRCW_CS10PC01 | HRCW_MODCK_H0101 )			\
			)

/* Use this HRCW for booting from address 0xfff0000 (JP3 in setting 2-3)  */
/* #define CFG_HRCW_MASTER 0x0cb23645 */

/* This value should actually be situated in the first 256 bytes of the FLASH
	which on the standard MPC8266ADS board is at address 0xFF800000
	The linker script places it at 0xFFF00000 instead.

	It still works, however, as long as the ADS board jumper JP3 is set to
	position 2-3 so the board is using the BCSR as Hardware Configuration Word

	If you want to use the one defined here instead, ust copy the first 256 bytes from
	0xfff00000 to 0xff800000  (for 8MB flash)

	- Rune

*/

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
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
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
/*#define CFG_HID0_INIT		0 */
#define CFG_HID0_INIT	(HID0_ICE  |\
			 HID0_DCE  |\
			 HID0_ICFI |\
			 HID0_DCI  |\
			 HID0_IFEM |\
			 HID0_ABE)

#define CFG_HID0_FINAL		(HID0_ICE | HID0_IFEM | HID0_ABE )

#define CFG_HID2		0

#define CFG_SYPCR		0xFFFFFFC3
#define CFG_BCR			0x004C0000
#define CFG_SIUMCR		0x4E64C000
#define CFG_SCCR		0x00000000

/*	local bus memory map
 *
 *	0x00000000-0x03FFFFFF	 64MB	SDRAM
 *	0x80000000-0x9FFFFFFF	512MB	outbound prefetchable PCI memory window
 *	0xA0000000-0xBFFFFFFF	512MB	outbound non-prefetchable PCI memory window
 *	0xF0000000-0xF001FFFF	128KB	MPC8266 internal memory
 *	0xF4000000-0xF7FFFFFF	 64MB	outbound PCI I/O window
 *	0xF8000000-0xF8007FFF	 32KB	BCSR
 *	0xF8100000-0xF8107FFF	 32KB	ATM UNI
 *	0xF8200000-0xF8207FFF	 32KB	PCI interrupt controller
 *	0xF8300000-0xF8307FFF	 32KB	EEPROM
 *	0xFE000000-0xFFFFFFFF	 32MB	flash
 */
#define CFG_BR0_PRELIM	0xFE001801		/* flash */
#define CFG_OR0_PRELIM	0xFE000836
#define CFG_BR1_PRELIM	(CFG_BCSR | 0x1801)	/* BCSR */
#define CFG_OR1_PRELIM	0xFFFF8010
#define CFG_BR4_PRELIM	0xF8300801		/* EEPROM */
#define CFG_OR4_PRELIM	0xFFFF8846
#define CFG_BR5_PRELIM	0xF8100801		/* PM5350 ATM UNI */
#define CFG_OR5_PRELIM	0xFFFF8E36
#define CFG_BR8_PRELIM	(CFG_PCI_INT | 0x1801)	/* PCI interrupt controller */
#define CFG_OR8_PRELIM	0xFFFF8010

#define CFG_RMR			0x0001
#define CFG_TMCNTSC		(TMCNTSC_SEC|TMCNTSC_ALR|TMCNTSC_TCF|TMCNTSC_TCE)
#define CFG_PISCR		(PISCR_PS|PISCR_PTF|PISCR_PTE)
#define CFG_RCCR		0
#define CFG_MPTPR		0x00001900
#define CFG_PSRT		0x00000021

/* This address must not exist */
#define CFG_RESET_ADDRESS	0xFCFFFF00

/* PCI Memory map (if different from default map */
#define CFG_PCI_SLV_MEM_LOCAL	CFG_SDRAM_BASE		/* Local base */
#define CFG_PCI_SLV_MEM_BUS		0x00000000		/* PCI base */
#define CFG_PICMR0_MASK_ATTRIB	(PICMR_MASK_512MB | PICMR_ENABLE | \
				 PICMR_PREFETCH_EN)

/*
 * These are the windows that allow the CPU to access PCI address space.
 * All three PCI master windows, which allow the CPU to access PCI
 * prefetch, non prefetch, and IO space (see below), must all fit within
 * these windows.
 */

/* PCIBR0 */
#define CFG_PCI_MSTR0_LOCAL		0x80000000		/* Local base */
#define CFG_PCIMSK0_MASK		PCIMSK_1GB		/* Size of window */
/* PCIBR1 */
#define CFG_PCI_MSTR1_LOCAL		0xF4000000		/* Local base */
#define CFG_PCIMSK1_MASK		PCIMSK_64MB		/* Size of window */

/*
 * Master window that allows the CPU to access PCI Memory (prefetch).
 * This window will be setup with the first set of Outbound ATU registers
 * in the bridge.
 */

#define CFG_PCI_MSTR_MEM_LOCAL	0x80000000			/* Local base */
#define CFG_PCI_MSTR_MEM_BUS	0x80000000			/* PCI base   */
#define CFG_CPU_PCI_MEM_START	PCI_MSTR_MEM_LOCAL
#define CFG_PCI_MSTR_MEM_SIZE	0x20000000			/* 512MB */
#define CFG_POCMR0_MASK_ATTRIB	(POCMR_MASK_512MB | POCMR_ENABLE | POCMR_PREFETCH_EN)

/*
 * Master window that allows the CPU to access PCI Memory (non-prefetch).
 * This window will be setup with the second set of Outbound ATU registers
 * in the bridge.
 */

#define CFG_PCI_MSTR_MEMIO_LOCAL    0xA0000000			/* Local base */
#define CFG_PCI_MSTR_MEMIO_BUS	    0xA0000000			/* PCI base   */
#define CFG_CPU_PCI_MEMIO_START	    PCI_MSTR_MEMIO_LOCAL
#define CFG_PCI_MSTR_MEMIO_SIZE	    0x20000000			/* 512MB */
#define CFG_POCMR1_MASK_ATTRIB	    (POCMR_MASK_512MB | POCMR_ENABLE)

/*
 * Master window that allows the CPU to access PCI IO space.
 * This window will be setup with the third set of Outbound ATU registers
 * in the bridge.
 */

#define CFG_PCI_MSTR_IO_LOCAL	    0xF4000000			/* Local base */
#define CFG_PCI_MSTR_IO_BUS	    0xF4000000			/* PCI base   */
#define CFG_CPU_PCI_IO_START	    PCI_MSTR_IO_LOCAL
#define CFG_PCI_MSTR_IO_SIZE	    0x04000000			/* 64MB */
#define CFG_POCMR2_MASK_ATTRIB	    (POCMR_MASK_64MB | POCMR_ENABLE | POCMR_PCI_IO)

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition, whole device */
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0xFFFFFFFF
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		""
#define MTDPARTS_DEFAULT	""
*/

#endif /* __CONFIG_H */

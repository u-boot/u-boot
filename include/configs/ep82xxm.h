/*
 * Copyright (C) 2006 Embedded Planet, LLC.
 *
 * U-Boot configuration for Embedded Planet EP82xxM boards.
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

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MPC8260
#define CPU_ID_STR		"MPC8270"

#define CONFIG_EP82XXM	/* Embedded Planet EP82xxM H 1.0 board */
			/* 256MB SDRAM / 64MB FLASH */

#undef DEBUG

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f */

/* Allow serial number (serial#) and MAC address (ethaddr) to be overwritten */
#define CONFIG_ENV_OVERWRITE

/*
 * Select serial console configuration
 *
 * If either CONFIG_CONS_ON_SMC or CONFIG_CONS_ON_SCC is selected, then
 * CONFIG_CONS_INDEX must be set to the channel number (1-2 for SMC, 1-4
 * for SCC).
 */
#define	CONFIG_CONS_ON_SMC		/* Console is on SMC         */
#undef  CONFIG_CONS_ON_SCC		/* It's not on SCC           */
#undef	CONFIG_CONS_NONE		/* It's not on external UART */
#define CONFIG_CONS_INDEX	1	/* SMC1 is used for console  */

#define CFG_BCSR		0xFA000000

/*
 * Select ethernet configuration
 *
 * If either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected,
 * then CONFIG_ETHER_INDEX must be set to the channel number (1-4 for
 * SCC, 1-3 for FCC)
 *
 * If CONFIG_ETHER_NONE is defined, then either the ethernet routines
 * must be defined elsewhere (as for the console), or CFG_CMD_NET must
 * be removed from CONFIG_COMMANDS to remove support for networking.
 */
#undef	CONFIG_ETHER_ON_SCC		/* Ethernet is not on SCC */
#define CONFIG_ETHER_ON_FCC		/* Ethernet is on FCC     */
#undef	CONFIG_ETHER_NONE		/* No external Ethernet   */

#define CONFIG_NET_MULTI

#define CONFIG_ETHER_ON_FCC2
#define CONFIG_ETHER_ON_FCC3

#define CFG_CMXFCR_MASK3	(CMXFCR_FC3 | CMXFCR_RF3CS_MSK | CMXFCR_TF3CS_MSK)
#define CFG_CMXFCR_VALUE3	(CMXFCR_RF3CS_CLK15 | CMXFCR_TF3CS_CLK16)
#define CFG_CMXFCR_MASK2	(CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)
#define CFG_CMXFCR_VALUE2	(CMXFCR_RF2CS_CLK13 | CMXFCR_TF2CS_CLK14)

#define CFG_CPMFCR_RAMTYPE	0
#define CFG_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#define CONFIG_MII			/* MII PHY management        */
#define CONFIG_BITBANGMII		/* Bit-banged MDIO interface */

/*
 * GPIO pins used for bit-banged MII communications
 */
#define MDIO_PORT		0	/* Not used - implemented in BCSR */
#define MDIO_ACTIVE		(*(vu_char *)(CFG_BCSR + 8) &= 0xFB)
#define MDIO_TRISTATE		(*(vu_char *)(CFG_BCSR + 8) |= 0x04)
#define MDIO_READ		(*(vu_char *)(CFG_BCSR + 8) & 1)

#define MDIO(bit)		if(bit) *(vu_char *)(CFG_BCSR + 8) |= 0x01; \
				else	*(vu_char *)(CFG_BCSR + 8) &= 0xFE

#define MDC(bit)		if(bit) *(vu_char *)(CFG_BCSR + 8) |= 0x02; \
				else	*(vu_char *)(CFG_BCSR + 8) &= 0xFD

#define MIIDELAY		udelay(1)


#ifndef CONFIG_8260_CLKIN
#define CONFIG_8260_CLKIN	66000000 /* in Hz */
#endif

#define CONFIG_BAUDRATE		115200

#define CFG_VXWORKS_MAC_PTR 0x4300 /* Pass Ethernet MAC to VxWorks */

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL   \
				| CFG_CMD_DHCP    \
				| CFG_CMD_ECHO    \
				| CFG_CMD_I2C     \
				| CFG_CMD_IMMAP   \
				| CFG_CMD_MII     \
				| CFG_CMD_PING    \
				| CFG_CMD_DATE    \
				| CFG_CMD_DTT	  \
				| CFG_CMD_EEPROM  \
				| CFG_CMD_PCI	  \
				| CFG_CMD_DIAG	  \
				)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CONFIG_ETHADDR		00:10:EC:00:88:65
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		00:10:EC:80:88:65
#define CONFIG_IPADDR		10.0.0.245
#define CONFIG_HOSTNAME		EP82xxM
#define CONFIG_SERVERIP		10.0.0.26
#define CONFIG_GATEWAYIP	10.0.0.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#define CFG_ENV_IN_OWN_SECT	1
#define CONFIG_AUTO_COMPLETE	1
#define	CONFIG_EXTRA_ENV_SETTINGS	"ethprime=FCC3 ETHERNET"

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#undef	CONFIG_KGDB_ON_SMC		/* define if kgdb on SMC */
#define CONFIG_KGDB_ON_SCC		/* define if kgdb on SCC */
#undef	CONFIG_KGDB_NONE		/* define if kgdb on something else */
#define CONFIG_KGDB_INDEX	1	/* which serial channel for kgdb */
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port at */
#endif

#define CONFIG_BZIP2	/* include support for bzip2 compressed images */
#undef	CONFIG_WATCHDOG			/* disable platform specific watchdog */

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT		"ep82xxm=> "	/* Monitor Command Prompt   */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size  */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size  */

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
/*
 * Define here the location of the environment variables (FLASH or EEPROM).
 * Note: DENX encourages to use redundant environment in FLASH.
 */
#if 1
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#else
#define CFG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars	*/
#endif

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_BASE		0xFC000000
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_MAX_FLASH_BANKS	1	/* max num of flash banks	*/
#define CFG_MAX_FLASH_SECT	512	/* max num of sects on one chip */
#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector in flinfo */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#endif /* CFG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
/* EEPROM Configuration */
#define CFG_EEPROM_SIZE	0x1000
#define CFG_I2C_EEPROM_ADDR	0x54
#define CFG_I2C_EEPROM_ADDR_LEN	1
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10

#ifdef CFG_ENV_IS_IN_EEPROM
#define CFG_ENV_SIZE		0x200	    /* Size of Environment vars */
#define CFG_ENV_OFFSET		0x0
#endif /* CFG_ENV_IS_IN_EEPROM */

/* RTC Configuration */
#define CONFIG_RTC_M41T11	1 	/* uses a M41T81 */
#define CFG_I2C_RTC_ADDR	0x68
#define CONFIG_M41T11_BASE_YEAR	1900

/* I2C SYSMON (LM75) */
#define CONFIG_DTT_LM75		1
#define CONFIG_DTT_SENSORS	{0}
#define CFG_DTT_MAX_TEMP	70
#define CFG_DTT_LOW_TEMP	-30
#define	CFG_DTT_HYSTERESIS	3

/*-----------------------------------------------------------------------
 * NVRAM Configuration
 *-----------------------------------------------------------------------
 */
#define CFG_NVRAM_BASE_ADDR	0xFA080000
#define CFG_NVRAM_SIZE		(128*1024)-16


/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_PNP			/* do pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */
#define CONFIG_PCI_BOOTDELAY	0

/* PCI Memory map (if different from default map */
#define CFG_PCI_SLV_MEM_LOCAL	CFG_SDRAM_BASE		/* Local base */
#define CFG_PCI_SLV_MEM_BUS		0x00000000	/* PCI base */
#define CFG_PICMR0_MASK_ATTRIB	(PICMR_MASK_512MB | PICMR_ENABLE | \
				 PICMR_PREFETCH_EN)

/*
 * These are the windows that allow the CPU to access PCI address space.
 * All three PCI master windows, which allow the CPU to access PCI
 * prefetch, non prefetch, and IO space (see below), must all fit within
 * these windows.
 */

/*
 * Master window that allows the CPU to access PCI Memory (prefetch).
 * This window will be setup with the second set of Outbound ATU registers
 * in the bridge.
 */

#define CFG_PCI_MSTR_MEM_LOCAL	0x80000000          /* Local base */
#define CFG_PCI_MSTR_MEM_BUS	0x80000000          /* PCI base   */
#define	CFG_CPU_PCI_MEM_START	PCI_MSTR_MEM_LOCAL
#define CFG_PCI_MSTR_MEM_SIZE	0x20000000          /* 512MB */
#define CFG_POCMR0_MASK_ATTRIB	(POCMR_MASK_512MB | POCMR_ENABLE | POCMR_PREFETCH_EN)

/*
 * Master window that allows the CPU to access PCI Memory (non-prefetch).
 * This window will be setup with the second set of Outbound ATU registers
 * in the bridge.
 */

#define CFG_PCI_MSTR_MEMIO_LOCAL    0xA0000000          /* Local base */
#define CFG_PCI_MSTR_MEMIO_BUS      0xA0000000          /* PCI base   */
#define CFG_CPU_PCI_MEMIO_START     PCI_MSTR_MEMIO_LOCAL
#define CFG_PCI_MSTR_MEMIO_SIZE     0x20000000          /* 512MB */
#define CFG_POCMR1_MASK_ATTRIB      (POCMR_MASK_512MB | POCMR_ENABLE)

/*
 * Master window that allows the CPU to access PCI IO space.
 * This window will be setup with the first set of Outbound ATU registers
 * in the bridge.
 */

#define CFG_PCI_MSTR_IO_LOCAL       0xF6000000          /* Local base */
#define CFG_PCI_MSTR_IO_BUS         0x00000000          /* PCI base   */
#define CFG_CPU_PCI_IO_START        PCI_MSTR_IO_LOCAL
#define CFG_PCI_MSTR_IO_SIZE        0x02000000          /* 64MB */
#define CFG_POCMR2_MASK_ATTRIB      (POCMR_MASK_32MB | POCMR_ENABLE | POCMR_PCI_IO)


/* PCIBR0 - for PCI IO*/
#define CFG_PCI_MSTR0_LOCAL		CFG_PCI_MSTR_IO_LOCAL		/* Local base */
#define CFG_PCIMSK0_MASK		~(CFG_PCI_MSTR_IO_SIZE - 1U)	/* Size of window */
/* PCIBR1 - prefetch and non-prefetch regions joined together */
#define CFG_PCI_MSTR1_LOCAL		CFG_PCI_MSTR_MEM_LOCAL
#define CFG_PCIMSK1_MASK		~(CFG_PCI_MSTR_MEM_SIZE + CFG_PCI_MSTR_MEMIO_SIZE - 1U)


#define	CFG_DIRECT_FLASH_TFTP

#if (CONFIG_COMMANDS & CFG_CMD_JFFS2)
#define CFG_JFFS2_FIRST_BANK	0
#define CFG_JFFS2_NUM_BANKS	CFG_MAX_FLASH_BANKS
#define CFG_JFFS2_FIRST_SECTOR  0
#define CFG_JFFS2_LAST_SECTOR   62
#define CFG_JFFS2_SORT_FRAGMENTS
#define CFG_JFFS_CUSTOM_PART
#endif /* CFG_CMD_JFFS2 */

#if (CONFIG_COMMANDS & CFG_CMD_I2C)
#define CONFIG_HARD_I2C		1	/* To enable I2C support	*/
#define CFG_I2C_SPEED		100000	/* I2C speed			*/
#define CFG_I2C_SLAVE		0x7F	/* I2C slave address		*/
#endif /* CFG_CMD_I2C */

#define CFG_MONITOR_BASE	TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#define CFG_RAMBOOT
#endif

#define CFG_MONITOR_LEN		(512 << 10)	/* Reserve 256KB for Monitor */

#define CFG_DEFAULT_IMMR	0x00010000
#define CFG_IMMR		0xF0000000

#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define CFG_INIT_RAM_END	0x2000	/* End of used area in DPRAM	*/
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET


/* Hard reset configuration word */
#define CFG_HRCW_MASTER		0 /*0x1C800641*/  /* Not used - provided by CPLD */
/* No slaves */
#define CFG_HRCW_SLAVE1 	0
#define CFG_HRCW_SLAVE2 	0
#define CFG_HRCW_SLAVE3 	0
#define CFG_HRCW_SLAVE4 	0
#define CFG_HRCW_SLAVE5 	0
#define CFG_HRCW_SLAVE6 	0
#define CFG_HRCW_SLAVE7 	0

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM		0x02	/* Software reboot                  */

#define CFG_MALLOC_LEN		(4096 << 10)	/* Reserve 4 MB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CFG_CACHELINE_SIZE	32	/* For MPC8260 CPUs */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

#define CFG_HID0_INIT		0
#define CFG_HID0_FINAL		0

#define CFG_HID2		0

#define CFG_SIUMCR		0x02610000
#define CFG_SYPCR		0xFFFF0689
#define CFG_BCR			0x8080E000
#define CFG_SCCR		0x00000001

#define CFG_RMR			0
#define CFG_TMCNTSC		0x000000C3
#define CFG_PISCR		0x00000083
#define CFG_RCCR		0

#define CFG_MPTPR		0x0A00
#define CFG_PSDMR		0xC432246E
#define CFG_PSRT		0x32

#define CFG_SDRAM_BASE		0x00000000
#define CFG_SDRAM_BR		(CFG_SDRAM_BASE | 0x00000041)
#define CFG_SDRAM_OR		0xF0002900

#define CFG_BR0_PRELIM		(CFG_FLASH_BASE | 0x00001801)
#define CFG_OR0_PRELIM		0xFC000882
#define CFG_BR4_PRELIM		(CFG_BCSR | 0x00001001)
#define CFG_OR4_PRELIM		0xFFF00050

#define CFG_RESET_ADDRESS	0xFFF00100

#endif /* __CONFIG_H */

/*
 * Copyright (C) 2006 Embedded Planet, LLC.
 *
 * U-Boot configuration for Embedded Planet EP82xxM boards.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MPC8260
#define CPU_ID_STR		"MPC8270"

#define CONFIG_EP82XXM	/* Embedded Planet EP82xxM H 1.0 board */
			/* 256MB SDRAM / 64MB FLASH */

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

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

#define CONFIG_SYS_BCSR		0xFA000000

/*
 * Select ethernet configuration
 *
 * If either CONFIG_ETHER_ON_SCC or CONFIG_ETHER_ON_FCC is selected,
 * then CONFIG_ETHER_INDEX must be set to the channel number (1-4 for
 * SCC, 1-3 for FCC)
 *
 * If CONFIG_ETHER_NONE is defined, then either the ethernet routines
 * must be defined elsewhere (as for the console), or CONFIG_CMD_NET
 * must be unset.
 */
#undef	CONFIG_ETHER_ON_SCC		/* Ethernet is not on SCC */
#define CONFIG_ETHER_ON_FCC		/* Ethernet is on FCC     */
#undef	CONFIG_ETHER_NONE		/* No external Ethernet   */


#define CONFIG_ETHER_ON_FCC2
#define CONFIG_ETHER_ON_FCC3

#define CONFIG_SYS_CMXFCR_MASK3	(CMXFCR_FC3 | CMXFCR_RF3CS_MSK | CMXFCR_TF3CS_MSK)
#define CONFIG_SYS_CMXFCR_VALUE3	(CMXFCR_RF3CS_CLK15 | CMXFCR_TF3CS_CLK16)
#define CONFIG_SYS_CMXFCR_MASK2	(CMXFCR_FC2 | CMXFCR_RF2CS_MSK | CMXFCR_TF2CS_MSK)
#define CONFIG_SYS_CMXFCR_VALUE2	(CMXFCR_RF2CS_CLK13 | CMXFCR_TF2CS_CLK14)

#define CONFIG_SYS_CPMFCR_RAMTYPE	0
#define CONFIG_SYS_FCC_PSMR		(FCC_PSMR_FDE | FCC_PSMR_LPB)

#define CONFIG_MII			/* MII PHY management        */
#define CONFIG_BITBANGMII		/* Bit-banged MDIO interface */

/*
 * GPIO pins used for bit-banged MII communications
 */
#define MDIO_PORT		0	/* Not used - implemented in BCSR */

#define MDIO_ACTIVE		(*(vu_char *)(CONFIG_SYS_BCSR + 8) &= 0xFB)
#define MDIO_TRISTATE		(*(vu_char *)(CONFIG_SYS_BCSR + 8) |= 0x04)
#define MDIO_READ		(*(vu_char *)(CONFIG_SYS_BCSR + 8) & 1)

#define MDIO(bit)		if(bit) *(vu_char *)(CONFIG_SYS_BCSR + 8) |= 0x01; \
				else	*(vu_char *)(CONFIG_SYS_BCSR + 8) &= 0xFE

#define MDC(bit)		if(bit) *(vu_char *)(CONFIG_SYS_BCSR + 8) |= 0x02; \
				else	*(vu_char *)(CONFIG_SYS_BCSR + 8) &= 0xFD

#define MIIDELAY		udelay(1)


#ifndef CONFIG_8260_CLKIN
#define CONFIG_8260_CLKIN	66000000 /* in Hz */
#endif

#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_VXWORKS_MAC_PTR 0x4300 /* Pass Ethernet MAC to VxWorks */


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


#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_PCI
#define CONFIG_CMD_DIAG


#define CONFIG_ETHADDR		00:10:EC:00:88:65
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		00:10:EC:80:88:65
#define CONFIG_IPADDR		10.0.0.245
#define CONFIG_HOSTNAME		EP82xxM
#define CONFIG_SERVERIP		10.0.0.26
#define CONFIG_GATEWAYIP	10.0.0.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */
#define CONFIG_ENV_IN_OWN_SECT	1
#define CONFIG_AUTO_COMPLETE	1
#define	CONFIG_EXTRA_ENV_SETTINGS	"ethprime=FCC3"

#if defined(CONFIG_CMD_KGDB)
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
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_PROMPT		"ep82xxm=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size  */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size  */

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
/*
 * Define here the location of the environment variables (FLASH or EEPROM).
 * Note: DENX encourages to use redundant environment in FLASH.
 */
#if 1
#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#else
#define CONFIG_ENV_IS_IN_EEPROM	1	/* use EEPROM for environment vars	*/
#endif

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_BASE		0xFC000000
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of flash banks	*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max num of sects on one chip */
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector in flinfo */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN)
#endif /* CONFIG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
/* EEPROM Configuration */
#define CONFIG_SYS_EEPROM_SIZE	0x1000
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x54
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

#ifdef CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_SIZE		0x200	    /* Size of Environment vars */
#define CONFIG_ENV_OFFSET		0x0
#endif /* CONFIG_ENV_IS_IN_EEPROM */

/* RTC Configuration */
#define CONFIG_RTC_M41T11	1	/* uses a M41T81 */
#define CONFIG_SYS_I2C_RTC_ADDR	0x68
#define CONFIG_M41T11_BASE_YEAR	1900

/* I2C SYSMON (LM75) */
#define CONFIG_DTT_LM75		1
#define CONFIG_DTT_SENSORS	{0}
#define CONFIG_SYS_DTT_MAX_TEMP	70
#define CONFIG_SYS_DTT_LOW_TEMP	-30
#define	CONFIG_SYS_DTT_HYSTERESIS	3

/*-----------------------------------------------------------------------
 * NVRAM Configuration
 *-----------------------------------------------------------------------
 */
#define CONFIG_SYS_NVRAM_BASE_ADDR	0xFA080000
#define CONFIG_SYS_NVRAM_SIZE		(128*1024)-16


/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_PNP			/* do pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */
#define CONFIG_PCI_BOOTDELAY	0

/* PCI Memory map (if different from default map */
#define CONFIG_SYS_PCI_SLV_MEM_LOCAL	CONFIG_SYS_SDRAM_BASE		/* Local base */
#define CONFIG_SYS_PCI_SLV_MEM_BUS		0x00000000	/* PCI base */
#define CONFIG_SYS_PICMR0_MASK_ATTRIB	(PICMR_MASK_512MB | PICMR_ENABLE | \
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

#define CONFIG_SYS_PCI_MSTR_MEM_LOCAL	0x80000000          /* Local base */
#define CONFIG_SYS_PCI_MSTR_MEM_BUS	0x80000000          /* PCI base   */
#define	CONFIG_SYS_CPU_PCI_MEM_START	PCI_MSTR_MEM_LOCAL
#define CONFIG_SYS_PCI_MSTR_MEM_SIZE	0x20000000          /* 512MB */
#define CONFIG_SYS_POCMR0_MASK_ATTRIB	(POCMR_MASK_512MB | POCMR_ENABLE | POCMR_PREFETCH_EN)

/*
 * Master window that allows the CPU to access PCI Memory (non-prefetch).
 * This window will be setup with the second set of Outbound ATU registers
 * in the bridge.
 */

#define CONFIG_SYS_PCI_MSTR_MEMIO_LOCAL    0xA0000000          /* Local base */
#define CONFIG_SYS_PCI_MSTR_MEMIO_BUS      0xA0000000          /* PCI base   */
#define CONFIG_SYS_CPU_PCI_MEMIO_START     PCI_MSTR_MEMIO_LOCAL
#define CONFIG_SYS_PCI_MSTR_MEMIO_SIZE     0x20000000          /* 512MB */
#define CONFIG_SYS_POCMR1_MASK_ATTRIB      (POCMR_MASK_512MB | POCMR_ENABLE)

/*
 * Master window that allows the CPU to access PCI IO space.
 * This window will be setup with the first set of Outbound ATU registers
 * in the bridge.
 */

#define CONFIG_SYS_PCI_MSTR_IO_LOCAL       0xF6000000          /* Local base */
#define CONFIG_SYS_PCI_MSTR_IO_BUS         0x00000000          /* PCI base   */
#define CONFIG_SYS_CPU_PCI_IO_START        PCI_MSTR_IO_LOCAL
#define CONFIG_SYS_PCI_MSTR_IO_SIZE        0x02000000          /* 64MB */
#define CONFIG_SYS_POCMR2_MASK_ATTRIB      (POCMR_MASK_32MB | POCMR_ENABLE | POCMR_PCI_IO)


/* PCIBR0 - for PCI IO*/
#define CONFIG_SYS_PCI_MSTR0_LOCAL		CONFIG_SYS_PCI_MSTR_IO_LOCAL		/* Local base */
#define CONFIG_SYS_PCIMSK0_MASK		~(CONFIG_SYS_PCI_MSTR_IO_SIZE - 1U)	/* Size of window */
/* PCIBR1 - prefetch and non-prefetch regions joined together */
#define CONFIG_SYS_PCI_MSTR1_LOCAL		CONFIG_SYS_PCI_MSTR_MEM_LOCAL
#define CONFIG_SYS_PCIMSK1_MASK		~(CONFIG_SYS_PCI_MSTR_MEM_SIZE + CONFIG_SYS_PCI_MSTR_MEMIO_SIZE - 1U)


#define	CONFIG_SYS_DIRECT_FLASH_TFTP

#if defined(CONFIG_CMD_JFFS2)
#define CONFIG_SYS_JFFS2_FIRST_BANK	0
#define CONFIG_SYS_JFFS2_NUM_BANKS	CONFIG_SYS_MAX_FLASH_BANKS
#define CONFIG_SYS_JFFS2_FIRST_SECTOR  0
#define CONFIG_SYS_JFFS2_LAST_SECTOR   62
#define CONFIG_SYS_JFFS2_SORT_FRAGMENTS
#define CONFIG_SYS_JFFS_CUSTOM_PART
#endif

#if defined(CONFIG_CMD_I2C)
#define CONFIG_HARD_I2C		1	/* To enable I2C support	*/
#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed			*/
#define CONFIG_SYS_I2C_SLAVE		0x7F	/* I2C slave address		*/
#endif

#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#define CONFIG_SYS_RAMBOOT
#endif

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)	/* Reserve 256KB for Monitor */

#define CONFIG_SYS_DEFAULT_IMMR	0x00010000
#define CONFIG_SYS_IMMR		0xF0000000

#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_IMMR
#define CONFIG_SYS_INIT_RAM_SIZE	0x2000	/* Size of used area in DPRAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET


/* Hard reset configuration word */
#define CONFIG_SYS_HRCW_MASTER		0 /*0x1C800641*/  /* Not used - provided by CPLD */
/* No slaves */
#define CONFIG_SYS_HRCW_SLAVE1		0
#define CONFIG_SYS_HRCW_SLAVE2		0
#define CONFIG_SYS_HRCW_SLAVE3		0
#define CONFIG_SYS_HRCW_SLAVE4		0
#define CONFIG_SYS_HRCW_SLAVE5		0
#define CONFIG_SYS_HRCW_SLAVE6		0
#define CONFIG_SYS_HRCW_SLAVE7		0

#define CONFIG_SYS_MALLOC_LEN		(4096 << 10)	/* Reserve 4 MB for malloc()	*/
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC8260 CPUs */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

#define CONFIG_SYS_HID0_INIT		0
#define CONFIG_SYS_HID0_FINAL		0

#define CONFIG_SYS_HID2		0

#define CONFIG_SYS_SIUMCR		0x02610000
#define CONFIG_SYS_SYPCR		0xFFFF0689
#define CONFIG_SYS_BCR			0x8080E000
#define CONFIG_SYS_SCCR		0x00000001

#define CONFIG_SYS_RMR			0
#define CONFIG_SYS_TMCNTSC		0x000000C3
#define CONFIG_SYS_PISCR		0x00000083
#define CONFIG_SYS_RCCR		0

#define CONFIG_SYS_MPTPR		0x0A00
#define CONFIG_SYS_PSDMR		0xC432246E
#define CONFIG_SYS_PSRT		0x32

#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_SDRAM_BR		(CONFIG_SYS_SDRAM_BASE | 0x00000041)
#define CONFIG_SYS_SDRAM_OR		0xF0002900

#define CONFIG_SYS_BR0_PRELIM		(CONFIG_SYS_FLASH_BASE | 0x00001801)
#define CONFIG_SYS_OR0_PRELIM		0xFC000882
#define CONFIG_SYS_BR4_PRELIM		(CONFIG_SYS_BCSR | 0x00001001)
#define CONFIG_SYS_OR4_PRELIM		0xFFF00050

#define CONFIG_SYS_RESET_ADDRESS	0xFFF00100

#endif /* __CONFIG_H */

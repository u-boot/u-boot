/*
 * Configuation settings for the Freescale MCF54455 EVB board.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
 * board/config.h - configuration options, board specific
 */

#ifndef _JAMICA54455_H
#define _JAMICA54455_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MCF5445x		/* define processor family */
#define CONFIG_M54455		/* define processor type */
#define CONFIG_M54455EVB	/* M54455EVB board */

#undef DEBUG

#define CONFIG_MCFUART
#define CFG_UART_PORT		(0)
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600 , 19200 , 38400 , 57600, 115200 }

#undef CONFIG_WATCHDOG

#define CONFIG_TIMESTAMP	/* Print image info with timestamp */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/* Command line configuration */
#include <config_cmd_default.h>

#define CONFIG_CMD_BOOTD
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_MISC
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO

#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS

/* Network configuration */
#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
#	define CONFIG_NET_MULTI	1
#	define CONFIG_MII		1
#	define CONFIG_CF_DOMII
#	define CFG_DISCOVER_PHY
#	define CFG_RX_ETH_BUFFER	8
#	define CFG_FAULT_ECHO_LINK_DOWN

#	define CFG_FEC0_PINMUX	0
#	define CFG_FEC1_PINMUX	0
#	define CFG_FEC0_MIIBASE	CFG_FEC0_IOBASE
#	define CFG_FEC1_MIIBASE	CFG_FEC0_IOBASE
#	define MCFFEC_TOUT_LOOP 50000
#	define CONFIG_HAS_ETH1

#	define CONFIG_BOOTDELAY	1	/* autoboot after 5 seconds */
#	define CONFIG_BOOTARGS		"root=/dev/mtdblock1 rw rootfstype=jffs2 ip=none mtdparts=physmap-flash.0:5M(kernel)ro,-(jffs2)"
#	define CONFIG_ETHADDR		00:e0:0c:bc:e5:60
#	define CONFIG_ETH1ADDR		00:e0:0c:bc:e5:61
#	define CONFIG_ETHPRIME		"FEC0"
#	define CONFIG_IPADDR		192.162.1.2
#	define CONFIG_NETMASK		255.255.255.0
#	define CONFIG_SERVERIP		192.162.1.1
#	define CONFIG_GATEWAYIP		192.162.1.1
#	define CONFIG_OVERWRITE_ETHADDR_ONCE

/* If CFG_DISCOVER_PHY is not defined - hardcoded */
#	ifndef CFG_DISCOVER_PHY
#		define FECDUPLEX	FULL
#		define FECSPEED		_100BASET
#	else
#		ifndef CFG_FAULT_ECHO_LINK_DOWN
#			define CFG_FAULT_ECHO_LINK_DOWN
#		endif
#	endif			/* CFG_DISCOVER_PHY */
#endif

#define CONFIG_HOSTNAME		M54455EVB
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"inpclk=" MK_STR(CFG_INPUT_CLKSRC) "\0"	\
	"loadaddr=40010000\0"			\
	"u-boot=u-boot.bin\0"			\
	"load=tftp ${loadaddr) ${u-boot}\0"	\
	"upd=run load; run prog\0"		\
	"prog=prot off 0 2ffff;"		\
	"era 0 2ffff;"				\
	"cp.b ${loadaddr} 0 ${filesize};"	\
	"save\0"				\
	""

/* ATA configuration */
#define CONFIG_ISO_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_IDE_RESET	1
#define CONFIG_IDE_PREINIT	1
#define CONFIG_ATAPI
#undef CONFIG_LBA48

#define CFG_IDE_MAXBUS		1
#define CFG_IDE_MAXDEVICE	2

#define CFG_ATA_BASE_ADDR	0x90000000
#define CFG_ATA_IDE0_OFFSET	0

#define CFG_ATA_DATA_OFFSET	0xA0	/* Offset for data I/O                            */
#define CFG_ATA_REG_OFFSET	0xA0	/* Offset for normal register accesses */
#define CFG_ATA_ALT_OFFSET	0xC0	/* Offset for alternate registers           */
#define CFG_ATA_STRIDE		4	/* Interval between registers                 */
#define _IO_BASE		0

/* Realtime clock */
#define CONFIG_MCFRTC
#undef RTC_DEBUG
#define CFG_RTC_OSCILLATOR	(32 * CFG_HZ)

/* Timer */
#define CONFIG_MCFTMR
#undef CONFIG_MCFPIT

/* I2c */
#define CONFIG_FSL_I2C
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged               */
#define CFG_I2C_SPEED		80000	/* I2C speed and slave address  */
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_OFFSET		0x58000
#define CFG_IMMR		CFG_MBAR

/* PCI */
#define CONFIG_PCI		1

#define CFG_PCI_MEM_BUS		0xA0000000
#define CFG_PCI_MEM_PHYS	CFG_PCI_MEM_BUS
#define CFG_PCI_MEM_SIZE	0x10000000

#define CFG_PCI_IO_BUS		0xB1000000
#define CFG_PCI_IO_PHYS		CFG_PCI_IO_BUS
#define CFG_PCI_IO_SIZE		0x01000000

#define CFG_PCI_CFG_BUS		0xB0000000
#define CFG_PCI_CFG_PHYS	CFG_PCI_CFG_BUS
#define CFG_PCI_CFG_SIZE	0x01000000

/* FPGA - Spartan 2 */
/* experiment
#define CONFIG_FPGA		CFG_SPARTAN3
#define CONFIG_FPGA_COUNT	1
#define CFG_FPGA_PROG_FEEDBACK
#define CFG_FPGA_CHECK_CTRLC
*/

/* Input, PCI, Flexbus, and VCO */
#define CONFIG_EXTRA_CLOCK

#define CONFIG_PRAM		512	/* 512 KB */

#define CFG_PROMPT		"-> "
#define CFG_LONGHELP		/* undef to save memory */

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE			1024	/* Console I/O Buffer Size */
#else
#define CFG_CBSIZE			256	/* Console I/O Buffer Size */
#endif
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16	/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size    */

#define CFG_LOAD_ADDR		(CFG_SDRAM_BASE + 0x10000)

#define CFG_HZ			1000

#define CFG_MBAR		0xFC000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	0x80000000
#define CFG_INIT_RAM_END	0x8000	/* End of used area in internal SRAM */
#define CFG_INIT_RAM_CTRL	0x221
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	((CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE) - 16)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x40000000
#define CFG_SDRAM_BASE1		0x48000000
#define CFG_SDRAM_SIZE		256	/* SDRAM size in MB */
#define CFG_SDRAM_CFG1		0x65311610
#define CFG_SDRAM_CFG2		0x59670000
#define CFG_SDRAM_CTRL		0xEA0B2000
#define CFG_SDRAM_EMOD		0x40010000
#define CFG_SDRAM_MODE		0x00010033

#define CFG_MEMTEST_START	CFG_SDRAM_BASE + 0x400
#define CFG_MEMTEST_END		((CFG_SDRAM_SIZE - 3) << 20)

#define CFG_MONITOR_BASE	(CFG_FLASH_BASE + 0x400)
#define CFG_BOOTPARAMS_LEN	64*1024
#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc() */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
/* Initial Memory map for Linux */
#define CFG_BOOTMAPSZ		(CFG_SDRAM_BASE + (CFG_SDRAM_SIZE << 20))

/* Configuration for environment
 * Environment is embedded in u-boot in the second sector of the flash
 */
#define CFG_ENV_OFFSET		0x4000
#define CFG_ENV_SECT_SIZE	0x2000
#define CFG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OVERWRITE	1
#undef CFG_ENV_IS_EMBEDDED

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#ifdef CFG_ATMEL_BOOT
#	define CFG_FLASH_BASE		0
#	define CFG_FLASH0_BASE		CFG_CS0_BASE
#	define CFG_FLASH1_BASE		CFG_CS1_BASE
#else
#	define CFG_FLASH_BASE		CFG_FLASH0_BASE
#	define CFG_FLASH0_BASE		CFG_CS1_BASE
#	define CFG_FLASH1_BASE		CFG_CS0_BASE
#endif

/* M54455EVB has one non CFI flash, defined CFG_FLASH_CFI will cause the system
/* M54455EVB has one non CFI flash, defined CFG_FLASH_CFI will cause the system
   keep reset. */
#undef CFG_FLASH_CFI
#ifdef CFG_FLASH_CFI

#	define CFG_FLASH_CFI_DRIVER	1
#	define CFG_FLASH_SIZE		0x1000000	/* Max size that the board might have */
#	define CFG_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#	define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks */
#	define CFG_MAX_FLASH_SECT	137	/* max number of sectors on one chip */
#	define CFG_FLASH_PROTECTION	/* "Real" (hardware) sectors protection */
#	define CFG_FLASH_CHECKSUM
#	define CFG_FLASH_BANKS_LIST	{ CFG_CS0_BASE, CFG_CS1_BASE }

#else

#	define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks */

#	define CFG_ATMEL_REGION		4
#	define CFG_ATMEL_TOTALSECT	11
#	define CFG_ATMEL_SECT		{1, 2, 1, 7}
#	define CFG_ATMEL_SECTSZ		{0x4000, 0x2000, 0x8000, 0x10000}
#	define CFG_INTEL_SECT		137

/* max number of sectors on one chip */
#	define CFG_MAX_FLASH_SECT	(CFG_ATMEL_TOTALSECT + CFG_INTEL_SECT)
#	define CFG_FLASH_ERASE_TOUT	2000	/* Atmel needs longer timeout */
#	define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */
#	define CFG_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#	define CFG_FLASH_UNLOCK_TOUT	100	/* Timeout for Flash Clear Lock Bits (in ms) */
#	define CFG_FLASH_PROTECTION	/* "Real" (hardware) sectors protection */
#	define CFG_FLASH_CHECKSUM

#endif

/*
 * This is setting for JFFS2 support in u-boot.
 * NOTE: Enable CONFIG_CMD_JFFS2 for JFFS2 support.
 */
#ifdef CFG_ATMEL_BOOT
#	define CONFIG_JFFS2_DEV		"nor0"
#	define CONFIG_JFFS2_PART_SIZE	0x01000000
#	define CONFIG_JFFS2_PART_OFFSET	CFG_FLASH1_BASE
#else
#	define CONFIG_JFFS2_DEV		"nor0"
#	define CONFIG_JFFS2_PART_SIZE	(0x01000000 - 0x500000)
#	define CONFIG_JFFS2_PART_OFFSET	(CFG_FLASH0_BASE + 0x500000)
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE		16

/*-----------------------------------------------------------------------
 * Memory bank definitions
 */
/*
 * CS0 - NOR Flash 1, 2, 4, or 8MB
 * CS1 - CompactFlash and registers
 * CS2 - CPLD
 * CS3 - FPGA
 * CS4 - Available
 * CS5 - Available
 */

#ifdef CFG_ATMEL_BOOT
 /* Atmel Flash */
#define CFG_CS0_BASE		0
#define CFG_CS0_MASK		0x00070001
#define CFG_CS0_CTRL		0x00001140
/* Intel Flash */
#define CFG_CS1_BASE		0x04000000
#define CFG_CS1_MASK		0x01FF0001
#define CFG_CS1_CTRL		0x003F3D60

#define CFG_ATMEL_BASE		CFG_CS0_BASE
#else
/* Intel Flash */
#define CFG_CS0_BASE		0
#define CFG_CS0_MASK		0x01FF0001
#define CFG_CS0_CTRL		0x003F3D60
 /* Atmel Flash */
#define CFG_CS1_BASE		0x04000000
#define CFG_CS1_MASK		0x00070001
#define CFG_CS1_CTRL		0x00001140

#define CFG_ATMEL_BASE		CFG_CS1_BASE
#endif

/* CPLD */
#define CFG_CS2_BASE		0x08000000
#define CFG_CS2_MASK		0x00070001
#define CFG_CS2_CTRL		0x003f1140

/* FPGA */
#define CFG_CS3_BASE		0x09000000
#define CFG_CS3_MASK		0x00070001
#define CFG_CS3_CTRL		0x00000020

#endif				/* _JAMICA54455_H */

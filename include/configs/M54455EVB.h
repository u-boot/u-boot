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

#ifndef _M54455EVB_H
#define _M54455EVB_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MCF5445x		/* define processor family */
#define CONFIG_M54455		/* define processor type */
#define CONFIG_M54455EVB	/* M54455EVB board */

#define CONFIG_MCFUART
#define CONFIG_SYS_UART_PORT		(0)
#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600 , 19200 , 38400 , 57600, 115200 }

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
#undef CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SPI
#define CONFIG_CMD_SF

#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_LOADS

/* Network configuration */
#define CONFIG_MCFFEC
#ifdef CONFIG_MCFFEC
#	define CONFIG_NET_MULTI		1
#	define CONFIG_MII		1
#	define CONFIG_MII_INIT		1
#	define CONFIG_SYS_DISCOVER_PHY
#	define CONFIG_SYS_RX_ETH_BUFFER	8
#	define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

#	define CONFIG_SYS_FEC0_PINMUX	0
#	define CONFIG_SYS_FEC1_PINMUX	0
#	define CONFIG_SYS_FEC0_MIIBASE	CONFIG_SYS_FEC0_IOBASE
#	define CONFIG_SYS_FEC1_MIIBASE	CONFIG_SYS_FEC0_IOBASE
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

/* If CONFIG_SYS_DISCOVER_PHY is not defined - hardcoded */
#	ifndef CONFIG_SYS_DISCOVER_PHY
#		define FECDUPLEX	FULL
#		define FECSPEED		_100BASET
#	else
#		ifndef CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#			define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
#		endif
#	endif			/* CONFIG_SYS_DISCOVER_PHY */
#endif

#define CONFIG_HOSTNAME		M54455EVB
#ifdef CONFIG_SYS_STMICRO_BOOT
/* ST Micro serial flash */
#define	CONFIG_SYS_LOAD_ADDR2		0x40010013
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"inpclk=" MK_STR(CONFIG_SYS_INPUT_CLKSRC) "\0"	\
	"loadaddr=0x40010000\0"			\
	"sbfhdr=sbfhdr.bin\0"			\
	"uboot=u-boot.bin\0"			\
	"load=tftp ${loadaddr} ${sbfhdr};"	\
	"tftp " MK_STR(CONFIG_SYS_LOAD_ADDR2) " ${uboot} \0"	\
	"upd=run load; run prog\0"		\
	"prog=sf probe 0:1 1000000 3;"		\
	"sf erase 0 30000;"			\
	"sf write ${loadaddr} 0 0x30000;"	\
	"save\0"				\
	""
#else
/* Atmel and Intel */
#ifdef CONFIG_SYS_ATMEL_BOOT
#	define CONFIG_SYS_UBOOT_END	0x0403FFFF
#elif defined(CONFIG_SYS_INTEL_BOOT)
#	define CONFIG_SYS_UBOOT_END	0x3FFFF
#endif
#define CONFIG_EXTRA_ENV_SETTINGS		\
	"netdev=eth0\0"				\
	"inpclk=" MK_STR(CONFIG_SYS_INPUT_CLKSRC) "\0"	\
	"loadaddr=0x40010000\0"			\
	"uboot=u-boot.bin\0"			\
	"load=tftp ${loadaddr} ${uboot}\0"	\
	"upd=run load; run prog\0"		\
	"prog=prot off " MK_STR(CONFIG_SYS_FLASH_BASE)	\
	" " MK_STR(CONFIG_SYS_UBOOT_END) ";"		\
	"era " MK_STR(CONFIG_SYS_FLASH_BASE) " "	\
	MK_STR(CONFIG_SYS_UBOOT_END) ";"		\
	"cp.b ${loadaddr} " MK_STR(CONFIG_SYS_FLASH_BASE)	\
	" ${filesize}; save\0"			\
	""
#endif

/* ATA configuration */
#define CONFIG_ISO_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_IDE_RESET	1
#define CONFIG_IDE_PREINIT	1
#define CONFIG_ATAPI
#undef CONFIG_LBA48

#define CONFIG_SYS_IDE_MAXBUS		1
#define CONFIG_SYS_IDE_MAXDEVICE	2

#define CONFIG_SYS_ATA_BASE_ADDR	0x90000000
#define CONFIG_SYS_ATA_IDE0_OFFSET	0

#define CONFIG_SYS_ATA_DATA_OFFSET	0xA0	/* Offset for data I/O                            */
#define CONFIG_SYS_ATA_REG_OFFSET	0xA0	/* Offset for normal register accesses */
#define CONFIG_SYS_ATA_ALT_OFFSET	0xC0	/* Offset for alternate registers           */
#define CONFIG_SYS_ATA_STRIDE		4	/* Interval between registers                 */

/* Realtime clock */
#define CONFIG_MCFRTC
#undef RTC_DEBUG
#define CONFIG_SYS_RTC_OSCILLATOR	(32 * CONFIG_SYS_HZ)

/* Timer */
#define CONFIG_MCFTMR
#undef CONFIG_MCFPIT

/* I2c */
#define CONFIG_FSL_I2C
#define CONFIG_HARD_I2C		/* I2C with hardware support */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged               */
#define CONFIG_SYS_I2C_SPEED		80000	/* I2C speed and slave address  */
#define CONFIG_SYS_I2C_SLAVE		0x7F
#define CONFIG_SYS_I2C_OFFSET		0x58000
#define CONFIG_SYS_IMMR		CONFIG_SYS_MBAR

/* DSPI and Serial Flash */
#define CONFIG_CF_SPI
#define CONFIG_CF_DSPI
#define CONFIG_HARD_SPI
#define CONFIG_SYS_SBFHDR_SIZE		0x13
#ifdef CONFIG_CMD_SPI
#	define CONFIG_SPI_FLASH
#	define CONFIG_SPI_FLASH_STMICRO

#	define CONFIG_SYS_DSPI_CTAR0		(DSPI_CTAR_TRSZ(7) | \
					 DSPI_CTAR_PCSSCK_1CLK | \
					 DSPI_CTAR_PASC(0) | \
					 DSPI_CTAR_PDT(0) | \
					 DSPI_CTAR_CSSCK(0) | \
					 DSPI_CTAR_ASC(0) | \
					 DSPI_CTAR_DT(1))
#endif

/* PCI */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define CONFIG_SYS_PCI_CACHE_LINE_SIZE	4

#define CONFIG_SYS_PCI_MEM_BUS		0xA0000000
#define CONFIG_SYS_PCI_MEM_PHYS	CONFIG_SYS_PCI_MEM_BUS
#define CONFIG_SYS_PCI_MEM_SIZE	0x10000000

#define CONFIG_SYS_PCI_IO_BUS		0xB1000000
#define CONFIG_SYS_PCI_IO_PHYS		CONFIG_SYS_PCI_IO_BUS
#define CONFIG_SYS_PCI_IO_SIZE		0x01000000

#define CONFIG_SYS_PCI_CFG_BUS		0xB0000000
#define CONFIG_SYS_PCI_CFG_PHYS	CONFIG_SYS_PCI_CFG_BUS
#define CONFIG_SYS_PCI_CFG_SIZE	0x01000000
#endif

/* FPGA - Spartan 2 */
/* experiment
#define CONFIG_FPGA		CONFIG_SYS_SPARTAN3
#define CONFIG_FPGA_COUNT	1
#define CONFIG_SYS_FPGA_PROG_FEEDBACK
#define CONFIG_SYS_FPGA_CHECK_CTRLC
*/

/* Input, PCI, Flexbus, and VCO */
#define CONFIG_EXTRA_CLOCK

#define CONFIG_PRAM		2048	/* 2048 KB */

#define CONFIG_SYS_PROMPT		"-> "
#define CONFIG_SYS_LONGHELP		/* undef to save memory */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE			1024	/* Console I/O Buffer Size */
#else
#define CONFIG_SYS_CBSIZE			256	/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x10000)

#define CONFIG_SYS_HZ			1000

#define CONFIG_SYS_MBAR		0xFC000000

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CONFIG_SYS_INIT_RAM_ADDR	0x80000000
#define CONFIG_SYS_INIT_RAM_SIZE		0x8000	/* Size of used area in internal SRAM */
#define CONFIG_SYS_INIT_RAM_CTRL	0x221
#define CONFIG_SYS_GBL_DATA_OFFSET	((CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE) - 32)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
#define CONFIG_SYS_SBFHDR_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - 32)

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x40000000
#define CONFIG_SYS_SDRAM_BASE1		0x48000000
#define CONFIG_SYS_SDRAM_SIZE		256	/* SDRAM size in MB */
#define CONFIG_SYS_SDRAM_CFG1		0x65311610
#define CONFIG_SYS_SDRAM_CFG2		0x59670000
#define CONFIG_SYS_SDRAM_CTRL		0xEA0B2000
#define CONFIG_SYS_SDRAM_EMOD		0x40010000
#define CONFIG_SYS_SDRAM_MODE		0x00010033
#define CONFIG_SYS_SDRAM_DRV_STRENGTH	0xAA

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE + 0x400
#define CONFIG_SYS_MEMTEST_END		((CONFIG_SYS_SDRAM_SIZE - 3) << 20)

#ifdef CONFIG_CF_SBF
#	define CONFIG_SERIAL_BOOT
#	define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_TEXT_BASE + 0x400)
#else
#	define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_FLASH_BASE + 0x400)
#endif
#define CONFIG_SYS_BOOTPARAMS_LEN	64*1024
#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor */

/* Reserve 256 kB for malloc() */
#define CONFIG_SYS_MALLOC_LEN		(256 << 10)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization ??
 */
/* Initial Memory map for Linux */
#define CONFIG_SYS_BOOTMAPSZ		(CONFIG_SYS_SDRAM_BASE + (CONFIG_SYS_SDRAM_SIZE << 20))

/*
 * Configuration for environment
 * Environment is not embedded in u-boot. First time runing may have env
 * crc error warning if there is no correct environment on the flash.
 */
#ifdef CONFIG_CF_SBF
#	define CONFIG_ENV_IS_IN_SPI_FLASH
#	define CONFIG_ENV_SPI_CS		1
#else
#	define CONFIG_ENV_IS_IN_FLASH	1
#endif
#undef CONFIG_ENV_OVERWRITE

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#ifdef CONFIG_SYS_STMICRO_BOOT
#	define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_CS0_BASE
#	define CONFIG_SYS_FLASH0_BASE		CONFIG_SYS_CS1_BASE
#	define CONFIG_ENV_OFFSET		0x30000
#	define CONFIG_ENV_SIZE		0x2000
#	define CONFIG_ENV_SECT_SIZE	0x10000
#endif
#ifdef CONFIG_SYS_ATMEL_BOOT
#	define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_CS0_BASE
#	define CONFIG_SYS_FLASH0_BASE		CONFIG_SYS_CS0_BASE
#	define CONFIG_SYS_FLASH1_BASE		CONFIG_SYS_CS1_BASE
#	define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x40000)
#	define CONFIG_ENV_SIZE		0x2000
#	define CONFIG_ENV_SECT_SIZE	0x10000
#endif
#ifdef CONFIG_SYS_INTEL_BOOT
#	define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_CS0_BASE
#	define CONFIG_SYS_FLASH0_BASE		CONFIG_SYS_CS0_BASE
#	define CONFIG_SYS_FLASH1_BASE		CONFIG_SYS_CS1_BASE
#	define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x40000)
#	define CONFIG_ENV_SIZE		0x2000
#	define CONFIG_ENV_SECT_SIZE	0x20000
#endif

#define CONFIG_SYS_FLASH_CFI
#ifdef CONFIG_SYS_FLASH_CFI

#	define CONFIG_FLASH_CFI_DRIVER	1
#	define CONFIG_SYS_FLASH_USE_BUFFER_WRITE	1
#	define CONFIG_SYS_FLASH_SIZE		0x1000000	/* Max size that the board might have */
#	define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#	define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks */
#	define CONFIG_SYS_MAX_FLASH_SECT	137	/* max number of sectors on one chip */
#	define CONFIG_SYS_FLASH_PROTECTION	/* "Real" (hardware) sectors protection */
#	define CONFIG_SYS_FLASH_CHECKSUM
#	define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_CS0_BASE, CONFIG_SYS_CS1_BASE }
#	define CONFIG_FLASH_CFI_LEGACY

#ifdef CONFIG_FLASH_CFI_LEGACY
#	define CONFIG_SYS_ATMEL_REGION		4
#	define CONFIG_SYS_ATMEL_TOTALSECT	11
#	define CONFIG_SYS_ATMEL_SECT		{1, 2, 1, 7}
#	define CONFIG_SYS_ATMEL_SECTSZ		{0x4000, 0x2000, 0x8000, 0x10000}
#endif
#endif

/*
 * This is setting for JFFS2 support in u-boot.
 * NOTE: Enable CONFIG_CMD_JFFS2 for JFFS2 support.
 */
#ifdef CONFIG_CMD_JFFS2
#ifdef CF_STMICRO_BOOT
#	define CONFIG_JFFS2_DEV		"nor1"
#	define CONFIG_JFFS2_PART_SIZE	0x01000000
#	define CONFIG_JFFS2_PART_OFFSET	(CONFIG_SYS_FLASH2_BASE + 0x500000)
#endif
#ifdef CONFIG_SYS_ATMEL_BOOT
#	define CONFIG_JFFS2_DEV		"nor1"
#	define CONFIG_JFFS2_PART_SIZE	0x01000000
#	define CONFIG_JFFS2_PART_OFFSET	(CONFIG_SYS_FLASH1_BASE + 0x500000)
#endif
#ifdef CONFIG_SYS_INTEL_BOOT
#	define CONFIG_JFFS2_DEV		"nor0"
#	define CONFIG_JFFS2_PART_SIZE	(0x01000000 - 0x500000)
#	define CONFIG_JFFS2_PART_OFFSET	(CONFIG_SYS_FLASH0_BASE + 0x500000)
#endif
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_SYS_CACHELINE_SIZE		16

#define ICACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 8)
#define DCACHE_STATUS			(CONFIG_SYS_INIT_RAM_ADDR + \
					 CONFIG_SYS_INIT_RAM_SIZE - 4)
#define CONFIG_SYS_ICACHE_INV		(CF_CACR_BCINVA + CF_CACR_ICINVA)
#define CONFIG_SYS_DCACHE_INV		(CF_CACR_DCINVA)
#define CONFIG_SYS_CACHE_ACR2		(CONFIG_SYS_SDRAM_BASE | \
					 CF_ADDRMASK(CONFIG_SYS_SDRAM_SIZE) | \
					 CF_ACR_EN | CF_ACR_SM_ALL)
#define CONFIG_SYS_CACHE_ICACR		(CF_CACR_BEC | CF_CACR_IEC | \
					 CF_CACR_ICINVA | CF_CACR_EUSP)
#define CONFIG_SYS_CACHE_DCACR		((CONFIG_SYS_CACHE_ICACR | \
					 CF_CACR_DEC | CF_CACR_DDCM_P | \
					 CF_CACR_DCINVA) & ~CF_CACR_ICINVA)

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

#if defined(CONFIG_SYS_ATMEL_BOOT) || defined(CONFIG_SYS_STMICRO_BOOT)
 /* Atmel Flash */
#define CONFIG_SYS_CS0_BASE		0x04000000
#define CONFIG_SYS_CS0_MASK		0x00070001
#define CONFIG_SYS_CS0_CTRL		0x00001140
/* Intel Flash */
#define CONFIG_SYS_CS1_BASE		0x00000000
#define CONFIG_SYS_CS1_MASK		0x01FF0001
#define CONFIG_SYS_CS1_CTRL		0x00000D60

#define CONFIG_SYS_ATMEL_BASE		CONFIG_SYS_CS0_BASE
#else
/* Intel Flash */
#define CONFIG_SYS_CS0_BASE		0x00000000
#define CONFIG_SYS_CS0_MASK		0x01FF0001
#define CONFIG_SYS_CS0_CTRL		0x00000D60
 /* Atmel Flash */
#define CONFIG_SYS_CS1_BASE		0x04000000
#define CONFIG_SYS_CS1_MASK		0x00070001
#define CONFIG_SYS_CS1_CTRL		0x00001140

#define CONFIG_SYS_ATMEL_BASE		CONFIG_SYS_CS1_BASE
#endif

/* CPLD */
#define CONFIG_SYS_CS2_BASE		0x08000000
#define CONFIG_SYS_CS2_MASK		0x00070001
#define CONFIG_SYS_CS2_CTRL		0x003f1140

/* FPGA */
#define CONFIG_SYS_CS3_BASE		0x09000000
#define CONFIG_SYS_CS3_MASK		0x00070001
#define CONFIG_SYS_CS3_CTRL		0x00000020

#endif				/* _M54455EVB_H */

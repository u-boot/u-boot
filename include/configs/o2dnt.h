/*
 * (C) Copyright 2005
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

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_MPC5200
#define CONFIG_O2DNT		1	/* ... on O2DNT board */

#define CFG_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz */

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM		0x02	/* Software reboot	     */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	5	/* console is on PSC5 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
/* #define CONFIG_PCI_SCAN_SHOW	1 */

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

#define CFG_XLB_PIPELINING	1

#define CONFIG_NET_MULTI	1
#define CONFIG_EEPRO100
#define CFG_RX_ETH_BUFFER	8  /* use 8 rx buffer on eepro100  */
#define CONFIG_NS8382X		1

#define ADD_PCI_CMD 		CFG_CMD_PCI

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#define CONFIG_TIMESTAMP	/* Print image info with timestamp */


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_NFS
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_PCI_CMD


#if (TEXT_BASE == 0xFF000000)		/* Boot low with 16 MB Flash */
#   define CFG_LOWBOOT		1
#else
#   error "TEXT_BASE must be 0xFF000000"
#endif

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"flash_nfs=run nfsargs addip;"					\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip;"					\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip;bootm\0"	\
	"rootpath=/opt/eldk/ppc_82xx\0"					\
	"bootfile=/tftpboot/MPC5200/uImage\0"				\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

#if defined(CONFIG_MPC5200)
/*
 * IPB Bus clocking configuration.
 */
#define CFG_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

#if defined(CFG_IPBCLK_EQUALS_XLBCLK)
/*
 * PCI Bus clocking configuration
 *
 * Actually a PCI Clock of 66 MHz is only set (in cpu_init.c) if
 * CFG_IPBCLK_EQUALS_XLBCLK is defined. This is because a PCI Clock
 *  of 66 MHz yet hasn't been tested with a IPB Bus Clock of 66 MHz.
 */
#define CFG_PCICLK_EQUALS_IPBCLK_DIV2	/* define for 66MHz speed */
#endif
#endif

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CFG_I2C_MODULE		1	/* Select I2C module #1 or #2 */

#define CFG_I2C_SPEED		100000 /* 100 kHz */
#define CFG_I2C_SLAVE		0x7F

/*
 * EEPROM configuration:
 *
 * O2DNT board is equiped with Ramtron FRAM device FM24CL16
 * 16 Kib Ferroelectric Nonvolatile serial RAM memory
 * organized as 2048 x 8 bits and addressable as eight I2C devices
 * 0x50 ... 0x57 each 256 bytes in size
 *
 */
#define CFG_I2C_FRAM
#define CFG_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CFG_EEPROM_PAGE_WRITE_BITS	3
/*
 * There is no write delay with FRAM, write operations are performed at bus
 * speed. Thus, no status polling or write delay is needed.
 */
/*#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	70*/


/*
 * Flash configuration
 */
#define CFG_FLASH_BASE		0xFF000000
#define CFG_FLASH_SIZE		0x01000000
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x00040000)

#define CFG_MAX_FLASH_BANKS	1	/* max num of memory banks      */
#define CFG_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */
#define CFG_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */
#define CFG_FLASH_PROTECTION		/* "Real" (hardware) sectors protection */

/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x20000
#define CFG_ENV_SECT_SIZE	0x20000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CFG_MBAR		0xF0000000
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000

/* Use SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE	/* End of used area in DPRAM */


#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
/*
 * Define CONFIG_FEC_10MBIT to force FEC at 10Mb
 */
/* #define CONFIG_FEC_10MBIT 1 */
#define CONFIG_PHY_ADDR		0x00

/*
 * GPIO configuration
 */
/*#define CFG_GPS_PORT_CONFIG	0x10002004 */
#define CFG_GPS_PORT_CONFIG	0x00002006	/* no CAN */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	    */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt   */

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on */
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Various low-level settings
 */
#if defined(CONFIG_MPC5200)
#define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
#define CFG_HID0_FINAL		HID0_ICE
#else
#define CFG_HID0_INIT		0
#define CFG_HID0_FINAL		0
#endif

#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE

#ifdef CFG_PCICLK_EQUALS_IPBCLK_DIV2
/*
 * For 66 MHz PCI clock additional Wait State is needed for CS0 (flash).
 */
#define CFG_BOOTCS_CFG		0x00057801 /* for pci_clk = 66 MHz */
#else
#define CFG_BOOTCS_CFG		0x00047801 /* for pci_clk = 33 MHz */
#endif

#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333333

#define CFG_RESET_ADDRESS	0xff000000

#endif /* __CONFIG_H */

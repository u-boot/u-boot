/*
 * (C) Copyright 2003-2005
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

#define CONFIG_MPC5200
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_PM520		1	/* ... on PM520 board */

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33MHz */

#define CONFIG_MISC_INIT_R

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM		0x02	/* Software reboot	     */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_BAUDRATE		9600	/* ... at 9600 bps */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }


/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define CONFIG_PCI		1
#define CONFIG_PCI_PNP		1
#define CONFIG_PCI_SCAN_SHOW	1
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000

#define CONFIG_NET_MULTI	1
#define CONFIG_MII		1
#define CONFIG_EEPRO100		1
#define CONFIG_SYS_RX_ETH_BUFFER	8  /* use 8 rx buffer on eepro100  */
#undef  CONFIG_NS8382X


/* Partitions */
#define CONFIG_DOS_PARTITION

/* USB */
#if 1
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE
#endif

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

#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_USB

#define CONFIG_CMD_PCI


/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=pm520\0"							\
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
	"rootpath=/opt/eldk30/ppc_82xx\0"					\
	"bootfile=/tftpboot/PM520/uImage\0"				\
	""

#define CONFIG_BOOTCOMMAND	"run flash_self"

/*
 * IPB Bus clocking configuration.
 */
#undef CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */
/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CONFIG_SYS_I2C_MODULE		2	/* Select I2C module #1 or #2 */

#define CONFIG_SYS_I2C_SPEED		100000 /* 100 kHz */
#define CONFIG_SYS_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x58
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	10

/*
 * RTC configuration
 */
#define CONFIG_RTC_PCF8563
#define CONFIG_SYS_I2C_RTC_ADDR		0x51

#define CONFIG_SYS_DOC_BASE		0xE0000000
#define CONFIG_SYS_DOC_SIZE		0x00100000

#if defined(CONFIG_BOOT_ROM)
/*
 * Flash configuration (8,16 or 32 MB)
 * TEXT base always at 0xFFF00000
 * ENV_ADDR always at  0xFFF40000
 * FLASH_BASE at 0xFA000000 for 64 MB
 *               0xFC000000 for 32 MB
 *               0xFD000000 for 16 MB
 *               0xFD800000 for  8 MB
 */
#define CONFIG_SYS_FLASH_BASE		0xFA000000
#define CONFIG_SYS_FLASH_SIZE		0x04000000
#define CONFIG_SYS_BOOTROM_BASE	0xFFF00000
#define CONFIG_SYS_BOOTROM_SIZE	0x00080000
#define CONFIG_ENV_ADDR		(0xFDF00000 + 0x40000)
#else
/*
 * Flash configuration (8,16 or 32 MB)
 * TEXT base always at 0xFFF00000
 * ENV_ADDR always at  0xFFF40000
 * FLASH_BASE at 0xFC000000 for 64 MB
 *               0xFE000000 for 32 MB
 *               0xFF000000 for 16 MB
 *               0xFF800000 for  8 MB
 */
#define CONFIG_SYS_FLASH_BASE		0xFC000000
#define CONFIG_SYS_FLASH_SIZE		0x04000000
#define CONFIG_ENV_ADDR		(0xFFF00000 + 0x40000)
#endif
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max num of memory banks      */

#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max num of sects on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */
#define CONFIG_SYS_FLASH_LOCK_TOUT	5	/* Timeout for Flash Set Lock Bit (in ms) */
#define CONFIG_SYS_FLASH_UNLOCK_TOUT	10000	/* Timeout for Flash Clear Lock Bits (in ms) */
#define CONFIG_SYS_FLASH_PROTECTION		/* "Real" (hardware) sectors protection */

#define PHYS_FLASH_SECT_SIZE	0x00040000 /* 256 KB sectors (x2) */

#undef CONFIG_FLASH_16BIT	/* Flash is 32-bit */


/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x10000
#define CONFIG_ENV_SECT_SIZE	0x40000
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xf0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000

/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_END	MPC5XXX_SRAM_SIZE	/* End of used area in DPRAM */


#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE    TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
#define CONFIG_MPC5xxx_FEC_MII100
/*
 * Define CONFIG_MPC5xxx_FEC_MII10 to force FEC at 10Mb
 */
/* #define CONFIG_MPC5xxx_FEC_MII10 */
#define CONFIG_PHY_ADDR		0x00

/*
 * GPIO configuration
 */
#define CONFIG_SYS_GPS_PORT_CONFIG	0x10000004

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt   */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#if defined(CONFIG_BOOT_ROM)
#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_BOOTROM_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_BOOTROM_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x00047800
#define CONFIG_SYS_CS0_START		CONFIG_SYS_BOOTROM_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_BOOTROM_SIZE
#define CONFIG_SYS_CS1_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS1_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS1_CFG		0x0004FF00
#else
#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x0004FF00
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS1_START		CONFIG_SYS_DOC_BASE
#define CONFIG_SYS_CS1_SIZE		CONFIG_SYS_DOC_SIZE
#define CONFIG_SYS_CS1_CFG		0x00047800
#endif

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x33333333

#define CONFIG_SYS_RESET_ADDRESS	0xff000000

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00005000

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/

#undef	CONFIG_IDE_RESET		/* reset for ide supported	*/
#define CONFIG_IDE_PREINIT

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	2	/* max. 2 drive per IDE bus	*/

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	MPC5XXX_ATA

/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	(0x0060)

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)

/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x005C)

/* Interval between registers                                                */
#define CONFIG_SYS_ATA_STRIDE          4

#endif /* __CONFIG_H */

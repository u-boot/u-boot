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

#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_ICECUBE		1	/* ... on IceCube board */

#define CFG_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz */

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH  */
#define BOOTFLAG_WARM		0x02	/* Software reboot	     */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }


#ifdef CONFIG_MPC5200	/* MPC5100 PCI is not supported yet. */
/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define CONFIG_PCI

#if defined(CONFIG_PCI)
#define CONFIG_PCI_PNP		1
#define CONFIG_PCI_SCAN_SHOW	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000
#endif

#define CFG_XLB_PIPELINING	1

#define CONFIG_NET_MULTI	1
#define CONFIG_MII		1
#define CONFIG_EEPRO100		1
#define CFG_RX_ETH_BUFFER	8  /* use 8 rx buffer on eepro100  */
#define CONFIG_NS8382X		1

#else
#define CONFIG_MII		1
#endif

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

/* USB */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#define CFG_OHCI_BE_CONTROLLER
#undef CFG_USB_OHCI_BOARD_INIT
#define CFG_USB_OHCI_CPU_INIT	1
#define CFG_USB_OHCI_REGS_BASE	MPC5XXX_USB
#define CFG_USB_OHCI_SLOT_NAME	"mpc5200"
#define CFG_USB_OHCI_MAX_ROOT_PORTS	15

#define	CONFIG_TIMESTAMP		/* Print image info with timestamp */


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

#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_NFS
#define CONFIG_CMD_SNTP
#define CONFIG_CMD_USB

#if defined(CONFIG_PCI)
#define CONFIG_CMD_PCI
#endif


#if (TEXT_BASE == 0xFF000000)		/* Boot low with 16 MB Flash */
#   define CFG_LOWBOOT	        1
#   define CFG_LOWBOOT16	1
#endif
#if (TEXT_BASE == 0xFF800000)		/* Boot low with  8 MB Flash */
#if defined(CONFIG_LITE5200B)
#   error CFG_LOWBOOT08 is incompatible with the Lite5200B
#else
#   define CFG_LOWBOOT	        1
#   define CFG_LOWBOOT08	1
#endif
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
#if defined(CONFIG_LITE5200B)
#define CFG_IPBCLK_EQUALS_XLBCLK 	/* define for 133MHz speed */
#else
#undef CFG_IPBCLK_EQUALS_XLBCLK   	/* define for 133MHz speed */
#endif
#endif /* CONFIG_MPC5200 */

/* pass open firmware flat tree */
#define CONFIG_OF_FLAT_TREE	1
#define CONFIG_OF_BOARD_SETUP	1

/* maximum size of the flat tree (8K) */
#define OF_FLAT_TREE_MAX_SIZE	8192

#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc5200@f0000000/serial@2000"

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CFG_I2C_MODULE		2	/* Select I2C module #1 or #2 */

#define CFG_I2C_SPEED		100000 /* 100 kHz */
#define CFG_I2C_SLAVE		0x7F

/*
 * EEPROM configuration
 */
#define CFG_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	70

/*
 * Flash configuration
 */
#if defined(CONFIG_LITE5200B)
#define CFG_FLASH_BASE		0xFE000000
#define CFG_FLASH_SIZE		0x01000000
#if !defined(CFG_LOWBOOT)
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x01760000 + 0x00800000)
#else	/* CFG_LOWBOOT */
#if defined(CFG_LOWBOOT08)
# error CFG_LOWBOOT08 is incompatible with the Lite5200B
#endif
#if defined(CFG_LOWBOOT16)
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x01060000)
#endif
#endif /* CFG_LOWBOOT */
#else /* !CONFIG_LITE5200B (IceCube)*/
#define CFG_FLASH_BASE		0xFF000000
#define CFG_FLASH_SIZE		0x01000000
#if !defined(CFG_LOWBOOT)
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x00740000 + 0x00800000)
#else	/* CFG_LOWBOOT */
#if defined(CFG_LOWBOOT08)
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x00040000 + 0x00800000)
#endif
#if defined(CFG_LOWBOOT16)
#define CFG_ENV_ADDR		(CFG_FLASH_BASE + 0x00040000)
#endif
#endif	/* CFG_LOWBOOT */
#endif /* CONFIG_LITE5200B */
#define CFG_MAX_FLASH_BANKS	2	/* max num of memory banks      */

#define CFG_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CFG_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CFG_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */

#undef CONFIG_FLASH_16BIT	/* Flash is 8-bit */

#if defined(CONFIG_LITE5200B)
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_CFI
#define CFG_FLASH_BANKS_LIST	{CFG_CS1_START,CFG_CS0_START}
#endif


/*
 * Environment settings
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_SIZE		0x10000
#if defined(CONFIG_LITE5200B)
#define CFG_ENV_SECT_SIZE	0x20000
#else
#define CFG_ENV_SECT_SIZE	0x10000
#endif
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

#define CFG_MONITOR_BASE    TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

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
#if defined(CONFIG_LITE5200B)
#define CONFIG_FEC_MII100	1
#endif

/*
 * GPIO configuration
 */
#ifdef CONFIG_MPC5200_DDR
#define CFG_GPS_PORT_CONFIG	0x90000004
#else
#define CFG_GPS_PORT_CONFIG	0x10000004
#endif

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

#if defined(CONFIG_LITE5200B)
#define CFG_CS1_START		CFG_FLASH_BASE
#define CFG_CS1_SIZE		CFG_FLASH_SIZE
#define CFG_CS1_CFG		0x00047800
#define CFG_CS0_START		(CFG_FLASH_BASE + CFG_FLASH_SIZE)
#define CFG_CS0_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_START	CFG_CS0_START
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x00047800
#else /* IceCube aka Lite5200 */
#ifdef CONFIG_MPC5200_DDR

#define CFG_BOOTCS_START	(CFG_CS1_START + CFG_CS1_SIZE)
#define CFG_BOOTCS_SIZE		0x00800000
#define CFG_BOOTCS_CFG		0x00047801
#define CFG_CS1_START		CFG_FLASH_BASE
#define CFG_CS1_SIZE		0x00800000
#define CFG_CS1_CFG		0x00047800

#else /* !CONFIG_MPC5200_DDR */

#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x00047801
#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

#endif /* CONFIG_MPC5200_DDR */
#endif /*CONFIG_LITE5200B */

#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333333

#define CFG_RESET_ADDRESS	0xff000000

/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00001000

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef	CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef	CONFIG_IDE_LED			/* LED   for ide not supported	*/

#define	CONFIG_IDE_RESET		/* reset for ide supported	*/
#define CONFIG_IDE_PREINIT

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	2	/* max. 1 drive per IDE bus	*/

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR	MPC5XXX_ATA

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	(0x0060)

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	(CFG_ATA_DATA_OFFSET)

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	(0x005C)

/* Interval between registers                                                */
#define CFG_ATA_STRIDE          4

#define CONFIG_ATAPI            1

#endif /* __CONFIG_H */

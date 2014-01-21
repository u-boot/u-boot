/*
 * (C) Copyright 2003-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU */
#define CONFIG_MPC5200		1	/* (more precisely a MPC5200 CPU) */
#define CONFIG_ICECUBE		1	/* ... on IceCube board */

/*
 * Valid values for CONFIG_SYS_TEXT_BASE are:
 * 0xFFF00000	boot high (standard configuration)
 * 0xFF000000	boot low for 16 MiB boards
 * 0xFF800000	boot low for  8 MiB boards
 * 0x00100000	boot from RAM (for testing only)
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define	CONFIG_SYS_TEXT_BASE	0xFFF00000
#endif

#define CONFIG_SYS_MPC5XXX_CLKIN	33000000 /* ... running at 33.000000MHz */

#define CONFIG_HIGH_BATS	1	/* High BATs supported */

/*
 * Serial console configuration
 */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1 */
#define CONFIG_BAUDRATE		115200	/* ... at 115200 bps */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }


/*
 * PCI Mapping:
 * 0x40000000 - 0x4fffffff - PCI Memory
 * 0x50000000 - 0x50ffffff - PCI IO Space
 */
#define CONFIG_PCI

#if defined(CONFIG_PCI)
#define CONFIG_PCI_PNP		1
#define CONFIG_PCI_SCAN_SHOW	1
#define CONFIG_PCIAUTO_SKIP_HOST_BRIDGE	1

#define CONFIG_PCI_MEM_BUS	0x40000000
#define CONFIG_PCI_MEM_PHYS	CONFIG_PCI_MEM_BUS
#define CONFIG_PCI_MEM_SIZE	0x10000000

#define CONFIG_PCI_IO_BUS	0x50000000
#define CONFIG_PCI_IO_PHYS	CONFIG_PCI_IO_BUS
#define CONFIG_PCI_IO_SIZE	0x01000000
#endif

#define CONFIG_SYS_XLB_PIPELINING	1

#define CONFIG_MII		1
#define CONFIG_EEPRO100		1
#define CONFIG_SYS_RX_ETH_BUFFER	8  /* use 8 rx buffer on eepro100  */
#define CONFIG_NS8382X		1

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

/* USB */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#define CONFIG_SYS_OHCI_BE_CONTROLLER
#undef CONFIG_SYS_USB_OHCI_BOARD_INIT
#define CONFIG_SYS_USB_OHCI_CPU_INIT	1
#define CONFIG_SYS_USB_OHCI_REGS_BASE	MPC5XXX_USB
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"mpc5200"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15

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


#if (CONFIG_SYS_TEXT_BASE == 0xFF000000)		/* Boot low with 16 MB Flash */
#   define CONFIG_SYS_LOWBOOT	        1
#   define CONFIG_SYS_LOWBOOT16	1
#endif
#if (CONFIG_SYS_TEXT_BASE == 0xFF800000)		/* Boot low with  8 MB Flash */
#if defined(CONFIG_LITE5200B)
#   error CONFIG_SYS_LOWBOOT08 is incompatible with the Lite5200B
#else
#   define CONFIG_SYS_LOWBOOT	        1
#   define CONFIG_SYS_LOWBOOT08	1
#endif
#endif

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

/*
 * IPB Bus clocking configuration.
 */
#if defined(CONFIG_LITE5200B)
#define CONFIG_SYS_IPBCLK_EQUALS_XLBCLK	/* define for 133MHz speed */
#else
#undef CONFIG_SYS_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,5200@0"
#define OF_SOC			"soc5200@f0000000"
#define OF_TBCLK		(bd->bi_busfreq / 4)
#define OF_STDOUT_PATH		"/soc5200@f0000000/serial@2000"

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
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50	/* 1010000x */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	70

/*
 * Flash configuration
 */
#if defined(CONFIG_LITE5200B)
#define CONFIG_SYS_FLASH_BASE		0xFE000000
#define CONFIG_SYS_FLASH_SIZE		0x01000000
#if !defined(CONFIG_SYS_LOWBOOT)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x01760000 + 0x00800000)
#else	/* CONFIG_SYS_LOWBOOT */
#if defined(CONFIG_SYS_LOWBOOT08)
# error CONFIG_SYS_LOWBOOT08 is incompatible with the Lite5200B
#endif
#if defined(CONFIG_SYS_LOWBOOT16)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x01060000)
#endif
#endif /* CONFIG_SYS_LOWBOOT */
#else /* !CONFIG_LITE5200B (IceCube)*/
#define CONFIG_SYS_FLASH_BASE		0xFF000000
#define CONFIG_SYS_FLASH_SIZE		0x01000000
#if !defined(CONFIG_SYS_LOWBOOT)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00740000 + 0x00800000)
#else	/* CONFIG_SYS_LOWBOOT */
#if defined(CONFIG_SYS_LOWBOOT08)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00040000 + 0x00800000)
#endif
#if defined(CONFIG_SYS_LOWBOOT16)
#define CONFIG_ENV_ADDR		(CONFIG_SYS_FLASH_BASE + 0x00040000)
#endif
#endif	/* CONFIG_SYS_LOWBOOT */
#endif /* CONFIG_LITE5200B */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max num of memory banks      */

#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max num of sects on one chip */

#define CONFIG_SYS_FLASH_ERASE_TOUT	240000	/* Flash Erase Timeout (in ms)  */
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)  */

#undef CONFIG_FLASH_16BIT	/* Flash is 8-bit */

#if defined(CONFIG_LITE5200B)
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_CS1_START,CONFIG_SYS_CS0_START}
#endif


/*
 * Environment settings
 */
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_SIZE		0x10000
#if defined(CONFIG_LITE5200B)
#define CONFIG_ENV_SECT_SIZE	0x20000
#else
#define CONFIG_ENV_SECT_SIZE	0x10000
#endif
#define CONFIG_ENV_OVERWRITE	1

/*
 * Memory map
 */
#define CONFIG_SYS_MBAR		0xF0000000
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_DEFAULT_MBAR	0x80000000

/* Use SRAM until RAM will be available */
#define CONFIG_SYS_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CONFIG_SYS_INIT_RAM_SIZE	MPC5XXX_SRAM_SIZE	/* Size of used area in DPRAM */


#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_BASE    CONFIG_SYS_TEXT_BASE
#if (CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE)
#   define CONFIG_SYS_RAMBOOT		1
#endif

#define CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(512 << 10)	/* Reserve 512 kB for malloc()	*/
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
#ifdef CONFIG_MPC5200_DDR
#define CONFIG_SYS_GPS_PORT_CONFIG	0x90000004
#else
#define CONFIG_SYS_GPS_PORT_CONFIG	0x10000004
#endif

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory	    */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size  */
#else
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size  */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_SYS_HUSH_PARSER		1	/* use "hush" command parser	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */

#define CONFIG_SYS_CACHELINE_SIZE	32	/* For MPC5xxx CPUs */
#if defined(CONFIG_CMD_KGDB)
#  define CONFIG_SYS_CACHELINE_SHIFT	5	/* log base 2 of the above value */
#endif

/*
 * Various low-level settings
 */
#define CONFIG_SYS_HID0_INIT		HID0_ICE | HID0_ICFI
#define CONFIG_SYS_HID0_FINAL		HID0_ICE

#if defined(CONFIG_LITE5200B)
#define CONFIG_SYS_CS1_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS1_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_CS1_CFG		0x00047800
#define CONFIG_SYS_CS0_START		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FLASH_SIZE)
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_CS0_START
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x00047800
#else /* IceCube aka Lite5200 */
#ifdef CONFIG_MPC5200_DDR

#define CONFIG_SYS_BOOTCS_START	(CONFIG_SYS_CS1_START + CONFIG_SYS_CS1_SIZE)
#define CONFIG_SYS_BOOTCS_SIZE		0x00800000
#define CONFIG_SYS_BOOTCS_CFG		0x00047801
#define CONFIG_SYS_CS1_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS1_SIZE		0x00800000
#define CONFIG_SYS_CS1_CFG		0x00047800

#else /* !CONFIG_MPC5200_DDR */

#define CONFIG_SYS_BOOTCS_START	CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_BOOTCS_SIZE		CONFIG_SYS_FLASH_SIZE
#define CONFIG_SYS_BOOTCS_CFG		0x00047801
#define CONFIG_SYS_CS0_START		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_CS0_SIZE		CONFIG_SYS_FLASH_SIZE

#endif /* CONFIG_MPC5200_DDR */
#endif /*CONFIG_LITE5200B */

#define CONFIG_SYS_CS_BURST		0x00000000
#define CONFIG_SYS_CS_DEADCYCLE	0x33333333

#define CONFIG_SYS_RESET_ADDRESS	0xff000000

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

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	2	/* max. 1 drive per IDE bus	*/

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

#define CONFIG_ATAPI            1

#endif /* __CONFIG_H */

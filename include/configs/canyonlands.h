/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/************************************************************************
 * canyonlands.h - configuration for Canyonlands (460EX)
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
/* This config file is used for Canyonlands (460EX) and Glacier (460GT)	*/
#ifndef CONFIG_CANYONLANDS
#define CONFIG_460GT		1	/* Specific PPC460GT		*/
#else
#define CONFIG_460EX		1	/* Specific PPC460EX		*/
#endif
#define CONFIG_440		1
#define CONFIG_4xx		1	/* ... PPC4xx family */

#define CONFIG_SYS_CLK_FREQ	66666667	/* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F	1	/* Call board_early_init_f */
#define CONFIG_BOARD_EARLY_INIT_R	1	/* Call board_early_init_r */
#define CONFIG_MISC_INIT_R		1	/* Call misc_init_r */

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_SDRAM_BASE		0x00000000	/* _must_ be 0	*/

#define CFG_PCI_MEMBASE		0x80000000	/* mapped PCI memory	*/
#define CFG_PCI_BASE		0xd0000000	/* internal PCI regs	*/
#define CFG_PCI_TARGBASE	CFG_PCI_MEMBASE

#define CFG_PCIE_MEMBASE	0xb0000000	/* mapped PCIe memory	*/
#define CFG_PCIE_MEMSIZE	0x08000000	/* smallest incr for PCIe port */
#define CFG_PCIE_BASE		0xc4000000	/* PCIe UTL regs */

#define CFG_PCIE0_CFGBASE	0xc0000000
#define CFG_PCIE1_CFGBASE	0xc1000000
#define CFG_PCIE0_XCFGBASE	0xc3000000
#define CFG_PCIE1_XCFGBASE	0xc3001000

#define	CFG_PCIE0_UTLBASE	0xc08010000ULL	/* 36bit physical addr	*/

/* base address of inbound PCIe window */
#define CFG_PCIE_INBOUND_BASE	0x000000000ULL	/* 36bit physical addr	*/

/* EBC stuff */
#define CFG_NAND_ADDR		0xE0000000
#define CFG_BCSR_BASE		0xE1000000
#define CFG_BOOT_BASE_ADDR	0xFF000000	/* EBC Boot Space: 0xFF000000	*/
#define CFG_FLASH_BASE		0xFC000000	/* later mapped to this addr	*/
#define CFG_FLASH_BASE_PHYS_H	0x4
#define CFG_FLASH_BASE_PHYS_L	0xCC000000
#define CFG_FLASH_BASE_PHYS	(((u64)CFG_FLASH_BASE_PHYS_H << 32) | \
				 (u64)CFG_FLASH_BASE_PHYS_L)
#define CFG_FLASH_SIZE		(64 << 20)

#define CFG_OCM_BASE		0xE3000000	/* OCM: 16k		*/
#define CFG_SRAM_BASE		0xE8000000	/* SRAM: 256k		*/
#define CFG_LOCAL_CONF_REGS	0xEF000000

#define CFG_PERIPHERAL_BASE	0xEF600000	/* internal peripherals */

#define CFG_AHB_BASE		0xE2000000	/* internal AHB peripherals	*/

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(384 * 1024)	/* Reserve 384 kB for Monitor */
#define CFG_MALLOC_LEN		(512 * 1024)	/* Reserve 512 kB for malloc()*/

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in OCM)
 *----------------------------------------------------------------------*/
#define CFG_INIT_RAM_ADDR	CFG_OCM_BASE	/* OCM			*/
#define CFG_INIT_RAM_END	(4 << 10)
#define CFG_GBL_DATA_SIZE	256		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI	1
#undef CONFIG_UART1_CONSOLE	/* define this if you want console on UART1 */

#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
/*
 * Define here the location of the environment variables (FLASH).
 */
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define	CFG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars */
#define CFG_NAND_CS		3	/* NAND chip connected to CSx */
#else
#define	CFG_ENV_IS_IN_NAND	1	/* use NAND for environment vars  */
#define CFG_NAND_CS		0	/* NAND chip connected to CSx */
#define CFG_ENV_IS_EMBEDDED	1	/* use embedded environment */
#endif

/*
 * IPL (Initial Program Loader, integrated inside CPU)
 * Will load first 4k from NAND (SPL) into cache and execute it from there.
 *
 * SPL (Secondary Program Loader)
 * Will load special U-Boot version (NUB) from NAND and execute it. This SPL
 * has to fit into 4kByte. It sets up the CPU and configures the SDRAM
 * controller and the NAND controller so that the special U-Boot image can be
 * loaded from NAND to SDRAM.
 *
 * NUB (NAND U-Boot)
 * This NAND U-Boot (NUB) is a special U-Boot version which can be started
 * from RAM. Therefore it mustn't (re-)configure the SDRAM controller.
 *
 * On 440EPx the SPL is copied to SDRAM before the NAND controller is
 * set up. While still running from cache, I experienced problems accessing
 * the NAND controller.	sr - 2006-08-25
 */
#define CFG_NAND_BOOT_SPL_SRC	0xfffff000	/* SPL location		      */
#define CFG_NAND_BOOT_SPL_SIZE	(4 << 10)	/* SPL size		      */
#define CFG_NAND_BOOT_SPL_DST	(CFG_OCM_BASE + (12 << 10)) /* Copy SPL here  */
#define CFG_NAND_U_BOOT_DST	0x01000000	/* Load NUB to this addr      */
#define CFG_NAND_U_BOOT_START	CFG_NAND_U_BOOT_DST	/* Start NUB from     */
							/*   this addr	      */
#define CFG_NAND_BOOT_SPL_DELTA	(CFG_NAND_BOOT_SPL_SRC - CFG_NAND_BOOT_SPL_DST)

/*
 * Define the partitioning of the NAND chip (only RAM U-Boot is needed here)
 */
#define CFG_NAND_U_BOOT_OFFS	(16 << 10)	/* Offset to RAM U-Boot image */
#define CFG_NAND_U_BOOT_SIZE	(384 << 10)	/* Size of RAM U-Boot image   */

/*
 * Now the NAND chip has to be defined (no autodetection used!)
 */
#define CFG_NAND_PAGE_SIZE	512		/* NAND chip page size	      */
#define CFG_NAND_BLOCK_SIZE	(16 << 10)	/* NAND chip block size	      */
#define CFG_NAND_PAGE_COUNT	32		/* NAND chip page count	      */
#define CFG_NAND_BAD_BLOCK_POS	5	      /* Location of bad block marker */
#undef CFG_NAND_4_ADDR_CYCLE		      /* No fourth addr used (<=32MB) */

#define CFG_NAND_ECCSIZE	256
#define CFG_NAND_ECCBYTES	3
#define CFG_NAND_ECCSTEPS	(CFG_NAND_PAGE_SIZE / CFG_NAND_ECCSIZE)
#define CFG_NAND_OOBSIZE	16
#define CFG_NAND_ECCTOTAL	(CFG_NAND_ECCBYTES * CFG_NAND_ECCSTEPS)
#define CFG_NAND_ECCPOS		{0, 1, 2, 3, 6, 7}

#ifdef CFG_ENV_IS_IN_NAND
/*
 * For NAND booting the environment is embedded in the U-Boot image. Please take
 * look at the file board/amcc/canyonlands/u-boot-nand.lds for details.
 */
#define CFG_ENV_SIZE		CFG_NAND_BLOCK_SIZE
#define CFG_ENV_OFFSET		(CFG_NAND_U_BOOT_OFFS + CFG_ENV_SIZE)
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET + CFG_ENV_SIZE)
#endif

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI			/* The flash is CFI compatible	*/
#define CFG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/
#define CFG_FLASH_CFI_AMD_RESET	1	/* Use AMD (Spansion) reset cmd */

#define CFG_FLASH_BANKS_LIST    {CFG_FLASH_BASE}
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000		/* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE - CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR - CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif /* CFG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * NAND-FLASH related
 *----------------------------------------------------------------------*/
#define CFG_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS		1
#define CFG_NAND_BASE		(CFG_NAND_ADDR + CFG_NAND_CS)
#define CFG_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips	*/

/*------------------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------------*/
#if !defined(CONFIG_NAND_U_BOOT)
/*
 * NAND booting U-Boot version uses a fixed initialization, since the whole
 * I2C SPD DIMM autodetection/calibration doesn't fit into the 4k of boot
 * code.
 */
#define CONFIG_SPD_EEPROM	1	/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS	{0x50, 0x51}	/* SPD i2c spd addresses*/
#define CONFIG_DDR_ECC		1	/* with ECC support		*/
#define CONFIG_DDR_RQDC_FIXED	0x80000038 /* fixed value for RQDC	*/
#endif
#define CFG_MBYTES_SDRAM        256	/* 256MB			*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	    /* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			    /* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR		(0xa8>>1)
#define CFG_I2C_EEPROM_ADDR_LEN		1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS	3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10

/* I2C SYSMON (LM75, AD7414 is almost compatible)			*/
#define CONFIG_DTT_LM75		1		/* ON Semi's LM75	*/
#define CONFIG_DTT_AD7414	1		/* use AD7414		*/
#define CONFIG_DTT_SENSORS	{0}		/* Sensor addresses	*/
#define CFG_DTT_MAX_TEMP	70
#define CFG_DTT_LOW_TEMP	-30
#define CFG_DTT_HYSTERESIS	3

/* RTC configuration */
#define CONFIG_RTC_M41T62	1
#define CFG_I2C_RTC_ADDR	0x68

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#define CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address, See schematics	*/
#define CONFIG_PHY1_ADDR	1
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
/* Only Glacier (460GT) has 4 EMAC interfaces */
#ifdef CONFIG_460GT
#define CONFIG_PHY2_ADDR	2
#define CONFIG_PHY3_ADDR	3
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3
#endif
#define CONFIG_NET_MULTI	1

#define CONFIG_PHY_RESET	1	/* reset phy upon startup	*/
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#define CONFIG_PHY_DYNAMIC_ANEG	1

#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

/*-----------------------------------------------------------------------
 * USB-OHCI
 *----------------------------------------------------------------------*/
/* Only Canyonlands (460EX) has USB */
#ifdef CONFIG_460EX
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#undef CFG_OHCI_BE_CONTROLLER		/* 460EX has little endian descriptors	*/
#define CFG_OHCI_SWAP_REG_ACCESS	/* 460EX has little endian register	*/
#define CFG_OHCI_USE_NPS		/* force NoPowerSwitching mode		*/
#define CFG_USB_OHCI_REGS_BASE	(CFG_AHB_BASE | 0xd0000)
#define CFG_USB_OHCI_SLOT_NAME	"ppc440"
#define CFG_USB_OHCI_MAX_ROOT_PORTS 15
#endif

/*-----------------------------------------------------------------------
 * Default environment
 *----------------------------------------------------------------------*/
#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

/* Setup some board specific values for the default environment variables */
#ifdef CONFIG_CANYONLANDS
#define CONFIG_HOSTNAME		canyonlands
#define CFG_BOOTFILE		"bootfile=canyonlands/uImage\0"
#define CFG_DTBFILE		"fdt_file=canyonlands/canyonlands.dtb\0"
#else
#define CONFIG_HOSTNAME		glacier
#define CFG_BOOTFILE		"bootfile=glacier/uImage\0"
#define CFG_DTBFILE		"fdt_file=glacier/glacier.dtb\0"
#endif

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	CFG_BOOTFILE							\
	CFG_DTBFILE							\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"net_nfs=tftp 200000 ${bootfile};"				\
		"run nfsargs addip addtty;"				\
		"bootm 200000\0"					\
	"net_nfs_fdt=tftp 200000 ${bootfile};"				\
		"tftp ${fdt_addr} ${fdt_file};"				\
		"run nfsargs addip addtty;"				\
		"bootm 200000 - ${fdt_addr}\0"				\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"rootpath=/opt/eldk/ppc_4xxFP\0"				\
	"fdt_addr=400000\0"						\
	"kernel_addr=fc000000\0"					\
	"ramdisk_addr=fc200000\0"					\
	"initrd_high=30000000\0"					\
	"load=tftp 200000 ${hostname}/u-boot.bin\0"			\
	"update=protect off fffa0000 ffffffff;era fffa0000 ffffffff;"	\
		"cp.b ${fileaddr} fffa0000 ${filesize};"		\
		"setenv filesize;saveenv\0"				\
	"upd=run load update\0"						\
	"nload=tftp 200000 ${hostname}/u-boot-nand.bin\0"		\
	"nupdate=nand erase 0 60000;nand write 200000 0 60000;"		\
		"setenv filesize;saveenv\0"				\
	"nupd=run nload nupdate\0"					\
	"pciconfighost=1\0"						\
	"pcie_mode=RP:RP\0"						\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE		/* allow baudrate change	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_SUBNETMASK

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DTT
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#ifdef CONFIG_460EX
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_USB
#endif

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16	/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE /* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000 /* memtest works on		*/
#define CFG_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000  /* default load address	*/
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_AUTO_COMPLETE	1	/* add autocompletion support	*/
#define CONFIG_LOOPW		1	/* enable loopw command         */
#define CONFIG_MX_CYCLIC	1	/* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */
#define CFG_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

#define CFG_HUSH_PARSER		1	/* Use the HUSH parser		*/
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_PNP			/* do pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup  */
#define CONFIG_PCI_CONFIG_HOST_BRIDGE

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT		/* let board init pci target    */
#undef	CFG_PCI_MASTER_INIT

#define CFG_PCI_SUBSYS_VENDORID 0x1014	/* IBM				*/
#define CFG_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever			*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Internal Definitions
 */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/

/*
 * Canyonlands has 64MBytes of NOR FLASH (Spansion 29GL512), but the
 * boot EBC mapping only supports a maximum of 16MBytes
 * (4.ff00.0000 - 4.ffff.ffff).
 * To solve this problem, the FLASH has to get remapped to another
 * EBC address which accepts bigger regions:
 *
 * 0xfc00.0000 -> 4.cc00.0000
 */

#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
/* Memory Bank 3 (NOR-FLASH) initialization					*/
#define CFG_EBC_PB3AP		0x10055e00
#define CFG_EBC_PB3CR		(CFG_BOOT_BASE_ADDR | 0x9a000)

/* Memory Bank 0 (NAND-FLASH) initialization						*/
#define CFG_EBC_PB0AP		0x018003c0
#define CFG_EBC_PB0CR		(CFG_NAND_ADDR | 0x1E000) /* BAS=NAND,BS=1MB,BU=R/W,BW=32bit*/
#else
/* Memory Bank 0 (NOR-FLASH) initialization					*/
#define CFG_EBC_PB0AP		0x10055e00
#define CFG_EBC_PB0CR		(CFG_BOOT_BASE_ADDR | 0x9a000)

/* Memory Bank 3 (NAND-FLASH) initialization						*/
#define CFG_EBC_PB3AP		0x018003c0
#define CFG_EBC_PB3CR		(CFG_NAND_ADDR | 0x1E000) /* BAS=NAND,BS=1MB,BU=R/W,BW=32bit*/
#endif

/* Memory Bank 2 (CPLD) initialization						*/
#define CFG_EBC_PB2AP		0x00804240
#define CFG_EBC_PB2CR		(CFG_BCSR_BASE | 0x18000) /* BAS=CPLD,BS=1M,BU=RW,BW=32bit */

#define CFG_EBC_CFG		0xB8400000		/*  EBC0_CFG */

/*
 * PPC4xx GPIO Configuration
 */
#ifdef CONFIG_460EX
/* 460EX: Use USB configuration */
#define CFG_4xx_GPIO_TABLE { /*	  Out		  GPIO	Alternate1	Alternate2	Alternate3 */ \
{											\
/* GPIO Core 0 */									\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO0	GMC1TxD(0)	USB2HostD(0)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO1	GMC1TxD(1)	USB2HostD(1)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO2	GMC1TxD(2)	USB2HostD(2)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO3	GMC1TxD(3)	USB2HostD(3)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO4	GMC1TxD(4)	USB2HostD(4)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO5	GMC1TxD(5)	USB2HostD(5)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO6	GMC1TxD(6)	USB2HostD(6)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO7	GMC1TxD(7)	USB2HostD(7)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO8	GMC1RxD(0)	USB2OTGD(0)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO9	GMC1RxD(1)	USB2OTGD(1)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO10 GMC1RxD(2)	USB2OTGD(2)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO11 GMC1RxD(3)	USB2OTGD(3)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO12 GMC1RxD(4)	USB2OTGD(4)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO13 GMC1RxD(5)	USB2OTGD(5)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO14 GMC1RxD(6)	USB2OTGD(6)	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO15 GMC1RxD(7)	USB2OTGD(7)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL,  GPIO_OUT_0}, /* GPIO16 GMC1TxER	USB2HostStop	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO17 GMC1CD		USB2HostNext	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO18 GMC1RxER	USB2HostDir	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO19 GMC1TxEN	USB2OTGStop	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO20 GMC1CRS	USB2OTGNext	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO21 GMC1RxDV	USB2OTGDir	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO22 NFRDY				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO23 NFREN				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO24 NFWEN				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO25 NFCLE				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO26 NFALE				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO27 IRQ(0)				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO28 IRQ(1)				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO29 IRQ(2)				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO30 PerPar0	DMAReq2		IRQ(7)*/ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO31 PerPar1	DMAAck2		IRQ(8)*/ \
},											\
{											\
/* GPIO Core 1 */									\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO32 PerPar2	EOT2/TC2	IRQ(9)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO33 PerPar3	DMAReq3		IRQ(4)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT3, GPIO_OUT_1}, /* GPIO34 UART0_DCD_N	UART1_DSR_CTS_N	UART2_SOUT*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO35 UART0_8PIN_DSR_N UART1_RTS_DTR_N UART2_SIN*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO36 UART0_8PIN_CTS_N DMAAck3	UART3_SIN*/ \
{GPIO1_BASE, GPIO_BI , GPIO_ALT2, GPIO_OUT_0}, /* GPIO37 UART0_RTS_N	EOT3/TC3	UART3_SOUT*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_1}, /* GPIO38 UART0_DTR_N	UART1_SOUT	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO39 UART0_RI_N	UART1_SIN	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO40 IRQ(3)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO41 CS(1)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO42 CS(2)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO43 CS(3)		DMAReq1		IRQ(10)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO44 CS(4)		DMAAck1		IRQ(11)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO45 CS(5)		EOT/TC1		IRQ(12)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO46 PerAddr(5)	DMAReq0		IRQ(13)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO47 PerAddr(6)	DMAAck0		IRQ(14)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO48 PerAddr(7)	EOT/TC0		IRQ(15)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO49  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO50  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO51  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO52  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO53  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO54  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO55  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO56  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO57  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO58  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO59  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO60  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO61  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO62  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO63  Unselect via TraceSelect Bit	*/	\
}											\
}
#else
/* 460GT: Use EMAC2+3 configuration */
#define CFG_4xx_GPIO_TABLE { /*	  Out		  GPIO	Alternate1	Alternate2	Alternate3 */ \
{											\
/* GPIO Core 0 */									\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO0	GMC1TxD(0)	USB2HostD(0)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO1	GMC1TxD(1)	USB2HostD(1)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO2	GMC1TxD(2)	USB2HostD(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO3	GMC1TxD(3)	USB2HostD(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO4	GMC1TxD(4)	USB2HostD(4)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO5	GMC1TxD(5)	USB2HostD(5)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO6	GMC1TxD(6)	USB2HostD(6)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO7	GMC1TxD(7)	USB2HostD(7)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO8	GMC1RxD(0)	USB2OTGD(0)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO9	GMC1RxD(1)	USB2OTGD(1)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO10 GMC1RxD(2)	USB2OTGD(2)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO11 GMC1RxD(3)	USB2OTGD(3)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO12 GMC1RxD(4)	USB2OTGD(4)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO13 GMC1RxD(5)	USB2OTGD(5)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO14 GMC1RxD(6)	USB2OTGD(6)	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO15 GMC1RxD(7)	USB2OTGD(7)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO16 GMC1TxER	USB2HostStop	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO17 GMC1CD		USB2HostNext	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO18 GMC1RxER	USB2HostDir	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO19 GMC1TxEN	USB2OTGStop	*/	\
{GPIO0_BASE, GPIO_BI , GPIO_ALT1, GPIO_OUT_0}, /* GPIO20 GMC1CRS	USB2OTGNext	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO21 GMC1RxDV	USB2OTGDir	*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO22 NFRDY				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO23 NFREN				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO24 NFWEN				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO25 NFCLE				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO26 NFALE				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO27 IRQ(0)				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO28 IRQ(1)				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO29 IRQ(2)				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO30 PerPar0	DMAReq2		IRQ(7)*/ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO31 PerPar1	DMAAck2		IRQ(8)*/ \
},											\
{											\
/* GPIO Core 1 */									\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO32 PerPar2	EOT2/TC2	IRQ(9)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO33 PerPar3	DMAReq3		IRQ(4)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT3, GPIO_OUT_1}, /* GPIO34 UART0_DCD_N	UART1_DSR_CTS_N	UART2_SOUT*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO35 UART0_8PIN_DSR_N UART1_RTS_DTR_N UART2_SIN*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO36 UART0_8PIN_CTS_N DMAAck3	UART3_SIN*/ \
{GPIO1_BASE, GPIO_BI , GPIO_ALT2, GPIO_OUT_0}, /* GPIO37 UART0_RTS_N	EOT3/TC3	UART3_SOUT*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_1}, /* GPIO38 UART0_DTR_N	UART1_SOUT	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO39 UART0_RI_N	UART1_SIN	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO40 IRQ(3)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO41 CS(1)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO42 CS(2)				*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO43 CS(3)		DMAReq1		IRQ(10)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO44 CS(4)		DMAAck1		IRQ(11)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO45 CS(5)		EOT/TC1		IRQ(12)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO46 PerAddr(5)	DMAReq0		IRQ(13)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO47 PerAddr(6)	DMAAck0		IRQ(14)*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO48 PerAddr(7)	EOT/TC0		IRQ(15)*/ \
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO49  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO50  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO51  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO52  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO53  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO54  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO55  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO56  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO57  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO58  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO59  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO60  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO61  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO62  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO63  Unselect via TraceSelect Bit	*/	\
}											\
}
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#endif	/* __CONFIG_H */

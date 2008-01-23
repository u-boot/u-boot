/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com.
 * Based on the sequoia configuration file.
 *
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
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
 * PMC440.h - configuration for esd PMC440 boards
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_440EPX		1	/* Specific PPC440EPx   */
#define CONFIG_440		1	/* ... PPC440 family    */
#define CONFIG_4xx		1	/* ... PPC4xx family    */

#define CONFIG_SYS_CLK_FREQ	33333400

#if 0 /* temporary disabled because OS/9 does not like dcache on startup */
#define CONFIG_4xx_DCACHE		/* enable dcache        */
#endif

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1	/* Call misc_init_r     */
#define CONFIG_BOARD_TYPES	1	/* support board types  */
/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(384  * 1024)	/* Reserve 384 kB for Monitor   */
#define CFG_MALLOC_LEN		(1024 * 1024)	/* Reserve 256 kB for malloc()  */

#define CONFIG_PRAM		0	/* use pram variable to overwrite */

#define CFG_BOOT_BASE_ADDR	0xf0000000
#define CFG_SDRAM_BASE		0x00000000	/* _must_ be 0          */
#define CFG_FLASH_BASE		0xfc000000	/* start of FLASH       */
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_NAND_ADDR		0xd0000000	/* NAND Flash           */
#define CFG_OCM_BASE		0xe0010000	/* ocm                  */
#define CFG_OCM_DATA_ADDR	CFG_OCM_BASE
#define CFG_PCI_BASE		0xe0000000	/* Internal PCI regs    */
#define CFG_PCI_MEMBASE		0x80000000	/* mapped pci memory    */
#define CFG_PCI_MEMBASE1	CFG_PCI_MEMBASE  + 0x10000000
#define CFG_PCI_MEMBASE2	CFG_PCI_MEMBASE1 + 0x10000000
#define CFG_PCI_MEMBASE3	CFG_PCI_MEMBASE2 + 0x10000000
#define CFG_PCI_MEMSIZE		0x80000000	/* 2GB! */

/* Don't change either of these */
#define CFG_PERIPHERAL_BASE	0xef600000	/* internal peripherals */

#define CFG_USB2D0_BASE		0xe0000100
#define CFG_USB_DEVICE		0xe0000000
#define CFG_USB_HOST		0xe0000400
#define CFG_FPGA_BASE0		0xef000000	/* 32 bit */
#define CFG_FPGA_BASE1		0xef100000	/* 16 bit */

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/
/* 440EPx/440GRx have 16KB of internal SRAM, so no need for D-Cache	*/
#define CFG_INIT_RAM_ADDR	CFG_OCM_BASE	/* OCM                  */
#define CFG_INIT_RAM_END	(4 << 10)
#define CFG_GBL_DATA_SIZE	256	/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_POST_WORD_ADDR

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef CFG_EXT_SERIAL_CLOCK
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI	1
#undef CONFIG_UART1_CONSOLE	/* console on front panel */

#define CFG_BAUDRATE_TABLE						\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CFG_ENV_IS_IN_EEPROM	1	/* use FLASH for environment vars */
#else
#define CFG_ENV_IS_IN_NAND	1	/* use NAND for environment vars */
#define CFG_ENV_IS_EMBEDDED	1	/* use embedded environment */
#endif

/*-----------------------------------------------------------------------
 * RTC
 *----------------------------------------------------------------------*/
#define CONFIG_RTC_RX8025

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI		/* The flash is CFI compatible  */
#define CFG_FLASH_CFI_DRIVER	/* Use common CFI driver        */

#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks           */
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip    */

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)      */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)      */

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)     */
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection        */

#define CFG_FLASH_EMPTY_INFO	/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash        */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000	/* size of one complete sector          */
#define CFG_ENV_ADDR		((-CFG_MONITOR_LEN)-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector     */

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif

#ifdef CFG_ENV_IS_IN_EEPROM
#define CFG_ENV_OFFSET		0	/* environment starts at the beginning of the EEPROM */
#define CFG_ENV_SIZE		0x1000	/* 4096 bytes may be used for env vars */
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
#if defined (CONFIG_NAND_U_BOOT)
#define CFG_NAND_BOOT_SPL_SRC	0xfffff000	/* SPL location                 */
#define CFG_NAND_BOOT_SPL_SIZE	(4 << 10)	/* SPL size                     */
#define CFG_NAND_BOOT_SPL_DST	(CFG_OCM_BASE + (12 << 10)) /* Copy SPL here    */
#define CFG_NAND_U_BOOT_DST	0x01000000	/* Load NUB to this addr        */
#define CFG_NAND_U_BOOT_START	CFG_NAND_U_BOOT_DST /* Start NUB from this addr */
#define CFG_NAND_BOOT_SPL_DELTA	(CFG_NAND_BOOT_SPL_SRC - CFG_NAND_BOOT_SPL_DST)

/*
 * Define the partitioning of the NAND chip (only RAM U-Boot is needed here)
 */
#define CFG_NAND_U_BOOT_OFFS	(16 << 10)	/* Offset to RAM U-Boot image   */
#define CFG_NAND_U_BOOT_SIZE	(384 << 10)	/* Size of RAM U-Boot image     */

/*
 * Now the NAND chip has to be defined (no autodetection used!)
 */
#define CFG_NAND_PAGE_SIZE	512	/* NAND chip page size          */
#define CFG_NAND_BLOCK_SIZE	(16 << 10) /* NAND chip block size      */
#define CFG_NAND_PAGE_COUNT	32	/* NAND chip page count         */
#define CFG_NAND_BAD_BLOCK_POS	5	/* Location of bad block marker */
#undef CFG_NAND_4_ADDR_CYCLE		/* No fourth addr used (<=32MB) */

#define CFG_NAND_ECCSIZE	256
#define CFG_NAND_ECCBYTES	3
#define CFG_NAND_ECCSTEPS	(CFG_NAND_PAGE_SIZE / CFG_NAND_ECCSIZE)
#define CFG_NAND_OOBSIZE	16
#define CFG_NAND_ECCTOTAL	(CFG_NAND_ECCBYTES * CFG_NAND_ECCSTEPS)
#define CFG_NAND_ECCPOS		{0, 1, 2, 3, 6, 7}
#endif

#ifdef CFG_ENV_IS_IN_NAND
/*
 * For NAND booting the environment is embedded in the U-Boot image. Please take
 * look at the file board/amcc/sequoia/u-boot-nand.lds for details.
 */
#define CFG_ENV_SIZE		CFG_NAND_BLOCK_SIZE
#define CFG_ENV_OFFSET		(CFG_NAND_U_BOOT_OFFS + CFG_ENV_SIZE)
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET + CFG_ENV_SIZE)
#endif

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CFG_MBYTES_SDRAM	(256)	/* 256MB                        */
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CONFIG_DDR_DATA_EYE	/* use DDR2 optimization        */
#endif

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	/* I2C with hardware support    */
#undef	CONFIG_SOFT_I2C		/* I2C bit-banged               */
#define CFG_I2C_SPEED		100000	/* I2C speed and slave address  */
#define CFG_I2C_SLAVE		0x7F

#define CONFIG_I2C_CMD_TREE	1
#define CONFIG_I2C_MULTI_BUS	1

#define CFG_I2C_MULTI_EEPROMS

#define CFG_I2C_EEPROM_ADDR		0x54
#define CFG_I2C_EEPROM_ADDR_LEN		2
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS	5
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	10
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x01

#define CFG_EEPROM_WREN			1
#define CFG_I2C_BOOT_EEPROM_ADDR	0x52

/*
 * standard dtt sensor configuration - bottom bit will determine local or
 * remote sensor of the TMP401
 */
#define CONFIG_DTT_SENSORS		{ 0, 1 }

/*
 * The PMC440 uses a TI TMP401 temperature sensor. This part
 * is basically compatible to the ADM1021 that is supported
 * by U-Boot.
 *
 * - i2c addr 0x4c
 * - conversion rate 0x02 = 0.25 conversions/second
 * - ALERT ouput disabled
 * - local temp sensor enabled, min set to 0 deg, max set to 70 deg
 * - remote temp sensor enabled, min set to 0 deg, max set to 70 deg
 */
#define CONFIG_DTT_ADM1021
#define CFG_DTT_ADM1021		{ { 0x4c, 0x02, 0, 1, 70, 0, 1, 70, 0} }

#define CONFIG_PREBOOT		/* enable preboot variable */

#undef	CONFIG_BOOTARGS

/* Setup some board specific values for the default environment variables */
#define CONFIG_HOSTNAME		pmc440
#define CFG_BOOTFILE		"bootfile=/tftpboot/pmc440/uImage\0"
#define CFG_ROOTPATH		"rootpath=/opt/eldk_410/ppc_4xx\0"

#define CONFIG_EXTRA_ENV_SETTINGS					\
	CFG_BOOTFILE							\
	CFG_ROOTPATH							\
	"netdev=eth0\0"							\
	"ethrotate=no\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
	"nfsroot=${serverip}:${rootpath}\0"				\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"		\
	":${hostname}:${netdev}:off panic=1\0"				\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0" \
	"flash_nfs=run nfsargs addip addtty;"				\
	"bootm ${kernel_addr}\0"					\
	"flash_self=run ramargs addip addtty;"				\
	"bootm ${kernel_addr} ${ramdisk_addr}\0"			\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"     \
	"bootm\0"							\
	"kernel_addr=FC000000\0"					\
	"ramdisk_addr=FC180000\0"					\
	"load=tftp 200000 /tftpboot/pmc440/u-boot.bin\0"		\
	"update=protect off FFFA0000 FFFFFFFF;era FFFA0000 FFFFFFFF;"	\
	"cp.b 200000 FFFA0000 60000\0"					\
	""

#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds     */

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download  */
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change        */

#define CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management           */
#define CONFIG_PHY_ADDR		0	/* PHY address, See schematics  */

#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0
#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NET_MULTI	1
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"   */
#define CONFIG_PHY1_ADDR	1
#define CONFIG_RESET_PHY_R	1

/* USB */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_USB_STORAGE
#define CFG_OHCI_BE_CONTROLLER

#define CFG_USB_OHCI_BOARD_INIT 1
#define CFG_USB_OHCI_CPU_INIT	1
#define CFG_USB_OHCI_REGS_BASE	CFG_USB_HOST
#define CFG_USB_OHCI_SLOT_NAME	"ppc440"
#define CFG_USB_OHCI_MAX_ROOT_PORTS	15

/* Comment this out to enable USB 1.1 device */
#define USB_2_0_DEVICE

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#include <config_cmd_default.h>

#define CONFIG_CMD_BSP
#define CONFIG_CMD_DATE
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DTT
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_USB
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM

/* POST support */
#define CONFIG_POST		(CFG_POST_MEMORY |	\
				 CFG_POST_CPU    |	\
				 CFG_POST_UART   |	\
				 CFG_POST_I2C    |	\
				 CFG_POST_CACHE  |	\
				 CFG_POST_FPU    |	\
				 CFG_POST_ETHER  |	\
				 CFG_POST_SPR)

#define CFG_POST_WORD_ADDR	(CFG_GBL_DATA_OFFSET - 0x4)

/* esd expects pram at end of physical memory.
 * So no logbuffer at the moment.
 */
#if 0
#define CONFIG_LOGBUFFER
#endif
#define CFG_POST_CACHE_ADDR	0x10000000	/* free virtual address     */

#define CFG_CONSOLE_IS_IN_ENV	/* Otherwise it catches logbuffer as output */

#define CONFIG_SUPPORT_VFAT

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CFG_LONGHELP			/* undef to save memory         */
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt       */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size      */
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size      */
#endif
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS		16	/* max number of command args   */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */

#define CFG_MEMTEST_START	0x0400000	/* memtest works on          */
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM       */

#define CFG_LOAD_ADDR		0x100000	/* default load address      */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

#define CONFIG_CMDLINE_EDITING	1	/* add command line history     */
#define CONFIG_LOOPW		1	/* enable loopw command         */
#define CONFIG_MX_CYCLIC	1	/* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

#define CONFIG_AUTOBOOT_KEYED	1
#define CONFIG_AUTOBOOT_PROMPT	"Press SPACE to abort autoboot in %d seconds\n"
#undef CONFIG_AUTOBOOT_DELAY_STR
#define CONFIG_AUTOBOOT_STOP_STR " "

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI		/* include pci support          */
#define CONFIG_PCI_PNP		/* do (not) pci plug-and-play   */
#define CFG_PCI_CACHE_LINE_SIZE	0	/* to avoid problems with PNP   */
#define CONFIG_PCI_SCAN_SHOW	/* show pci devices on startup  */
#define CFG_PCI_TARGBASE	0x80000000 /* PCIaddr mapped to CFG_PCI_MEMBASE */

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT
#define CFG_PCI_MASTER_INIT

/* PCI identification */
#define CFG_PCI_SUBSYS_VENDORID 0x12FE	/* PCI Vendor ID: esd gmbh      */
#define CFG_PCI_SUBSYS_ID_NONMONARCH 0x0441	/* PCI Device ID: Non-Monarch */
#define CFG_PCI_SUBSYS_ID_MONARCH 0x0440	/* PCI Device ID: Monarch */
#define CFG_PCI_CLASSCODE_NONMONARCH	PCI_CLASS_PROCESSOR_POWERPC
#define CFG_PCI_CLASSCODE_MONARCH	PCI_CLASS_BRIDGE_HOST

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FPGA stuff
 *----------------------------------------------------------------------*/
#define CONFIG_FPGA
#define CONFIG_FPGA_XILINX
#define CONFIG_FPGA_SPARTAN2
#define CONFIG_FPGA_SPARTAN3

#define CONFIG_FPGA_COUNT	2
/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/

/*
 * On Sequoia CS0 and CS3 are switched when configuring for NAND booting
 */
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CFG_NAND_CS		2	/* NAND chip connected to CSx   */

/* Memory Bank 0 (NOR-FLASH) initialization */
#define CFG_EBC_PB0AP		0x03017200
#define CFG_EBC_PB0CR		(CFG_FLASH_BASE | 0xda000)

/* Memory Bank 2 (NAND-FLASH) initialization */
#define CFG_EBC_PB2AP		0x018003c0
#define CFG_EBC_PB2CR		(CFG_NAND_ADDR | 0x1c000)
#else
#define CFG_NAND_CS		0	/* NAND chip connected to CSx   */
/* Memory Bank 2 (NOR-FLASH) initialization */
#define CFG_EBC_PB2AP		0x03017200
#define CFG_EBC_PB2CR		(CFG_FLASH_BASE | 0xda000)

/* Memory Bank 0 (NAND-FLASH) initialization */
#define CFG_EBC_PB0AP		0x018003c0
#define CFG_EBC_PB0CR		(CFG_NAND_ADDR | 0x1c000)
#endif

/* Memory Bank 4 (FPGA / 32Bit) initialization */
#define CFG_EBC_PB4AP		0x03840f40	/* BME=0,TWT=7,CSN=1,TH=7,RE=1,SOR=0,BEM=1 */
#define CFG_EBC_PB4CR		(CFG_FPGA_BASE0 | 0x1c000)	/* BS=1M,BU=R/W,BW=32bit */

/* Memory Bank 5 (FPGA / 16Bit) initialization */
#define CFG_EBC_PB5AP		0x03840f40	/* BME=0,TWT=3,CSN=1,TH=0,RE=1,SOR=0,BEM=1 */
#define CFG_EBC_PB5CR		(CFG_FPGA_BASE1 | 0x1a000)	/* BS=1M,BU=R/W,BW=16bit */

/*-----------------------------------------------------------------------
 * NAND FLASH
 *----------------------------------------------------------------------*/
#define CFG_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS		1
#define CFG_NAND_BASE		(CFG_NAND_ADDR + CFG_NAND_CS)
#define CFG_NAND_SELECT_DEVICE	1 /* nand driver supports mutipl. chips */
#define CFG_NAND_QUIET_TEST	1

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01	/* Normal Power-On: Boot from FLASH     */
#define BOOTFLAG_WARM	0x02	/* Software reboot                      */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#endif /* __CONFIG_H */

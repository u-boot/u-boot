/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/************************************************************************
 * kilauea.h - configuration for AMCC Kilauea (405EX)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_KILAUEA		1		/* Board is Kilauea	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_405EX		1		/* Specifc 405EX support*/
#define CONFIG_SYS_CLK_FREQ	33333333	/* ext frequency to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1		/* Call misc_init_r	*/
#define CONFIG_BOARD_EMAC_COUNT

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFC000000
#define CFG_NAND_ADDR		0xF8000000
#define CFG_FPGA_BASE		0xF0000000
#define CFG_PERIPHERAL_BASE	0xEF600000      /* internal peripherals*/
#define CFG_MONITOR_LEN		(384 * 1024)	/* Reserve 384 kB for Monitor	*/
#define CFG_MALLOC_LEN		(512 * 1024)	/* Reserve 512 kB for malloc()	*/
#define CFG_MONITOR_BASE	(TEXT_BASE)

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/
#define CFG_INIT_RAM_ADDR	0x02000000	/* inside of SDRAM	*/
#define CFG_INIT_RAM_END	(4 << 10)
#define CFG_GBL_DATA_SIZE	256		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
/* reserve some memory for POST and BOOT limit info */
#define CFG_INIT_SP_OFFSET	(CFG_GBL_DATA_OFFSET - 16)

/* extra data in init-ram */
#define CFG_POST_WORD_ADDR	(CFG_GBL_DATA_OFFSET - 4)
#define CFG_POST_MAGIC		(CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET - 8)
#define CFG_POST_VAL		(CFG_INIT_RAM_ADDR + CFG_GBL_DATA_OFFSET - 12)
#define CFG_OCM_DATA_ADDR	CFG_INIT_RAM_ADDR /* for commproc.c	*/

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CFG_EXT_SERIAL_CLOCK	11059200	/* ext. 11.059MHz clk	*/
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI     1
/* define this if you want console on UART1 */
#undef CONFIG_UART1_CONSOLE

#define CFG_BAUDRATE_TABLE						\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#else
#define CFG_ENV_IS_IN_NAND	1	/* use NAND for environment vars	*/
#define CFG_ENV_IS_EMBEDDED	1	/* use embedded environment */
#endif

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI			/* The flash is CFI compatible	*/
#define CFG_FLASH_CFI_DRIVER		/* Use common CFI driver	*/

#define CFG_FLASH_BANKS_LIST    {CFG_FLASH_BASE}
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x20000 	/* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif /* CFG_ENV_IS_IN_FLASH */

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
#define CFG_NAND_BOOT_SPL_SRC	0xfffff000	/* SPL location			*/
#define CFG_NAND_BOOT_SPL_SIZE	(4 << 10)	/* SPL size			*/
#define CFG_NAND_BOOT_SPL_DST	0x00800000	/* Copy SPL here		*/
#define CFG_NAND_U_BOOT_DST	0x01000000	/* Load NUB to this addr	*/
#define CFG_NAND_U_BOOT_START	CFG_NAND_U_BOOT_DST /* Start NUB from this addr	*/
#define CFG_NAND_BOOT_SPL_DELTA	(CFG_NAND_BOOT_SPL_SRC - CFG_NAND_BOOT_SPL_DST)

/*
 * Define the partitioning of the NAND chip (only RAM U-Boot is needed here)
 */
#define CFG_NAND_U_BOOT_OFFS	(16 << 10)	/* Offset to RAM U-Boot image	*/
#define CFG_NAND_U_BOOT_SIZE	(384 << 10)	/* Size of RAM U-Boot image	*/

/*
 * Now the NAND chip has to be defined (no autodetection used!)
 */
#define CFG_NAND_PAGE_SIZE	512		/* NAND chip page size		*/
#define CFG_NAND_BLOCK_SIZE	(16 << 10)	/* NAND chip block size		*/
#define CFG_NAND_PAGE_COUNT	32		/* NAND chip page count		*/
#define CFG_NAND_BAD_BLOCK_POS	5		/* Location of bad block marker	*/
#define CFG_NAND_4_ADDR_CYCLE	1		/* Fourth addr used (>32MB)	*/

#define CFG_NAND_ECCSIZE	256
#define CFG_NAND_ECCBYTES	3
#define CFG_NAND_ECCSTEPS	(CFG_NAND_PAGE_SIZE / CFG_NAND_ECCSIZE)
#define CFG_NAND_OOBSIZE	16
#define CFG_NAND_ECCTOTAL	(CFG_NAND_ECCBYTES * CFG_NAND_ECCSTEPS)
#define CFG_NAND_ECCPOS		{0, 1, 2, 3, 6, 7}

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
 * NAND FLASH
 *----------------------------------------------------------------------*/
#define CFG_MAX_NAND_DEVICE	1
#define NAND_MAX_CHIPS		1
#define CFG_NAND_BASE		(CFG_NAND_ADDR + CFG_NAND_CS)
#define CFG_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips	*/

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CFG_MBYTES_SDRAM        (256)		/* 256MB			*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	6	/* 24C02 requires 5ms delay */
#define CFG_I2C_EEPROM_ADDR	0x52	/* I2C boot EEPROM (24C02BN)	*/
#define CFG_I2C_EEPROM_ADDR_LEN	1	/* Bytes of address		*/

/* Standard DTT sensor configuration */
#define CONFIG_DTT_DS1775	1
#define CONFIG_DTT_SENSORS	{ 0 }
#define CFG_I2C_DTT_ADDR	0x48

/* RTC configuration */
#define CONFIG_RTC_DS1338	1
#define CFG_I2C_RTC_ADDR	0x68

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#define CONFIG_M88E1111_PHY	1
#define CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		1	/* PHY address, See schematics	*/

#define CONFIG_PHY_RESET	1	/* reset phy upon startup	*/
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0		1

#define CONFIG_NET_MULTI	1
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"   */
#define CONFIG_PHY1_ADDR	2

#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"logversion=2\0"						\
	"netdev=eth0\0"							\
	"hostname=kilauea\0"						\
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
	"rootpath=/opt/eldk/ppc_4xx\0"					\
	"bootfile=kilauea/uImage\0"					\
	"fdt_file=kilauea/kilauea.dtb\0"				\
	"fdt_addr=400000\0"						\
	"kernel_addr=fc000000\0"					\
	"ramdisk_addr=fc200000\0"					\
	"initrd_high=30000000\0"					\
	"load=tftp 200000 kilauea/u-boot.bin\0"				\
	"update=protect off fffa0000 ffffffff;era fffa0000 ffffffff;"	\
		"cp.b ${fileaddr} fffa0000 ${filesize};"		\
		"setenv filesize;saveenv\0"				\
	"upd=run load update\0"						\
	"nload=tftp 200000 kilauea/u-boot-nand.bin\0"			\
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

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_DTT
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_LOG
#define CONFIG_CMD_MII
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SNTP

/* POST support */
#define CONFIG_POST		(CFG_POST_MEMORY	| \
				 CFG_POST_CACHE		| \
				 CFG_POST_CPU		| \
				 CFG_POST_ETHER		| \
				 CFG_POST_I2C		| \
				 CFG_POST_MEMORY	| \
				 CFG_POST_UART)

/* Define here the base-addresses of the UARTs to test in POST */
#define CFG_POST_UART_TABLE	{UART0_BASE, UART1_BASE}

#define CONFIG_LOGBUFFER
#define CFG_POST_CACHE_ADDR	0x00800000 /* free virtual address	*/

#define CFG_CONSOLE_IS_IN_ENV /* Otherwise it catches logbuffer as output */

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE              (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	        16	/* max number of command args	*/
#define CFG_BARGSIZE	        CFG_CBSIZE /* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000 /* memtest works on		*/
#define CFG_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000  /* default load address	*/
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ		        1000	/* decrementer freq: 1 ms ticks	*/

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */
#define CFG_CONSOLE_INFO_QUIET	1	/* don't print console @ startup*/

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_PNP		1	/* do pci plug-and-play		*/
#define CONFIG_PCI_SCAN_SHOW	1	/* show pci devices on startup	*/
#define CONFIG_PCI_CONFIG_HOST_BRIDGE

/*-----------------------------------------------------------------------
 * PCIe stuff
 *----------------------------------------------------------------------*/
#define CFG_PCIE_MEMBASE	0x90000000	/* mapped PCIe memory	*/
#define CFG_PCIE_MEMSIZE	0x08000000      /* 128 Meg, smallest incr per port */

#define	CFG_PCIE0_CFGBASE	0xa0000000      /* remote access */
#define	CFG_PCIE0_XCFGBASE	0xb0000000      /* local access */
#define	CFG_PCIE0_CFGMASK	0xe0000001      /* 512 Meg */

#define	CFG_PCIE1_CFGBASE	0xc0000000      /* remote access */
#define	CFG_PCIE1_XCFGBASE	0xd0000000      /* local access */
#define	CFG_PCIE1_CFGMASK	0xe0000001      /* 512 Meg */

#define	CFG_PCIE0_UTLBASE	0xef502000
#define	CFG_PCIE1_UTLBASE	0xef503000

/* base address of inbound PCIe window */
#define CFG_PCIE_INBOUND_BASE	0x0000000000000000ULL

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
/* booting from NAND, so NAND chips select has to be on CS 0 */
#define CFG_NAND_CS		0		/* NAND chip connected to CSx	*/

/* Memory Bank 1 (NOR-FLASH) initialization					*/
#define CFG_EBC_PB1AP		0x05806500
#define CFG_EBC_PB1CR           0xFC0DA000  /* BAS=0xFC0,BS=64MB,BU=R/W,BW=16bit*/

/* Memory Bank 0 (NAND-FLASH) initialization					*/
#define CFG_EBC_PB0AP		0x018003c0
#define CFG_EBC_PB0CR		(CFG_NAND_ADDR | 0x1e000)
#else
#define CFG_NAND_CS		1		/* NAND chip connected to CSx	*/

/* Memory Bank 0 (NOR-FLASH) initialization					*/
#define CFG_EBC_PB0AP		0x05806500
#define CFG_EBC_PB0CR           0xFC0DA000  /* BAS=0xFC0,BS=64MB,BU=R/W,BW=16bit*/

/* Memory Bank 1 (NAND-FLASH) initialization					*/
#define CFG_EBC_PB1AP		0x018003c0
#define CFG_EBC_PB1CR		(CFG_NAND_ADDR | 0x1e000)
#endif

/* Memory Bank 2 (FPGA) initialization						*/
#define CFG_EBC_PB2AP           0x9400C800
#define CFG_EBC_PB2CR           0xF0018000 /*  BAS=0x800,BS=1MB,BU=R/W,BW=8bit	*/

#define CFG_EBC_CFG		0x7FC00000 /*  EBC0_CFG */

/*-----------------------------------------------------------------------
 * GPIO Setup
 *----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------
 * Definitions for GPIO setup (PPC405EX specific)
 *
 * GPIO0[0-3]      - EBC data 0-3	inputs/outputs
 * GPIO0[4-7]      - USB data 4-7	inputs/outputs
 * GPIO0[8-11]     - NFCE# 1-3 inputs/outputs, GPIO11: IRQ6 inputs
 * GPIO0[12-15]    - USB data 0-3	inputs/outputs
 * GPIO0[16-21]    - UART0 control signal inputs/outputs
 *
 * GPIO0[22-25,27] - EBC control signal inputs/outputs
 * GPIO0[26]	   - Instruction trace outputs
 * GPIO0[28]	   - Float, N/C
 * GPIO0[29-31]    - DMA control signal inputs/outputs
 */
#define CFG_GPIO0_OSRL		0x00AA54AA
#define CFG_GPIO0_OSRH		0x21800000
#define CFG_GPIO0_TSRL		0x00AA55AA
#define CFG_GPIO0_TSRH		0xA5A00000

#define CFG_GPIO0_ISR1L		0x00000100
#define CFG_GPIO0_ISR1H		0x04000000
#define CFG_GPIO0_ISR2L		0x00550055
#define CFG_GPIO0_ISR2H		0x40100000

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*-----------------------------------------------------------------------
 * Some Kilauea stuff..., mainly fpga registers
 */
#define CFG_FPGA_REG_BASE		CFG_FPGA_BASE
#define CFG_FPGA_FIFO_BASE		(in32(CFG_FPGA_BASE) | (1 << 11))

/* interrupt */
#define CFG_FPGA_SLIC0_R_DPRAM_INT	0x80000000
#define CFG_FPGA_SLIC0_W_DPRAM_INT	0x40000000
#define CFG_FPGA_SLIC1_R_DPRAM_INT	0x20000000
#define CFG_FPGA_SLIC1_W_DPRAM_INT	0x10000000
#define CFG_FPGA_PHY0_INT		0x08000000
#define CFG_FPGA_PHY1_INT		0x04000000
#define CFG_FPGA_SLIC0_INT		0x02000000
#define CFG_FPGA_SLIC1_INT		0x01000000

/* DPRAM setting */
/* 00: 32B; 01: 64B; 10: 128B; 11: 256B  */
#define CFG_FPGA_DPRAM_R_INT_LINE	0x00400000	/* 64 B */
#define CFG_FPGA_DPRAM_W_INT_LINE	0x00100000	/* 64 B */
#define CFG_FPGA_DPRAM_RW_TYPE		0x00080000
#define CFG_FPGA_DPRAM_RST		0x00040000
#define CFG_FPGA_UART0_FO		0x00020000
#define CFG_FPGA_UART1_FO		0x00010000

/* loopback */
#define CFG_FPGA_CHIPSIDE_LOOPBACK	0x00004000
#define CFG_FPGA_LINESIDE_LOOPBACK	0x00008000
#define CFG_FPGA_SLIC0_ENABLE		0x00002000
#define CFG_FPGA_SLIC1_ENABLE		0x00001000
#define CFG_FPGA_SLIC0_CS		0x00000800
#define CFG_FPGA_SLIC1_CS		0x00000400
#define CFG_FPGA_USER_LED0		0x00000200
#define CFG_FPGA_USER_LED1		0x00000100

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#define OF_CPU			"PowerPC,405EX@0"

#endif	/* __CONFIG_H */

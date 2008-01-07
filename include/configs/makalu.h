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
 * makalu.h - configuration for AMCC Makalu (405EX)
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_MAKALU		1		/* Board is Makalu	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_405EX		1		/* Specifc 405EX support*/
#define CONFIG_SYS_CLK_FREQ	33330000	/* ext frequency to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1		/* Call board_early_init_f */
#define CONFIG_MISC_INIT_R	1		/* Call misc_init_r	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFC000000
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
#undef CFG_EXT_SERIAL_CLOCK			/* no ext. clk		*/
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI     1
/* define this if you want console on UART1 */
#undef CONFIG_UART1_CONSOLE

#define CFG_BAUDRATE_TABLE						\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

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

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CFG_MBYTES_SDRAM	256

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
#define CONFIG_RTC_X1205	1
#define CFG_I2C_RTC_ADDR	0x6f

/*-----------------------------------------------------------------------
 * Ethernet
 *----------------------------------------------------------------------*/
#define CONFIG_M88E1111_PHY	1
#define CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		6	/* PHY address, See schematics	*/

#define CONFIG_PHY_RESET	1	/* reset phy upon startup	*/
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CONFIG_HAS_ETH0		1

#define CONFIG_NET_MULTI	1
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"   */
#define CONFIG_PHY1_ADDR	0

#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"logversion=2\0"						\
	"netdev=eth0\0"							\
	"hostname=makalu\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"addmisc=setenv bootargs ${bootargs} rtc-x1205.probe=0,0x6f\0"	\
	"net_nfs=tftp 200000 ${bootfile};"				\
		"run nfsargs addip addtty addmisc;"			\
		"bootm 200000\0"					\
	"net_nfs_fdt=tftp 200000 ${bootfile};"				\
		"tftp ${fdt_addr} ${fdt_file};"				\
		"run nfsargs addip addtty addmisc;"			\
		"bootm 200000 - ${fdt_addr}\0"				\
	"flash_nfs=run nfsargs addip addtty addmisc;"			\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty addmisc;"			\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"rootpath=/opt/eldk/ppc_4xx\0"					\
	"bootfile=makalu/uImage\0"					\
	"fdt_file=makalu/makalu.dtb\0"					\
	"fdt_addr=400000\0"						\
	"kernel_addr=fc000000\0"					\
	"ramdisk_addr=fc200000\0"					\
	"initrd_high=30000000\0"					\
	"load=tftp 200000 makalu/u-boot.bin\0"				\
	"update=protect off fffa0000 ffffffff;era fffa0000 ffffffff;"	\
		"cp.b ${fileaddr} fffa0000 ${filesize};"		\
		"setenv filesize;saveenv\0"				\
	"upd=run load update\0"						\
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
/* Memory Bank 0 (NOR-FLASH) initialization					*/
#define CFG_EBC_PB0AP		0x08033700
#define CFG_EBC_PB0CR		(CFG_FLASH_BASE | 0xda000)

/* Memory Bank 2 (CPLD) initialization						*/
#define CFG_EBC_PB2AP           0x9400C800
#define CFG_EBC_PB2CR           0xF0018000 /*  BAS=0x800,BS=1MB,BU=R/W,BW=8bit	*/

#define CFG_EBC_CFG		0x7FC00000 /*  EBC0_CFG */

/*-----------------------------------------------------------------------
 * GPIO Setup
 *----------------------------------------------------------------------*/
#define CFG_4xx_GPIO_TABLE { /*	  Out		  GPIO	Alternate1	Alternate2	Alternate3 */ \
{											\
/* GPIO Core 0 */									\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO0	EBC_DATA_PAR(0)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO1	EBC_DATA_PAR(1)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO2	EBC_DATA_PAR(2)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO3	EBC_DATA_PAR(3)			*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO4	EBC_DATA(20)	USB2_DATA(4)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO5	EBC_DATA(21)	USB2_DATA(5)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO6	EBC_DATA(22)	USB2_DATA(6)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO7	EBC_DATA(23)	USB2_DATA(7)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO8	CS(1)/NFCE(1)	IRQ(7)		*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO9	CS(2)/NFCE(2)	IRQ(8)		*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO10 CS(3)/NFCE(3)	IRQ(9)		*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO11 IRQ(6)				*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO12 EBC_DATA(16)	USB2_DATA(0)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO13 EBC_DATA(17)	USB2_DATA(1)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO14 EBC_DATA(18)	USB2_DATA(2)	*/	\
{GPIO0_BASE, GPIO_BI,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO15 EBC_DATA(19)	USB2_DATA(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO16 UART0_DCD	UART1_CTS	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO17 UART0_DSR	UART1_RTS	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_0}, /* GPIO18 UART0_CTS			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO19 UART0_RTS			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO20 UART0_DTR	UART1_TX	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO21 UART0_RI	UART1_RX	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO22 EBC_HOLD_REQ	DMA_ACK2	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL,  GPIO_OUT_0}, /* GPIO23 EBC_HOLD_ACK	DMA_REQ2	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO24 EBC_EXT_REQ	DMA_EOT2	IRQ(4) */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO25 EBC_EXT_ACK	DMA_ACK3	IRQ(3) */ \
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO26 EBC_ADDR(5)	DMA_EOT0	TS(3) */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL , GPIO_OUT_0}, /* GPIO27 EBC_BUS_REQ	DMA_EOT3	IRQ(5) */ \
{GPIO0_BASE, GPIO_IN,  GPIO_SEL , GPIO_OUT_0}, /* GPIO28				*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO29 DMA_EOT1	IRQ(2)		*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL,  GPIO_OUT_0}, /* GPIO30 DMA_REQ1	IRQ(1)		*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT2, GPIO_OUT_0}, /* GPIO31 DMA_ACK1	IRQ(0)		*/	\
}												\
}

#define CFG_GPIO_PCIE_RST	23
#define CFG_GPIO_PCIE_CLKREQ	27
#define CFG_GPIO_PCIE_WAKE	28

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

/* pass open firmware flat tree */
#define CONFIG_OF_LIBFDT	1
#define CONFIG_OF_BOARD_SETUP	1

#endif	/* __CONFIG_H */

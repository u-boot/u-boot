/*
 * (C) Copyright 2007
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
 * lwmon5.h - configuration for lwmon5 board
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_LWMON5		1		/* Board is lwmon5	*/
#define CONFIG_440EPX		1		/* Specific PPC440EPx	*/
#define CONFIG_440		1		/* ... PPC440 family	*/
#define CONFIG_4xx		1		/* ... PPC4xx family	*/
#define CONFIG_SYS_CLK_FREQ	33300000	/* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/
#define CONFIG_MISC_INIT_R	1	/* Call misc_init_r		*/
#define CONFIG_ADD_RAM_INFO	1	/* Print additional info	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(512 * 1024)	/* Reserve 512 kB for Monitor	*/
#define CFG_MALLOC_LEN		(512 * 1024)	/* Reserve 512 kB for malloc()	*/

#define CFG_BOOT_BASE_ADDR	0xf0000000
#define CFG_SDRAM_BASE		0x00000000	/* _must_ be 0		*/
#define CFG_FLASH_BASE		0xfc000000	/* start of FLASH	*/
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_LIME_BASE_0         0xc0000000
#define CFG_LIME_BASE_1         0xc1000000
#define CFG_LIME_BASE_2         0xc2000000
#define CFG_LIME_BASE_3         0xc3000000
#define CFG_FPGA_BASE_0         0xc4000000
#define CFG_FPGA_BASE_1         0xc4200000
#define CFG_OCM_BASE		0xe0010000      /* ocm			*/
#define CFG_PCI_BASE		0xe0000000      /* Internal PCI regs	*/
#define CFG_PCI_MEMBASE		0x80000000	/* mapped pci memory	*/
#define CFG_PCI_MEMBASE1	CFG_PCI_MEMBASE  + 0x10000000
#define CFG_PCI_MEMBASE2	CFG_PCI_MEMBASE1 + 0x10000000
#define CFG_PCI_MEMBASE3	CFG_PCI_MEMBASE2 + 0x10000000

/* Don't change either of these */
#define CFG_PERIPHERAL_BASE	0xef600000	/* internal peripherals	*/

#define CFG_USB2D0_BASE		0xe0000100
#define CFG_USB_DEVICE		0xe0000000
#define CFG_USB_HOST		0xe0000400

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer
 *----------------------------------------------------------------------*/
/* 440EPx/440GRx have 16KB of internal SRAM, so no need for D-Cache	*/
#define CFG_INIT_RAM_OCM	1		/* OCM as init ram	*/
#define CFG_INIT_RAM_ADDR	CFG_OCM_BASE	/* OCM			*/

#define CFG_INIT_RAM_END	(4 << 10)
#define CFG_GBL_DATA_SIZE	256		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef CFG_EXT_SERIAL_CLOCK		/* no external clock provided	*/
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI     1
/* define this if you want console on UART1 */
#define CONFIG_UART1_CONSOLE	1	/* use UART1 as console		*/

#define CFG_BAUDRATE_TABLE						\
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI				/* The flash is CFI compatible	*/
#define CFG_FLASH_CFI_DRIVER			/* Use common CFI driver	*/

#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection	*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash	*/

#define CFG_ENV_SECT_SIZE	0x40000	/* size of one complete sector		*/
#define CFG_ENV_ADDR		((-CFG_MONITOR_LEN)-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CFG_MBYTES_SDRAM	(256)		/* 256MB			*/
#define CFG_DDR_CACHED_ADDR	0x40000000	/* setup 2nd TLB cached here	*/
#define CONFIG_DDR_DATA_EYE	1		/* use DDR2 optimization	*/
#if 0 /* test-only: disable ECC for now */
#define CONFIG_DDR_ECC		1		/* enable ECC			*/
#endif

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1		/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C				/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000		/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR	(0xa8>>1)
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

#define CONFIG_RTC_PCF8563	1		/* enable Philips PCF8563 RTC	*/
#define CFG_I2C_RTC_ADDR	0x51		/* Philips PCF8563 RTC address	*/

#define CONFIG_PREBOOT	"echo;"						\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"hostname=lwmon5\0"						\
	"netdev=eth0\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS1,${baudrate}\0"\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"     \
	        "bootm\0"						\
	"rootpath=/opt/eldk/ppc_4xxFP\0"				\
	"bootfile=/tftpboot/lwmon5/uImage\0"				\
	"kernel_addr=FC000000\0"					\
	"ramdisk_addr=FC180000\0"					\
	"load=tftp 200000 /tftpboot/${hostname}/u-boot.bin\0"		\
	"update=protect off FFF80000 FFFFFFFF;era FFF80000 FFFFFFFF;"	\
		"cp.b 200000 FFF80000 80000\0"			        \
	"upd=run load;run update\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define	CONFIG_IBM_EMAC4_V4	1
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		3	/* PHY address, See schematics	*/

#define CONFIG_PHY_RESET        1	/* reset phy upon startup         */

#define CONFIG_HAS_ETH0
#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NET_MULTI	1
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#define CONFIG_PHY1_ADDR	1

/* USB */
#ifdef CONFIG_440EPX
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE

/* Comment this out to enable USB 1.1 device */
#define USB_2_0_DEVICE

#endif /* CONFIG_440EPX */

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION


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
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM

#ifdef CONFIG_440EPX
#define CONFIG_CMD_USB
#endif


/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CONFIG_SUPPORT_VFAT

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

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#undef CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup  */
#define CFG_PCI_TARGBASE        0x80000000 /* PCIaddr mapped to CFG_PCI_MEMBASE*/

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT
#define CFG_PCI_MASTER_INIT

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC				*/
#define CFG_PCI_SUBSYS_ID       0xcafe	/* Whatever			*/

#define CONFIG_HW_WATCHDOG	1	/* Use external HW-Watchdog	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#define CFG_FLASH		CFG_FLASH_BASE

/* Memory Bank 0 (NOR-FLASH) initialization					*/
#define CFG_EBC_PB0AP		0x03050200
#define CFG_EBC_PB0CR		(CFG_FLASH | 0xdc000)

/* Memory Bank 1 (Lime) initialization						*/
#define CFG_EBC_PB1AP		0x01004380
#define CFG_EBC_PB1CR		(CFG_LIME_BASE_0 | 0xdc000)

/* Memory Bank 2 (FPGA) initialization						*/
#define CFG_EBC_PB2AP		0x01004400
#define CFG_EBC_PB2CR		(CFG_FPGA_BASE_0 | 0x1c000)

/* Memory Bank 3 (FPGA2) initialization						*/
#define CFG_EBC_PB3AP		0x01004400
#define CFG_EBC_PB3CR		(CFG_FPGA_BASE_1 | 0x1c000)

#define CFG_EBC_CFG		0xb8400000

/*-----------------------------------------------------------------------
 * Graphics (Fujitsu Lime)
 *----------------------------------------------------------------------*/
/* SDRAM Clock frequency adjustment register */
#define CFG_LIME_SDRAM_CLOCK	0xC1FC0000
/* Lime Clock frequency is to set 133MHz */
#define CFG_LIME_CLOCK_133MHZ	0x10000

/* SDRAM Parameter register */
#define CFG_LIME_MMR		0xC1FCFFFC
/* SDRAM parameter value */
#define CFG_LIME_MMR_VALUE	0x414FB7F2

/*-----------------------------------------------------------------------
 * GPIO Setup
 *----------------------------------------------------------------------*/
#define CFG_GPIO_PHY1_RST	12
#define CFG_GPIO_FLASH_WP	14
#define CFG_GPIO_PHY0_RST	22
#define CFG_GPIO_WATCHDOG	58
#define CFG_GPIO_LIME_S		59
#define CFG_GPIO_LIME_RST	60

/*-----------------------------------------------------------------------
 * PPC440 GPIO Configuration
 */
#define CFG_440_GPIO_TABLE { /*	  Out		  GPIO	Alternate1	Alternate2	Alternate3 */ \
{											\
/* GPIO Core 0 */									\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO0	EBC_ADDR(7)	DMA_REQ(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO1	EBC_ADDR(6)	DMA_ACK(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO2	EBC_ADDR(5)	DMA_EOT/TC(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO3	EBC_ADDR(4)	DMA_REQ(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO4	EBC_ADDR(3)	DMA_ACK(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO5	EBC_ADDR(2)	DMA_EOT/TC(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO6	EBC_CS_N(1)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO7	EBC_CS_N(2)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO8	EBC_CS_N(3)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO9	EBC_CS_N(4)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO10 EBC_CS_N(5)			*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO11 EBC_BUS_ERR			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO12				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO13				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO14				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO15				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO16 GMCTxD(4)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO17 GMCTxD(5)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO18 GMCTxD(6)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO19 GMCTxD(7)			*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO20 RejectPkt0			*/	\
{GPIO0_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO21 RejectPkt1			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO22				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO23 SCPD0				*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO24 GMCTxD(2)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_0}, /* GPIO25 GMCTxD(3)			*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO26				*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO27 EXT_EBC_REQ	USB2D_RXERROR	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO28		USB2D_TXVALID	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO29 EBC_EXT_HDLA	USB2D_PAD_SUSPNDM */	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO30 EBC_EXT_ACK	USB2D_XCVRSELECT*/	\
{GPIO0_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO31 EBC_EXR_BUSREQ	USB2D_TERMSELECT*/	\
},											\
{											\
/* GPIO Core 1 */									\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO32 USB2D_OPMODE0	EBC_DATA(2)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO33 USB2D_OPMODE1	EBC_DATA(3)	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT3, GPIO_OUT_0}, /* GPIO34 UART0_DCD_N	UART1_DSR_CTS_N	UART2_SOUT*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT3, GPIO_OUT_0}, /* GPIO35 UART0_8PIN_DSR_N UART1_RTS_DTR_N UART2_SIN*/ \
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO36 UART0_8PIN_CTS_N EBC_DATA(0)	UART3_SIN*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_0}, /* GPIO37 UART0_RTS_N	EBC_DATA(1)	UART3_SOUT*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_0}, /* GPIO38 UART0_DTR_N	UART1_SOUT	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT2, GPIO_OUT_0}, /* GPIO39 UART0_RI_N	UART1_SIN	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO40 UIC_IRQ(0)			*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO41 UIC_IRQ(1)			*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO42 UIC_IRQ(2)			*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO43 UIC_IRQ(3)			*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO44 UIC_IRQ(4)	DMA_ACK(1)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO45 UIC_IRQ(6)	DMA_EOT/TC(1)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO46 UIC_IRQ(7)	DMA_REQ(0)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO47 UIC_IRQ(8)	DMA_ACK(0)	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_ALT1, GPIO_OUT_0}, /* GPIO48 UIC_IRQ(9)	DMA_EOT/TC(0)	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO49  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL , GPIO_OUT_0}, /* GPIO50  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO51  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO52  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO53  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO54  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO55  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO56  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO57  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_1}, /* GPIO58  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO59  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO60  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO61  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN , GPIO_SEL , GPIO_OUT_0}, /* GPIO62  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_SEL , GPIO_OUT_0}, /* GPIO63  Unselect via TraceSelect Bit	*/	\
}											\
}

/*-----------------------------------------------------------------------
 * Cache Configuration
 *----------------------------------------------------------------------*/
#define CFG_DCACHE_SIZE		(32<<10)  /* For AMCC 440 CPUs			*/
#define CFG_CACHELINE_SIZE	32	      /* ...			            */
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	      /* log base 2 of the above value	*/
#endif

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	    /* which serial port to use */
#endif
#endif	/* __CONFIG_H */

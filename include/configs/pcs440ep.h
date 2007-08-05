/*
 * (C) Copyright 2006
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
 * pcs440ep.h - configuration for PCS440EP board
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_PCS440EP		1	/* Board is PCS440EP            */
#define CONFIG_440EP		1	/* Specific PPC440EP support    */
#define CONFIG_440		1	/* ... PPC440 family	        */
#define CONFIG_4xx		1	/* ... PPC4xx family	        */
#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1     /* Call board_early_init_f	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(384 * 1024)	/* Reserve 384 kB for Monitor	*/
#define CFG_MALLOC_LEN		(256 * 1024)	/* Reserve 256 kB for malloc()	*/
#define CFG_MONITOR_BASE	(-CFG_MONITOR_LEN)
#define CFG_SDRAM_BASE	        0x00000000	    /* _must_ be 0	*/
#define CFG_FLASH_BASE	        0xfff00000	    /* start of FLASH	*/
#define CFG_PCI_MEMBASE	        0xa0000000	    /* mapped pci memory*/
#define CFG_PCI_MEMBASE1        CFG_PCI_MEMBASE  + 0x10000000
#define CFG_PCI_MEMBASE2        CFG_PCI_MEMBASE1 + 0x10000000
#define CFG_PCI_MEMBASE3        CFG_PCI_MEMBASE2 + 0x10000000

/*Don't change either of these*/
#define CFG_PERIPHERAL_BASE     0xef600000	    /* internal peripherals*/
#define CFG_PCI_BASE	        0xe0000000	    /* internal PCI regs*/
/*Don't change either of these*/

#define CFG_USB_DEVICE          0x50000000
#define CFG_BOOT_BASE_ADDR      0xf0000000

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in SDRAM)
 *----------------------------------------------------------------------*/
#define CFG_INIT_RAM_DCACHE	1		/* d-cache as init ram	*/
#define CFG_INIT_RAM_ADDR	0x70000000		/* DCache       */
#define CFG_INIT_RAM_END	(8 << 10)
#define CFG_GBL_DATA_SIZE	256			/* num bytes initial data*/
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef CFG_EXT_SERIAL_CLOCK		/* no external clk used		*/
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI     1
/*define this if you want console on UART1*/
#undef CONFIG_UART1_CONSOLE

#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_WORD_SIZE	unsigned char	/* flash word size (width)	*/
#define CFG_FLASH_ADDR0		0x5555	/* 1st address for flash config cycles	*/
#define CFG_FLASH_ADDR1		0x2AAA	/* 2nd address for flash config cycles	*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000	/* size of one complete sector		*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

#define CONFIG_ENV_OVERWRITE	1

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif /* CFG_ENV_IS_IN_FLASH */

#define ENV_NAME_REVLEV	"revision_level"
#define ENV_NAME_SOLDER	"solder_switch"
#define ENV_NAME_DIP	"dip"

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SPD_EEPROM               /* Use SPD EEPROM for setup             */
#undef CONFIG_DDR_ECC			/* don't use ECC			*/
#define SPD_EEPROM_ADDRESS      {0x50}
#define	CONFIG_PROG_SDRAM_TLB	1

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	    /* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			    /* I2C bit-banged		*/
#define CFG_I2C_SPEED		100000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_EEPROM_ADDR	(0xa4>>1)
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=pcs440ep\0"						\
	"use_eeprom_ethaddr=default\0"					\
	"cs_test=off\0"							\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addtty=setenv bootargs ${bootargs} console=ttyS0,${baudrate}\0"\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};run nfsargs addip addtty;"     \
	        "bootm\0"						\
	"rootpath=/opt/eldk/ppc_4xx\0"					\
	"bootfile=/tftpboot/pcs440ep/uImage\0"				\
	"kernel_addr=FFF00000\0"					\
	"ramdisk_addr=FFF00000\0"					\
	"load=tftp 100000 /tftpboot/pcs440ep/u-boot.bin\0"		\
	"update=protect off FFFA0000 FFFFFFFF;era FFFA0000 FFFFFFFF;"	\
		"cp.b 100000 FFFA0000 60000\0"			        \
	"upd=run load;run update\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_PREBOOT	"echo;" \
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

/* check U-Boot image with SHA1 sum */
#define CONFIG_SHA1_CHECK_UB_IMG	1
#define CONFIG_SHA1_START		CFG_MONITOR_BASE
#define CONFIG_SHA1_LEN			CFG_MONITOR_LEN

/*-----------------------------------------------------------------------
 * Definitions for status LED
 */
#define CONFIG_STATUS_LED	1	/* Status LED enabled		*/
#define CONFIG_BOARD_SPECIFIC_LED	1

#define STATUS_LED_BIT		0x08			/* LED 1 is on GPIO_PPC_1 */
#define STATUS_LED_PERIOD	((CFG_HZ / 2) / 5)	/* blink at 5 Hz */
#define STATUS_LED_STATE	STATUS_LED_OFF
#define STATUS_LED_BIT1		0x04			/* LED 2 is on GPIO_PPC_2 */
#define STATUS_LED_PERIOD1	((CFG_HZ / 2) / 5)	/* blink at 5 Hz */
#define STATUS_LED_STATE1	STATUS_LED_ON
#define STATUS_LED_BIT2		0x02			/* LED 3 is on GPIO_PPC_3 */
#define STATUS_LED_PERIOD2	((CFG_HZ / 2) / 5)	/* blink at 5 Hz */
#define STATUS_LED_STATE2	STATUS_LED_OFF
#define STATUS_LED_BIT3		0x01			/* LED 4 is on GPIO_PPC_4 */
#define STATUS_LED_PERIOD3	((CFG_HZ / 2) / 5)	/* blink at 5 Hz */
#define STATUS_LED_STATE3	STATUS_LED_OFF

#define CONFIG_SHOW_BOOT_PROGRESS	1

#define CONFIG_BAUDRATE		115200

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_NET_MULTI        1	/* required for netconsole      */
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#define CONFIG_PHY_ADDR		1	/* PHY address, See schematics	*/
#define CONFIG_PHY1_ADDR        2

#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NETCONSOLE		/* include NetConsole support	*/

/* Partitions */
#define CONFIG_MAC_PARTITION
#define CONFIG_DOS_PARTITION
#define CONFIG_ISO_PARTITION

#ifdef CONFIG_440EP
/* USB */
#define CONFIG_USB_OHCI
#define CONFIG_USB_STORAGE

/*Comment this out to enable USB 1.1 device*/
#define USB_2_0_DEVICE
#endif /*CONFIG_440EP*/

#ifdef DEBUG
#define CONFIG_PANIC_HANG
#else
#define CONFIG_HW_WATCHDOG			/* watchdog */
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
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_USB


#define CONFIG_SUPPORT_VFAT

/*
 * Miscellaneous configurable options
 */
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

#define CFG_MEMTEST_START	0x0400000 /* memtest works on	        */
#define CFG_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */
#define CONFIG_LYNXKDI          1       /* support kdi files            */

#define CFG_HZ		        1000	/* decrementer freq: 1 ms ticks */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#undef  CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */
#define CFG_PCI_TARGBASE        0x80000000 /* PCIaddr mapped to CFG_PCI_MEMBASE*/

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT
#define CFG_PCI_MASTER_INIT

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CFG_PCI_SUBSYS_ID       0xcafe	/* Whatever */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#define FLASH_BASE0_PRELIM	0xFFF00000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0xFFF80000	/* FLASH bank #1	*/

#define CFG_FLASH		FLASH_BASE0_PRELIM
#define CFG_SRAM		0xF1000000
#define CFG_FPGA		0xF2000000
#define CFG_CF1			0xF0000000
#define CFG_CF2			0xF0100000

/* Memory Bank 0 (Flash Bank 0, NOR-FLASH) initialization			*/
#define CFG_EBC_PB0AP		0x02010000	/* TWT=4,OEN=1			*/
#define CFG_EBC_PB0CR		(CFG_FLASH | 0x18000) /* BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 1 (SRAM) initialization						*/
#define CFG_EBC_PB1AP		0x01810040	/* TWT=3,OEN=1,BEM=1		*/
#define CFG_EBC_PB1CR		(CFG_SRAM | 0x5A000) /* BS=4MB,BU=R/W,BW=16bit	*/

/* Memory Bank 2 (FPGA) initialization						*/
#define CFG_EBC_PB2AP		0x01010440	/* TWT=2,OEN=1,TH=2,BEM=1	*/
#define CFG_EBC_PB2CR		(CFG_FPGA | 0x5A000) /* BS=4MB,BU=R/W,BW=16bit	*/

/* Memory Bank 3 (CompactFlash) initialization					*/
#define CFG_EBC_PB3AP		0x080BD400
#define CFG_EBC_PB3CR		(CFG_CF1 | 0x1A000) /* BS=1MB,BU=R/W,BW=16bit	*/

/* Memory Bank 4 (CompactFlash) initialization					*/
#define CFG_EBC_PB4AP		0x080BD400
#define CFG_EBC_PB4CR		(CFG_CF2 | 0x1A000) /* BS=1MB,BU=R/W,BW=16bit	*/

/*-----------------------------------------------------------------------
 * PPC440 GPIO Configuration
 */
#define CFG_440_GPIO_TABLE { /*	  Out		       GPIO	Alternate1	Alternate2   Alternate3 */ \
{											\
/* GPIO Core 0 */									\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO0	EBC_ADDR(7)	DMA_REQ(2)	*/ \
{GPIO0_BASE, GPIO_OUT, GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO1	EBC_ADDR(6)	DMA_ACK(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO2	EBC_ADDR(5)	DMA_EOT/TC(2)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO3	EBC_ADDR(4)	DMA_REQ(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO4	EBC_ADDR(3)	DMA_ACK(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO5	EBC_ADDR(2)	DMA_EOT/TC(3)	*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO6	EBC_CS_N(1)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO7	EBC_CS_N(2)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO8	EBC_CS_N(3)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO9	EBC_CS_N(4)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO10	EBC_CS_N(5)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO11	EBC_BUS_ERR			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO12	ZII_p0Rxd(0)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO13	ZII_p0Rxd(1)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO14	ZII_p0Rxd(2)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO15	ZII_p0Rxd(3)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO16	ZII_p0Txd(0)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO17	ZII_p0Txd(1)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO18	ZII_p0Txd(2)			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO19	ZII_p0Txd(3)			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO20	ZII_p0Rx_er			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO21	ZII_p0Rx_dv			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO22	ZII_p0RxCrs			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO23	ZII_p0Tx_er			*/	\
{GPIO0_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO24	ZII_p0Tx_en			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO25	ZII_p0Col			*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO26			USB2D_RXVALID	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO27	EXT_EBC_REQ	USB2D_RXERROR	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO28			USB2D_TXVALID	*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO29	EBC_EXT_HDLA	USB2D_PAD_SUSPNDM */	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO30	EBC_EXT_ACK	USB2D_XCVRSELECT*/	\
{GPIO0_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO31	EBC_EXR_BUSREQ	USB2D_TERMSELECT*/	\
},											\
{											\
/* GPIO Core 1 */									\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO32	USB2D_OPMODE0			*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO33	USB2D_OPMODE1			*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT3, GPIO_OUT_NO_CHG}, /* GPIO34	UART0_DCD_N	UART1_DSR_CTS_N	UART2_SOUT*/ \
{GPIO1_BASE, GPIO_IN,  GPIO_ALT3, GPIO_OUT_NO_CHG}, /* GPIO35	UART0_8PIN_DSR_N UART1_RTS_DTR_N UART2_SIN*/ \
{GPIO1_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO36	UART0_8PIN_CTS_N		UART3_SIN*/ \
{GPIO1_BASE, GPIO_OUT, GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO37	UART0_RTS_N			*/	\
{GPIO1_BASE, GPIO_OUT, GPIO_ALT2, GPIO_OUT_NO_CHG}, /* GPIO38	UART0_DTR_N	UART1_SOUT	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_ALT2, GPIO_OUT_NO_CHG}, /* GPIO39	UART0_RI_N	UART1_SIN	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO40	UIC_IRQ(0)			*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO41	UIC_IRQ(1)			*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO42	UIC_IRQ(2)			*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO43	UIC_IRQ(3)			*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_ALT1, GPIO_OUT_NO_CHG}, /* GPIO44	UIC_IRQ(4)	DMA_ACK(1)	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO45	UIC_IRQ(6)	DMA_EOT/TC(1)	*/	\
{GPIO1_BASE, GPIO_BI,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO46	UIC_IRQ(7)	DMA_REQ(0)	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO47	UIC_IRQ(8)	DMA_ACK(0)	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO48	UIC_IRQ(9)	DMA_EOT/TC(0)	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO49  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO50  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO51  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO52  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO53  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO54  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO55  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO56  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO57  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO58  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO59  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO60  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO61  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO62  Unselect via TraceSelect Bit	*/	\
{GPIO1_BASE, GPIO_IN,  GPIO_SEL, GPIO_OUT_NO_CHG},  /* GPIO63  Unselect via TraceSelect Bit	*/	\
}											\
}

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		(32<<10) /* For AMCC 440 CPUs			*/
#define CFG_CACHELINE_SIZE	32	/* ...			*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
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
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*-----------------------------------------------------------------------
 * IDE/ATA stuff Supports IDE harddisk
 *-----------------------------------------------------------------------
 */

#undef  CONFIG_IDE_8xx_PCCARD		/* Use IDE with PC Card	Adapter	*/

#undef  CONFIG_IDE_8xx_DIRECT		/* Direct IDE    not supported	*/
#undef  CONFIG_IDE_LED			/* LED   for ide not supported	*/

#define CFG_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CFG_IDE_MAXDEVICE	1	/* max. 2 drives per IDE bus	*/

#define CONFIG_IDE_PREINIT	1
#define CONFIG_IDE_RESET	1

#define CFG_ATA_IDE0_OFFSET	0x0000

#define CFG_ATA_BASE_ADDR	CFG_CF1

/* Offset for data I/O			*/
#define CFG_ATA_DATA_OFFSET	0

/* Offset for normal register accesses	*/
#define CFG_ATA_REG_OFFSET	(CFG_ATA_DATA_OFFSET)

/* Offset for alternate registers	*/
#define CFG_ATA_ALT_OFFSET	(0x0000)

/* These addresses need to be shifted one place to the left
 * ( bus per_addr 20 -30 is connectsd on CF bus A10-A0)
 * These values are shifted
 */
#define CFG_ATA_PORT_ADDR(port) ((port) << 1)

#endif	/* __CONFIG_H */

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


/* new uImage format support */
#define CONFIG_FIT		1
#define CONFIG_OF_LIBFDT	1
#define CONFIG_FIT_VERBOSE	1 /* enable fit_format_{error,warning}() */

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_PCS440EP		1	/* Board is PCS440EP            */
#define CONFIG_440EP		1	/* Specific PPC440EP support    */
#define CONFIG_440		1	/* ... PPC440 family	        */
#define CONFIG_4xx		1	/* ... PPC4xx family	        */

#define	CONFIG_SYS_TEXT_BASE	0xFFFA0000

#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1     /* Call board_early_init_f	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MONITOR_LEN		(384 * 1024)	/* Reserve 384 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN		(256 * 1024)	/* Reserve 256 kB for malloc()	*/
#define CONFIG_SYS_MONITOR_BASE	(-CONFIG_SYS_MONITOR_LEN)
#define CONFIG_SYS_SDRAM_BASE	        0x00000000	    /* _must_ be 0	*/
#define CONFIG_SYS_FLASH_BASE	        0xfff00000	    /* start of FLASH	*/
#define CONFIG_SYS_PCI_MEMBASE	        0xa0000000	    /* mapped pci memory*/
#define CONFIG_SYS_PCI_MEMBASE1        CONFIG_SYS_PCI_MEMBASE  + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE2        CONFIG_SYS_PCI_MEMBASE1 + 0x10000000
#define CONFIG_SYS_PCI_MEMBASE3        CONFIG_SYS_PCI_MEMBASE2 + 0x10000000

/*Don't change either of these*/
#define CONFIG_SYS_PCI_BASE	        0xe0000000	    /* internal PCI regs*/
/*Don't change either of these*/

#define CONFIG_SYS_USB_DEVICE          0x50000000
#define CONFIG_SYS_BOOT_BASE_ADDR      0xf0000000

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in SDRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_INIT_RAM_DCACHE	1		/* d-cache as init ram	*/
#define CONFIG_SYS_INIT_RAM_ADDR	0x70000000		/* DCache       */
#define CONFIG_SYS_INIT_RAM_SIZE	(4 << 10)
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()
#undef CONFIG_SYS_EXT_SERIAL_CLOCK		/* no external clk used		*/
#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_WORD_SIZE	unsigned char	/* flash word size (width)	*/
#define CONFIG_SYS_FLASH_ADDR0		0x5555	/* 1st address for flash config cycles	*/
#define CONFIG_SYS_FLASH_ADDR1		0x2AAA	/* 2nd address for flash config cycles	*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#ifdef CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_SECT_SIZE	0x10000	/* size of one complete sector		*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

#define CONFIG_ENV_OVERWRITE	1

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)
#endif /* CONFIG_ENV_IS_IN_FLASH */

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
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		0x7F

#define CONFIG_SYS_I2C_EEPROM_ADDR	(0xa4>>1)
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS 10

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
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
	"upd=run load update\0"						\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

/* check U-Boot image with SHA1 sum */
#define CONFIG_SHA1_CHECK_UB_IMG	1
#define CONFIG_SHA1_START		CONFIG_SYS_MONITOR_BASE
#define CONFIG_SHA1_LEN			CONFIG_SYS_MONITOR_LEN

/*-----------------------------------------------------------------------
 * Definitions for status LED
 */
#define CONFIG_STATUS_LED	1	/* Status LED enabled		*/
#define CONFIG_BOARD_SPECIFIC_LED	1

#define STATUS_LED_BIT		0x08			/* DIAG1 is on GPIO_PPC_1 */
#define STATUS_LED_PERIOD	((CONFIG_SYS_HZ / 2) / 5)	/* blink at 5 Hz */
#define STATUS_LED_STATE	STATUS_LED_OFF
#define STATUS_LED_BIT1		0x04			/* DIAG2 is on GPIO_PPC_2 */
#define STATUS_LED_PERIOD1	((CONFIG_SYS_HZ / 2) / 5)	/* blink at 5 Hz */
#define STATUS_LED_STATE1	STATUS_LED_ON
#define STATUS_LED_BIT2		0x02			/* DIAG3 is on GPIO_PPC_3 */
#define STATUS_LED_PERIOD2	((CONFIG_SYS_HZ / 2) / 5)	/* blink at 5 Hz */
#define STATUS_LED_STATE2	STATUS_LED_OFF
#define STATUS_LED_BIT3		0x01			/* DIAG4 is on GPIO_PPC_4 */
#define STATUS_LED_PERIOD3	((CONFIG_SYS_HZ / 2) / 5)	/* blink at 5 Hz */
#define STATUS_LED_STATE3	STATUS_LED_OFF

#define CONFIG_SHOW_BOOT_PROGRESS	1

#define CONFIG_BAUDRATE		115200

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#define CONFIG_PHY_ADDR		1	/* PHY address, See schematics	*/
#define CONFIG_PHY1_ADDR        2

#define CONFIG_SYS_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

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
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_FAT
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IDE
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_REISER
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_USB

#define CONFIG_SUPPORT_VFAT

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	        1024	/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	        256	/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE              (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	        16	/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	        CONFIG_SYS_CBSIZE /* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000 /* memtest works on	        */
#define CONFIG_SYS_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_into (bd_t) */
#define CONFIG_LYNXKDI          1       /* support kdi files            */

#define CONFIG_SYS_HZ		        1000	/* decrementer freq: 1 ms ticks */

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#undef  CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */
#define CONFIG_SYS_PCI_TARGBASE        0x80000000 /* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE*/

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT
#define CONFIG_SYS_PCI_MASTER_INIT

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CONFIG_SYS_PCI_SUBSYS_ID       0xcafe	/* Whatever */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#define FLASH_BASE0_PRELIM	0xFFF00000	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0xFFF80000	/* FLASH bank #1	*/

#define CONFIG_SYS_FLASH		FLASH_BASE0_PRELIM
#define CONFIG_SYS_SRAM		0xF1000000
#define CONFIG_SYS_FPGA		0xF2000000
#define CONFIG_SYS_CF1			0xF0000000
#define CONFIG_SYS_CF2			0xF0100000

/* Memory Bank 0 (Flash Bank 0, NOR-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB0AP		0x02010000	/* TWT=4,OEN=1			*/
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_FLASH | 0x18000) /* BS=1MB,BU=R/W,BW=8bit	*/

/* Memory Bank 1 (SRAM) initialization						*/
#define CONFIG_SYS_EBC_PB1AP		0x01810040	/* TWT=3,OEN=1,BEM=1		*/
#define CONFIG_SYS_EBC_PB1CR		(CONFIG_SYS_SRAM | 0x5A000) /* BS=4MB,BU=R/W,BW=16bit	*/

/* Memory Bank 2 (FPGA) initialization						*/
#define CONFIG_SYS_EBC_PB2AP		0x01010440	/* TWT=2,OEN=1,TH=2,BEM=1	*/
#define CONFIG_SYS_EBC_PB2CR		(CONFIG_SYS_FPGA | 0x5A000) /* BS=4MB,BU=R/W,BW=16bit	*/

/* Memory Bank 3 (CompactFlash) initialization					*/
#define CONFIG_SYS_EBC_PB3AP		0x080BD400
#define CONFIG_SYS_EBC_PB3CR		(CONFIG_SYS_CF1 | 0x1A000) /* BS=1MB,BU=R/W,BW=16bit	*/

/* Memory Bank 4 (CompactFlash) initialization					*/
#define CONFIG_SYS_EBC_PB4AP		0x080BD400
#define CONFIG_SYS_EBC_PB4CR		(CONFIG_SYS_CF2 | 0x1A000) /* BS=1MB,BU=R/W,BW=16bit	*/

/*-----------------------------------------------------------------------
 * PPC440 GPIO Configuration
 */
#define CONFIG_SYS_4xx_GPIO_TABLE { /*	  Out		       GPIO	Alternate1	Alternate2   Alternate3 */ \
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

#define CONFIG_SYS_IDE_MAXBUS		1	/* max. 1 IDE bus		*/
#define CONFIG_SYS_IDE_MAXDEVICE	1	/* max. 2 drives per IDE bus	*/

#define CONFIG_IDE_PREINIT	1
#define CONFIG_IDE_RESET	1

#define CONFIG_SYS_ATA_IDE0_OFFSET	0x0000

#define CONFIG_SYS_ATA_BASE_ADDR	CONFIG_SYS_CF1

/* Offset for data I/O			*/
#define CONFIG_SYS_ATA_DATA_OFFSET	0

/* Offset for normal register accesses	*/
#define CONFIG_SYS_ATA_REG_OFFSET	(CONFIG_SYS_ATA_DATA_OFFSET)

/* Offset for alternate registers	*/
#define CONFIG_SYS_ATA_ALT_OFFSET	(0x0000)

#endif	/* __CONFIG_H */

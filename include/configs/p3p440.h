/*
 * (C) Copyright 2005-2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

/************************************************************************
 * board/config_p3p440.h - configuration for Prodrive P3P440
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_P3P440		1	    /* Board is P3P440		*/
#define CONFIG_440GP		1	    /* Specifc GP support	*/
#define CONFIG_440		1	    /* ... PPC440 family	*/
#define CONFIG_4xx		1	    /* ... PPC4xx family	*/
#define CONFIG_BOARD_EARLY_INIT_F 1	    /* Call board_early_init_f	*/
#define CONFIG_MISC_INIT_R	1	    /* Call misc_init_r		*/

#define	CONFIG_SYS_TEXT_BASE	0xFFFC0000

#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_SDRAM_BASE	    0x00000000	    /* _must_ be 0		*/
#define CONFIG_SYS_FLASH_BASE	    0xff800000	    /* start of FLASH		*/
#define CONFIG_SYS_MONITOR_BASE    0xfffc0000	    /* start of monitor		*/
#define CONFIG_SYS_PCI_MEMBASE	    0x80000000	    /* mapped pci memory	*/
#define CONFIG_SYS_ISRAM_BASE	    0xc0000000	    /* internal SRAM		*/
#define CONFIG_SYS_PCI_BASE	    0xd0000000	    /* internal PCI regs	*/

#define CONFIG_SYS_USB_BASE	    (CONFIG_SYS_PERIPHERAL_BASE + 0x00000000)

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_ISRAM_BASE  /* Initial RAM address	*/
#define CONFIG_SYS_INIT_RAM_SIZE	0x2000	    /* Size of used area in RAM	*/

#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Mon*/
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc*/

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SDRAM_BANK0	1	/* init onboard DDR SDRAM bank 0*/
#define CONFIG_SDRAM_ECC		/* enable ECC support		*/
#define CONFIG_SYS_SDRAM_TABLE	{ \
		{(256 << 20), 13, 0x000C4001}, /* 256MB mode 3, 13x10(4)*/ \
		{(64 << 20),  12, 0x00082001}} /* 64MB mode 2, 12x9(4)	*/

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef CONFIG_SYS_EXT_SERIAL_CLOCK
#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_BAUDRATE_TABLE						\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,		\
			57600, 115200, 230400, 460800, 921600 }

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		100000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F
#define CONFIG_SYS_I2C_NOPROBES	{ {0, 0x69} }	/* Don't probe these addrs */

/*-----------------------------------------------------------------------
 * I2C RTC
 *----------------------------------------------------------------------*/
#define CONFIG_RTC_MAX6900	1		/* MAX6900 RTC		*/

/*-----------------------------------------------------------------------
 * I2C EEPROM (PCF8594C) for environment
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_I2C_EEPROM_ADDR	0x54	/* EEPROM PCF8594C		*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
#define CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS 3	/* The Philips PCF8594C has	*/
					/* 8 byte page write mode using */
					/* last 3 bits of the address	*/
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	40   /* and takes up to 40 msec */

/*-----------------------------------------------------------------------
 * Default configuration (environment varibles...)
 *----------------------------------------------------------------------*/
#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=p3p440\0"						\
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
	"bootfile=/tftpboot/p3p440/uImage\0"				\
	"kernel_addr=ff800000\0"					\
	"ramdisk_addr=ff810000\0"					\
	"load=tftp 100000 /tftpboot/p3p440/u-boot.bin\0"		\
	"update=protect off fffc0000 ffffffff;era fffc0000 ffffffff;"	\
		"cp.b 100000 fffc0000 40000;"			        \
		"setenv filesize;saveenv\0"				\
	"upd=run load update\0"						\
	"unlock=yes\0"							\
	""
#define CONFIG_BOOTCOMMAND	"run net_nfs"

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_BAUDRATE		115200

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0x1c	/* PHY address			*/
#define CONFIG_HAS_ETH1
#define CONFIG_PHY1_ADDR	0x1d	/* EMAC1 PHY address		*/
#define CONFIG_SYS_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NETCONSOLE		/* include NetConsole support	*/


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
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_SNTP


#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CONFIG_SYS_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_AUTO_COMPLETE	1       /* add autocompletion support   */
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI			            /* include pci support	        */
#define	CONFIG_PCI_INDIRECT_BRIDGE 1	/* indirect PCI bridge support */
#define CONFIG_PCI_PNP			        /* do pci plug-and-play         */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */
#define CONFIG_SYS_PCI_TARGBASE    0x80000000  /* PCIaddr mapped to CONFIG_SYS_PCI_MEMBASE */

/* Board-specific PCI */
#define CONFIG_SYS_PCI_TARGET_INIT	            /* let board init pci target    */

#define CONFIG_DISABLE_PISE_TEST	/* disable PISE test (PCIX only)*/

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0xcafe  /* Whatever */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH0		0xFF800000
#define CONFIG_SYS_FLASH1		0xFF000000
#define CONFIG_SYS_FLASH2		0xFE800000
#define CONFIG_SYS_FLASH3		0xFE000000
#define CONFIG_SYS_USB			0xF0000000

/* Memory Bank 0 (Flash Bank 0, NOR-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB0AP		0x03050200
#define CONFIG_SYS_EBC_PB0CR		(CONFIG_SYS_FLASH0 | 0x7A000) /* BAS=0xFF8,BS=8MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (Flash Bank 1, NOR-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB1AP		0x03050200
#define CONFIG_SYS_EBC_PB1CR		(CONFIG_SYS_FLASH1 | 0x7A000) /* BAS=0xFF8,BS=8MB,BU=R/W,BW=16bit */

/* Memory Bank 2 (Flash Bank 2, NOR-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB2AP		0x03050200
#define CONFIG_SYS_EBC_PB2CR		(CONFIG_SYS_FLASH2 | 0x7A000) /* BAS=0xFF8,BS=8MB,BU=R/W,BW=16bit */

/* Memory Bank 3 (Flash Bank 3, NOR-FLASH) initialization			*/
#define CONFIG_SYS_EBC_PB3AP		0x03050200
#define CONFIG_SYS_EBC_PB3CR		(CONFIG_SYS_FLASH3 | 0x7A000) /* BAS=0xFF8,BS=8MB,BU=R/W,BW=16bit */

/* Memory Bank 7 (USB controller) initialization				*/
#define CONFIG_SYS_EBC_PB7AP		0x02015000
#define CONFIG_SYS_EBC_PB7CR		(CONFIG_SYS_USB | 0xFE000) /* BAS=0xF00,BS=128MB,BU=R/W,BW=16bit*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CONFIG_SYS_FLASH_CFI				/* The flash is CFI compatible	*/
#define CONFIG_FLASH_CFI_DRIVER			/* Use common CFI driver	*/

#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH3, CONFIG_SYS_FLASH2, CONFIG_SYS_FLASH1, CONFIG_SYS_FLASH0 }

#define CONFIG_SYS_MAX_FLASH_BANKS	4	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CONFIG_SYS_FLASH_PROTECTION	1	/* use hardware flash protection	*/

#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash	*/

#define CONFIG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

#define CONFIG_ENV_SECT_SIZE	0x20000	/* size of one complete sector		*/
#define CONFIG_ENV_ADDR		(CONFIG_SYS_MONITOR_BASE-CONFIG_ENV_SECT_SIZE)
#define	CONFIG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CONFIG_ENV_ADDR_REDUND	(CONFIG_ENV_ADDR-CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND	(CONFIG_ENV_SIZE)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif
#endif	/* __CONFIG_H */

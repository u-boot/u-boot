/*
 * (C) Copyright 2005-2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
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
#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_SDRAM_BASE	    0x00000000	    /* _must_ be 0		*/
#define CFG_FLASH_BASE	    0xff800000	    /* start of FLASH		*/
#define CFG_MONITOR_BASE    0xfffc0000	    /* start of monitor		*/
#define CFG_PCI_MEMBASE	    0x80000000	    /* mapped pci memory	*/
#define CFG_PERIPHERAL_BASE 0xe0000000	    /* internal peripherals	*/
#define CFG_ISRAM_BASE	    0xc0000000	    /* internal SRAM		*/
#define CFG_PCI_BASE	    0xd0000000	    /* internal PCI regs	*/

#define CFG_USB_BASE	    (CFG_PERIPHERAL_BASE + 0x00000000)

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CFG_INIT_RAM_ADDR	CFG_ISRAM_BASE  /* Initial RAM address	*/
#define CFG_INIT_RAM_END	0x2000	    /* End of used area in RAM	*/
#define CFG_GBL_DATA_SIZE	128	    /* num bytes initial data	*/

#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Mon*/
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc*/

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SDRAM_BANK0	1	/* init onboard DDR SDRAM bank 0*/
#define CONFIG_SDRAM_ECC		/* enable ECC support		*/
#define CFG_SDRAM_TABLE	{ \
		{(256 << 20), 13, 0x000C4001}, /* 256MB mode 3, 13x10(4)*/ \
		{(64 << 20),  12, 0x00082001}} /* 64MB mode 2, 12x9(4)	*/

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#undef CFG_EXT_SERIAL_CLOCK
#define CONFIG_BAUDRATE		115200

#define CFG_BAUDRATE_TABLE						\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,		\
			57600, 115200, 230400, 460800, 921600 }

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CFG_I2C_SPEED		100000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F
#define CFG_I2C_NOPROBES	{0x69}	/* Don't probe these addrs	*/

/*-----------------------------------------------------------------------
 * I2C RTC
 *----------------------------------------------------------------------*/
#define CONFIG_RTC_MAX6900	1		/* MAX6900 RTC		*/

/*-----------------------------------------------------------------------
 * I2C EEPROM (PCF8594C) for environment
 *----------------------------------------------------------------------*/
#define CFG_I2C_EEPROM_ADDR	0x54	/* EEPROM PCF8594C		*/
#define CFG_I2C_EEPROM_ADDR_LEN 1	/* Bytes of address		*/
/* mask of address bits that overflow into the "EEPROM chip address"	*/
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	0x07
#define CFG_EEPROM_PAGE_WRITE_BITS 3	/* The Philips PCF8594C has	*/
					/* 8 byte page write mode using */
					/* last 3 bits of the address	*/
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	40   /* and takes up to 40 msec */
#define CFG_EEPROM_PAGE_WRITE_ENABLE

/*-----------------------------------------------------------------------
 * Default configuration (environment varibles...)
 *----------------------------------------------------------------------*/
#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
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
	"upd=run load;run update\0"					\
	"unlock=yes\0"							\
	""
#define CONFIG_BOOTCOMMAND	"run net_nfs"

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_BAUDRATE		115200

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0x1c	/* PHY address			*/
#define CONFIG_HAS_ETH1
#define CONFIG_PHY1_ADDR	0x1d	/* EMAC1 PHY address		*/
#define CONFIG_NET_MULTI	1
#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NETCONSOLE		/* include NetConsole support	*/

#define CONFIG_COMMANDS	       (CONFIG_CMD_DFL	| \
				CFG_CMD_ASKENV	| \
				CFG_CMD_DATE	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_DIAG	| \
				CFG_CMD_ELF	| \
				CFG_CMD_I2C	| \
				CFG_CMD_IRQ	| \
				CFG_CMD_MII	| \
				CFG_CMD_NET	| \
				CFG_CMD_NFS	| \
				CFG_CMD_PCI	| \
				CFG_CMD_PING	| \
				CFG_CMD_REGINFO	| \
				CFG_CMD_EEPROM	| \
				CFG_CMD_SNTP	)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 *----------------------------------------------------------------------*/
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_AUTO_COMPLETE	1       /* add autocompletion support   */
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

/*-----------------------------------------------------------------------
 * PCI stuff
 *----------------------------------------------------------------------*/
/* General PCI */
#define CONFIG_PCI			            /* include pci support	        */
#define CONFIG_PCI_PNP			        /* do pci plug-and-play         */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */
#define CFG_PCI_TARGBASE    0x80000000  /* PCIaddr mapped to CFG_PCI_MEMBASE */

/* Board-specific PCI */
#define CFG_PCI_PRE_INIT                /* enable board pci_pre_init()  */
#define CFG_PCI_TARGET_INIT	            /* let board init pci target    */

#define CONFIG_DISABLE_PISE_TEST	/* disable PISE test (PCIX only)*/

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CFG_PCI_SUBSYS_DEVICEID 0xcafe  /* Whatever */

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 *----------------------------------------------------------------------*/
#define CFG_FLASH0		0xFF800000
#define CFG_FLASH1		0xFF000000
#define CFG_FLASH2		0xFE800000
#define CFG_FLASH3		0xFE000000
#define CFG_USB			0xF0000000

/* Memory Bank 0 (Flash Bank 0, NOR-FLASH) initialization			*/
#define CFG_EBC_PB0AP		0x03050200
#define CFG_EBC_PB0CR		(CFG_FLASH0 | 0x7A000) /* BAS=0xFF8,BS=8MB,BU=R/W,BW=16bit */

/* Memory Bank 1 (Flash Bank 1, NOR-FLASH) initialization			*/
#define CFG_EBC_PB1AP		0x03050200
#define CFG_EBC_PB1CR		(CFG_FLASH1 | 0x7A000) /* BAS=0xFF8,BS=8MB,BU=R/W,BW=16bit */

/* Memory Bank 2 (Flash Bank 2, NOR-FLASH) initialization			*/
#define CFG_EBC_PB2AP		0x03050200
#define CFG_EBC_PB2CR		(CFG_FLASH2 | 0x7A000) /* BAS=0xFF8,BS=8MB,BU=R/W,BW=16bit */

/* Memory Bank 3 (Flash Bank 3, NOR-FLASH) initialization			*/
#define CFG_EBC_PB3AP		0x03050200
#define CFG_EBC_PB3CR		(CFG_FLASH3 | 0x7A000) /* BAS=0xFF8,BS=8MB,BU=R/W,BW=16bit */

/* Memory Bank 7 (USB controller) initialization				*/
#define CFG_EBC_PB7AP		0x02015000
#define CFG_EBC_PB7CR		(CFG_USB | 0xFE000) /* BAS=0xF00,BS=128MB,BU=R/W,BW=16bit*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI				/* The flash is CFI compatible	*/
#define CFG_FLASH_CFI_DRIVER			/* Use common CFI driver	*/

#define CFG_FLASH_BANKS_LIST { CFG_FLASH3, CFG_FLASH2, CFG_FLASH1, CFG_FLASH0 }

#define CFG_MAX_FLASH_BANKS	4	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_PROTECTION	1	/* use hardware flash protection	*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash	*/

#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

#define CFG_ENV_SECT_SIZE	0x20000	/* size of one complete sector		*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		(32<<10)	/* For AMCC 405 CPUs		*/
#define CFG_CACHELINE_SIZE	32	/* ...					*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

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
#endif	/* __CONFIG_H */

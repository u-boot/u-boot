/*
 * (C) Copyright 2005-2007
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
 * bamboo.h - configuration for BAMBOO board
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_BAMBOO		1	/* Board is BAMBOO              */
#define CONFIG_440EP		1	/* Specific PPC440EP support    */
#define CONFIG_4xx		1	/* ... PPC4xx family	        */
#define CONFIG_SYS_CLK_FREQ	33333333    /* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1     /* Call board_early_init_f	*/

/*
 * Please note that, if NAND support is enabled, the 2nd ethernet port
 * can't be used because of pin multiplexing. So, if you want to use the
 * 2nd ethernet port you have to "undef" the following define.
 */
#define CONFIG_BAMBOO_NAND      1       /* enable nand flash support    */

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(384 * 1024)	/* Reserve 384 kB for Monitor	*/
#define CFG_MALLOC_LEN		(256 * 1024)	/* Reserve 256 kB for malloc()	*/
#define CFG_MONITOR_BASE	TEXT_BASE
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
#define CFG_NVRAM_BASE_ADDR     0x80000000
#define CFG_BOOT_BASE_ADDR      0xf0000000
#define CFG_NAND_ADDR           0x90000000
#define CFG_NAND2_ADDR          0x94000000

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in SDRAM)
 *----------------------------------------------------------------------*/
#define CFG_INIT_RAM_DCACHE	1		/* d-cache as init ram	*/
#define CFG_INIT_RAM_ADDR	0x70000000		/* DCache       */
#define CFG_INIT_RAM_END	(4 << 10)
#define CFG_GBL_DATA_SIZE	256		    	/* num bytes initial data	*/
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CFG_EXT_SERIAL_CLOCK	11059200 /* use external 11.059MHz clk	*/
#define CONFIG_BAUDRATE		115200
#define CONFIG_SERIAL_MULTI     1
/* define this if you want console on UART1 */
#undef CONFIG_UART1_CONSOLE

#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * NVRAM/RTC
 *
 * NOTE: The RTC registers are located at 0x7FFF0 - 0x7FFFF
 * The DS1558 code assumes this condition
 *
 *----------------------------------------------------------------------*/
#define CFG_NVRAM_SIZE	        (0x2000 - 0x10) /* NVRAM size(8k)- RTC regs     */
#define CONFIG_RTC_DS1556	1		         /* DS1556 RTC		*/

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
#define CFG_MAX_FLASH_BANKS	3		    /* number of banks	    */
#define CFG_MAX_FLASH_SECT	256		    /* sectors per device   */

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_ADDR0         0x555
#define CFG_FLASH_ADDR1         0x2aa
#define CFG_FLASH_WORD_SIZE     unsigned char

#define CFG_FLASH_2ND_16BIT_DEV 1         /* bamboo has 8 and 16bit device      */
#define CFG_FLASH_2ND_ADDR      0x87800000  /* bamboo has 8 and 16bit device    */

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000 	/* size of one complete sector	*/
#define CFG_ENV_ADDR		((-CFG_MONITOR_LEN)-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

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
#define CFG_MAX_NAND_DEVICE	2
#define NAND_MAX_CHIPS		CFG_MAX_NAND_DEVICE
#define CFG_NAND_BASE		(CFG_NAND_ADDR + CFG_NAND_CS)
#define CFG_NAND_BASE_LIST	{ CFG_NAND_BASE, CFG_NAND_ADDR + 2 }
#define CFG_NAND_SELECT_DEVICE  1	/* nand driver supports mutipl. chips	*/

#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)
#define CFG_NAND_CS		1
#else
#define CFG_NAND_CS		0		/* NAND chip connected to CSx	*/
/* Memory Bank 0 (NAND-FLASH) initialization					*/
#define CFG_EBC_PB0AP		0x018003c0
#define CFG_EBC_PB0CR		(CFG_NAND_ADDR | 0x1c000)
#endif

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------------- */
#define CONFIG_SPD_EEPROM               /* Use SPD EEPROM for setup             */
#undef CONFIG_DDR_ECC			/* don't use ECC			*/
#define CFG_SIMULATE_SPD_EEPROM	0xff	/* simulate spd eeprom on this address	*/
#define SPD_EEPROM_ADDRESS	{CFG_SIMULATE_SPD_EEPROM, 0x50, 0x51}
#define CFG_MBYTES_SDRAM	(64)	/* 64MB fixed size for early-sdram-init */

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	    /* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			    /* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR	(0xa8>>1)
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

#ifdef CFG_ENV_IS_IN_EEPROM
#define CFG_ENV_SIZE		0x200	    /* Size of Environment vars */
#define CFG_ENV_OFFSET		0x0
#endif /* CFG_ENV_IS_IN_EEPROM */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=bamboo\0"						\
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
	"bootfile=/tftpboot/bamboo/uImage\0"				\
	"kernel_addr=fff00000\0"					\
	"ramdisk_addr=fff10000\0"					\
	"initrd_high=30000000\0"					\
	"load=tftp 100000 /tftpboot/bamboo/u-boot.bin\0"		\
	"update=protect off fffa0000 ffffffff;era fffa0000 ffffffff;"	\
		"cp.b 100000 fffa0000 60000;"			        \
		"setenv filesize;saveenv\0"				\
	"upd=run load;run update\0"					\
	""
#define CONFIG_BOOTCOMMAND	"run flash_self"

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_BAUDRATE		115200

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address, See schematics	*/
#define CONFIG_PHY1_ADDR        1

#ifndef CONFIG_BAMBOO_NAND
#define CONFIG_HAS_ETH1		1	/* add support for "eth1addr"	*/
#endif /* CONFIG_BAMBOO_NAND */

#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NETCONSOLE		/* include NetConsole support	*/
#define CONFIG_NET_MULTI        1       /* required for netconsole      */

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

#ifdef CONFIG_BAMBOO_NAND
#define _CFG_CMD_NAND CFG_CMD_NAND
#else
#define _CFG_CMD_NAND 0
#endif /* CONFIG_BAMBOO_NAND */

#define CONFIG_COMMANDS	       (CONFIG_CMD_DFL	| \
				CFG_CMD_ASKENV	| \
				CFG_CMD_DATE	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_DIAG	| \
				CFG_CMD_ELF	| \
				CFG_CMD_EEPROM	| \
				CFG_CMD_I2C	| \
				CFG_CMD_IRQ	| \
				CFG_CMD_MII	| \
				CFG_CMD_NET	| \
				CFG_CMD_NFS	| \
				CFG_CMD_PCI	| \
				CFG_CMD_PING	| \
				CFG_CMD_REGINFO	| \
				CFG_CMD_SDRAM	| \
				CFG_CMD_USB	| \
				CFG_CMD_FAT	| \
				CFG_CMD_EXT2	| \
				_CFG_CMD_NAND	| \
				CFG_CMD_SNTP	)

#define CONFIG_SUPPORT_VFAT

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	        "=> "	/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
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

#define CONFIG_CMDLINE_EDITING	1	/* add command line history	*/
#define CONFIG_LOOPW            1       /* enable loopw command         */
#define CONFIG_MX_CYCLIC        1       /* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE 1	/* include version env variable */

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
#define CFG_PCI_PRE_INIT                /* enable board pci_pre_init()  */
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
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		(32<<10) /* For AMCC 440 CPUs			*/
#define CFG_CACHELINE_SIZE	32	/* ...			*/
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

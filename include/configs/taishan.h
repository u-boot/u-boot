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
 * TAISHAN.h - configuration for AMCC 440GX Ref
 ***********************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_TAISHAN		1	/* Board is taishan		*/
#define CONFIG_440GX		1	/* Specifc GX support		*/
#define CONFIG_440		1	/* ... PPC440 family		*/
#define CONFIG_4xx		1	/* ... PPC4xx family		*/
#undef	CFG_DRAM_TEST			/* Disable-takes long time!	*/
#define CONFIG_SYS_CLK_FREQ	33333333 /* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_pre_init		*/
#define CONFIG_MISC_INIT_R	1	/* Call misc_init_r		*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_SDRAM_BASE		0x00000000	/* _must_ be 0		*/
#define CFG_FLASH_BASE		0xfc000000	/* start of FLASH	*/
#define CFG_MONITOR_BASE	0xfffc0000	/* start of monitor	*/
#define CFG_PCI_MEMBASE		0x80000000	/* mapped pci memory	*/
#define CFG_PERIPHERAL_BASE	0xe0000000	/* internal peripherals	*/
#define CFG_ISRAM_BASE		0xc0000000	/* internal SRAM	*/
#define CFG_PCI_BASE		0xd0000000	/* internal PCI regs	*/

#define CFG_EBC0_FLASH_BASE	CFG_FLASH_BASE
#define CFG_EBC1_FPGA_BASE	(CFG_PERIPHERAL_BASE + 0x01000000)
#define CFG_EBC2_LCM_BASE	(CFG_PERIPHERAL_BASE + 0x02000000)
#define CFG_EBC3_CONN_BASE	(CFG_PERIPHERAL_BASE + 0x08000000)

#define CFG_GPIO_BASE		(CFG_PERIPHERAL_BASE + 0x00000700)

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in internal SRAM)
 *----------------------------------------------------------------------*/
#define CFG_TEMP_STACK_OCM	1
#define CFG_OCM_DATA_ADDR	CFG_ISRAM_BASE
#define CFG_INIT_RAM_ADDR	CFG_ISRAM_BASE  /* Initial RAM address	*/
#define CFG_INIT_RAM_END	0x2000		/* End of used area in RAM*/
#define CFG_GBL_DATA_SIZE	128		/* num bytes initial data*/

#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_POST_WORD_ADDR	(CFG_GBL_DATA_OFFSET - 0x4)
#define CFG_INIT_SP_OFFSET	CFG_POST_WORD_ADDR

#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Mon*/
#define CFG_MALLOC_LEN		(1024 * 1024)	/* Reserve 1024 kB for malloc*/

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CONFIG_UART1_CONSOLE	1	/* use of UART1 as console	*/
#define CONFIG_SERIAL_MULTI     1	/* enable serial multi support	*/
#define CFG_EXT_SERIAL_CLOCK	(1843200 * 6)	/* Ext clk @ 11.059 MHz */
#define CONFIG_BAUDRATE		115200

#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_FLASH_CFI
#define CFG_FLASH_CFI_DRIVER
#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/

#define CFG_FLASH_BANKS_LIST    {CFG_FLASH_BASE}
#define CFG_MAX_FLASH_BANKS     1		    /* number of banks	    */
#define CFG_MAX_FLASH_SECT	1024		    /* sectors per device   */

#undef	CFG_FLASH_CHECKSUM
#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_ENV_SECT_SIZE	0x40000 /* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/*-----------------------------------------------------------------------
 * E2PROM bootstrap configure value
 *----------------------------------------------------------------------*/

/*
 * 800/133/66
 * IIC 0~15: 86 78 11 6a 61 A7 04 62 00 00 00 00 00 00 00 00
 */

/*
 * 800/160/80
 * IIC 0~15: 86 78 c1 a6 09 67 04 63 00 00 00 00 00 00 00 00
 */

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#undef CONFIG_SPD_EEPROM		/* Don't use SPD EEPROM for setup	*/
#define CONFIG_SDRAM_BANK0	1	/* init onboard DDR SDRAM bank 0	*/
#define	CFG_SDRAM0_TR0		0xC10A401A
#undef CONFIG_SDRAM_ECC			/* enable ECC support			*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#undef CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR	0x50
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

#define CFG_BOOTSTRAP_IIC_ADDR	0x50

/* I2C SYSMON (LM75, AD7414 is almost compatible)			*/
#define CONFIG_DTT_LM75		1		/* ON Semi's LM75	*/
#define CONFIG_DTT_SENSORS	{0}		/* Sensor addresses	*/
#define CFG_DTT_MAX_TEMP	70
#define CFG_DTT_LOW_TEMP	-30
#define CFG_DTT_HYSTERESIS	3

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \\\"run flash_nfs\\\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define	CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=taishan\0"						\
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
	"rootpath=/opt/eldk/ppc_4xx\0"					\
	"bootfile=/tftpboot/taishan/uImage\0"				\
	"kernel_addr=fc000000\0"					\
	"ramdisk_addr=fc180000\0"					\
	"initrd_high=30000000\0"					\
	"load=tftp 100000 /tftpboot/taishan/u-boot.bin\0"		\
	"update=protect off fffc0000 ffffffff;era fffc0000 ffffffff;"	\
		"cp.b 100000 fffc0000 40000;"			        \
		"setenv filesize;saveenv\0"				\
	"upd=run load;run update\0"					\
	"fixedip=setenv bootargs $(bootargs) ip=$(ipaddr):$(serverip):" \
	"$(gatewayip):$(netmask):$(hostname):$(netdev):off panic=1\0"	\
	"dhcp=setenv bootargs $(bootargs) ip=dhcp\0"			\
	"kozio=bootm 0xffe00000\0"					\
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

/*-----------------------------------------------------------------------
 * Networking
 *----------------------------------------------------------------------*/
#define CONFIG_EMAC_NR_START	2	/* start with EMAC 2 (skip 0&1)	*/
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_NET_MULTI	1
#define CONFIG_PHY_ADDR		0xff	     /* no phy on EMAC0		*/
#define CONFIG_PHY1_ADDR	0xff	     /* no phy on EMAC1		*/
#define CONFIG_PHY2_ADDR	0x1
#define CONFIG_PHY3_ADDR	0x3
#define CONFIG_ET1011C_PHY	1
#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#define CONFIG_HAS_ETH3
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */
#define CONFIG_PHY_RESET        1       /* reset phy upon startup         */
#define CONFIG_PHY_RESET_DELAY	1000
#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

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
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_DTT
#define CONFIG_CMD_ELF
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO


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

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
/* General PCI */
#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
#define CONFIG_EEPRO100       1		/* include PCI EEPRO100		*/
#define CONFIG_PCI_SCAN_SHOW		/* show pci devices on startup	*/
#define CFG_PCI_TARGBASE    0x80000000	/* PCIaddr mapped to CFG_PCI_MEMBASE */

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT		/* let board init pci target    */

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CFG_PCI_SUBSYS_DEVICEID 0xcafe	/* Whatever */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

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
#endif	/* __CONFIG_H */

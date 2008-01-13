/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 * John Otken, jotken@softadvances.com
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
 * luan.h - configuration for LUAN board
 ***********************************************************************/
#ifndef __CONFIG_H
#define __CONFIG_H

/*-----------------------------------------------------------------------
 * High Level Configuration Options
 *----------------------------------------------------------------------*/
#define CONFIG_LUAN		1	/* Board is Luan		*/
#define CONFIG_440SP		1	/* Specific PPC440SP support    */
#define CONFIG_4xx		1	/* PPC4xx family	        */
#define CONFIG_440		1
#define CONFIG_SYS_CLK_FREQ	33333333 /* external freq to pll	*/

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/

/*-----------------------------------------------------------------------
 * Base addresses -- Note these are effective addresses where the
 * actual resources get mapped (not physical addresses)
 *----------------------------------------------------------------------*/
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Monitor */
#define CFG_MALLOC_LEN		(256 * 1024)	/* Reserve 256 kB for malloc  */
#define CFG_MONITOR_BASE	(-CFG_MONITOR_LEN)
#define CFG_SDRAM_BASE	        0x00000000	/* MUST be zero */

#define CFG_LARGE_FLASH		0xffc00000	/* 4MB flash address CS0 */
#define CFG_SMALL_FLASH		0xff900000	/* 1MB flash address CS2 */
#define CFG_SRAM_BASE		0xff800000	/* 1MB SRAM  address CS2 */
#define CFG_EPLD_BASE		0xff000000	/* EPLD and FRAM     CS1 */

#define CFG_ISRAM_BASE	        0xf8000000	/* internal 8k SRAM (L2 cache) */

#define CFG_PERIPHERAL_BASE     0xf0000000	/* internal peripherals */

#define CFG_PCI_MEMBASE		0x80000000	/* mapped pci memory */
#define CFG_PCI_BASE		0xd0000000	/* internal PCI regs */
#define CFG_PCI_TARGBASE	0x80000000	/* PCIaddr mapped to CFG_PCI_MEMBASE */

#if CFG_LARGE_FLASH == 0xffc00000
#define CFG_FLASH_BASE		CFG_LARGE_FLASH
#else
#define CFG_FLASH_BASE		CFG_SMALL_FLASH
#endif

#undef CFG_DRAM_TEST
#if CFG_SRAM_BASE
#define CFG_KBYTES_SDRAM	1024*2
#else
#define CFG_KBYTES_SDRAM	1024
#endif

/*-----------------------------------------------------------------------
 * Initial RAM & stack pointer (placed in SDRAM)
 *----------------------------------------------------------------------*/
#define CFG_INIT_RAM_ADDR	CFG_ISRAM_BASE
#define CFG_INIT_RAM_END	(8 << 10)
#define CFG_GBL_DATA_SIZE	256		/* num bytes initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Serial Port
 *----------------------------------------------------------------------*/
#define CFG_EXT_SERIAL_CLOCK	11059200 /* external 11.059MHz clk */
#define CONFIG_BAUDRATE		115200
#undef  CONFIG_SERIAL_MULTI
#undef  CONFIG_UART1_CONSOLE		/* define if you want console on UART1 */

#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
/*
 * Define here the location of the environment variables (FLASH or EEPROM).
 * Note: DENX encourages to use redundant environment in FLASH.
 */
#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

/*-----------------------------------------------------------------------
 * FLASH related
 *----------------------------------------------------------------------*/
#define CFG_MAX_FLASH_BANKS	3	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	64	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */

#define CFG_FLASH_ADDR0         0x555
#define CFG_FLASH_ADDR1         0x2aa
#define CFG_FLASH_WORD_SIZE     unsigned char

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_SECT_SIZE	0x10000 /* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR-CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)
#endif /* CFG_ENV_IS_IN_FLASH */

/*-----------------------------------------------------------------------
 * DDR SDRAM
 *----------------------------------------------------------------------*/
#define CONFIG_SPD_EEPROM	1	/* Use SPD EEPROM for setup	*/
#define SPD_EEPROM_ADDRESS	{0x53, 0x52}	/* SPD i2c spd addresses*/
#define CONFIG_DDR_ECC		1	/* with ECC support		*/

/*-----------------------------------------------------------------------
 * I2C
 *----------------------------------------------------------------------*/
#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F

#define CFG_I2C_MULTI_EEPROMS
#define CFG_I2C_EEPROM_ADDR	(0xa8>>1)
#define CFG_I2C_EEPROM_ADDR_LEN 1
#define CFG_EEPROM_PAGE_WRITE_ENABLE
#define CFG_EEPROM_PAGE_WRITE_BITS 3
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=luan\0"						\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=$(serverip):$(rootpath)\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs $(bootargs) "				\
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask)"	\
		":$(hostname):$(netdev):off panic=1\0"			\
	"addtty=setenv bootargs $(bootargs) console=ttyS0,$(baudrate)\0"\
	"flash_nfs=run nfsargs addip addtty;"				\
		"bootm $(kernel_addr)\0"				\
	"flash_self=run ramargs addip addtty;"				\
		"bootm $(kernel_addr) $(ramdisk_addr)\0"		\
	"net_nfs=tftp 200000 $(bootfile);run nfsargs addip addtty;"     \
	        "bootm\0"						\
	"rootpath=/opt/eldk/ppc_4xx\0"					\
	"bootfile=/tftpboot/luan/uImage\0"				\
	"kernel_addr=fc000000\0"					\
	"ramdisk_addr=fc100000\0"					\
	"initrd_high=30000000\0"					\
	"load=tftp 100000 /tftpboot/luan/u-boot.bin\0"			\
	"update=protect off fffc0000 ffffffff;era fffc0000 ffffffff;"	\
		"cp.b 100000 fffc0000 40000;"			        \
		"setenv filesize;saveenv\0"				\
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

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		1
#define CONFIG_CIS8201_PHY	1	/* Enable 'special' RGMII mode for Cicada phy */
#define CONFIG_PHY_GIGE		1	/* Include GbE speed/duplex detection */

#define CFG_RX_ETH_BUFFER	32	/* Number of ethernet rx buffers & descriptors */

#define CONFIG_NETCONSOLE		/* include NetConsole support	*/
#define CONFIG_NET_MULTI		/* needed for NetConsole	*/

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
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_NFS
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_SDRAM

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

#define CFG_LOAD_ADDR		0x100000 /* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */
#undef  CONFIG_LYNXKDI			/* support kdi files            */

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
#if defined(CONFIG_CMD_PCI)

/* General PCI */
#define CONFIG_PCI			/* include pci support	        */
#define CONFIG_PCI_PNP			/* do (not) pci plug-and-play   */
#define CONFIG_PCI_SCAN_SHOW            /* show pci devices on startup  */

/* Board-specific PCI */
#define CFG_PCI_TARGET_INIT
#undef  CFG_PCI_MASTER_INIT

#define CFG_PCI_SUBSYS_VENDORID 0x10e8	/* AMCC */
#define CFG_PCI_SUBSYS_DEVICEID 0x4403	/* whatever */

#endif

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

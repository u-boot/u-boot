/*
 * (C) Copyright 2001
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_405GP		1	/* This is a PPC405 CPU		*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_SBC405		1	/* ...on a WR SBC405 board	*/

#define	CONFIG_SYS_TEXT_BASE	0xFFFC0000

#define CONFIG_BOARD_EARLY_INIT_F 1	/* call board_early_init_f()	*/
#define CONFIG_MISC_INIT_R	1	/* call misc_init_r()		*/

#define CONFIG_SYS_CLK_FREQ	33333333 /* external frequency to pll	*/

#define CONFIG_BAUDRATE		9600

#define CONFIG_PREBOOT	"echo;echo Welcome to U-Boot for the sbc405;echo;echo Type \"? or help\" to get on-line help;echo"

#define CONFIG_RAMBOOT								\
	"setenv bootargs root=/dev/ram rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;"	\
	"bootm ffc00000 ffca0000"
#define CONFIG_NFSBOOT								\
	"setenv bootargs root=/dev/nfs rw nfsroot=${serverip}:${rootpath} "	\
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off;"	\
	"bootm ffc00000"

#undef CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND      "version;echo;tftpboot ${loadaddr} ${loadfile};bootvx"      /* autoboot command     */


#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_PHY_RESET_DELAY	300	/* Intel LXT971A needs this	*/

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootargs=emac(0,0)host:/T221ppc/target/config/sbc405/vxWorks.st " \
		"e=192.168.193.102:ffffffe0 h=192.168.193.100 u=target pw=hello " \
		"f=0x08 tn=sbc405 o=emac \0" \
	"env_startaddr=FF000000\0" \
	"env_endaddr=FF03FFFF\0" \
	"loadfile=vxWorks.st\0" \
	"loadaddr=0x01000000\0" \
	"net_load=tftpboot ${loadaddr} ${loadfile}\0" \
	"uboot_startaddr=FFFC0000\0" \
	"uboot_endaddr=FFFFFFFF\0" \
	"update=tftp ${loadaddr} u-boot.bin;" \
		"protect off ${uboot_startaddr} ${uboot_endaddr};" \
		"era ${uboot_startaddr} ${uboot_endaddr};" \
		"cp.b ${loadaddr} ${uboot_startaddr} ${filesize};" \
		"protect on ${uboot_startaddr} ${uboot_endaddr}\0" \
	"zapenv=protect off ${env_startaddr} ${env_endaddr};" \
		"era ${env_startaddr} ${env_endaddr};" \
		"protect on ${env_startaddr} ${env_endaddr}\0"

#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE


#define CONFIG_ENV_OVERWRITE


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BSP
#define CONFIG_CMD_ELF
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING
#define CONFIG_CMD_SDRAM


#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define CONFIG_SDRAM_BANK0	1	/* init onboard SDRAM bank 0	*/

#define CONFIG_ETHADDR	DE:AD:BE:EF:01:01	/* Ethernet address	*/
#define CONFIG_IPADDR		192.168.193.102
#define CONFIG_NETMASK		255.255.255.224
#define CONFIG_SERVERIP		192.168.193.119
#define CONFIG_GATEWAYIP	192.168.193.97

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP			/* undef to save memory		*/
#define CONFIG_SYS_PROMPT	"=> "		/* Monitor Command Prompt	*/

#undef CONFIG_SYS_HUSH_PARSER			/* use "hush" command parser	*/

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

#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef CONFIG_SYS_EXT_SERIAL_CLOCK		/* no external serial clock used */
#define CONFIG_SYS_BASE_BAUD		691200

/* The following table includes the supported baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE					\
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,	\
	 57600, 115200, 230400, 460800, 921600 }

#define CONFIG_SYS_LOAD_ADDR	0x100000	/* default load address */
#define CONFIG_SYS_EXTBDINFO	1		/* To use extended board_info (bd_t) */

#define CONFIG_SYS_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_VERSION_VARIABLE	1	/* include version env variable */

#define CONFIG_SYS_RX_ETH_BUFFER	16	/* use 16 rx buffer on 405 emac */

#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_PPC4XX
#define CONFIG_SYS_I2C_PPC4XX_CH0
#define CONFIG_SYS_I2C_PPC4XX_SPEED_0		400000
#define CONFIG_SYS_I2C_PPC4XX_SLAVE_0		0x7F

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER	0	/* configure as pci adapter	*/
#define PCI_HOST_FORCE		1	/* configure as pci host	*/
#define PCI_HOST_AUTO		2	/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_INDIRECT_BRIDGE	/* indirect PCI bridge support */
#define CONFIG_PCI_HOST	PCI_HOST_FORCE	/* select pci host function	*/
#define CONFIG_PCI_PNP			/* do pci plug-and-play		*/
					/* resource configuration	*/

#define CONFIG_PCI_SCAN_SHOW		/* print pci devices @ startup	*/

#define CONFIG_SYS_PCI_SUBSYS_VENDORID	0x12FE	/* PCI Vendor ID: esd gmbh	*/
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID	0x0408	/* PCI Device ID: PMC-405	*/
#define CONFIG_SYS_PCI_CLASSCODE	0x0b20	/* PCI Class Code: Processor/PPC*/
#define CONFIG_SYS_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CONFIG_SYS_PCI_PTM1MS	0xfc000001	/* 64MB, enable hard-wired to 1 */
#define CONFIG_SYS_PCI_PTM1PCI	0x00000000	/* Host: use this pci address	*/
#define CONFIG_SYS_PCI_PTM2LA	0xffc00000	/* point to flash		*/
#define CONFIG_SYS_PCI_PTM2MS	0xffc00001	/* 4MB, enable			*/
#define CONFIG_SYS_PCI_PTM2PCI	0x04000000	/* Host: use this pci address	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_MONITOR_BASE	0xFFFC0000
#define CONFIG_SYS_MONITOR_LEN	(256 * 1024)	/* Reserve 256 kB for Monitor	*/
#define CONFIG_SYS_MALLOC_LEN	(128 * 1024)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ	(8 << 20)	/* Initial Memory map for Linux */

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_FLASH_BASE		0xFF000000
#define CONFIG_SYS_FLASH_CFI		1	/* Flash is CFI conformant		*/
#define CONFIG_SYS_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CONFIG_SYS_FLASH_ERASE_TOUT	120000	/* Flash Erase Timeout (in ms)		*/
#define CONFIG_SYS_FLASH_INCREMENT	0x01000000
#undef CONFIG_SYS_FLASH_PROTECTION		/* don't use hardware protection	*/
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	500	/* Flash Write Timeout (in ms)		*/
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max number of sectors on one chip	*/

/*-----------------------------------------------------------------------
 * Environment Variable setup
 */
#define CONFIG_ENV_ADDR	CONFIG_SYS_FLASH_BASE	/* starting right at the beginning	*/
#define CONFIG_ENV_IS_IN_FLASH	1
#define CONFIG_ENV_OFFSET		0	/* starting right at the beginning	*/
#define CONFIG_ENV_SECT_SIZE	0x40000	/* see README - env sector total size	*/
#define CONFIG_ENV_SIZE		0x40000	/* Total Size of Environment Sector	*/

/*-----------------------------------------------------------------------
 * External Bus Controller (EBC) Setup
 */
#define FLASH0_BA	CONFIG_SYS_FLASH_BASE		/* FLASH 0 Base Address		*/

/* Memory Bank 0 (Flash Bank 0) initialization					*/
#define CONFIG_SYS_EBC_PB0AP	0x92015480
#define CONFIG_SYS_EBC_PB0CR	FLASH0_BA | 0x9C000 /* BAS=0xFF0,BS=16MB,BU=R/W,BW=32bit*/

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in data cache)
 */

/* use on chip memory ( OCM ) for temperary stack until sdram is tested */
#define CONFIG_SYS_TEMP_STACK_OCM	1

/* On Chip Memory location */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000

#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE /* Size of used area in RAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 */
#define SPD_EEPROM_ADDRESS	0x50
#define CONFIG_SPD_EEPROM	1	/* use SPD EEPROM for setup		*/

#endif	/* __CONFIG_H */

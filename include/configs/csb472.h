/*
 * (C) Copyright 2004
 * Tolunay Orkun, Nextio Inc., torkun@nextio.com
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_405GP		1	/* This is a PPC405GP CPU	*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family   */
#define CONFIG_CSB472		1	/* on a Cogent CSB472 board     */
#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f()    */
#define CONFIG_LAST_STAGE_INIT	1	/* Call last_stage_init()	*/
#define CONFIG_SYS_CLK_FREQ     25000000 /* external frequency to pll   */

#define	CONFIG_SYS_TEXT_BASE	0xFFFC0000

/*
 * OS Bootstrap configuration
 *
 */

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled */
#else
#define CONFIG_BOOTDELAY	3	/* autoboot after X seconds	*/
#endif

#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check keypress when bootdelay = 0 */

#if 1
#undef  CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND \
	"setenv bootargs console=ttyS0,38400 debug " \
	"root=/dev/ram rw ramdisk_size=4096 " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; " \
	"bootm ff800000 ff900000"
#endif

#if 0
#undef	CONFIG_BOOTARGS
#define CONFIG_BOOTCOMMAND \
	"bootp; " \
	"setenv bootargs console=ttyS0,38400 debug " \
	"root=/dev/nfs rw nfsroot=${serverip}:${rootpath} " \
	"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off; " \
	"bootm"
#endif

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_DNS2


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BEDBUG
#define CONFIG_CMD_ELF
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_I2C
#define CONFIG_CMD_PCI
#define CONFIG_CMD_DATE
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP

/*
 * Serial download configuration
 *
 */
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CONFIG_SYS_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

/*
 * KGDB Configuration
 *
 */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 *
 */
#undef	CONFIG_SYS_HUSH_PARSER			/* use "hush" command parser */
#ifdef	CONFIG_SYS_HUSH_PARSER
#define	CONFIG_SYS_PROMPT_HUSH_PS2	"> "	/* hush shell secondary prompt */
#endif

#define CONFIG_SYS_LONGHELP			/* undef to save memory	*/
#define CONFIG_SYS_PROMPT		"=> "	/* Monitor Command Prompt */
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#else
#define	CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#endif
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size */

#define CONFIG_SYS_MEMTEST_START	0x0400000 /* memtest works on */
#define CONFIG_SYS_MEMTEST_END		0x0C00000 /* 4 ... 12 MB in DRAM */

#define	CONFIG_SYS_HZ			1000	/* decrementer freq: 1 ms ticks	*/
#define CONFIG_SYS_EXTBDINFO		1	/* To use extended board_info (bd_t) */
#define CONFIG_SYS_LOAD_ADDR		0x100000 /* default load address */

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20) /* Initial Memory map for Linux */

/*
 * watchdog configuration
 *
 */
#undef  CONFIG_WATCHDOG			/* watchdog disabled */

/*
 * UART configuration
 *
 */
#define CONFIG_CONS_INDEX		1	/* Use UART0		*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

#undef CONFIG_SYS_EXT_SERIAL_CLOCK		/* use internal serial clock */
#define CONFIG_SYS_BASE_BAUD		691200
#define CONFIG_BAUDRATE		38400	/* Default baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE      \
    { 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400 }

/*
 * I2C configuration
 *
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SYS_I2C_SPEED		100000	/* I2C speed			*/
#define CONFIG_SYS_I2C_SLAVE		0x7F	/* I2C slave address		*/

/*
 * MII PHY configuration
 *
 */
#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/
#define CONFIG_PHY_CMD_DELAY	40	/* PHY COMMAND delay		*/
					/* 32usec min. for LXT971A	*/
#define CONFIG_PHY_RESET_DELAY	300	/* PHY RESET recovery delay	*/
#define CONFIG_NET_MULTI

/*
 * RTC configuration
 *
 * Note that DS1307 RTC is limited to 100Khz I2C bus.
 *
 */
#define CONFIG_RTC_DS1307		/* Use Dallas 1307 RTC		*/

/*
 * PCI stuff
 *
 */
#define CONFIG_PCI			/* include pci support	        */
#define PCI_HOST_ADAPTER	0	/* configure ar pci adapter     */
#define PCI_HOST_FORCE		1	/* configure as pci host        */
#define PCI_HOST_AUTO		2	/* detected via arbiter enable  */

#define CONFIG_PCI_HOST	PCI_HOST_FORCE  /* select pci host function     */
#define CONFIG_PCI_PNP			/* do pci plug-and-play         */
					/* resource configuration       */
#undef  CONFIG_PCI_SCAN_SHOW            /* print pci devices @ startup  */
#define CONFIG_PCI_BOOTDELAY    0       /* enable pci bootdelay variable*/

#define CONFIG_SYS_PCI_SUBSYS_VENDORID 0x0000  /* PCI Vendor ID: to-do!!!      */
#define CONFIG_SYS_PCI_SUBSYS_DEVICEID 0x0000  /* PCI Device ID: to-do!!!      */
#define CONFIG_SYS_PCI_PTM1LA  0x00000000      /* point to sdram               */
#define CONFIG_SYS_PCI_PTM1MS  0x80000001      /* 2GB, enable hard-wired to 1  */
#define CONFIG_SYS_PCI_PTM1PCI 0x00000000      /* Host: use this pci address   */
#define CONFIG_SYS_PCI_PTM2LA  0x00000000      /* disabled                     */
#define CONFIG_SYS_PCI_PTM2MS  0x00000000      /* disabled                     */
#define CONFIG_SYS_PCI_PTM2PCI 0x04000000      /* Host: use this pci address   */

/*
 * IDE stuff
 *
 */
#undef  CONFIG_IDE_PCMCIA               /* no pcmcia interface required */
#undef  CONFIG_IDE_LED                  /* no led for ide supported     */
#undef  CONFIG_IDE_RESET                /* no reset for ide supported   */

/*
 * Environment configuration
 *
 */
#define CONFIG_ENV_IS_IN_FLASH	1	/* environment is in FLASH	*/
#undef CONFIG_ENV_IS_IN_NVRAM
#undef CONFIG_ENV_IS_IN_EEPROM

/*
 * General Memory organization
 *
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFF800000
#define CONFIG_SYS_FLASH_SIZE		0x00800000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024) /* Reserve 256 KB for Monitor */
#define CONFIG_SYS_MALLOC_LEN		(128 * 1024) /* Reserve 128 KB for malloc() */

#if CONFIG_SYS_MONITOR_BASE < CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_RAMSTART
#endif

#if defined(CONFIG_ENV_IS_IN_FLASH)
#define CONFIG_ENV_IN_OWN_SECTOR	1	   /* Give Environment own sector */
#define CONFIG_ENV_ADDR		0xFFF00000 /* Address of Environment Sector */
#define	CONFIG_ENV_SIZE		0x00001000 /* Size of Environment */
#define CONFIG_ENV_SECT_SIZE	0x00040000 /* Size of Environment Sector */
#endif

/*
 * FLASH Device configuration
 *
 */
#define CONFIG_SYS_FLASH_CFI		1	/* flash is CFI conformant	*/
#define CONFIG_FLASH_CFI_DRIVER	1	/* use common cfi driver	*/
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster) */
#define CONFIG_SYS_MAX_FLASH_BANKS	1	/* max # of memory banks	*/
#define CONFIG_SYS_FLASH_INCREMENT	0	/* there is only one bank	*/
#define CONFIG_SYS_MAX_FLASH_SECT	64	/* max # of sectors on one chip	*/
#define CONFIG_SYS_FLASH_PROTECTION	1	/* hardware flash protection	*/
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

/*
 * On Chip Memory location/size
 *
 */
#define CONFIG_SYS_OCM_DATA_ADDR	0xF8000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000

/*
 * Global info and initial stack
 *
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of on-chip SRAM */
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE /* Size of used area in RAM */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET

/*
 * Miscellaneous board specific definitions
 *
 */
#define CONFIG_I2CFAST		1	/* enable "i2cfast" env. setting     */

#endif	/* __CONFIG_H */

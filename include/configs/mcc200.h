/*
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_MPC5200
#define CONFIG_MPC5xxx		1	/* This is an MPC5xxx CPU		*/
#define CONFIG_MCC200		1	/* ... on MCC200 board			*/

#define CFG_MPC5XXX_CLKIN	33000000 /* ... running at 33MHz		*/

#define CONFIG_MISC_INIT_R

#define BOOTFLAG_COLD		0x01	/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM		0x02	/* Software reboot			*/

#define CFG_CACHELINE_SIZE	32	/* For MPC5xxx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#  define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*
 * Serial console configuration
 *
 *  To select console on the one of 8 external UARTs,
 * define CONFIG_QUART_CONSOLE as 1, 2, 3, or 4 for the first Quad UART,
 * or as 5, 6, 7, or 8 for the second Quad UART.
 * COM11, COM12, COM13, COM14 are located on the second Quad UART.
 *
 *  CONFIG_PSC_CONSOLE must be undefined in this case.
 */
#if !defined(CONFIG_PRS200)
/* MCC200 configuration: */
#ifdef CONFIG_CONSOLE_COM12
#define CONFIG_QUART_CONSOLE	6	/* console is on UARTF of QUART2	*/
#else
#define CONFIG_QUART_CONSOLE	8	/* console is on UARTH of QUART2	*/
#endif
#else
/* PRS200 configuration: */
#undef CONFIG_QUART_CONSOLE
#endif /* CONFIG_PRS200 */
/*
 *  To select console on PSC1, define CONFIG_PSC_CONSOLE as 1
 * and undefine CONFIG_QUART_CONSOLE.
 */
#if !defined(CONFIG_PRS200)
/* MCC200 configuration: */
#define CONFIG_SERIAL_MULTI	1
#define CONFIG_PSC_CONSOLE	1	/* PSC1 may be COM */
#define CONFIG_PSC_CONSOLE2	2	/* PSC2 is PSoC */
#else
/* PRS200 configuration: */
#define CONFIG_PSC_CONSOLE	1	/* console is on PSC1		*/
#endif
#if defined(CONFIG_QUART_CONSOLE) && defined(CONFIG_PSC_CONSOLE) && \
	!defined(CONFIG_SERIAL_MULTI)
#error "Select only one console device!"
#endif
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 230400 }

#define CONFIG_MII		1

#define CONFIG_DOS_PARTITION

/* USB */
#define CONFIG_USB_OHCI
#define ADD_USB_CMD		CFG_CMD_USB | CFG_CMD_FAT
#define CONFIG_USB_STORAGE
/* automatic software updates (see board/mcc200/auto_update.c) */
#define CONFIG_AUTO_UPDATE 1

/*
 * Supported commands
 */
#define CONFIG_COMMANDS	       (CONFIG_CMD_DFL	| \
				ADD_USB_CMD	| \
				CFG_CMD_BEDBUG	| \
				CFG_CMD_FAT	| \
				CFG_CMD_I2C)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Autobooting
 */
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds */

#define CONFIG_PREBOOT	"echo;"	\
	"echo Type \"run flash_nfs\" to mount root filesystem over NFS;" \
	"echo"

#undef	CONFIG_BOOTARGS

#define XMK_STR(x)		#x
#define MK_STR(x)		XMK_STR(x)

#ifdef CONFIG_PRS200
# define CFG__BOARDNAME		"prs200"
# define CFG__LINUX_CONSOLE	"ttyS0"
#else
# define CFG__BOARDNAME		"mcc200"
# define CFG__LINUX_CONSOLE	"ttyEU7"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"netdev=eth0\0"							\
	"hostname=" CFG__BOARDNAME "\0"					\
	"nfsargs=setenv bootargs root=/dev/nfs rw "			\
		"nfsroot=${serverip}:${rootpath}\0"			\
	"ramargs=setenv bootargs root=/dev/ram rw\0"			\
	"addip=setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}"	\
		":${hostname}:${netdev}:off panic=1\0"			\
	"addcons=setenv bootargs ${bootargs} "				\
		"console=${console},${baudrate}\0"			\
	"flash_nfs=run nfsargs addip addcons;"				\
		"bootm ${kernel_addr}\0"				\
	"flash_self=run ramargs addip addcons;"				\
		"bootm ${kernel_addr} ${ramdisk_addr}\0"		\
	"net_nfs=tftp 200000 ${bootfile};"				\
		"run nfsargs addip addcons;bootm\0"			\
	"console=" CFG__LINUX_CONSOLE "\0"				\
	"rootpath=/opt/eldk/ppc_6xx\0"					\
	"bootfile=/tftpboot/" CFG__BOARDNAME "/uImage\0"		\
	"load=tftp 200000 /tftpboot/" CFG__BOARDNAME "/u-boot.bin\0"	\
	"text_base=" MK_STR(TEXT_BASE) "\0"				\
	"update=protect off ${text_base} +${filesize};"			\
		"era ${text_base} +${filesize};"			\
		"cp.b 200000 ${text_base} ${filesize}\0"		\
	"unlock=yes\0"							\
	""
#undef MK_STR
#undef XMK_STR

#define CONFIG_BOOTCOMMAND	"run flash_self"

#define CFG_HUSH_PARSER		1	/* use "hush" command parser	*/
#define CFG_PROMPT_HUSH_PS2	"> "

/*
 * IPB Bus clocking configuration.
 */
#define CFG_IPBCLK_EQUALS_XLBCLK		/* define for 133MHz speed */

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support */
#define CFG_I2C_MODULE		2	/* Select I2C module #1 or #2 */

#define CFG_I2C_SPEED		100000 /* 100 kHz */
#define CFG_I2C_SLAVE		0x7F

/*
 * Flash configuration (8,16 or 32 MB)
 * TEXT base always at 0xFFF00000
 * ENV_ADDR always at  0xFFF40000
 * FLASH_BASE at 0xFC000000 for 64 MB (only 32MB are supported, not enough addr lines!!!)
 *		 0xFE000000 for 32 MB
 *		 0xFF000000 for 16 MB
 *		 0xFF800000 for  8 MB
 */
#define CFG_FLASH_BASE		0xfc000000
#define CFG_FLASH_SIZE		0x04000000

#define CFG_FLASH_CFI				/* The flash is CFI compatible	*/
#define CFG_FLASH_CFI_DRIVER			/* Use common CFI driver	*/

#define CFG_FLASH_BANKS_LIST	{ CFG_FLASH_BASE }

#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip	*/

#define CFG_FLASH_USE_BUFFER_WRITE 1	/* use buffered writes (20x faster)	*/
#define CFG_FLASH_PROTECTION	1	/* hardware flash protection		*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_EMPTY_INFO		/* print 'E' for empty sector on flinfo */
#define CFG_FLASH_QUIET_TEST	1	/* don't warn upon unknown flash	*/

#define CFG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars	*/

#define CFG_ENV_SECT_SIZE	0x40000	/* size of one complete sector	*/
#define CFG_ENV_ADDR		(CFG_MONITOR_BASE + CFG_MONITOR_LEN)
#define	CFG_ENV_SIZE		0x2000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_ADDR_REDUND	(CFG_ENV_ADDR + CFG_ENV_SECT_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

#define CONFIG_ENV_OVERWRITE	1	/* allow modification of vendor params */

#if TEXT_BASE == CFG_FLASH_BASE
#define CFG_LOWBOOT	1
#endif

/*
 * Memory map
 */
#define CFG_MBAR		0xf0000000
#define CFG_SDRAM_BASE		0x00000000
#define CFG_DEFAULT_MBAR	0x80000000

/* Use SRAM until RAM will be available */
#define CFG_INIT_RAM_ADDR	MPC5XXX_SRAM
#define CFG_INIT_RAM_END	MPC5XXX_SRAM_SIZE	/* End of used area in DPRAM */


#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

#define CFG_MONITOR_BASE	TEXT_BASE
#if (CFG_MONITOR_BASE < CFG_FLASH_BASE)
#   define CFG_RAMBOOT		1
#endif

#define CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MALLOC_LEN		(512 << 10)	/* Reserve 512 kB for malloc()	*/
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */

/*
 * Ethernet configuration
 */
#define CONFIG_MPC5xxx_FEC	1
/*
 * Define CONFIG_FEC_10MBIT to force FEC at 10Mb
 */
/* #define CONFIG_FEC_10MBIT 1 */
#define CONFIG_PHY_ADDR		1

/*
 * LCD Splash Screen
 */
#if !defined(CONFIG_PRS200)
#define CONFIG_LCD		1
#define CONFIG_PROGRESSBAR 1
#endif

#if defined(CONFIG_LCD)
#define CONFIG_SPLASH_SCREEN	1
#define CFG_CONSOLE_IS_IN_ENV	1
#define LCD_BPP			LCD_MONOCHROME
#endif

/*
 * GPIO configuration
 */
/* 0x10000004 = 32MB SDRAM */
/* 0x90000004 = 64MB SDRAM */
#if defined(CONFIG_LCD)
/* set PSC2 in UART mode */
#define CFG_GPS_PORT_CONFIG	0x00000044
#else
#define CFG_GPS_PORT_CONFIG	0x00000004
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT		"=> "	/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE		1024	/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE		256	/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print Buffer Size	*/
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00f00000	/* 1 ... 15 MB in DRAM	*/

#define CFG_LOAD_ADDR		0x100000	/* default load address */

#define CFG_HZ			1000	/* decrementer freq: 1 ms ticks */

/*
 * Various low-level settings
 */
#define CFG_HID0_INIT		HID0_ICE | HID0_ICFI
#define CFG_HID0_FINAL		HID0_ICE

#define CFG_BOOTCS_START	CFG_FLASH_BASE
#define CFG_BOOTCS_SIZE		CFG_FLASH_SIZE
#define CFG_BOOTCS_CFG		0x0004fb00
#define CFG_CS0_START		CFG_FLASH_BASE
#define CFG_CS0_SIZE		CFG_FLASH_SIZE

/* Quad UART @0x80000000 (MBAR is relocated to 0xF0000000) */
#define CFG_CS2_START		0x80000000
#define CFG_CS2_SIZE		0x00001000
#define CFG_CS2_CFG		0x1d300

/* Second Quad UART @0x80010000 */
#define CFG_CS1_START		0x80010000
#define CFG_CS1_SIZE		0x00001000
#define CFG_CS1_CFG		0x1d300

/*
 *  Select one of quarts as a default
 * console. If undefined - PSC console
 * wil be default
 */
#define CFG_CS_BURST		0x00000000
#define CFG_CS_DEADCYCLE	0x33333333

#define CFG_RESET_ADDRESS	0xff000000

/*
 * QUART Expanders support
 */
#if defined(CONFIG_QUART_CONSOLE)
/*
 * We'll use NS16550 chip routines,
 */
#define CFG_NS16550		1
#define CFG_NS16550_SERIAL	1
#define CONFIG_CONS_INDEX	1
/*
 *  To achieve necessary offset on SC16C554
 * A0-A2 (register select) pins with NS16550
 * functions (in struct NS16550), REG_SIZE
 * should be 4, because A0-A2 pins are connected
 * to DA2-DA4 address bus lines.
 */
#define CFG_NS16550_REG_SIZE	4
/*
 * LocalPlus Bus already inited in cpu_init_f(),
 * so can work with QUART's chip selects.
 * One of four SC16C554 UARTs is selected with
 * A3-A4 (DA5-DA6) lines.
 */
#if (CONFIG_QUART_CONSOLE > 0) && (CONFIG_QUART_CONSOLE < 5) && !defined(CONFIG_PRS200)
#define CFG_NS16550_COM1	(CFG_CS2_START | (CONFIG_QUART_CONSOLE - 1)<<5)
#elif (CONFIG_QUART_CONSOLE > 4) && (CONFIG_QUART_CONSOLE < 9)
#define CFG_NS16550_COM1	(CFG_CS1_START | (CONFIG_QUART_CONSOLE - 5)<<5)
#elif
#error "Wrong QUART expander number."
#endif

/*
 * SC16C554 chip's external crystal oscillator frequency
 * is 7.3728 MHz
 */
#define CFG_NS16550_CLK		7372800
#endif /* CONFIG_QUART_CONSOLE */
/*-----------------------------------------------------------------------
 * USB stuff
 *-----------------------------------------------------------------------
 */
#define CONFIG_USB_CLOCK	0x0001BBBB
#define CONFIG_USB_CONFIG	0x00005000

#endif /* __CONFIG_H */

/*
 * AMIRIX.h: AMIRIX specific config options
 *
 * Author : Frank Smith (smith at amirix dot com)
 *
 * Derived from : other configuration header files in this tree
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License (GPL) version 2, incorporated herein by
 * reference. Drivers based on or derived from this code fall under the GPL
 * and must retain the authorship, copyright and this license notice. This
 * file is not a complete program and may only be used when the entire
 * program is licensed under the GPL.
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#undef DEBUG

#define CONFIG_405	1		/* This is a PPC405 CPU	    */
#define CONFIG_4xx	1		/* ...member of PPC4xx family	*/

#define CONFIG_AP1000	1		/* ...on an AP1000 board    */

#define CONFIG_PCI	1

#define CFG_HUSH_PARSER 1		/* use "hush" command parser	*/
#define CFG_PROMPT		"0> "
#define CFG_PROMPT_HUSH_PS2	"> "

#define CONFIG_COMMAND_EDIT	1
#define CONFIG_COMMAND_HISTORY	1
#define CONFIG_COMPLETE_ADDRESSES 1

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_FLASH_USE_BUFFER_WRITE

#ifdef CFG_ENV_IS_IN_NVRAM
#undef CFG_ENV_IS_IN_FLASH
#else
#ifdef CFG_ENV_IS_IN_FLASH
#undef CFG_ENV_IS_IN_NVRAM
#endif
#endif

#define CONFIG_BAUDRATE		57600
#define CONFIG_BOOTDELAY	3	/* autoboot after 3 seconds */

#define CONFIG_BOOTCOMMAND	""	/* autoboot command */

/* Size (bytes) of interrupt driven serial port buffer.
 * Set to 0 to use polling instead of interrupts.
 * Setting to 0 will also disable RTS/CTS handshaking.
 */
#undef	CONFIG_SERIAL_SOFTWARE_FIFO

#define CONFIG_BOOTARGS		"console=ttyS0,57600"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_COMMANDS	       (CONFIG_CMD_DFL	| \
				CFG_CMD_ASKENV	| \
				CFG_CMD_DHCP	| \
				CFG_CMD_ELF	| \
				CFG_CMD_IRQ	| \
				CFG_CMD_MVENV	| \
				CFG_CMD_PCI	| \
				CFG_CMD_PING	\
			       )

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#undef CONFIG_WATCHDOG			/* watchdog disabled	    */

#define CONFIG_SYS_CLK_FREQ	30000000

#define CONFIG_SPD_EEPROM	1	/* use SPD EEPROM for setup    */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory	    */
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size  */
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
/* usually: (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) */
#define CFG_PBSIZE	(CFG_CBSIZE+4+16)	/* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_ALT_MEMTEST		1
#define CFG_MEMTEST_START	0x00400000	/* memtest works on */
#define CFG_MEMTEST_END		0x01000000	/* 4 ... 16 MB in DRAM	*/

/*
 * If CFG_EXT_SERIAL_CLOCK, then the UART divisor is 1.
 * If CFG_405_UART_ERRATA_59, then UART divisor is 31.
 * Otherwise, UART divisor is determined by CPU Clock and CFG_BASE_BAUD value.
 * The Linux BASE_BAUD define should match this configuration.
 *    baseBaud = cpuClock/(uartDivisor*16)
 * If CFG_405_UART_ERRATA_59 and 200MHz CPU clock,
 * set Linux BASE_BAUD to 403200.
 */
#undef	CFG_EXT_SERIAL_CLOCK		/* external serial clock */
#undef	CFG_405_UART_ERRATA_59		/* 405GP/CR Rev. D silicon */

#define CFG_NS16550_CLK		40000000
#define CFG_DUART_CHAN		0
#define CFG_NS16550_COM1	(0x4C000000 + 0x1000)
#define CFG_NS16550_COM2	(0x4C800000 + 0x1000)
#define CFG_NS16550_REG_SIZE	4
#define CFG_NS16550		1
#define CFG_INIT_CHAN1		1
#define CFG_INIT_CHAN2		0

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CFG_LOAD_ADDR		0x00200000	/* default load address */
#define CFG_EXTBDINFO		1		/* To use extended board_into (bd_t) */

#define CFG_HZ			1000		/* decrementer freq: 1 ms ticks */

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0x20000000
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 * 1024)	/* Reserve 196 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_FLASH_CFI		1
#define CFG_PROGFLASH_BASE	CFG_FLASH_BASE
#define CFG_CONFFLASH_BASE	0x24000000

#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks	    */
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)  */
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_PROTECTION	1	/* use hardware protection	    */

/* BEG ENVIRONNEMENT FLASH */
#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_OFFSET		0x00040000 /* Offset of Environment Sector	*/
#define CFG_ENV_SIZE		0x1000	/* Total Size of Environment Sector */
#define CFG_ENV_SECT_SIZE	0x20000 /* see README - env sector total size	*/
#endif
/* END ENVIRONNEMENT FLASH */
/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CFG_NVRAM_BASE_ADDR	0xf0000000	/* NVRAM base address	*/
#define CFG_NVRAM_SIZE		0x1ff8		/* NVRAM size	*/

#ifdef CFG_ENV_IS_IN_NVRAM
#define CFG_ENV_SIZE		0x1000		/* Size of Environment vars */
#define CFG_ENV_ADDR	    \
    (CFG_NVRAM_BASE_ADDR+CFG_NVRAM_SIZE-CFG_ENV_SIZE)	/* Env	*/
#endif
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value    */
#endif

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	CFG_FLASH_BASE	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0		/* FLASH bank #1	*/

/* Configuration Port location */
#define CONFIG_PORT_ADDR	0xF0000500

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */

#define CFG_INIT_RAM_ADDR	0x400000  /* inside of SDRAM			 */
#define CFG_INIT_RAM_END	0x2000	/* End of used area in RAM	       */
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Definitions for Serial Presence Detect EEPROM address
 * (to get SDRAM settings)
 */
#define SPD_EEPROM_ADDRESS	0x50

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH */
#define BOOTFLAG_WARM	0x02		/* Software reboot		*/

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

/* JFFS2 stuff */

#define CFG_JFFS2_FIRST_BANK	0
#define CFG_JFFS2_NUM_BANKS	1
#define CFG_JFFS2_FIRST_SECTOR	1

#define CONFIG_NET_MULTI
#define CONFIG_E1000

#define CFG_ETH_DEV_FN		0x0800
#define CFG_ETH_IOBASE		0x31000000
#define CFG_ETH_MEMBASE		0x32000000

#endif	/* __CONFIG_H */

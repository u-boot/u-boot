/*
 * (C) Copyright 2008
 *  Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@uam.es
 *  This work has been supported by: QTechnology  http://qtec.com/
 *
 *  (C) Copyright 2008
 *  Georg Schardt <schardt@team-ctech.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
*/

#ifndef __CONFIG_XLX_H
#define __CONFIG_XLX_H
/*
#define DEBUG
#define ET_DEBUG
*/

/*Mem Map*/
#define CONFIG_SYS_SDRAM_BASE		0x0
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(192 * 1024)
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 128 * 1024)

/*Cmd*/
#include <config_cmd_default.h>
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_CACHE
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_ELF
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_REGINFO
#undef CONFIG_CMD_JFFS2
#undef CONFIG_CMD_MTDPARTS
#undef CONFIG_CMD_SPI
#undef CONFIG_CMD_I2C
#undef CONFIG_CMD_DTT
#undef CONFIG_CMD_NET
#undef CONFIG_CMD_PING
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_EEPROM
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_NFS

/*Misc*/
#define CONFIG_BOOTDELAY		5/* autoboot after 5 seconds     */
#define CONFIG_SYS_LONGHELP		/* undef to save memory         */
#if defined(CONFIG_CMD_KGDB)
#define CONFIG_SYS_CBSIZE		1024/* Console I/O Buffer Size      */
#else
#define CONFIG_SYS_CBSIZE		256/* Console I/O Buffer Size      */
#endif
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE +\
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16
					/* max number of command args   */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
					/* Boot Argument Buffer Size */
#define CONFIG_SYS_MEMTEST_START	0x00400000
					/* memtest works on           */
#define CONFIG_SYS_MEMTEST_END		0x00C00000
					/* 4 ... 12 MB in DRAM        */
#define CONFIG_SYS_LOAD_ADDR		0x00400000
					/* default load address       */
#define CONFIG_SYS_EXTBDINFO		1
					/* Extended board_into (bd_t) */
#define CONFIG_SYS_HZ			1000
					/* decrementer freq: 1 ms ticks */
#define CONFIG_CMDLINE_EDITING		/* add command line history     */
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support   */
#define CONFIG_LOOPW			/* enable loopw command         */
#define CONFIG_MX_CYCLIC		/* enable mdc/mwc commands      */
#define CONFIG_ZERO_BOOTDELAY_CHECK	/* check for keypress on bootdelay==0 */
#define CONFIG_VERSION_VARIABLE		/* include version env variable */
#define CONFIG_SYS_CONSOLE_INFO_QUIET	/* don't print console @ startup */
#define CONFIG_SYS_HUSH_PARSER		/* Use the HUSH parser          */
#define CONFIG_LOADS_ECHO		/* echo on for serial download  */
#define CONFIG_SYS_LOADS_BAUD_CHANGE	/* allow baudrate change        */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)
				/* Initial Memory map for Linux */

/*Stack*/
#define CONFIG_SYS_INIT_RAM_ADDR	0x800000/* Initial RAM address    */
#define CONFIG_SYS_INIT_RAM_SIZE		0x2000	/* Size of used area in RAM  */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE \
				- GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
/*Speed*/
#define CONFIG_SYS_CLK_FREQ	XPAR_CORE_CLOCK_FREQ_HZ

/*Flash*/
#ifdef XPAR_FLASH_MEM0_BASEADDR
#define	CONFIG_SYS_FLASH_BASE		XPAR_FLASH_MEM0_BASEADDR
#define	CONFIG_SYS_FLASH_CFI		1
#define	CONFIG_FLASH_CFI_DRIVER	1
#define	CONFIG_SYS_FLASH_EMPTY_INFO	1
#define	CONFIG_SYS_MAX_FLASH_BANKS	1
#define	CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_CMD_JFFS2
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define CONFIG_FLASH_CFI_MTD
#else
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SYS_NO_FLASH
#endif

/* serial communication */
#ifdef XPAR_UARTLITE_0_BASEADDR
#define CONFIG_XILINX_UARTLITE
#define XILINX_UARTLITE_BASEADDR	XPAR_UARTLITE_0_BASEADDR
#define CONFIG_BAUDRATE			XPAR_UARTLITE_0_BAUDRATE
#define CONFIG_SYS_BAUDRATE_TABLE	{ CONFIG_BAUDRATE }
#else
#ifdef XPAR_UARTNS550_0_BASEADDR
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	4
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		XPAR_UARTNS550_0_BASEADDR
#define CONFIG_SYS_NS16550_CLK		XPAR_UARTNS550_0_CLOCK_FREQ_HZ
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 115200 }
#endif
#endif

#endif						/* __CONFIG_H */

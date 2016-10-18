/*
 * (C) Copyright 2008
 *  Ricado Ribalda-Universidad Autonoma de Madrid-ricardo.ribalda@gmail.com
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
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_REGINFO
#undef CONFIG_CMD_JFFS2
#undef CONFIG_CMD_MTDPARTS
#undef CONFIG_CMD_DTT
#undef CONFIG_CMD_EEPROM

/*Misc*/
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
					/* decrementer freq: 1 ms ticks */
#define CONFIG_CMDLINE_EDITING		/* add command line history     */
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support   */
#define CONFIG_MX_CYCLIC		/* enable mdc/mwc commands      */
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

#define CONFIG_BAUDRATE			115200
/* The following table includes the supported baudrates */
# define CONFIG_SYS_BAUDRATE_TABLE \
	{300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#endif						/* __CONFIG_H */

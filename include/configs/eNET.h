/*
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
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

#define CONFIG_RELOC_FIXUP_WORKS

/*
 * Stuff still to be dealt with -
 */
#define CONFIG_RTC_MC146818

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define DEBUG_PARSER

#define CONFIG_X86			1	/* Intel X86 CPU */
#define CONFIG_SYS_SC520		1	/* AMD SC520 */
#define CONFIG_SYS_SC520_SSI
#define CONFIG_SHOW_BOOT_PROGRESS	1
#define CONFIG_LAST_STAGE_INIT		1

/*
 * If CONFIG_HW_WATCHDOG is not defined, the watchdog jumper on the
 * bottom (processor) board MUST be removed!
 */
#undef CONFIG_WATCHDOG
#undef CONFIG_HW_WATCHDOG

 /*-----------------------------------------------------------------------
  * Video Configuration
  */
#undef CONFIG_VIDEO			/* No Video Hardware */
#undef CONFIG_CFB_CONSOLE

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 128*1024)

#define CONFIG_BAUDRATE		9600

/*-----------------------------------------------------------------------
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_BDI		/* bdinfo			*/
#define CONFIG_CMD_BOOTD	/* bootd			*/
#define CONFIG_CMD_CONSOLE	/* coninfo			*/
#define CONFIG_CMD_ECHO		/* echo arguments		*/
#define CONFIG_CMD_FLASH	/* flinfo, erase, protect	*/
#define CONFIG_CMD_FPGA		/* FPGA configuration Support	*/
#define CONFIG_CMD_IMI		/* iminfo			*/
#define CONFIG_CMD_IMLS		/* List all found images	*/
#define CONFIG_CMD_IRQ		/* IRQ Information		*/
#define CONFIG_CMD_ITEST	/* Integer (and string) test	*/
#define CONFIG_CMD_LOADB	/* loadb			*/
#define CONFIG_CMD_LOADS	/* loads			*/
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC		/* Misc functions like sleep etc*/
#undef CONFIG_CMD_NET		/* bootp, tftpboot, rarpboot	*/
#undef CONFIG_CMD_NFS		/* NFS support			*/
#define CONFIG_CMD_PCI		/* PCI support			*/
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#define CONFIG_CMD_SETGETDCR	/* DCR support on 4xx		*/
#define CONFIG_CMD_SOURCE	/* "source" command Support	*/
#define CONFIG_CMD_XIMG		/* Load part of Multi Image	*/

#define CONFIG_BOOTDELAY		15
#define CONFIG_BOOTARGS			"root=/dev/mtdblock0 console=ttyS0,9600"
/* #define CONFIG_BOOTCOMMAND		"bootm 38000000" */

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE		115200		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX		2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"boot > "	/* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					 sizeof(CONFIG_SYS_PROMPT) + \
					 16)		/* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16		/* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00100000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x01000000	/* 1 ... 16 MB in DRAM	*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address	*/

#define	CONFIG_SYS_HZ			1024		/* incrementer freq: 1kHz */

						/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*-----------------------------------------------------------------------
 * SDRAM Configuration
 */
#define CONFIG_SYS_SDRAM_DRCTMCTL	0x18
#define CONFIG_NR_DRAM_BANKS		4

/* CONFIG_SYS_SDRAM_DRCTMCTL Overrides the following*/
#undef CONFIG_SYS_SDRAM_PRECHARGE_DELAY
#undef CONFIG_SYS_SDRAM_REFRESH_RATE
#undef CONFIG_SYS_SDRAM_RAS_CAS_DELAY
#undef CONFIG_SYS_SDRAM_CAS_LATENCY_2T
#undef CONFIG_SYS_SDRAM_CAS_LATENCY_3T

/*-----------------------------------------------------------------------
 * CPU Features
 */
#define CONFIG_SYS_SC520_HIGH_SPEED	0	/* 100 or 133MHz */
#undef  CONFIG_SYS_SC520_RESET			/* use SC520 MMCR's to reset cpu */
#define CONFIG_SYS_SC520_TIMER			/* use SC520 swtimers */
#undef  CONFIG_SYS_GENERIC_TIMER		/* use the i8254 PIT timers */
#undef  CONFIG_SYS_TSC_TIMER			/* use the Pentium TSC timers */
#define CONFIG_SYS_USE_SIO_UART		0       /* prefer the uarts on the SIO to those
					 * in the SC520 on the CDP */
#define CONFIG_SYS_PCAT_INTERRUPTS
#define CONFIG_SYS_NUM_IRQS		16

/*-----------------------------------------------------------------------
 * Memory organization
 */
#define CONFIG_SYS_STACK_SIZE		0x8000  	/* Size of bootloader stack */
#define CONFIG_SYS_BL_START_FLASH	0x38040000	/* Address of relocated code */
#define CONFIG_SYS_BL_START_RAM		0x03fd0000	/* Address of relocated code */
#define CONFIG_SYS_MONITOR_BASE		TEXT_BASE
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)    /* Reserve 256 kB for Mon	*/
#define CONFIG_SYS_FLASH_BASE		0x38000000	/* Boot Flash */
#define CONFIG_SYS_FLASH_BASE_1		0x10000000	/* StrataFlash 1 */
#define CONFIG_SYS_FLASH_BASE_2		0x11000000	/* StrataFlash 2 */

/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Erase */
#define CONFIG_SYS_FLASH_WRITE_TOUT	(2*CONFIG_SYS_HZ) /* Timeout for Flash Write */

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

 /*-----------------------------------------------------------------------
  * FLASH configuration
  */
#define CONFIG_FLASH_CFI_DRIVER				/* Use the common driver */
#define CONFIG_FLASH_CFI_LEGACY
#define CONFIG_SYS_FLASH_CFI				/* Flash is CFI conformant */
#define CONFIG_SYS_MAX_FLASH_BANKS	3	/* max number of memory banks */
#define CONFIG_SYS_FLASH_BANKS_LIST	{CONFIG_SYS_FLASH_BASE,   \
					 CONFIG_SYS_FLASH_BASE_1, \
					 CONFIG_SYS_FLASH_BASE_2}
#define CONFIG_SYS_FLASH_EMPTY_INFO
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_SYS_MAX_FLASH_SECT	128	/* max number of sectors on one chip */
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CONFIG_SYS_FLASH_LEGACY_512Kx8

 /*-----------------------------------------------------------------------
  * Environment configuration
  */
#define CONFIG_ENV_IS_IN_FLASH		1
#define CONFIG_ENV_SECT_SIZE		0x20000 /* Total Size of Environment Sector */
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_ADDR			CONFIG_SYS_FLASH_BASE_1
/* Redundant Copy */
#define CONFIG_ENV_ADDR_REDUND		(CONFIG_SYS_FLASH_BASE_1 + \
					 CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SECT_SIZE


 /*-----------------------------------------------------------------------
  * PCI configuration
  */
#define CONFIG_PCI                                /* include pci support */
#define CONFIG_PCI_PNP                            /* pci plug-and-play */
#define CONFIG_SYS_FIRST_PCI_IRQ   10
#define CONFIG_SYS_SECOND_PCI_IRQ  9
#define CONFIG_SYS_THIRD_PCI_IRQ   11
#define CONFIG_SYS_FORTH_PCI_IRQ   15

/*-----------------------------------------------------------------------
 * Hardware watchdog configuration
 */
#define CONFIG_SYS_WATCHDOG_PIO_BIT  		0x8000
#define CONFIG_SYS_WATCHDIG_PIO_DATA 		SC520_PIODATA15_0
#define CONFIG_SYS_WATCHDIG_PIO_CLR  		SC520_PIOCLR15_0
#define CONFIG_SYS_WATCHDIG_PIO_SET  		SC520_PIOSET15_0

/*-----------------------------------------------------------------------
 * FPGA configuration
 */
#define CONFIG_SYS_FPGA_PROGRAM_PIO_BIT		0x2000
#define CONFIG_SYS_FPGA_INIT_PIO_BIT		0x4000
#define CONFIG_SYS_FPGA_DONE_PIO_BIT		0x8000
#define CONFIG_SYS_FPGA_PIO_DATA 		SC520_PIODATA31_16
#define CONFIG_SYS_FPGA_PIO_DIRECTION 		SC520_PIODIR31_16
#define CONFIG_SYS_FPGA_PIO_CLR  		SC520_PIOCLR31_16
#define CONFIG_SYS_FPGA_PIO_SET  		SC520_PIOSET31_16
#define CONFIG_SYS_FPGA_PROGRAM_BIT_DROP_TIME	1	/* milliseconds */
#define CONFIG_SYS_FPGA_MAX_INIT_TIME		10	/* milliseconds */
#define CONFIG_SYS_FPGA_MAX_FINALISE_TIME	10	/* milliseconds */
#define CONFIG_SYS_FPGA_SSI_DATA_RATE		8333	/* kHz (33.3333MHz xtal) */

#ifndef __ASSEMBLER__
extern unsigned long ip;

#define PRINTIP				asm ("call 0\n" \
					    "0:\n" \
					    "pop %%eax\n" \
					    "movl %%eax, %0\n" \
					    :"=r"(ip) \
					    : /* No Input Registers */ \
					    :"%eax"); \
					    printf("IP: 0x%08lx (File: %s, Line: %d)\n", ip, __FILE__, __LINE__);

#endif
#endif	/* __CONFIG_H */

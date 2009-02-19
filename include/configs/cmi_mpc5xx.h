/*
 * (C) Copyright 2003
 * Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation,
 */

/*
 * File:		cmi_mpc5xx.h
 *
 * Discription:		Config header file for cmi
 *			board  using an MPC5xx CPU
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#define CONFIG_MPC555		1		/* This is an MPC555 CPU		*/
#define CONFIG_CMI		1		/* Using the customized cmi board	*/

/* Serial Console Configuration */
#define	CONFIG_5xx_CONS_SCI1
#undef	CONFIG_5xx_CONS_SCI2

#define CONFIG_BAUDRATE		57600


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

#undef	CONFIG_CMD_NET		/* disabeled - causes compile errors */

#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_LOADB
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BDI
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_RUN
#define CONFIG_CMD_IMI


#if 0
#define CONFIG_BOOTDELAY	-1		/* autoboot disabled			*/
#else
#define CONFIG_BOOTDELAY	5		/* autoboot after 5 seconds		*/
#endif
#define CONFIG_BOOTCOMMAND	"go 02034004"	/* autoboot command			*/

#define CONFIG_BOOTARGS		""		/* Assuming OS Image in 4 flash sector at offset 4004 */

#define CONFIG_WATCHDOG				/* turn on platform specific watchdog	*/

#define CONFIG_STATUS_LED	1		/* Enable status led */

#define CONFIG_LOADS_ECHO	1		/* Echo on for serial download */

/*
 * Miscellaneous configurable options
 */

#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#define	CONFIG_SYS_PROMPT		"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16	       /* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00000000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x000fa000	/* 1 MB in SRAM			*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address		*/

#define	CONFIG_SYS_HZ			1000		/* Decrementer freq: 1 ms ticks	*/

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 1250000 }


/*
 * Low Level Configuration Settings
 */

/*
 * Internal Memory Mapped (This is not the IMMR content)
 */
#define CONFIG_SYS_IMMR		0x01000000		/* Physical start adress of internal memory map */

/*
 * Definitions for initial stack pointer and data area
 */
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_IMMR + 0x003f9800)	/* Physical start adress of internal MPC555 writable RAM */
#define	CONFIG_SYS_INIT_RAM_END	(CONFIG_SYS_IMMR + 0x003fffff)	/* Physical end adress of internal MPC555 used RAM area	*/
#define	CONFIG_SYS_GBL_DATA_SIZE	64			/* Size in bytes reserved for initial global data */
#define CONFIG_SYS_GBL_DATA_OFFSET	((CONFIG_SYS_INIT_RAM_END - CONFIG_SYS_INIT_RAM_ADDR) - CONFIG_SYS_GBL_DATA_SIZE) /* Offset from the beginning of ram */
#define	CONFIG_SYS_INIT_SP_ADDR	0x013fa000		/* Physical start adress of inital stack */

/*
 * Start addresses for the final memory configuration
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000	/* Monitor won't change memory map			*/
#define CONFIG_SYS_FLASH_BASE		0x02000000	/* External flash */
#define PLD_BASE		0x03000000	/* PLD  */
#define ANYBUS_BASE		0x03010000	/* Anybus Module */

#define CONFIG_SYS_RESET_ADRESS	0x01000000	/* Adress which causes reset */
#define	CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_FLASH_BASE	/* TEXT_BASE is defined in the board config.mk file.	*/
						/* This adress is given to the linker with -Ttext to	*/
						/* locate the text section at this adress.		*/
#define	CONFIG_SYS_MONITOR_LEN		(192 << 10)	/* Reserve 192 kB for Monitor				*/
#define	CONFIG_SYS_MALLOC_LEN		(64 << 10)	/* Reserve 128 kB for malloc()				*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux		*/


/*-----------------------------------------------------------------------
 * FLASH organization
 *-----------------------------------------------------------------------
 *
 */

#define CONFIG_SYS_MAX_FLASH_BANKS	1		/* Max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	64		/* Max number of sectors on one chip	*/
#define CONFIG_SYS_FLASH_ERASE_TOUT	180000		/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT	600		/* Timeout for Flash Write (in ms)	*/
#define CONFIG_SYS_FLASH_PROTECTION    1		/* Physically section protection on	*/

#define	CONFIG_ENV_IS_IN_FLASH	1

#ifdef	CONFIG_ENV_IS_IN_FLASH
#define CONFIG_ENV_OFFSET		0x00020000	/* Environment starts at this adress	*/
#define	CONFIG_ENV_SIZE		0x00010000	/* Set whole sector as env		*/
#define	CONFIG_SYS_USE_PPCENV				/* Environment embedded in sect .ppcenv */
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE | SYPCR_SWRI| SYPCR_SWP)
#else
#define CONFIG_SYS_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWP)
#endif	/* CONFIG_WATCHDOG */

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CONFIG_SYS_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CONFIG_SYS_PISCR	(PISCR_PITF)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF00
#define CONFIG_SYS_SCCR	(SCCR_TBS     | SCCR_RTDIV    | SCCR_RTSEL    | \
			 SCCR_COM00   | SCCR_DFNL000 | SCCR_DFNH000)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration
 *-----------------------------------------------------------------------
 * Data show cycle
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00)		/* Disable data show cycle	*/

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register
 *-----------------------------------------------------------------------
 * Set all bits to 40 Mhz
 *
 */
#define CONFIG_SYS_OSC_CLK	((uint)4000000)		/* Oscillator clock is 4MHz	*/
#define CONFIG_SYS_PLPRCR	(PLPRCR_MF_9 | PLPRCR_DIVF_0)


/*-----------------------------------------------------------------------
 * UMCR - UIMB Module Configuration Register
 *-----------------------------------------------------------------------
 *
 */
#define CONFIG_SYS_UMCR	(UMCR_FSPEED)		/* IMB clock same as U-bus	*/

/*-----------------------------------------------------------------------
 * ICTRL - I-Bus Support Control Register
 */
#define CONFIG_SYS_ICTRL	(ICTRL_ISCT_SER_7)	/* Take out of serialized mode	*/

/*-----------------------------------------------------------------------
 * USIU - Memory Controller Register
 *-----------------------------------------------------------------------
 */

#define CONFIG_SYS_BR0_PRELIM		(CONFIG_SYS_FLASH_BASE | BR_V | BR_BI | BR_PS_16)
#define CONFIG_SYS_OR0_PRELIM		(OR_ADDR_MK_FF | OR_SCY_3)
#define CONFIG_SYS_BR1_PRELIM		(ANYBUS_BASE)
#define CONFIG_SYS_OR1_PRELIM		(OR_ADDR_MK_FFFF | OR_SCY_1 | OR_ETHR)
#define CONFIG_SYS_BR2_PRELIM		(CONFIG_SYS_SDRAM_BASE | BR_V | BR_PS_32)
#define CONFIG_SYS_OR2_PRELIM		(OR_ADDR_MK_FF)
#define CONFIG_SYS_BR3_PRELIM		(PLD_BASE | BR_V | BR_BI | BR_LBDIR | BR_PS_8)
#define CONFIG_SYS_OR3_PRELIM		(OR_ADDR_MK_FF | OR_TRLX | OR_BSCY | OR_SCY_8 | \
				 OR_ACS_10 | OR_ETHR | OR_CSNT)

#define FLASH_BASE0_PRELIM	CONFIG_SYS_FLASH_BASE	/* We don't realign the flash	*/

/*-----------------------------------------------------------------------
 * DER - Timer Decrementer
 *-----------------------------------------------------------------------
 * Initialise to zero
 */
#define CONFIG_SYS_DER			0x00000000


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01			/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02			/* Software reboot			*/

#endif	/* __CONFIG_H */

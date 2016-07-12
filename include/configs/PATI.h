/*
 * (C) Copyright 2003
 * Denis Peter d.peter@mpl.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * File:		PATI.h
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#define CONFIG_MPC555		1		/* This is an MPC555 CPU		*/
#define CONFIG_PATI		1		/* ...On a PATI board	*/

#define	CONFIG_SYS_TEXT_BASE	0xFFF00000

/* Serial Console Configuration */
#define	CONFIG_5xx_CONS_SCI1
#undef	CONFIG_5xx_CONS_SCI2

#define CONFIG_BAUDRATE		9600

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
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_BSP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_IRQ

#define CONFIG_BOOTCOMMAND	""	/* autoboot command			*/

#define CONFIG_BOOTARGS		""		/* */

#define CONFIG_WATCHDOG				/* turn on platform specific watchdog	*/

/*#define CONFIG_STATUS_LED	1		*/ /* Enable status led */

#define CONFIG_LOADS_ECHO	1		/* Echo on for serial download */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	/* stdin, stdout and stderr are in evironment */
#define CONFIG_PREBOOT

#define	CONFIG_SYS_LONGHELP				/* undef to save memory		*/
#if defined(CONFIG_CMD_KGDB)
#define	CONFIG_SYS_CBSIZE		1024		/* Console I/O Buffer Size	*/
#else
#define	CONFIG_SYS_CBSIZE		256		/* Console I/O Buffer Size	*/
#endif
#define	CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16) /* Print Buffer Size */
#define	CONFIG_SYS_MAXARGS		16	       /* max number of command args	*/
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size	*/

#define CONFIG_SYS_MEMTEST_START	0x00010000	/* memtest works on		*/
#define CONFIG_SYS_MEMTEST_END		0x00A00000	/* 10 MB in SRAM			*/

#define	CONFIG_SYS_LOAD_ADDR		0x100000	/* default load address		*/

#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, 1250000 }

#define CONFIG_BOARD_EARLY_INIT_F

/***********************************************************************
 * Last Stage Init
 ***********************************************************************/
#define CONFIG_LAST_STAGE_INIT

/*
 * Low Level Configuration Settings
 */

/*
 * Internal Memory Mapped (This is not the IMMR content)
 */
#define CONFIG_SYS_IMMR		0x01C00000		/* Physical start adress of internal memory map */

/*
 * Definitions for initial stack pointer and data area
 */
#define CONFIG_SYS_INIT_RAM_ADDR	(CONFIG_SYS_IMMR + 0x003f9800)	/* Physical start adress of internal MPC555 writable RAM */
#define	CONFIG_SYS_INIT_RAM_SIZE	(CONFIG_SYS_IMMR + 0x003fffff)	/* Physical end adress of internal MPC555 used RAM area	*/
#define CONFIG_SYS_GBL_DATA_OFFSET	((CONFIG_SYS_INIT_RAM_SIZE - CONFIG_SYS_INIT_RAM_ADDR) - GENERATED_GBL_DATA_SIZE) /* Offset from the beginning of ram */
#define	CONFIG_SYS_INIT_SP_ADDR	(CONFIG_SYS_IMMR + 0x03fa000)	/* Physical start adress of inital stack */
/*
 * Start addresses for the final memory configuration
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define	CONFIG_SYS_SDRAM_BASE		0x00000000	/* Monitor won't change memory map			*/
#define CONFIG_SYS_FLASH_BASE		0xffC00000	/* External flash */
#define PCI_BASE		0x03000000	/* PCI Base (CS2) */
#define PCI_CONFIG_BASE		0x04000000	/* PCI & PLD  (CS3) */
#define PLD_CONFIG_BASE		0x04001000	/* PLD  (CS3) */

#define	CONFIG_SYS_MONITOR_BASE	0xFFF00000
/* CONFIG_SYS_FLASH_BASE	*/ /* CONFIG_SYS_TEXT_BASE is defined in the board config.mk file.	*/
						/* This adress is given to the linker with -Ttext to	*/
						/* locate the text section at this adress.		*/
#define	CONFIG_SYS_MONITOR_LEN		(256 << 10)	/* Reserve 192 kB for Monitor				*/
#define	CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()				*/

#define CONFIG_SYS_RESET_ADDRESS	(PLD_CONFIG_BASE + 0x10)	 /* Adress which causes reset */

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

#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_SYS_FLASH_EMPTY_INFO

#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER

#define CONFIG_FLASH_SHOW_PROGRESS	45

#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	128

#define	CONFIG_ENV_IS_IN_EEPROM
#ifdef	CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_OFFSET		0
#define CONFIG_ENV_SIZE		2048
#endif

#undef  CONFIG_ENV_IS_IN_FLASH
#ifdef	CONFIG_ENV_IS_IN_FLASH
#define	CONFIG_ENV_SIZE		0x00002000		/* Set whole sector as env		*/
#define CONFIG_ENV_OFFSET		((0 - CONFIG_SYS_FLASH_BASE) - CONFIG_ENV_SIZE)		/* Environment starts at this adress	*/
#endif

#define CONFIG_SPI		1
#define CONFIG_SYS_SPI_CS_USED	0x09 /* CS0 and CS3 are used */
#define CONFIG_SYS_SPI_CS_BASE	0x08 /* CS3 is active low */
#define CONFIG_SYS_SPI_CS_ACT	0x00 /* CS3 is active low */
/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * SW Watchdog freeze
 */
#undef CONFIG_WATCHDOG
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
#define CONFIG_SYS_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF00
#define CONFIG_SYS_SCCR	(SCCR_TBS     | SCCR_RTDIV    | SCCR_RTSEL    | \
			 SCCR_COM01   | SCCR_DFNL000 | SCCR_DFNH000)

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration
 *-----------------------------------------------------------------------
 * Data show cycle
 */
#define CONFIG_SYS_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_EARB | SIUMCR_GPC01 | SIUMCR_MLRC11) /* Disable data show cycle	*/

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
#define CONFIG_SYS_BR0_PRELIM		(CONFIG_SYS_FLASH_BASE | BR_V | BR_BI | BR_PS_16 | BR_SETA)
#define CONFIG_SYS_OR0_PRELIM		(0xffc00000) /* SCY is not used if external TA is set */
/* SDRAM */
#define CONFIG_SYS_BR1_PRELIM		(CONFIG_SYS_SDRAM_BASE | BR_V | BR_BI | BR_PS_32 | BR_SETA)
#define CONFIG_SYS_OR1_PRELIM		(OR_ADDR_MK_FF) /* SCY is not used if external TA is set */
/* PCI */
#define CONFIG_SYS_BR2_PRELIM		(PCI_BASE | BR_V | BR_PS_32 | BR_SETA)
#define CONFIG_SYS_OR2_PRELIM		(OR_ADDR_MK_FF)
/* config registers: */
#define CONFIG_SYS_BR3_PRELIM		(PCI_CONFIG_BASE | BR_V | BR_BI | BR_PS_32 | BR_SETA)
#define CONFIG_SYS_OR3_PRELIM		(0xffff0000)

#define FLASH_BASE0_PRELIM	CONFIG_SYS_FLASH_BASE	/* We don't realign the flash	*/

/*-----------------------------------------------------------------------
 * DER - Timer Decrementer
 *-----------------------------------------------------------------------
 * Initialise to zero
 */
#define CONFIG_SYS_DER			0x00000000

#define VERSION_TAG "released"
#define CONFIG_ISO_STRING "MEV-10084-001"

#define CONFIG_IDENT_STRING "\n(c) 2003 by MPL AG Switzerland, " CONFIG_ISO_STRING " " VERSION_TAG

#endif	/* __CONFIG_H */

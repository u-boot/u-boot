/*
 * U-Boot configuration for SIXNET SXNI855T CPU board.
 * This board is based (loosely) on the Motorola FADS board, so this
 * file is based (loosely) on config_FADS860T.h, see it for additional
 * credits.
 *
 * Copyright (c) 2000-2002 Dave Ellis, SIXNET, dge@sixnetio.com
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/*
 * Memory map:
 *
 *   ff100000 -> ff13ffff : FPGA        CS1
 *   ff030000 -> ff03ffff : EXPANSION   CS7
 *   ff020000 -> ff02ffff : DATA FLASH  CS4
 *   ff018000 -> ff01ffff : UART B      CS6/UPMB
 *   ff010000 -> ff017fff : UART A      CS5/UPMB
 *   ff000000 -> ff00ffff : IMAP                   internal to the MPC855T
 *   f8000000 -> fbffffff : FLASH       CS0        up to 64MB
 *   f4000000 -> f7ffffff : NVSRAM      CS2        up to 64MB
 *   00000000 -> 0fffffff : SDRAM       CS3/UPMA   up to 256MB
 */

/* ------------------------------------------------------------------------- */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#include <mpc8xx_irq.h>

#define CONFIG_SXNI855T		1	/* SIXNET IPm 855T CPU module */

/* The 855T is just a stripped 860T and needs code for 860, so for now
 * at least define 860, 860T and 855T
 */
#define CONFIG_MPC860		1
#define CONFIG_MPC860T		1
#define CONFIG_MPC855T		1

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_SCC1
#undef	CONFIG_8xx_CONS_NONE
#define CONFIG_BAUDRATE		9600
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/

#define MPC8XX_FACT		10	/* 50 MHz is 5 MHz in times 10	*/

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define CONFIG_HAS_ETH1

/*-----------------------------------------------------------------------
 * Definitions for status LED
 */
#define	CONFIG_STATUS_LED	1	/* Status LED enabled		*/

# define STATUS_LED_PAR		im_ioport.iop_papar
# define STATUS_LED_DIR		im_ioport.iop_padir
# define STATUS_LED_ODR		im_ioport.iop_paodr
# define STATUS_LED_DAT		im_ioport.iop_padat

# define STATUS_LED_BIT		0x8000		/* LED 0 is on PA.0 */
# define STATUS_LED_PERIOD	((CFG_HZ / 2) / 5)	/* blink at 5 Hz */
# define STATUS_LED_STATE	STATUS_LED_BLINKING

# define STATUS_LED_ACTIVE	0		/* LED on for bit == 0	*/

# define STATUS_LED_BOOT	0		/* LED 0 used for boot status */

#ifdef DEV	/* development (debug) settings */
#define CONFIG_BOOT_LED_STATE	STATUS_LED_OFF
#else		/* production settings */
#define CONFIG_BOOT_LED_STATE	STATUS_LED_ON
#endif

#define CONFIG_SHOW_BOOT_PROGRESS 1

#define CONFIG_BOOTCOMMAND	"bootm f8040000 f8100000" /* autoboot command */
#define CONFIG_BOOTARGS		"root=/dev/ram ip=off"

#define CONFIG_MISC_INIT_R		/* have misc_init_r() function */
#define CONFIG_BOARD_POSTCLK_INIT	/* have board_postclk_init() function */

#undef	CONFIG_WATCHDOG			/* watchdog disabled		*/

#define	CONFIG_RTC_DS1306		/* Dallas 1306 real time clock	*/

#define	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
/*
 * Software (bit-bang) I2C driver configuration
 */
#define PB_SCL		0x00000020	/* PB 26 */
#define PB_SDA		0x00000010	/* PB 27 */

#define I2C_INIT	(immr->im_cpm.cp_pbdir |=  PB_SCL)
#define I2C_ACTIVE	(immr->im_cpm.cp_pbdir |=  PB_SDA)
#define I2C_TRISTATE	(immr->im_cpm.cp_pbdir &= ~PB_SDA)
#define I2C_READ	((immr->im_cpm.cp_pbdat & PB_SDA) != 0)
#define I2C_SDA(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SDA; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SDA
#define I2C_SCL(bit)	if(bit) immr->im_cpm.cp_pbdat |=  PB_SCL; \
			else    immr->im_cpm.cp_pbdat &= ~PB_SCL
#define I2C_DELAY	udelay(5)	/* 1/4 I2C clock duration */

# define CFG_I2C_SPEED		50000
# define CFG_I2C_SLAVE		0xFE
# define CFG_I2C_EEPROM_ADDR	0x50	/* Atmel 24C64			*/
# define CFG_I2C_EEPROM_ADDR_LEN 2	/* two byte address		*/

#define	CONFIG_FEC_ENET		1	/* use FEC ethernet  */
#define	CONFIG_MII		1

#define CFG_DISCOVER_PHY

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL		| \
				 CFG_CMD_EEPROM		| \
				 CFG_CMD_JFFS2		| \
				 CFG_CMD_NAND		| \
				 CFG_CMD_DATE)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#define CFG_JFFS2_SORT_FRAGMENTS

/*
 * JFFS2 partitions
 *
 */
/* No command line, one static partition */
#undef CONFIG_JFFS2_CMDLINE

/*
#define CONFIG_JFFS2_DEV		"nor0"
#define CONFIG_JFFS2_PART_SIZE		0x00780000
#define CONFIG_JFFS2_PART_OFFSET	0x00080000
*/

#define CONFIG_JFFS2_DEV		"nand0"
#define CONFIG_JFFS2_PART_SIZE		0x00200000
#define CONFIG_JFFS2_PART_OFFSET	0x00000000

/* mtdparts command line support */
/* Note: fake mtd_id used, no linux mtd map file */
/*
#define CONFIG_JFFS2_CMDLINE
#define MTDIDS_DEFAULT		"nor0=sixnet-0,nand0=sixnet-nand"
#define MTDPARTS_DEFAULT	"mtdparts=sixnet-0:7680k@512k();sixnet-nand:2m(jffs2-nand)"
*/

/* NAND flash support */
#define CFG_NAND_LEGACY
#define CONFIG_MTD_NAND_ECC_JFFS2
#define CFG_MAX_NAND_DEVICE	1	/* Max number of NAND devices	*/
#define SECTORSIZE 512

#define ADDR_COLUMN 1
#define ADDR_PAGE 2
#define ADDR_COLUMN_PAGE 3

#define NAND_ChipID_UNKNOWN 	0x00
#define NAND_MAX_FLOORS 1
#define NAND_MAX_CHIPS 1

/* DFBUSY is available on Port C, bit 12; 0 if busy */
#define NAND_WAIT_READY(nand)	\
	while (!(((volatile immap_t *)CFG_IMMR)->im_ioport.iop_pcdat & 0x0008));
#define WRITE_NAND_COMMAND(d, adr) WRITE_NAND((d), (adr))
#define WRITE_NAND_ADDRESS(d, adr) WRITE_NAND((d), (adr))
#define WRITE_NAND(d, adr)	\
	 do { (*(volatile uint8_t *)(adr) = (uint8_t)(d)); } while (0)
#define READ_NAND(adr) (*(volatile uint8_t *)(adr))
#define	CLE_LO	0x01	/* 0 selects CLE mode (CLE high) */
#define	ALE_LO	0x02	/* 0 selects ALE mode (ALE high) */
#define	CE_LO	0x04	/* 1 selects chip (CE low) */
#define	nand_setcr(cr, val) do {*(volatile uint8_t*)(cr) = (val);} while (0)
#define NAND_DISABLE_CE(nand) \
	nand_setcr((nand)->IO_ADDR + 1, ALE_LO | CLE_LO)
#define NAND_ENABLE_CE(nand) \
	nand_setcr((nand)->IO_ADDR + 1, CE_LO | ALE_LO | CLE_LO)
#define NAND_CTL_CLRALE(nandptr) \
	nand_setcr((nandptr) + 1, CE_LO | ALE_LO | CLE_LO)
#define NAND_CTL_SETALE(nandptr) \
	nand_setcr((nandptr) + 1, CE_LO | CLE_LO)
#define NAND_CTL_CLRCLE(nandptr) \
	nand_setcr((nandptr) + 1, CE_LO | ALE_LO | CLE_LO)
#define NAND_CTL_SETCLE(nandptr) \
	nand_setcr((nandptr) + 1, CE_LO | ALE_LO)

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save a little memory */
#define	CFG_PROMPT		"=>"	/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0100000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0400000	/* 1 ... 4 MB in DRAM	*/

#define CFG_LOAD_ADDR	 	0x00100000

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xFF000000
#define CFG_IMMR_SIZE		((uint)(64 * 1024))

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define	CFG_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define	CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define	CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define	CFG_SDRAM_BASE		0x00000000
#define	CFG_SRAM_BASE		0xF4000000
#define	CFG_SRAM_SIZE		0x04000000	/* autosize up to 64Mbyte */

#define CFG_FLASH_BASE		0xF8000000
#define CFG_FLASH_SIZE		((uint)(8 * 1024 * 1024))	/* max 8Mbyte */

#define CFG_DFLASH_BASE		0xff020000 /* DiskOnChip or NAND FLASH */
#define CFG_DFLASH_SIZE		0x00010000

#define CFG_FPGA_BASE		0xFF100000	/* Xilinx FPGA */
#define CFG_FPGA_PROG		0xFF130000	/* Programming address */
#define CFG_FPGA_SIZE		0x00040000	/* 256KiB usable */

#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define	CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
/* Intel 28F640 has 135, 127 64K sectors in 8MB, + 8 more for 8K boot blocks.
 * AMD 29LV641 has 128 64K sectors in 8MB
 */
#define CFG_MAX_FLASH_SECT	135	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control					11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration					11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control					11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register	15-30
 *-----------------------------------------------------------------------
 * set the PLL, the low-power modes and the reset control (15-29)
 */
#define CFG_PLPRCR	(((MPC8XX_FACT-1) << PLPRCR_MF_SHIFT) |	\
				PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CFG_SCCR	(SCCR_TBS|SCCR_COM00|SCCR_DFSYNC00|SCCR_DFBRG00|SCCR_DFNL000|SCCR_DFNH000|SCCR_DFLCD000|SCCR_DFALCD00)

 /*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
#define CFG_DER		0

/* Because of the way the 860 starts up and assigns CS0 the
 * entire address space, we have to set the memory controller
 * differently.  Normally, you write the option register
 * first, and then enable the chip select by writing the
 * base register.  For CS0, you must write the base register
 * first, followed by the option register.
 */

/*
 * Init Memory Controller:
 *
 **********************************************************
 * BR0 and OR0 (FLASH)
 */

#define CFG_PRELIM_OR0_AM	0xFC000000	/* OR addr mask */

/* FLASH timing: ACS = 10, TRLX = 1, CSNT = 1, SCY = 3, EHTR = 0	*/
#define CFG_OR_TIMING_FLASH	(OR_CSNT_SAM  | OR_ACS_DIV4 | OR_BI | OR_SCY_3_CLK | OR_TRLX)

#define CFG_OR0_PRELIM	(CFG_PRELIM_OR0_AM | CFG_OR_TIMING_FLASH)

#define CONFIG_FLASH_16BIT
#define CFG_BR0_PRELIM	((CFG_FLASH_BASE & BR_BA_MSK) | BR_PS_16 | BR_V )
#define CFG_FLASH_PROTECTION	/* need to lock/unlock sectors in hardware */

/**********************************************************
 * BR1 and OR1 (FPGA)
 * These preliminary values are also the final values.
 */
#define CFG_OR_TIMING_FPGA \
	(OR_CSNT_SAM | OR_ACS_DIV2 | OR_BI | OR_SCY_4_CLK | OR_EHTR | OR_TRLX)
#define CFG_BR1_PRELIM	((CFG_FPGA_BASE & BR_BA_MSK) | BR_PS_8 | BR_V )
#define CFG_OR1_PRELIM	(((-CFG_FPGA_SIZE) & OR_AM_MSK) | CFG_OR_TIMING_FPGA)

/**********************************************************
 * BR4 and OR4 (data flash)
 * These preliminary values are also the final values.
 */
#define CFG_OR_TIMING_DFLASH \
	(OR_CSNT_SAM | OR_ACS_DIV4 | OR_BI | OR_SCY_2_CLK | OR_EHTR | OR_TRLX)
#define CFG_BR4_PRELIM	((CFG_DFLASH_BASE & BR_BA_MSK) | BR_PS_8 | BR_V )
#define CFG_OR4_PRELIM	(((-CFG_DFLASH_SIZE) & OR_AM_MSK) | CFG_OR_TIMING_DFLASH)

/**********************************************************
 * BR5/6 and OR5/6 (Dual UART)
 */
#define CFG_DUART_SIZE	0x8000	/* 32K window, only uses 8 bytes */
#define CFG_DUARTA_BASE	0xff010000
#define CFG_DUARTB_BASE	0xff018000

#define DUART_MBMR	0
#define DUART_OR_VALUE (ORMASK(CFG_DUART_SIZE) | OR_G5LS| OR_BI)
#define DUART_BR_VALUE (BR_MS_UPMB | BR_PS_8 | BR_V)
#define DUART_BR5_VALUE ((CFG_DUARTA_BASE & BR_BA_MSK ) | DUART_BR_VALUE)
#define DUART_BR6_VALUE ((CFG_DUARTB_BASE & BR_BA_MSK ) | DUART_BR_VALUE)

/**********************************************************
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#define CONFIG_RESET_ON_PANIC		/* reset if system panic() */

#define CFG_ENV_IS_IN_FLASH
#ifdef CFG_ENV_IS_IN_FLASH
  /* environment is in FLASH */
  #define CFG_ENV_ADDR		0xF8040000	/* AM29LV641 or AM29LV800BT */
  #define CFG_ENV_ADDR_REDUND	0xF8050000	/* AM29LV641 or AM29LV800BT */
  #define CFG_ENV_SECT_SIZE	0x00010000
  #define CFG_ENV_SIZE		0x00002000
#else
  /* environment is in EEPROM */
  #define CFG_ENV_IS_IN_EEPROM		1
  #define CFG_ENV_OFFSET		0	/* at beginning of EEPROM */
  #define CFG_ENV_SIZE		     1024	/* Use only a part of it*/
#endif

#if 1
#define CONFIG_AUTOBOOT_KEYED		/* use key strings to stop autoboot */
#define CONFIG_AUTOBOOT_PROMPT		"autoboot in %d seconds\n"
#define CONFIG_AUTOBOOT_DELAY_STR	"delayabit"
#define CONFIG_AUTOBOOT_STOP_STR	" " /* easy to stop for now */
#endif

#endif	/* __CONFIG_H */

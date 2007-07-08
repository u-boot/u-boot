/*
 * (C) Copyright 2003 Picture Elements, Inc.
 * Stephen Williams <steve@icarus.com>
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
 * High Level Configuration Options for the JSE board
 * (Theoretically easy to change, but the board is fixed.)
 */

#define CONFIG_JSE 1
  /* JSE has a PPC405GPr */
#define CONFIG_405GP 1
  /* ... which is a 4xxx series */
#define CONFIG_4xx   1
  /* ... with a 33MHz OSC. connected to the SysCLK input */
#define CONFIG_SYS_CLK_FREQ	33333333
  /* ... with on-chip memory here (4KBytes) */
#define CFG_OCM_DATA_ADDR 0xF4000000
#define CFG_OCM_DATA_SIZE 0x00001000
  /* Do not set up locked dcache as init ram. */
#undef CFG_INIT_DCACHE_CS

  /* Map the SystemACE chip (CS#1) here. (Must be a multiple of 1Meg) */
#define CONFIG_SYSTEMACE 1
#define CFG_SYSTEMACE_BASE 0xf0000000
#define CFG_SYSTEMACE_WIDTH 8
#define CONFIG_DOS_PARTITION 1

  /* Use the On-Chip-Memory (OCM) as a temporary stack for the startup code. */
#define CFG_TEMP_STACK_OCM 1
  /* ... place INIT RAM in the OCM address */
# define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR
  /* ... give it the whole init ram */
# define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE
  /* ... Shave a bit off the end for global data */
# define CFG_GBL_DATA_SIZE	128
# define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
  /* ... and place the stack pointer at the top of what's left. */
# define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

  /* Enable board_pre_init function */
#define CONFIG_BOARD_PRE_INIT	1
#define CONFIG_BOARD_EARLY_INIT_F 1
  /* Disable post-clk setup init function */
#undef CONFIG_BOARD_POSTCLK_INIT
  /* Disable call to post_init_f: late init function. */
#undef CONFIG_POST
  /* Enable DRAM test. */
#define CFG_DRAM_TEST 1
  /* Enable misc_init_r function. */
#define CONFIG_MISC_INIT_R 1

  /* JSE has EEPROM chips that are good for environment. */
#undef	CFG_ENV_IS_IN_NVRAM
#undef	CFG_ENV_IS_IN_FLASH
#define CFG_ENV_IS_IN_EEPROM 1
#undef	CFG_ENV_IS_NOWHERE

  /* This is the 7bit address of the device, not including P. */
#define CFG_I2C_EEPROM_ADDR 0x50
  /* After the device address, need one more address byte. */
#define CFG_I2C_EEPROM_ADDR_LEN 1
  /* The EEPROM is 512 bytes. */
#define CFG_EEPROM_SIZE 512
  /* The EEPROM can do 16byte ( 1 << 4 ) page writes. */
#define CFG_EEPROM_PAGE_WRITE_BITS 4
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS 10
  /* Put the environment in the second half. */
#define CFG_ENV_OFFSET	0x00
#define CFG_ENV_SIZE	512


  /* The JSE connects UART1 to the console tap connector. */
#define CONFIG_UART1_CONSOLE	1
  /* Set console baudrate to 9600 */
#define CONFIG_BAUDRATE		9600

/* Size (bytes) of interrupt driven serial port buffer.
 * Set to 0 to use polling instead of interrupts.
 * Setting to 0 will also disable RTS/CTS handshaking.
 */
#undef CONFIG_SERIAL_SOFTWARE_FIFO

/*
 * Configuration related to auto-boot.
 *
 * CONFIG_BOOTDELAY sets the delay (in seconds) that U-Boot will wait
 * before resorting to autoboot. This value can be overridden by the
 * bootdelay environment variable.
 *
 * CONFIG_AUTOBOOT_PROMPT is the string that U-Boot emits to warn the
 * user that an autoboot will happen.
 *
 * CONFIG_BOOTCOMMAND is the sequence of commands that U-Boot will
 * execute to boot the JSE. This loads the uimage and initrd.img files
 * from CompactFlash into memory, then boots them from memory.
 *
 * CONFIG_BOOTARGS is the arguments passed to the Linux kernel to get
 * it going on the JSE.
 */
#define CONFIG_BOOTDELAY	5
#define CONFIG_BOOTARGS		"root=/dev/ram0 init=/linuxrc rw"
#define CONFIG_BOOTCOMMAND	"fatload ace 0 2000000 uimage; fatload ace 0 2100000 initrd.img; bootm 2000000 2100000"


#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		1	/* PHY address			*/


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_ELF
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_IRQ
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_CMD_PCI
#define CONFIG_CMD_PING


  /* watchdog disabled */
#undef CONFIG_WATCHDOG
  /* SPD EEPROM (sdram speed config) disabled */
#undef CONFIG_SPD_EEPROM
#undef SPD_EEPROM_ADDRESS

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/

#define CFG_HUSH_PARSER			/* use "hush" command parser	*/
#ifdef	CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2	"> "
#endif

#if defined(CONFIG_CMD_KGDB)
#define CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

/*
 * If CFG_EXT_SERIAL_CLOCK, then the UART divisor is 1.
 * If CFG_405_UART_ERRATA_59, then UART divisor is 31.
 * Otherwise, UART divisor is determined by CPU Clock and CFG_BASE_BAUD value.
 * The Linux BASE_BAUD define should match this configuration.
 *    baseBaud = cpuClock/(uartDivisor*16)
 * If CFG_405_UART_ERRATA_59 and 200MHz CPU clock,
 * set Linux BASE_BAUD to 403200.
 */
#undef	CFG_EXT_SERIAL_CLOCK	       /* external serial clock */
#undef	CFG_405_UART_ERRATA_59	       /* 405GP/CR Rev. D silicon */
#define CFG_BASE_BAUD	    691200

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CFG_LOAD_ADDR		0x100000	/* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define CFG_HZ		1000		/* decrementer freq: 1 ms ticks */

#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#undef	CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F


/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#define PCI_HOST_ADAPTER 0		/* configure ar pci adapter	*/
#define PCI_HOST_FORCE	1		/* configure as pci host	*/
#define PCI_HOST_AUTO	2		/* detected via arbiter enable	*/

#define CONFIG_PCI			/* include pci support		*/
#define CONFIG_PCI_HOST PCI_HOST_FORCE	/* select pci host function	*/
#undef	CONFIG_PCI_PNP			/* do pci plug-and-play		*/
					/* resource configuration	*/

#define CFG_PCI_SUBSYS_VENDORID 0x0000	/* PCI Vendor ID: to-do!!!	*/
#define CFG_PCI_SUBSYS_DEVICEID 0x0000	/* PCI Device ID: to-do!!!	*/
#define CFG_PCI_PTM1LA	0x00000000	/* point to sdram		*/
#define CFG_PCI_PTM1MS	0x80000001	/* 2GB, enable hard-wired to 1	*/
#define CFG_PCI_PTM1PCI 0x00000000	/* Host: use this pci address	*/
#define CFG_PCI_PTM2LA	0x00000000	/* disabled			*/
#define CFG_PCI_PTM2MS	0x00000000	/* disabled			*/
#define CFG_PCI_PTM2PCI 0x04000000	/* Host: use this pci address	*/

/*-----------------------------------------------------------------------
 * External peripheral base address
 *-----------------------------------------------------------------------
 */
#undef	CONFIG_IDE_LED			/* no led for ide supported	*/
#undef	CONFIG_IDE_RESET		/* no reset for ide supported	*/

#define CFG_KEY_REG_BASE_ADDR	0xF0100000
#define CFG_IR_REG_BASE_ADDR	0xF0200000
#define CFG_FPGA_REG_BASE_ADDR	0xF0300000

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFFF80000
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define CFG_MONITOR_LEN		(256 * 1024)	/* Reserve 256 kB for Monitor	*/
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
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		16384	/* For AMCC 405GPr CPUs	*/
#define CFG_CACHELINE_SIZE	32	/* ...			*/
#if defined(CONFIG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value */
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


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#if defined(CONFIG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif
#endif	/* __CONFIG_H */

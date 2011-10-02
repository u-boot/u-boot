/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * David Updegraff, Cray, Inc.  dave@cray.com: our 405 is walnut-lite..
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

#define CONFIG_CRAYL1
/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_405GP		1	/* This is a PPC405 CPU	*/
#define CONFIG_4xx		    1   /* ...member of PPC405 family */

/*
 * Note: I make an "image" from U-Boot itself, which prefixes 0x40
 * bytes of header info, hence start address is thus shifted.
 */
#define	CONFIG_SYS_TEXT_BASE	0xFFFD0040

#define CONFIG_SYS_CLK_FREQ 25000000
#define CONFIG_BAUDRATE		9600
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/

#define CONFIG_PPC4xx_EMAC
#define CONFIG_MII		    1	/* MII PHY management */
#define	CONFIG_PHY_ADDR		1	/* PHY address; handling of ENET */
#define CONFIG_BOARD_EARLY_INIT_F 1	/* early setup for 405gp */
#define CONFIG_MISC_INIT_R	1	/* so that a misc_init_r() is called */

#define CONFIG_CONS_INDEX	1	/* Use UART0			*/
#define CONFIG_SYS_NS16550
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

/* set PRAM to keep U-Boot out, mem= to keep linux out, and initrd_hi to
 * keep possible initrd ramdisk decompression out.  This is in k (1024 bytes)
 #define CONFIG_PRAM			16
 */
#define	CONFIG_LOADADDR		0x100000	/* where TFTP images go */
#undef CONFIG_BOOTARGS

/* Bootcmd is overridden by the bootscript in board/cray/L1
 */
#define	CONFIG_SYS_AUTOLOAD		"no"
#define CONFIG_BOOTCOMMAND	"dhcp"

/*
 * ..during experiments..
 #define CONFIG_SERVERIP         10.0.0.1
 #define CONFIG_ETHADDR          00:40:a6:80:14:5
 */
#define CONFIG_HARD_I2C         1		/* hardware support for i2c */
#define CONFIG_PPC4XX_I2C		/* use PPC4xx driver		*/
#define CONFIG_SDRAM_BANK0		1
#define CONFIG_SYS_I2C_SPEED		    400000	/* I2C speed and slave address	*/
#define CONFIG_SYS_I2C_SLAVE		    0x7F
#define CONFIG_SYS_I2C_EEPROM_ADDR     0x57
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_IDENT_STRING     "Cray L1"
#define CONFIG_ENV_OVERWRITE     1
#define	CONFIG_SYS_HZ		             1000	/* decrementer freq: 1 ms ticks	*/
#define CONFIG_SYS_HUSH_PARSER			1
#define CONFIG_SYS_PROMPT_HUSH_PS2		"> "
#define CONFIG_SOURCE			1


/*
 * Command line configuration.
 */

#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_BDI
#define CONFIG_CMD_CONSOLE
#define CONFIG_CMD_DATE
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_EEPROM
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_I2C
#define CONFIG_CMD_IMI
#define CONFIG_CMD_IMMAP
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_NET
#define CONFIG_CMD_REGINFO
#define CONFIG_CMD_RUN
#define CONFIG_CMD_SAVEENV
#define CONFIG_CMD_SETGETDCR
#define CONFIG_CMD_SOURCE


/*
 * BOOTP options
 */
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_VENDOREX
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_BOOTFILESIZE


/*
 * how many time to fail & restart a net-TFTP before giving up & resetting
 * the board hoping that a reset of net interface might help..
 */
#define CONFIG_NET_RESET 5

/*
 * bauds.  Just to make it compile; in our case, I read the base_baud
 * from the DCR anyway, so its kinda-tied to the above ref. clock which in turn
 * drives the system clock.
 */
#define CONFIG_SYS_BASE_BAUD       403225
#define CONFIG_SYS_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_PROMPT	"=> "				/* Monitor Command Prompt	*/
#define	CONFIG_SYS_CBSIZE	256				/* Console I/O Buffer Size	*/
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE			/* Boot Argument Buffer Size	*/
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)	/* Print Buffer Size */
#define CONFIG_SYS_MAXARGS	16				/* max number of command args	*/


#define CONFIG_SYS_LOAD_ADDR		0x100000	/* where to load what we get from TFTP */
#define CONFIG_SYS_TFTP_LOADADDR	CONFIG_SYS_LOAD_ADDR
#define CONFIG_SYS_EXTBDINFO		1		/* To use extended board_into (bd_t) */
#define CONFIG_SYS_DRAM_TEST		1

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CONFIG_SYS_SDRAM_BASE _must_ start at 0
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000
#define CONFIG_SYS_FLASH_BASE		0xFFC00000
#define CONFIG_SYS_MONITOR_BASE	CONFIG_SYS_TEXT_BASE


#define CONFIG_SYS_MONITOR_LEN		(192 * 1024)	/* Reserve 192 kB for Monitor	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CONFIG_SYS_MAX_FLASH_BANKS	 1		/* max number of memory banks		*/
#define CONFIG_SYS_MAX_FLASH_SECT	 64		/* max number of sectors on one chip	*/
#define CONFIG_SYS_FLASH_ERASE_TOUT 120000	/* Timeout for Flash Erase (in ms)	*/
#define CONFIG_SYS_FLASH_WRITE_TOUT 500	/* Timeout for Flash Write (in ms)	*/

/* BEG ENVIRONNEMENT FLASH: needs to be a whole FlashSector  */
#define CONFIG_ENV_OFFSET		0x3c8000
#define CONFIG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars */
#define	CONFIG_ENV_SIZE		0x1000	 /* Total Size of Environment area	*/
#define CONFIG_ENV_SECT_SIZE	0x10000	 /* see README - env sector total size	*/

/* Memory tests: U-BOOT relocates itself to the top of Ram, so its at
 * 32meg-(128k+some_malloc_space+copy-of-ENV sector)..
 */
#define CONFIG_SYS_SDRAM_SIZE		32		/* megs of ram */
#define CONFIG_SYS_MEMTEST_START	0x2000  /* memtest works from the end of */
									/* the exception vector table */
									/* to the end of the DRAM  */
									/* less monitor and malloc area */
#define CONFIG_SYS_STACK_USAGE		0x10000 /* Reserve 64k for the stack usage */
#define CONFIG_SYS_MALLOC_LEN		(128 << 10)	/* 128k for malloc space */
#define CONFIG_SYS_MEM_END_USAGE	( CONFIG_SYS_MONITOR_LEN \
				+ CONFIG_SYS_MALLOC_LEN \
				+ CONFIG_ENV_SECT_SIZE \
				+ CONFIG_SYS_STACK_USAGE )

#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_SIZE * 1024 * 1024 - CONFIG_SYS_MEM_END_USAGE)
/* END ENVIRONNEMENT FLASH */

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	CONFIG_SYS_FLASH_BASE	/* FLASH bank #0	*/


/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in OnChipMem )
 */
#if 1
/* On Chip Memory location */
#define CONFIG_SYS_TEMP_STACK_OCM	1
#define CONFIG_SYS_OCM_DATA_ADDR	0xF0000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000

#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR /* inside of SDRAM		*/
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE /* Size of used area in RAM	*/
#define CONFIG_SYS_GBL_DATA_OFFSET    (CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET      CONFIG_SYS_GBL_DATA_OFFSET
#else
#define CONFIG_SYS_OCM_DATA_ADDR	0xF0000000
#define CONFIG_SYS_OCM_DATA_SIZE	0x1000
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_OCM_DATA_ADDR	/* inside of On Chip SRAM    */
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_OCM_DATA_SIZE	/* Size of On Chip SRAM	     */
#define CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_OFFSET	CONFIG_SYS_GBL_DATA_OFFSET
#endif

/*-----------------------------------------------------------------------
 * Definitions for Serial Presence Detect EEPROM address
 */
#define EEPROM_WRITE_ADDRESS 0xA0
#define EEPROM_READ_ADDRESS  0xA1

#endif	/* __CONFIG_H */

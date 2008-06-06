/*
 * (C) Copyright 2000-2004
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_405GP		1	/* This is a PPC405GP CPU	*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family   */
#define CONFIG_EXBITGEN		1	/* on a Exbit Generic board     */

#define CONFIG_BOARD_EARLY_INIT_F 1	/* Call board_early_init_f	*/

#define CONFIG_SYS_CLK_FREQ     25000000 /* external frequency to pll   */

/* I2C configuration */
#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#define CFG_I2C_SPEED		40000	/* I2C speed			*/
#define CFG_I2C_SLAVE		0x7F	/* I2C slave address		*/

/* environment is in EEPROM */
#define CFG_ENV_IS_IN_EEPROM    1
#undef CFG_ENV_IS_IN_FLASH
#undef CFG_ENV_IS_IN_NVRAM

#ifdef CFG_ENV_IS_IN_EEPROM
#define CFG_I2C_EEPROM_ADDR		0x56    /* 1010110 */
#define CFG_I2C_EEPROM_ADDR_LEN		1	/* 8-bit internal addressing */
#define CFG_I2C_EEPROM_ADDR_OVERFLOW	1	/* ... and 1 bit in I2C address */
#define CFG_EEPROM_PAGE_WRITE_BITS	3	/* 4 bytes per page */
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	40	/* write takes up to 40 msec */
#define CFG_ENV_OFFSET		4	/* Offset of Environment Sector	*/
#define	CFG_ENV_SIZE		350	/* that is 350 bytes only!	*/
#endif

#define CONFIG_BOOTDELAY	10	/* autoboot after 10 seconds	*/
/* Explanation:
   autbooting is altogether disabled and cannot be
   enabled if CONFIG_BOOTDELAY is negative.
   If you want shorter bootdelay, then
   - "setenv bootdelay <delay>" to the proper value
*/

#define CONFIG_BOOTCOMMAND	"bootm 20400000 20800000"

#define CONFIG_BOOTARGS		"root=/dev/ram "  \
				"ramdisk_size=32768 " \
				"console=ttyS0,115200 " \
				"ram=128M debug"

#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_MII		1	/* MII PHY management		*/
#define CONFIG_PHY_ADDR		0	/* PHY address			*/


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


#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if defined(CONFIG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

/* UART configuration */
#define CFG_BASE_BAUD		691200

/* Default baud rate */
#define CONFIG_BAUDRATE		115200

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE      \
	{ 300, 600, 1200, 2400, 4800, 9600, 19200, 38400,     \
	 57600, 115200, 230400, 460800, 921600 }

#define CFG_CLKS_IN_HZ		1	/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		0x100000	/* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define	CFG_HZ			1000	/* decrementer freq: 1 ms ticks	*/

/*-----------------------------------------------------------------------
 * PCI stuff
 *-----------------------------------------------------------------------
 */
#undef CONFIG_PCI			/* no pci support	        */

/*-----------------------------------------------------------------------
 * External peripheral base address
 *-----------------------------------------------------------------------
 */
#undef  CONFIG_IDE_PCMCIA               /* no pcmcia interface required */
#undef  CONFIG_IDE_LED                  /* no led for ide supported     */
#undef  CONFIG_IDE_RESET                /* no reset for ide supported   */

#define	CFG_KEY_REG_BASE_ADDR	0xF0100000
#define	CFG_IR_REG_BASE_ADDR	0xF0200000
#define	CFG_FPGA_REG_BASE_ADDR	0xF0300000

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH0_BASE		0xFFF80000
#define CFG_FLASH0_SIZE		0x00080000
#define CFG_FLASH1_BASE		0x20000000
#define CFG_FLASH1_SIZE		0x02000000
#define CFG_FLASH_BASE		CFG_FLASH0_BASE
#define CFG_FLASH_SIZE		CFG_FLASH0_SIZE
#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 * 1024)	/* Reserve 196 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/

#if CFG_MONITOR_BASE < CFG_FLASH0_BASE
#define CFG_RAMSTART
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	5	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	128	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#ifdef CFG_ENV_IS_IN_FLASH
#define CFG_ENV_OFFSET		0x00060000 /* Offset of Environment Sector      */
#define	CFG_ENV_SIZE		0x00010000 /* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x00010000 /* see README - env sector total size */
#endif

/* On Chip Memory location/size */
#define CFG_OCM_DATA_ADDR	0xF8000000
#define CFG_OCM_DATA_SIZE	0x1000

/* Global info and initial stack */
#define CFG_INIT_RAM_ADDR	CFG_OCM_DATA_ADDR /* inside of on-chip SRAM	*/
#define CFG_INIT_RAM_END	CFG_OCM_DATA_SIZE /* End of used area in RAM	*/
#define CFG_GBL_DATA_SIZE	128 /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

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

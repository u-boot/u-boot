/*
 * Rick Bronson <rick@efn.org>
 *
 * Configuation settings for the AT91RM9200DK board.
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
 */

/*
 * Adatped for KwikByte KB920x board from at91rm9200dk.h: 22APR2005
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* ARM asynchronous clock */
#define AT91C_MAIN_CLOCK	180000000	/* from 10 MHz crystal */
#define AT91C_MASTER_CLOCK	60000000	/* peripheral clock (AT91C_MASTER_CLOCK / 3) */

#define AT91_SLOW_CLOCK		32768	/* slow clock */

#define CONFIG_ARM920T		1	/* This is an ARM920T Core	*/
#define CONFIG_AT91RM9200	1	/* It's an Atmel AT91RM9200 SoC	*/
/* Only define one of the following, based on board type		*/
/* #define	CONFIG_KB9200		1	 KwikByte KB9202 board	*/
/* #define	CONFIG_KB9201		1	 KwikByte KB9202 board	*/
#define	CONFIG_KB9202		1	/* KwikByte KB9202 board	*/

#define	CONFIG_KB920x		1	/* Any KB920x board		*/
#undef  CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/
#define USE_920T_MMU		1

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

#define	CONFIG_SKIP_LOWLEVEL_INIT

#define	CFG_LONGHELP

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

#define CONFIG_BAUDRATE 115200

/*
 * Hardware drivers
 */

/* define one of these to choose the DBGU, USART0  or USART1 as console */
#define CONFIG_DBGU
#undef CONFIG_USART0
#undef CONFIG_USART1

#undef	CONFIG_HWFLOW			/* don't include RTS/CTS flow control support	*/

#undef	CONFIG_MODEM_SUPPORT		/* disable modem initialization stuff */

#define CONFIG_BOOTDELAY	3
#define CONFIG_ENV_OVERWRITE	1


/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_I2C
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP

#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_MISC


#define CONFIG_NR_DRAM_BANKS 1
#define PHYS_SDRAM 0x20000000
#define PHYS_SDRAM_SIZE 0x2000000  /* 32 megs */

#define CFG_MEMTEST_START		PHYS_SDRAM
#define CFG_MEMTEST_END			CFG_MEMTEST_START + PHYS_SDRAM_SIZE - (512*1024)

#define CONFIG_DRIVER_ETHER
#define CONFIG_NET_RETRY_COUNT		20

#define CFG_FLASH_BASE			0x10000000

#ifdef CONFIG_KB9202
#define PHYS_FLASH_SIZE			0x1000000
#else
#define PHYS_FLASH_SIZE			0x200000
#endif

#define CFG_MAX_FLASH_BANKS		1
#define CFG_MAX_FLASH_SECT		256

#define	CONFIG_HARD_I2C

#define	CFG_ENV_IS_IN_EEPROM

#ifdef CONFIG_KB9202
#define CFG_ENV_OFFSET			0x3E00
#define CFG_ENV_SIZE			0x0200
#else
#define CFG_ENV_OFFSET			0x1000
#define CFG_ENV_SIZE			0x1000
#endif
#define	CFG_I2C_EEPROM_ADDR		0x50
#define	CFG_EEPROM_PAGE_WRITE_BITS	6
#define	CFG_I2C_EEPROM_ADDR_LEN		2
#define	CFG_I2C_SPEED			50000
#define	CFG_I2C_SLAVE			0 /* not used */
#define	CFG_EEPROM_PAGE_WRITE_DELAY_MS	10

#define CFG_LOAD_ADDR		0x21000000  /* default load address */

#define CFG_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }

#define CFG_PROMPT		"U-Boot> "	/* Monitor Command Prompt */
#define CFG_CBSIZE		256		/* Console I/O Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */

#define	CFG_FLASH_CFI_DRIVER
#define	CFG_FLASH_CFI

#ifndef __ASSEMBLY__
/*-----------------------------------------------------------------------
 * Board specific extension for bd_info
 *
 * This structure is embedded in the global bd_info (bd_t) structure
 * and can be used by the board specific code (eg board/...)
 */

struct bd_info_ext {
	/* helper variable for board environment handling
	 *
	 * env_crc_valid == 0    =>   uninitialised
	 * env_crc_valid  > 0    =>   environment crc in flash is valid
	 * env_crc_valid  < 0    =>   environment crc in flash is invalid
	 */
	int env_crc_valid;
};
#endif

#define CFG_HZ 1000
#define CFG_HZ_CLOCK AT91C_MASTER_CLOCK/2	/* AT91C_TC0_CMR is implicitly set to */
					/* AT91C_TC_TIMER_DIV1_CLOCK */

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif

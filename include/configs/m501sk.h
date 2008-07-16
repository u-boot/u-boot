/*
 * Based on Modifications by Alan Lu / Artila and
 * Rick Bronson <rick@efn.org>
 *
 * Configuration settings for the Artila M-501 starter kit,
 * with V02 processor card.
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

#ifndef __CONFIG_H
#define __CONFIG_H

/* ARM asynchronous clock */
/* from 18.432 MHz crystal (18432000 / 4 * 39) */
#define AT91C_MAIN_CLOCK	179712000
/* Perip clock (AT91C_MASTER_CLOCK / 3) */
#define AT91C_MASTER_CLOCK	59904000
#define AT91_SLOW_CLOCK	32768 /* slow clock */

#define CONFIG_AT91RM9200DK	1 /* on an AT91RM9200DK Board    */
#undef CONFIG_USE_IRQ /* we don't need IRQ/FIQ stuff */
#define CONFIG_CMDLINE_TAG	1 /* enable passing of ATAGs    */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG		1

#define CONFIG_MENUPROMPT		"."

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* Bytes reserved for initial data */

#define CONFIG_BAUDRATE			115200

/* Hardcode so no __divsi3 : AT91C_MASTER_CLOCK / baudrate / 16 */
#define CFG_AT91C_BRGR_DIVISOR	33

/*
 * Hardware drivers
 */
#define CFG_FLASH_CFI		1
#define CFG_FLASH_CFI_DRIVER	1
#define CFG_ENV_SECT_SIZE	0x20000
#define CFG_FLASH_USE_BUFFER_WRITE
#define CFG_FLASH_PROTECTION	/*for Intel P30 Flash*/
#define CONFIG_HARD_I2C
#define CFG_I2C_SPEED		100
#define CFG_I2C_SLAVE		0
#define CFG_CONSOLE_INFO_QUIET
#undef CFG_ENV_IS_IN_EEPROM
#define CFG_I2C_EEPROM_ADDR	0x50
#define CFG_I2C_EEPROM_ADDR_LEN	1
#define CFG_EEPROM_AT24C16
#define CFG_I2C_RTC_ADDR		0x32
#undef CONFIG_RTC_DS1338
#define CONFIG_RTC_RS5C372A
#undef CONFIG_POST
#define CONFIG_M501SK
#define CONFIG_CMC_PU2

/* define one of these to choose the DBGU, USART0  or USART1 as console */
#define CONFIG_DBGU
#undef CONFIG_USART0
#undef CONFIG_USART1

#undef CONFIG_HWFLOW		/* don't include RTS/CTS flow control support */
#undef CONFIG_MODEM_SUPPORT	/* disable modem initialization stuff */

#define CONFIG_BOOTARGS	"mem=32M console=ttyS0,115200 "	\
			"initrd=0x20800000,8192000 ramdisk_size=15360 "	\
			"root=/dev/ram0 rw mtdparts=phys_mapped_flash:"	\
			"128k(loader)ro,128k(reserved)ro,1408k(linux)"	\
			"ro,2560k(ramdisk)ro,-(userdisk)"
#define CONFIG_BOOTCOMMAND	"bootm 10040000 101a0000"
#define CONFIG_BOOTDELAY	1
#define CONFIG_BAUDRATE	115200
#define CONFIG_IPADDR		192.168.1.100
#define CONFIG_SERVERIP	192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.254
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_BOOTFILE	uImage
#define CONFIG_ETHADDR		00:13:48:aa:bb:cc
#define CONFIG_ENV_OVERWRITE	1
#define BOARD_LATE_INIT

#define	CONFIG_EXTRA_ENV_SETTINGS \
		"unlock=yes\0"

#define CONFIG_CMD_JFFS2
#undef CONFIG_CMD_EEPROM
#define CONFIG_CMD_NET
#define CONFIG_CMD_RUN
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MEMORY
#define CONFIG_CMD_PING
#define CONFIG_CMD_SDRAM
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_I2C
#define CONFIG_CMD_DATE
#define CONFIG_CMD_POST
#define CONFIG_CMD_MISC
#define CONFIG_CMD_LOADS
#define CONFIG_CMD_IMI
#define CONFIG_CMD_NFS
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_ENV

#define CFG_HUSH_PARSER
#define CONFIG_AUTO_COMPLETE
#define CFG_PROMPT_HUSH_PS2	    ">>"

#define CFG_MAX_NAND_DEVICE	0 /* Max number of NAND devices */
#define SECTORSIZE			     512

#define ADDR_COLUMN		1
#define ADDR_PAGE		2
#define ADDR_COLUMN_PAGE	3

#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM		0x20000000
#define PHYS_SDRAM_SIZE	0x2000000 /* 32 megs */

#define CFG_MEMTEST_START	0x21000000 /* PHYS_SDRAM */
/* CFG_MEMTEST_START + PHYS_SDRAM_SIZE - 262144 */
#define CFG_MEMTEST_END	0x00100000

#define CONFIG_DRIVER_ETHER
#define CONFIG_NET_RETRY_COUNT	20
#define CONFIG_AT91C_USE_RMII

#define PHYS_FLASH_1		0x10000000
#define PHYS_FLASH_SIZE	0x800000 /* 8 megs main flash */
#define CFG_FLASH_BASE		PHYS_FLASH_1
#define CFG_MAX_FLASH_BANKS	1
#define CFG_MAX_FLASH_SECT	256
#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ) /* Timeout for Flash Write */

#ifdef CFG_ENV_IS_IN_DATAFLASH
#define CFG_ENV_OFFSET		0x20000
#define CFG_ENV_ADDR		(CFG_DATAFLASH_LOGIC_ADDR_CS0 + CFG_ENV_OFFSET)
#define CFG_ENV_SIZE		0x2000
#else
#define CFG_ENV_IS_IN_FLASH
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + 0x00020000)
#define CFG_ENV_SIZE		2048
#endif

#ifdef CFG_ENV_IS_IN_EEPROM
#define CFG_ENV_OFFSET		1024
#define CFG_ENV_SIZE		1024
#endif

#define CFG_LOAD_ADDR		0x21000000 /* default load address */

/* use for protect flash sectors */
#define CFG_BOOT_SIZE		0x6000 /* 24 KBytes */
#define CFG_U_BOOT_BASE	(PHYS_FLASH_1 + 0x10000)
#define CFG_U_BOOT_SIZE	0x10000 /* 64 KBytes */

#define CFG_BAUDRATE_TABLE	{ 115200 , 19200, 38400, 57600, 9600 }

#define CFG_PROMPT		"U-Boot> " /* Monitor Command Prompt */
#define CFG_CBSIZE		512 /* Console I/O Buffer Size */
#define CFG_MAXARGS		16 /* max number of command args */
/* Print Buffer Size */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)

#define CFG_HZ 1000
#define CFG_HZ_CLOCK		AT91C_MASTER_CLOCK/2

#define CONFIG_STACKSIZE	(32*1024) /* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif

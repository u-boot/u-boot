/*
 * Ulf Samuelsson <ulf@atmel.com>
 * Rick Bronson <rick@efn.org>
 *
 * Configuration settings for the AT91RM9200EK board.
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
/*
 * from 18.432 MHz crystal
 * (18432000 / 4 * 39)
 */
#define AT91C_MAIN_CLOCK	179712000
/*
 * peripheral clock
 * (AT91C_MASTER_CLOCK / 3)
 */
#define AT91C_MASTER_CLOCK	59904000

#define AT91_SLOW_CLOCK		32768	/* slow clock */

#define CONFIG_ARM920T		1	/* This is an ARM920T Core	*/
#define CONFIG_AT91RM9200	1	/* It's an Atmel AT91RM9200 SoC	*/
#define CONFIG_AT91RM9200EK	1	/* on an AT91RM9200EK Board	*/
#undef  CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/
#define USE_920T_MMU		1

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

/*
 * LowLevel Init
 */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_USE_MAIN_OSCILLATOR		1
/* flash */
#define CONFIG_SYS_EBI_CFGR_VAL	0x00000000
#define CONFIG_SYS_SMC_CSR0_VAL	0x00003284 /* 16bit, 2 TDF, 4 WS */

/* clocks */
#define CONFIG_SYS_PLLAR_VAL	0x20263E04 /* 179.712000 MHz for PCK */
#define CONFIG_SYS_PLLBR_VAL	0x10483E0E /* 48.054857 MHz (divider by 2 for USB) */
/* PCK/3 = MCK Master Clock = 59.904000MHz from PLLA */
#define CONFIG_SYS_MCKR_VAL	0x00000202

/* sdram */
#define CONFIG_SYS_PIOC_ASR_VAL	0xFFFF0000 /* Configure PIOC as peripheral (D16/D31) */
#define CONFIG_SYS_PIOC_BSR_VAL	0x00000000
#define CONFIG_SYS_PIOC_PDR_VAL	0xFFFF0000
#define CONFIG_SYS_EBI_CSA_VAL	0x00000002 /* CS1=CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRC_CR_VAL	0x2188c155 /* set up the CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRAM	0x20000000 /* address of the CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRAM1	0x20000080 /* address of the CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRAM_VAL	0x00000000 /* value written to CONFIG_SYS_SDRAM */
#define CONFIG_SYS_SDRC_MR_VAL	0x00000002 /* Precharge All */
#define CONFIG_SYS_SDRC_MR_VAL1	0x00000004 /* refresh */
#define CONFIG_SYS_SDRC_MR_VAL2	0x00000003 /* Load Mode Register */
#define CONFIG_SYS_SDRC_MR_VAL3	0x00000000 /* Normal Mode */
#define CONFIG_SYS_SDRC_TR_VAL	0x000002E0 /* Write refresh rate */
#else
#define CONFIG_SKIP_RELOCATE_UBOOT
#endif	/* CONFIG_SKIP_LOWLEVEL_INIT */

/* hardcode so no __divsi3 : AT91C_MASTER_CLOCK / baudrate / 16 */
#define CONFIG_SYS_AT91C_BRGR_DIVISOR	33

/*
 * Memory Configuration
 */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			0x20000000
#define PHYS_SDRAM_SIZE			0x02000000	/* 32 megs */

#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END		\
		(CONFIG_SYS_MEMTEST_START + PHYS_SDRAM_SIZE - 262144)

/*
 * Hardware drivers
 */

/*
 * UART Configuration
 *
 * define one of these to choose the DBGU,
 * USART0 or USART1 as console
 */
#define CONFIG_AT91RM9200_USART
#define CONFIG_DBGU
#undef CONFIG_USART0
#undef CONFIG_USART1
/* don't include RTS/CTS flow control support	*/
#undef	CONFIG_HWFLOW
/* disable modem initialization stuff */
#undef	CONFIG_MODEM_SUPPORT

#define CONFIG_SYS_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }
#define CONFIG_BAUDRATE			115200

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>

#define CONFIG_CMD_DHCP
#define CONFIG_CMD_FAT
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING

#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_MISC
#undef CONFIG_CMD_LOADS

#include <asm/arch/AT91RM9200.h>	/* needed for port definitions */
/* Options for MMC/SD Card */
#define CONFIG_DOS_PARTITION	1
#undef CONFIG_MMC
#define CONFIG_SYS_MMC_BASE		0xFFFB4000
#define CONFIG_SYS_MMC_BLOCKSIZE	512

/*
 * Network Driver Setting
 */
#define CONFIG_DRIVER_ETHER
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_AT91C_USE_RMII

/*
 * AC Characteristics
 * DLYBS = tCSS = 250ns min and DLYBCT = tCSH = 250ns
 */
#define DATAFLASH_TCSS	(0xC << 16)
#define DATAFLASH_TCHS	(0x1 << 24)

#if defined(CONFIG_HAS_DATAFLASH)
#define CONFIG_SYS_SPI_WRITE_TOUT		(5 * CONFIG_SYS_HZ)
#define CONFIG_SYS_MAX_DATAFLASH_BANKS		2
#define CONFIG_SYS_MAX_DATAFLASH_PAGES		16384
/* Logical adress for CS0 */
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000
/* Logical adress for CS3 */
#define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS3	0xD0000000
#define	CONFIG_SYS_SUPPORT_BLOCK_ERASE		1
#define	CONFIG_SYS_DATAFLASH_MMC_PIO		AT91C_PIO_PB22
#endif

/*
 * NOR Flash
 */
#define CONFIG_SYS_FLASH_BASE			0x10000000
#define PHYS_FLASH_SIZE				0x800000	/* 8MB */
#define CONFIG_SYS_FLASH_CFI			1
#define CONFIG_FLASH_CFI_DRIVER			1
#define CONFIG_SYS_MAX_FLASH_BANKS		1
#define CONFIG_SYS_MAX_FLASH_SECT		256
#define CONFIG_SYS_FLASH_PROTECTION

/*
 * Environment Settings
 */
#ifdef CONFIG_ENV_IS_IN_DATAFLASH
/*
 * Datasflash Environment Settings
 */
#define CONFIG_ENV_OFFSET			0x4200
#define CONFIG_ENV_ADDR			\
		(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + CONFIG_ENV_OFFSET)
/* 8 * 1056 really , but start.s is not OK with this*/
#define CONFIG_ENV_SIZE			0x2000

#else
/*
 * NOR Flash Environment Settings
 */
#define CONFIG_ENV_IS_IN_FLASH		1

#ifdef CONFIG_SKIP_LOWLEVEL_INIT
/*
 * between boot.bin and u-boot.bin.gz
 */
#define CONFIG_ENV_ADDR			(CONFIG_SYS_FLASH_BASE + 0xe000)
#define CONFIG_ENV_SIZE			0x10000 /* sectors are 64K here */
#else
/*
 * after u-boot.bin
 */
#define CONFIG_ENV_ADDR			\
		(CONFIG_SYS_FLASH_BASE + CONFIG_SYS_MONITOR_LEN)
#define CONFIG_ENV_SIZE			0x10000 /* sectors are 64K here */
/* The following #defines are needed to get flash environment right */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_MONITOR_LEN		\
		(CONFIG_SYS_BOOT_SIZE + CONFIG_SYS_U_BOOT_SIZE)
#endif	/* CONFIG_SKIP_LOWLEVEL_INIT */

#endif	/* CONFIG_ENV_IS_IN_DATAFLASH */

/*
 * Boot option
 */
#define CONFIG_BOOTDELAY		3

#ifdef CONFIG_SKIP_LOWLEVEL_INIT
/* boot.bin, env, u-boot.bin.gz */
#define CONFIG_SYS_BOOT_SIZE		0x6000 /* 24 KBytes */
#define CONFIG_SYS_U_BOOT_BASE		(CONFIG_SYS_FLASH_BASE + 0x10000)
#define CONFIG_SYS_U_BOOT_SIZE		0x10000 /* 64 KBytes */
#else
/* u-boot.bin */
#define CONFIG_SYS_BOOT_SIZE		0x0 /* 0 KBytes */
#define CONFIG_SYS_U_BOOT_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_U_BOOT_SIZE		0x40000 /* 128 KBytes */
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */

#define CONFIG_SYS_LOAD_ADDR		0x21000000 /* default load address */
#define CONFIG_ENV_OVERWRITE	1

/*
 * USB Config
 */
#define CONFIG_CMD_USB
#define CONFIG_USB_OHCI_NEW	1
#define CONFIG_USB_KEYBOARD	1
#define CONFIG_USB_STORAGE	1
#define CONFIG_DOS_PARTITION	1

#undef CONFIG_SYS_USB_OHCI_BOARD_INIT
#define CONFIG_SYS_USB_OHCI_CPU_INIT		1
#define CONFIG_SYS_USB_OHCI_REGS_BASE		AT91_USB_HOST_BASE
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91rm9200"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	15

/*
 * I2C
 */
#define CONFIG_HARD_I2C

#ifdef CONFIG_HARD_I2C
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C_SPEED		0	/* not used */
#define CONFIG_SYS_I2C_SLAVE		0	/* not used */
#endif

/*
 * Shell Settings
 */
#define CONFIG_CMDLINE_EDITING		1
#define CONFIG_SYS_LONGHELP		1
#define CONFIG_AUTO_COMPLETE		1
#define CONFIG_SYS_HUSH_PARSER		1
#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		\
		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

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
	 * env_crc_valid == 0	=>	uninitialised
	 * env_crc_valid > 0	=>	environment crc in flash is valid
	 * env_crc_valid < 0	=>	environment crc in flash is invalid
	 */
	int env_crc_valid;
};
#endif

#define CONFIG_SYS_HZ 1000
/*
 * AT91C_TC0_CMR is implicitly set to
 * AT91C_TC_TIMER_DIV1_CLOCK
 */
#define CONFIG_SYS_HZ_CLOCK (AT91C_MASTER_CLOCK / 2)

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * CONFIG_ENV_SIZE + 128 * 1024 \
					     , 0x1000)
/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

#define CONFIG_STACKSIZE		(32 * 1024)	/* regular stack */
#define CONFIG_STACKSIZE_IRQ		(4 * 1024) /* Unsure if to big or to small*/
#define CONFIG_STACKSIZE_FIQ		(4 * 1024) /* Unsure if to big or to small*/
#endif

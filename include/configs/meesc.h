/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2009-2011
 * Daniel Gorsulowski <daniel.gorsulowski@esd.eu>
 * esd electronic system design gmbh <www.esd.eu>
 *
 * Configuation settings for the esd MEESC board.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * SoC must be defined first, before hardware.h is included.
 * In this case SoC is defined in boards.cfg.
 */
#include <asm/hardware.h>

/*
 * Warning: changing CONFIG_SYS_TEXT_BASE requires
 * adapting the initial boot program.
 * Since the linker has to swallow that define, we must use a pure
 * hex number here!
 */
#define CONFIG_SYS_TEXT_BASE		0x20002000

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768	/* 32.768 kHz crystal */
#define CONFIG_SYS_AT91_MAIN_CLOCK	16000000/* 16.0 MHz crystal */
#define CONFIG_SYS_HZ			1000	/* decrementer freq */

/* Misc CPU related */
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_ARCH_CPU_INIT
#define CONFIG_BOARD_EARLY_INIT_F		/* call board_early_init_f() */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SERIAL_TAG
#define CONFIG_REVISION_TAG
#define CONFIG_CMDLINE_TAG			/* enable passing of ATAGs */
#define CONFIG_MISC_INIT_R			/* Call misc_init_r */
#undef CONFIG_USE_IRQ				/* don't need IRQ/FIQ stuff */

#define CONFIG_DISPLAY_BOARDINFO		/* call checkboard() */
#define CONFIG_DISPLAY_CPUINFO			/* display cpu info and speed */
#define CONFIG_PREBOOT				/* enable preboot variable */

/*
 * Hardware drivers
 */

/* required until arch/arm/include/asm/arch-at91/at91sam9263.h is reworked */
#define ATMEL_PMC_UHP			AT91SAM926x_PMC_UHP

/* general purpose I/O */
#define CONFIG_AT91_GPIO

/* Console output */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define CONFIG_USART_ID			ATMEL_ID_SYS
#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200, 19200, 38400, 57600, 9600}

#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK

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
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_IMLS

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_NAND
#define CONFIG_CMD_USB

/* LED */
#define CONFIG_AT91_LED

/*
 * SDRAM: 1 bank, min 32, max 128 MB
 * Initialized before u-boot gets started.
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x20000000 /* ATMEL_BASE_CS1 */
#define CONFIG_SYS_SDRAM_SIZE		0x02000000

#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_SDRAM_BASE + 0x00100000)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x01E00000)
#define CONFIG_SYS_LOAD_ADDR		(CONFIG_SYS_SDRAM_BASE + 0x00100000)

/*
 * Initial stack pointer: 4k - GENERATED_GBL_DATA_SIZE in internal SRAM,
 * leaving the correct space for initial global data structure above
 * that address while providing maximum stack area below.
 */
#define CONFIG_SYS_INIT_SP_ADDR \
	(ATMEL_BASE_SRAM0 + 0x1000 - GENERATED_GBL_DATA_SIZE)

/* DataFlash */
#ifdef CONFIG_SYS_USE_DATAFLASH
# define CONFIG_ATMEL_DATAFLASH_SPI
# define CONFIG_HAS_DATAFLASH
# define CONFIG_SYS_SPI_WRITE_TOUT		(5 * CONFIG_SYS_HZ)
# define CONFIG_SYS_MAX_DATAFLASH_BANKS		1
# define CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0	0xC0000000	/* CS0 */
# define AT91_SPI_CLK				15000000
# define DATAFLASH_TCSS				(0x1a << 16)
# define DATAFLASH_TCHS				(0x1 << 24)
#endif

/* NOR flash is not populated, disable it */
#define CONFIG_SYS_NO_FLASH

/* NAND flash */
#ifdef CONFIG_CMD_NAND
# define CONFIG_NAND_ATMEL
# define CONFIG_SYS_MAX_NAND_DEVICE		1
# define CONFIG_SYS_NAND_BASE			0x40000000 /* ATMEL_BASE_CS3 */
# define CONFIG_SYS_NAND_DBW_8
# define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
# define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
# define CONFIG_SYS_NAND_ENABLE_PIN		AT91_PIO_PORTD, 15
# define CONFIG_SYS_NAND_READY_PIN		AT91_PIO_PORTA, 22
# define CONFIG_SYS_64BIT_VSPRINTF		/* needed for nand_util.c */
#endif

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_MULTI
#define CONFIG_FIT
#define CONFIG_NET_RETRY_COUNT			20
#undef CONFIG_RESET_PHY_R

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00a00000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"at91sam9263"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2

/* CAN */
#define CONFIG_AT91_CAN

/* hw-controller addresses */
#define CONFIG_ET1100_BASE		0x70000000

#ifdef CONFIG_SYS_USE_DATAFLASH

/* bootstrap + u-boot + env in dataflash on CS0 */
# define CONFIG_ENV_IS_IN_DATAFLASH
# define CONFIG_SYS_MONITOR_BASE	(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + \
					0x8400)
# define CONFIG_ENV_OFFSET		0x4200
# define CONFIG_ENV_ADDR		(CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0 + \
					CONFIG_ENV_OFFSET)
# define CONFIG_ENV_SIZE		0x4200

#elif CONFIG_SYS_USE_NANDFLASH

/* bootstrap + u-boot + env + linux in nandflash */
# define CONFIG_ENV_IS_IN_NAND		1
# define CONFIG_ENV_OFFSET		0xC0000
# define CONFIG_ENV_SIZE		0x20000

#endif

#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		512
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * CONFIG_ENV_SIZE + \
					128*1024, 0x1000)

#define CONFIG_STACKSIZE		(32 * 1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
# error CONFIG_USE_IRQ not supported
#endif

#endif
